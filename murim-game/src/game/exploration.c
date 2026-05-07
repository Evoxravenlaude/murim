/*
 * exploration.c — Climbing & Swimming Traversal (Zelda TotK style)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Hold movement toward walls to climb. Drains fatigue.
 * Swimming auto-activates in ZONE_UNDERWATER (speed penalty).
 */
#include "exploration.h"
#include "world.h"
#include "../ui/system_ui.h"
#include <math.h>
#include <stdio.h>

void exploration_init(Entity *player)
{
    player->environment.is_climbing    = false;
    player->environment.climb_stamina_drain = 8.0f;
    player->environment.swim_speed_mult     = 0.55f;
}

bool exploration_is_near_climbable(const Game *game, Vec2 pos)
{
    /* Check tiles directly adjacent in player facing direction */
    int offsets[4][2] = {{0,-TILE_SIZE},{0,TILE_SIZE},{-TILE_SIZE,0},{TILE_SIZE,0}};
    for (int i = 0; i < 4; i++) {
        float tx = pos.x + offsets[i][0];
        float ty = pos.y + offsets[i][1];
        TileType t = world_get_tile_at(&game->world, tx, ty);
        if (t == TILE_MOUNTAIN || t == TILE_WALL || t == TILE_TREE || t == TILE_STONE)
            return true;
    }
    return false;
}

void exploration_update(Game *game, float dt)
{
    Entity *player = &game->entities[game->player_id];
    bool moving = (player->vel.x != 0 || player->vel.y != 0);
    bool shift   = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    /* ── CLIMBING ─────────────────────────────────────── */
    bool near_wall = exploration_is_near_climbable(game, player->pos);
    bool want_climb = moving && near_wall && !shift; /* hold movement into wall, no dash */

    if (want_climb && player->stats.fatigue > 5.0f) {
        if (!player->environment.is_climbing) {
            player->environment.is_climbing = true;
            system_notify(game, NOTIFY_INFO, "[ Climbing ]", "Scaling the wall...");
        }
        /* Allow upward movement when climbing */
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
            player->vel.y = -(float)player->stats.speed * 0.6f;
        }
        /* Climbing drains fatigue */
        player->stats.fatigue -= player->environment.climb_stamina_drain * dt;
        if (player->stats.fatigue < 0) player->stats.fatigue = 0;
    } else {
        if (player->environment.is_climbing) {
            player->environment.is_climbing = false;
        }
    }

    /* Fatigue depleted while climbing → fall */
    if (player->environment.is_climbing && player->stats.fatigue <= 0) {
        player->environment.is_climbing = false;
        player->vel.y = 200.0f; /* fall */
        system_notify(game, NOTIFY_WARNING, "[ Climbing ]", "Exhausted! You fell.");
    }

    /* ── SWIMMING ─────────────────────────────────────── */
    if (player->environment.is_underwater) {
        /* Reduce speed while underwater */
        player->vel.x *= player->environment.swim_speed_mult;
        player->vel.y *= player->environment.swim_speed_mult;
    }
}

void exploration_draw_hud(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];

    /* Climbing indicator */
    if (player->environment.is_climbing) {
        float fatigue_ratio = player->stats.fatigue / 100.0f;
        int bx = SCREEN_WIDTH/2 - 60, by = SCREEN_HEIGHT - 130;
        DrawRectangle(bx, by, 120, 10, (Color){20,20,20,180});
        Color fc = fatigue_ratio > 0.3f ? (Color){100,220,100,255} : (Color){255,80,80,255};
        DrawRectangle(bx, by, (int)(120*fatigue_ratio), 10, fc);
        DrawRectangleLines(bx, by, 120, 10, (Color){140,200,140,200});
        DrawText("Stamina", bx-2, by-14, 10, (Color){180,220,180,200});
    }

    /* Swim indicator */
    if (player->environment.is_underwater) {
        const char *msg = "~ Swimming ~";
        int tw = MeasureText(msg, 12);
        float pulse = sinf((float)GetTime()*3.0f)*0.3f+0.7f;
        DrawText(msg, SCREEN_WIDTH/2-tw/2, SCREEN_HEIGHT/2+80, 12,
                 (Color){100,200,255,(unsigned char)(200*pulse)});
    }
}
