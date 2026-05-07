/*
 * fishing.c — Fishing Minigame (Stardew / Zelda style)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Press H near water to cast. Wait for bite. Press H again.
 * Hold H to reel (progress up, tension up). Release to rest.
 * Tension >= 100 = line breaks. Progress >= 100 = caught!
 */
#include "fishing.h"
#include "world.h"
#include "alchemy.h"
#include "../ui/system_ui.h"
#include "../engine/particles.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* ─── Fish table ──────────────────────────────────────── */
static const struct { const char *name; ItemRarity rarity; int value; } FISH_TABLE[] = {
    { "Common Carp",        RARITY_COMMON,    20  },
    { "Silver Trout",       RARITY_UNCOMMON,  60  },
    { "Golden Koi",         RARITY_RARE,      150 },
    { "Spirit Eel",         RARITY_EPIC,      400 },
    { "Dragon Turtle",      RARITY_LEGENDARY, 1000},
    { "Phoenix Salmon",     RARITY_MYTHIC,    3000},
};
#define NUM_FISH 6

static bool   s_active           = false;
static bool   s_fish_hooked      = false;
static float  s_wait_timer       = 0;
static float  s_tension          = 0;
static float  s_reel_progress    = 0;
static float  s_fish_pull_timer  = 0;
static float  s_fish_pull_rate   = 0;
static int    s_fish_type        = 0;
static float  s_bobber_bob       = 0;
static Vec2   s_bobber_pos       = {0,0};
static float  s_cast_time        = 0;

static bool is_near_water(const Game *game)
{
    const Entity *p = &game->entities[game->player_id];
    int offsets[8][2] = {{0,-TILE_SIZE},{0,TILE_SIZE},{-TILE_SIZE,0},{TILE_SIZE,0},
                         {-TILE_SIZE,-TILE_SIZE},{TILE_SIZE,-TILE_SIZE},{-TILE_SIZE,TILE_SIZE},{TILE_SIZE,TILE_SIZE}};
    for (int i=0;i<8;i++) {
        TileType t = world_get_tile_at(&game->world, p->pos.x+offsets[i][0], p->pos.y+offsets[i][1]);
        if (t == TILE_WATER) return true;
    }
    return false;
}

void fishing_init(Entity *player)
{
    player->fishing.active       = false;
    player->fishing.fish_hooked  = false;
    player->fishing.tension      = 0;
    player->fishing.reel_progress= 0;
    s_active = false;
}

bool fishing_start(Game *game)
{
    if (s_active) return false;
    if (!is_near_water(game)) {
        system_notify(game, NOTIFY_WARNING, "[ Fishing ]", "You must be near water!");
        return false;
    }
    Entity *player = &game->entities[game->player_id];
    s_active       = true;
    s_fish_hooked  = false;
    s_tension      = 0;
    s_reel_progress = 0;
    s_cast_time    = 0;
    /* Cast bobber toward facing direction */
    float angle = 0;
    switch (player->dir) {
    case DIR_UP:    angle = -1.5708f; break;
    case DIR_DOWN:  angle =  1.5708f; break;
    case DIR_LEFT:  angle =  3.1416f; break;
    default:        angle =  0;       break;
    }
    float dist = 60.0f + (float)(rand()%40);
    s_bobber_pos.x = player->pos.x + cosf(angle)*dist;
    s_bobber_pos.y = player->pos.y + sinf(angle)*dist;
    s_wait_timer   = 3.0f + (float)(rand()%80)/10.0f;
    s_fish_type    = rand() % NUM_FISH;
    s_fish_pull_rate = 15.0f + FISH_TABLE[s_fish_type].rarity * 5.0f;

    /* Give player starting fish spot resource */
    alchemy_add_resource(game, RESOURCE_FISH_SPOT, 0);

    system_notify(game, NOTIFY_INFO, "[ Fishing ]", "Line cast. Wait for a bite...");
    game->state = STATE_FISHING;
    return true;
}

