/*
 * events.c — Dynamic World Events (open world feel)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Random events: Beast Stampedes, Gate Breaks, Traveling Merchants,
 * Treasure Rain, and Sect Wars make the world feel alive.
 */
#include "events.h"
#include "npc.h"
#include "dungeon.h"
#include "../ui/system_ui.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include "world.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static float s_event_spawn_timer = 90.0f; /* First event after 90s */

void events_init(Game *game)
{
    memset(game->world_events, 0, sizeof(game->world_events));
    s_event_spawn_timer = 90.0f;
}

static int find_free_event(Game *game)
{
    for (int i = 0; i < MAX_WORLD_EVENTS; i++)
        if (!game->world_events[i].active) return i;
    return -1;
}

void events_spawn(Game *game, WorldEventType type, Vec2 pos)
{
    int idx = find_free_event(game);
    if (idx < 0) return;
    WorldEvent *ev = &game->world_events[idx];
    ev->active    = true;
    ev->type      = type;
    ev->pos       = pos;
    ev->completed = false;
    ev->timer     = 0;

    Entity *player = &game->entities[game->player_id];

    switch (type) {
    case EVENT_BEAST_STAMPEDE: {
        ev->duration    = 60.0f;
        ev->reward_exp  = 300;
        ev->reward_gold = 150;
        strncpy(ev->description, "Beast Stampede!", sizeof(ev->description)-1);
        /* Spawn 8-12 beasts charging in same direction */
        int count = 8 + rand() % 5;
        float charge_angle = ((float)(rand()%360))*DEG2RAD;
        for (int i = 0; i < count; i++) {
            float scatter = ((float)(rand()%100-50))*DEG2RAD*0.3f;
            float dist = 100.0f + (float)(rand()%200);
            Vec2 bp = { pos.x+cosf(charge_angle+scatter)*dist, pos.y+sinf(charge_angle+scatter)*dist };
            if (!world_pos_walkable(&game->world, bp.x, bp.y)) continue;
            int bid = npc_spawn(game, ENTITY_BEAST, bp, "Stampeding Beast");
            if (bid >= 0) {
                game->entities[bid].vel.x = cosf(charge_angle)*120.0f;
                game->entities[bid].vel.y = sinf(charge_angle)*120.0f;
                game->entities[bid].ai_state = AI_CHASE;
            }
        }
        system_notify(game, NOTIFY_WARNING, "[ WORLD EVENT ]", "Beast Stampede approaches!");
        break;
    }
    case EVENT_TRAVELING_MERCHANT: {
        ev->duration = 120.0f;
        ev->reward_exp = 0; ev->reward_gold = 0;
        strncpy(ev->description, "Traveling Merchant appeared!", sizeof(ev->description)-1);
        Vec2 mp = { player->pos.x + (float)(rand()%200-100), player->pos.y + (float)(rand()%200-100) };
        if (world_pos_walkable(&game->world, mp.x, mp.y))
            npc_spawn(game, ENTITY_NPC_MERCHANT, mp, "Traveling Merchant");
        system_notify(game, NOTIFY_INFO, "[ WORLD EVENT ]", "A merchant passes through!");
        break;
    }
    case EVENT_TREASURE_RAIN: {
        ev->duration = 10.0f;
        ev->reward_exp = 200; ev->reward_gold = 500;
        strncpy(ev->description, "Treasure Rain!", sizeof(ev->description)-1);
        /* Scatter loot drops */
        for (int i = 0; i < MAX_LOOT_DROPS && i < 15; i++) {
            LootDrop *ld = &game->loot_drops[i];
            if (ld->active) continue;
            float angle = ((float)(rand()%360))*DEG2RAD;
            float dist  = (float)(rand()%300);
            ld->active   = true;
            ld->pos.x    = player->pos.x + cosf(angle)*dist;
            ld->pos.y    = player->pos.y + sinf(angle)*dist;
            ld->item     = ITEM_SPIRIT_STONE;
            ld->rarity   = (ItemRarity)(rand()%3);
            ld->quantity = 1 + rand()%3;
            ld->timer    = 45.0f;
            ld->bob_timer = 0;
        }
        particle_burst(game, player->pos, (Color){255,215,0,255}, 50, 300.0f, 2.0f, 8.0f);
        system_notify(game, NOTIFY_SUCCESS, "[ WORLD EVENT ]", "Spirit stones rain from the heavens!");
        break;
    }
    case EVENT_SECT_WAR: {
        ev->duration = 90.0f;
        ev->reward_exp = 500; ev->reward_gold = 300;
        strncpy(ev->description, "Sect War erupts!", sizeof(ev->description)-1);
        /* Spawn two groups of hostile NPCs fighting each other */
        for (int i = 0; i < 6; i++) {
            float ang = ((float)(rand()%360))*DEG2RAD;
            Vec2 sp = { pos.x+cosf(ang)*150.0f, pos.y+sinf(ang)*150.0f };
            if (world_pos_walkable(&game->world, sp.x, sp.y))
                npc_spawn(game, ENTITY_NPC_HOSTILE, sp, i<3?"Heavenly Sect Disciple":"Demon Cult Disciple");
        }
        system_notify(game, NOTIFY_WARNING, "[ WORLD EVENT ]", "Sect War! Two factions clash nearby.");
        break;
    }
    case EVENT_HEAVENLY_TRIBULATION: {
        ev->duration = 30.0f;
        strncpy(ev->description, "Heavenly Tribulation!", sizeof(ev->description)-1);
        system_notify(game, NOTIFY_SYSTEM, "[ WORLD EVENT ]", "A tribulation lightning storm descends!");
        break;
    }
    default:
        ev->duration = 30.0f;
        strncpy(ev->description, "Unknown Event", sizeof(ev->description)-1);
        break;
    }
}

