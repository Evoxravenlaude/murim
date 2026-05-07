/*
 * mounts.c — Beast Mounting & Mounted Combat
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Press M near a deployed tamed beast to mount it.
 * Mounted speed is boosted. Attacks still work while mounted.
 * Mount stamina drains at full gallop; dismounts when empty.
 */
#include "mounts.h"
#include "../ui/system_ui.h"
#include "../engine/particles.h"
#include <math.h>
#include <stdio.h>

void mounts_init(Entity *player)
{
    player->mount.is_mounted       = false;
    player->mount.mount_beast_idx  = -1;
    player->mount.mount_speed_mult = 2.2f;
    player->mount.mount_stamina    = 100.0f;
    player->mount.mount_max_stamina = 100.0f;
    player->mount.can_mount_combat = true;
}

bool mounts_try_mount(Game *game)
{
    Entity *player = &game->entities[game->player_id];
    if (player->mount.is_mounted) {
        mounts_dismount(game);
        return false;
    }

    /* Find nearest deployed tamed beast */
    for (int i = 0; i < MAX_TAMED_BEASTS; i++) {
        TamedBeast *tb = &game->tamed_beasts[i];
        if (!tb->active || !tb->is_deployed || tb->entity_id < 0) continue;
        Entity *be = &game->entities[tb->entity_id];
        if (!be->active) continue;
        float dx = be->pos.x-player->pos.x, dy = be->pos.y-player->pos.y;
        if (dx*dx+dy*dy > 50.0f*50.0f) continue;

        /* Mount! */
        player->mount.is_mounted       = true;
        player->mount.mount_beast_idx  = i;
        player->mount.mount_stamina    = player->mount.mount_max_stamina;
        system_notify(game, NOTIFY_SUCCESS, "[ Mounted ]", tb->name);
        particle_burst(game, player->pos, (Color){200,180,100,200}, 10, 80.0f, 0.4f, 3.0f);
        return true;
    }
    system_notify(game, NOTIFY_WARNING, "[ Mount ]", "No deployed beast nearby.");
    return false;
}

void mounts_dismount(Game *game)
{
    Entity *player = &game->entities[game->player_id];
    if (!player->mount.is_mounted) return;
    player->mount.is_mounted      = false;
    player->mount.mount_beast_idx = -1;
    system_notify(game, NOTIFY_INFO, "[ Dismounted ]", "");
}

void mounts_update(Game *game, float dt)
{
    Entity *player = &game->entities[game->player_id];
    if (!player->mount.is_mounted) return;

    int bidx = player->mount.mount_beast_idx;
    if (bidx < 0 || bidx >= MAX_TAMED_BEASTS) { mounts_dismount(game); return; }
    TamedBeast *tb = &game->tamed_beasts[bidx];
    if (!tb->active || !tb->is_deployed || tb->entity_id < 0) { mounts_dismount(game); return; }

    Entity *be = &game->entities[tb->entity_id];
    if (!be->active) { mounts_dismount(game); return; }

    /* Sync mount entity position to player */
    be->pos = player->pos;
    be->vel = player->vel;

    /* Boost player speed */
    player->vel.x *= player->mount.mount_speed_mult;
    player->vel.y *= player->mount.mount_speed_mult;

    /* Stamina drain while moving fast */
    float speed_sq = player->vel.x*player->vel.x + player->vel.y*player->vel.y;
    if (speed_sq > 100.0f) {
        player->mount.mount_stamina -= 8.0f * dt;
        if (player->mount.mount_stamina < 0) {
            player->mount.mount_stamina = 0;
            mounts_dismount(game);
            system_notify(game, NOTIFY_WARNING, "[ Mount Exhausted ]", "Your beast needs rest!");
        }
    } else {
        /* Recover stamina when not sprinting */
        player->mount.mount_stamina += 12.0f * dt;
        if (player->mount.mount_stamina > player->mount.mount_max_stamina)
            player->mount.mount_stamina = player->mount.mount_max_stamina;
    }
}

void mounts_draw_hud(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    if (!player->mount.is_mounted) return;

    int bx = SCREEN_WIDTH/2-80, by = SCREEN_HEIGHT-80;
    float ratio = player->mount.mount_stamina / player->mount.mount_max_stamina;
    DrawRectangle(bx-2, by-2, 164, 14, (Color){10,10,10,180});
    Color sc = ratio > 0.4f ? (Color){200,160,60,255} : (Color){255,80,80,255};
    DrawRectangle(bx, by, (int)(160*ratio), 10, sc);
    DrawRectangleLines(bx, by, 160, 10, (Color){200,160,60,200});
    DrawText("Mount", bx-2, by-14, 10, (Color){200,180,100,200});

    int bidx = player->mount.mount_beast_idx;
    if (bidx>=0&&bidx<MAX_TAMED_BEASTS&&game->tamed_beasts[bidx].active)
        DrawText(game->tamed_beasts[bidx].name, bx+170, by-2, 12, (Color){220,200,140,220});
}
