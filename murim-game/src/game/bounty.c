/*
 * bounty.c — Karma & Sect Bounty System (GTA5)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * 5-star bounty with escalating response:
 *   ★      Rogue disciples dispatched
 *   ★★     Sect warriors hunt you
 *   ★★★    Sect Elders track you across map
 *   ★★★★   Grandmasters mobilize
 *   ★★★★★  Heavenly Tribulation — lightning from the sky
 */
#include "bounty.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include "../ui/system_ui.h"
#include "npc.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void bounty_init(Entity *player)
{
    player->bounty.karma = 0;
    player->bounty.bounty_level = 0;
    player->bounty.bounty_timer = 0;
    player->bounty.hunter_spawn_timer = 0;
    player->bounty.hunters_dispatched = 0;
    player->bounty.heavens_angry = false;
    player->bounty.innocents_killed = 0;
    player->bounty.sects_robbed = 0;
}

void bounty_add_karma(Game *game, int amount)
{
    Entity *player = &game->entities[game->player_id];
    player->bounty.karma += amount;

    if (player->bounty.karma < -1000) player->bounty.karma = -1000;
    if (player->bounty.karma > 1000) player->bounty.karma = 1000;

    /* Calculate bounty level from negative karma */
    int old_level = player->bounty.bounty_level;
    if (player->bounty.karma <= -800) player->bounty.bounty_level = 5;
    else if (player->bounty.karma <= -600) player->bounty.bounty_level = 4;
    else if (player->bounty.karma <= -400) player->bounty.bounty_level = 3;
    else if (player->bounty.karma <= -200) player->bounty.bounty_level = 2;
    else if (player->bounty.karma <= -50) player->bounty.bounty_level = 1;
    else player->bounty.bounty_level = 0;

    if (player->bounty.bounty_level > old_level) {
        player->bounty.bounty_timer = 120.0f; /* 2 min decay timer */
        char buf[64];
        snprintf(buf, sizeof(buf), "Bounty increased to %d stars!", player->bounty.bounty_level);
        system_notify(game, NOTIFY_WARNING, "[ WANTED ]", buf);
    }

    if (player->bounty.bounty_level >= 5 && !player->bounty.heavens_angry) {
        player->bounty.heavens_angry = true;
        system_notify(game, NOTIFY_SYSTEM, "[ HEAVENLY WRATH ]",
                      "The Heavens have deemed you unworthy!");
    }
}

void bounty_on_kill(Game *game, const Entity *killed)
{
    Entity *player = &game->entities[game->player_id];

    switch (killed->type) {
    case ENTITY_NPC_FRIENDLY:
    case ENTITY_VILLAGER:
        bounty_add_karma(game, -100);
        player->bounty.innocents_killed++;
        system_notify(game, NOTIFY_WARNING, "[ Sin ]", "You killed an innocent!");
        break;
    case ENTITY_NPC_MERCHANT:
        bounty_add_karma(game, -150);
        player->bounty.sects_robbed++;
        break;
    case ENTITY_NPC_ELDER:
        bounty_add_karma(game, -300);
        system_notify(game, NOTIFY_WARNING, "[ GRAVE SIN ]", "You slew an Elder!");
        break;
    case ENTITY_NPC_HOSTILE:
        bounty_add_karma(game, +20); /* Good karma for killing villains */
        break;
    case ENTITY_BEAST:
        bounty_add_karma(game, +5);
        break;
    case ENTITY_BOSS:
        bounty_add_karma(game, +50);
        break;
    default:
        break;
    }
}