void events_update(Game *game, float dt)
{
    Entity *player = &game->entities[game->player_id];

    /* Random event spawn timer */
    s_event_spawn_timer -= dt;
    if (s_event_spawn_timer <= 0) {
        s_event_spawn_timer = 60.0f + (float)(rand()%120);
        float angle = ((float)(rand()%360))*DEG2RAD;
        float dist  = 300.0f + (float)(rand()%400);
        Vec2 pos = { player->pos.x+cosf(angle)*dist, player->pos.y+sinf(angle)*dist };

        /* Weighted random event */
        int roll = rand()%100;
        WorldEventType type;
        if      (roll < 30) type = EVENT_BEAST_STAMPEDE;
        else if (roll < 50) type = EVENT_TRAVELING_MERCHANT;
        else if (roll < 65) type = EVENT_TREASURE_RAIN;
        else if (roll < 80) type = EVENT_SECT_WAR;
        else                type = EVENT_HEAVENLY_TRIBULATION;

        events_spawn(game, type, pos);
    }

    /* Update active events */
    for (int i = 0; i < MAX_WORLD_EVENTS; i++) {
        WorldEvent *ev = &game->world_events[i];
        if (!ev->active) continue;
        ev->timer += dt;

        /* Heavenly Tribulation: periodic lightning */
        if (ev->type == EVENT_HEAVENLY_TRIBULATION) {
            static float trib_cd = 3.0f;
            trib_cd -= dt;
            if (trib_cd <= 0) {
                trib_cd = 1.5f + (float)(rand()%30)/10.0f;
                Vec2 strike = {
                    player->pos.x + (rand()%200-100),
                    player->pos.y + (rand()%200-100)
                };
                particle_burst(game, strike, (Color){255,255,200,255}, 25, 150.0f, 0.5f, 5.0f);
                camera_shake(game, 12.0f, 0.3f);
                float dx=strike.x-player->pos.x, dy=strike.y-player->pos.y;
                if (dx*dx+dy*dy < 50.0f*50.0f) player->stats.hp -= 30;
            }
        }

        /* Update loot drops: pickup + despawn */
        for (int j = 0; j < MAX_LOOT_DROPS; j++) {
            LootDrop *ld = &game->loot_drops[j];
            if (!ld->active) continue;
            ld->bob_timer += dt;
            ld->timer     -= dt;
            if (ld->timer <= 0) { ld->active = false; continue; }
            /* Pickup check */
            float dx = ld->pos.x-player->pos.x, dy = ld->pos.y-player->pos.y;
            if (dx*dx+dy*dy < 20.0f*20.0f) {
                player->gold += ld->quantity * 10;
                player->experience += 5;
                ld->active = false;
                system_notify(game, NOTIFY_LOOT, "[ Looted ]", "Picked up spirit stone!");
            }
        }

        if (ev->timer >= ev->duration) {
            ev->active    = false;
            ev->completed = true;
        }
    }
}

void events_draw(const Game *game)
{
    /* Draw active event indicators on screen edge */
    const Entity *player = &game->entities[game->player_id];
    int y = 140;
    for (int i = 0; i < MAX_WORLD_EVENTS; i++) {
        const WorldEvent *ev = &game->world_events[i];
        if (!ev->active) continue;
        float dx = ev->pos.x-player->pos.x, dy = ev->pos.y-player->pos.y;
        float dist = sqrtf(dx*dx+dy*dy);

        char buf[80];
        snprintf(buf, sizeof(buf), "⚡ %s (%.0fm)", ev->description, dist);
        float remaining = (ev->duration - ev->timer) / ev->duration;
        Color c = remaining > 0.5f ? (Color){255,200,50,220} : (Color){255,80,80,220};
        DrawText(buf, 12, y, 11, c);
        y += 16;

        /* Draw loot drops in world space */
        for (int j = 0; j < MAX_LOOT_DROPS; j++) {
            const LootDrop *ld = &game->loot_drops[j];
            if (!ld->active) continue;
            float bob = sinf(game->game_time*3.0f+j)*3.0f;
            DrawCircle((int)ld->pos.x, (int)(ld->pos.y+bob), 6, rarity_color(ld->rarity));
            DrawCircleLines((int)ld->pos.x, (int)(ld->pos.y+bob), 8, (Color){255,215,0,120});
        }
    }
}