void fishing_update(Game *game, float dt)
{
    if (!s_active) return;
    Entity *player = &game->entities[game->player_id];
    s_bobber_bob += dt * 2.5f;
    s_cast_time  += dt;

    if (!s_fish_hooked) {
        /* Waiting for bite */
        s_wait_timer -= dt;
        if (s_wait_timer <= 0) {
            s_fish_hooked = true;
            s_tension     = 0;
            s_reel_progress = 0;
            particle_burst(game, s_bobber_pos, (Color){100,200,255,200}, 8, 40.0f, 0.4f, 2.0f);
            system_notify(game, NOTIFY_INFO, "[ Fishing ]", "Fish on! Hold H to reel!");
        }
        /* Player can cancel */
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_H)) {
            fishing_cancel(game);
        }
        return;
    }

    /* Fish hooked — reel mechanic */
    bool reeling = IsKeyDown(KEY_H);

    /* Fish pulls tension up periodically */
    s_fish_pull_timer -= dt;
    if (s_fish_pull_timer <= 0) {
        float pull_strength = 8.0f + FISH_TABLE[s_fish_type].rarity * 4.0f;
        s_tension += pull_strength;
        s_fish_pull_timer = 0.8f + (float)(rand()%20)/20.0f;
    }

    if (reeling) {
        s_reel_progress += 18.0f * dt;
        s_tension       += 12.0f * dt; /* Reeling also increases tension */
    } else {
        /* Resting: tension drops */
        s_tension -= 20.0f * dt;
        if (s_tension < 0) s_tension = 0;
    }

    /* Clamp tension */
    if (s_tension > 100.0f) {
        /* Line break! */
        s_active = false; s_fish_hooked = false;
        system_notify(game, NOTIFY_WARNING, "[ Fishing ]", "The line snapped!");
        particle_burst(game, s_bobber_pos, (Color){255,200,100,200}, 12, 60.0f, 0.5f, 3.0f);
        game->state = STATE_PLAYING; return;
    }

    if (s_reel_progress >= 100.0f) {
        /* Caught! */
        s_active = false; s_fish_hooked = false;
        int val  = FISH_TABLE[s_fish_type].value;
        player->gold += val;
        player->experience += val / 2;
        char buf[80]; snprintf(buf, sizeof(buf), "Caught %s! +%d gold", FISH_TABLE[s_fish_type].name, val);
        system_notify(game, NOTIFY_SUCCESS, "[ Fish Caught! ]", buf);
        particle_burst(game, player->pos, rarity_color(FISH_TABLE[s_fish_type].rarity), 20, 100.0f, 0.8f, 4.0f);
        game->state = STATE_PLAYING;
    }
}

void fishing_cancel(Game *game)
{
    s_active = false; s_fish_hooked = false;
    game->state = STATE_PLAYING;
}

void fishing_draw(const Game *game)
{
    if (!s_active) return;
    (void)game;

    /* Draw bobber in world (called inside BeginMode2D) */
    float bob = sinf(s_bobber_bob) * (s_fish_hooked ? 6.0f : 3.0f);
    DrawCircle((int)s_bobber_pos.x, (int)(s_bobber_pos.y+bob), 5, (Color){220,80,80,230});
    DrawCircle((int)s_bobber_pos.x, (int)(s_bobber_pos.y+bob), 3, WHITE);

    if (!s_fish_hooked) {
        /* Waiting text */
        const char *msg = "...";
        DrawText(msg, (int)s_bobber_pos.x-8, (int)s_bobber_pos.y-20, 14, (Color){200,200,200,180});
        return;
    }

    /* ── Fishing HUD (screen space) ─────────────────── */
    int bx = SCREEN_WIDTH/2 - 100, by = SCREEN_HEIGHT - 180;

    /* Fish name */
    DrawText(FISH_TABLE[s_fish_type].name, bx, by-22, 14, rarity_color(FISH_TABLE[s_fish_type].rarity));

    /* Reel progress bar */
    DrawRectangle(bx, by, 200, 14, (Color){20,20,20,200});
    DrawRectangle(bx, by, (int)(200*s_reel_progress/100.0f), 14, (Color){80,200,80,255});
    DrawRectangleLines(bx, by, 200, 14, (Color){100,220,100,200});
    DrawText("Reel", bx+80, by+2, 10, WHITE);

    /* Tension bar */
    int ty = by + 20;
    float tension_ratio = s_tension / 100.0f;
    Color tc = tension_ratio > 0.7f ? (Color){255,50,50,255} : (Color){255,160,50,255};
    DrawRectangle(bx, ty, 200, 10, (Color){20,20,20,200});
    DrawRectangle(bx, ty, (int)(200*tension_ratio), 10, tc);
    DrawRectangleLines(bx, ty, 200, 10, tc);
    DrawText("Tension", bx+75, ty, 10, tc);

    /* Instructions */
    DrawText("[H] Hold = Reel  |  Release = Rest  |  [ESC] Cancel",
             bx-30, ty+18, 10, (Color){160,160,160,200});
}