static void spawn_bounty_hunters(Game *game)
{
    Entity *player = &game->entities[game->player_id];
    int level = player->bounty.bounty_level;
    int count = level; /* 1 hunter per star */

    for (int i = 0; i < count && i < 5; i++) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        float dist = 200.0f + (float)(rand() % 100);
        Vec2 pos = {
            player->pos.x + cosf(angle) * dist,
            player->pos.y + sinf(angle) * dist
        };

        const char *name;
        CultivationRealm realm;
        if (level >= 4) {
            name = "Grandmaster Hunter";
            realm = REALM_NASCENT_SOUL;
        } else if (level >= 3) {
            name = "Sect Elder Hunter";
            realm = REALM_CORE_FORMATION;
        } else if (level >= 2) {
            name = "Sect Warrior";
            realm = REALM_FOUNDATION;
        } else {
            name = "Bounty Disciple";
            realm = REALM_QI_GATHERING;
        }

        int id = npc_spawn(game, ENTITY_NPC_HOSTILE, pos, name);
        if (id >= 0) {
            Entity *hunter = &game->entities[id];
            hunter->cultivation.realm = realm;
            hunter->stats.attack *= (1 + level);
            hunter->stats.max_hp *= (1 + level);
            hunter->stats.hp = hunter->stats.max_hp;
            hunter->detection_range = 200.0f;
            hunter->ai_state = AI_CHASE;
            hunter->color = (Color){ 180, 30, 30, 255 };
            player->bounty.hunters_dispatched++;
        }
    }
}

void bounty_update(Game *game, float dt)
{
    Entity *player = &game->entities[game->player_id];
    if (player->bounty.bounty_level <= 0) return;

    /* Bounty decay over time */
    player->bounty.bounty_timer -= dt;
    if (player->bounty.bounty_timer <= 0) {
        bounty_add_karma(game, +30); /* Slowly forgiven */
        player->bounty.bounty_timer = 30.0f;
    }

    /* Spawn bounty hunters periodically */
    player->bounty.hunter_spawn_timer -= dt;
    if (player->bounty.hunter_spawn_timer <= 0) {
        spawn_bounty_hunters(game);
        /* Faster spawns at higher bounty */
        player->bounty.hunter_spawn_timer = 30.0f / player->bounty.bounty_level;
        system_notify(game, NOTIFY_WARNING, "[ Bounty Hunters ]",
                      "Hunters are closing in on your position!");
    }

    /* Heavenly Tribulation at max bounty */
    if (player->bounty.heavens_angry) {
        static float trib_timer = 0;
        trib_timer -= dt;
        if (trib_timer <= 0) {
            /* Lightning strike near player */
            Vec2 strike = {
                player->pos.x + (rand() % 100 - 50),
                player->pos.y + (rand() % 100 - 50)
            };
            particle_burst(game, strike, (Color){255,255,200,255}, 30, 200.0f, 0.5f, 6.0f);
            camera_shake(game, 20.0f, 0.5f);

            /* Damage if close */
            float dx = strike.x - player->pos.x;
            float dy = strike.y - player->pos.y;
            if (dx*dx + dy*dy < 40.0f*40.0f) {
                player->stats.hp -= 50;
            }
            trib_timer = 3.0f + (float)(rand() % 30) / 10.0f;
        }
    }
}

void bounty_draw_stars(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    int level = player->bounty.bounty_level;
    if (level <= 0) return;

    int sx = SCREEN_WIDTH / 2 - (level * 14);
    int sy = 8;

    for (int i = 0; i < BOUNTY_MAX_LEVEL; i++) {
        Color c;
        if (i < level) {
            float pulse = sinf(game->game_time * 4.0f + i) * 0.3f + 0.7f;
            if (level >= 5) {
                c = (Color){ 255, 50, 50, (unsigned char)(255 * pulse) };
            } else {
                c = (Color){ 255, 200, 50, (unsigned char)(255 * pulse) };
            }
        } else {
            c = (Color){ 60, 60, 60, 100 };
        }
        DrawText("★", sx + i * 28, sy, 22, c);
    }

    /* Karma number */
    char buf[32];
    snprintf(buf, sizeof(buf), "Karma: %d", player->bounty.karma);
    Color kc = player->bounty.karma >= 0 ?
        (Color){100,200,100,200} : (Color){255,100,100,200};
    DrawText(buf, sx - 10, sy + 24, 10, kc);
}
