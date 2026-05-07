/*
 * camera.c — Smooth-follow camera with screen shake
 */
#include "camera.h"
#include <math.h>
#include <stdlib.h>

void camera_init(Camera2D *camera, Vec2 target)
{
    camera->target = (Vector2){ target.x, target.y };
    camera->offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera->rotation = 0.0f;
    camera->zoom = 2.0f;
}

void camera_update(Camera2D *camera, Vec2 target, float dt)
{
    /* Smooth lerp follow */
    float smoothing = 5.0f * dt;
    if (smoothing > 1.0f) smoothing = 1.0f;

    camera->target.x += (target.x - camera->target.x) * smoothing;
    camera->target.y += (target.y - camera->target.y) * smoothing;

    /* Clamp to world bounds */
    float half_view_x = (SCREEN_WIDTH / 2.0f) / camera->zoom;
    float half_view_y = (SCREEN_HEIGHT / 2.0f) / camera->zoom;

    float max_x = WORLD_TILES_X * TILE_SIZE - half_view_x;
    float max_y = WORLD_TILES_Y * TILE_SIZE - half_view_y;

    if (camera->target.x < half_view_x) camera->target.x = half_view_x;
    if (camera->target.y < half_view_y) camera->target.y = half_view_y;
    if (camera->target.x > max_x) camera->target.x = max_x;
    if (camera->target.y > max_y) camera->target.y = max_y;
}

void camera_shake(Game *game, float intensity, float duration)
{
    game->screen_shake = intensity;
    game->screen_shake_timer = duration;
}

void camera_update_shake(Game *game, float dt)
{
    if (game->screen_shake_timer > 0) {
        game->screen_shake_timer -= dt;
        float intensity = game->screen_shake * (game->screen_shake_timer / 0.3f);
        game->camera.target.x += ((float)(rand() % 100) / 100.0f - 0.5f) * intensity * 2.0f;
        game->camera.target.y += ((float)(rand() % 100) / 100.0f - 0.5f) * intensity * 2.0f;
    }
}
