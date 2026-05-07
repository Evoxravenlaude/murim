/*
 * environment.c — Temperature, Altitude, Oxygen, Hazards
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Zone detection based on tile type and world position.
 * Subnautica: underwater oxygen, swim speed
 * Neverness to Everest: altitude sickness, cold/heat damage
 */
#include "environment.h"
#include "../engine/particles.h"
#include "../ui/system_ui.h"
#include "world.h"
#include <math.h>
#include <stdio.h>

void environment_init(Entity *player)
{
    player->environment.current_zone = ZONE_NORMAL;
    player->environment.temperature = 20.0f;
    player->environment.altitude = 0;
    player->environment.oxygen = 100.0f;
    player->environment.oxygen_max = 100.0f;
    player->environment.oxygen_drain = 5.0f;
    player->environment.is_underwater = false;
    player->environment.is_climbing = false;
    player->environment.climb_stamina_drain = 3.0f;
    player->environment.swim_speed_mult = 0.6f;
    player->environment.heat_damage_timer = 0;
    player->environment.cold_slow_factor = 1.0f;
}

EnvironmentZone environment_get_zone(const Game *game, Vec2 pos)
{
    TileType tile = world_get_tile_at(&game->world, pos.x, pos.y);

    switch (tile) {
    case TILE_WATER:
        return ZONE_UNDERWATER;
    case TILE_SAND:
    case TILE_LAVA:
        return ZONE_HOT;
    case TILE_ICE:
        return ZONE_COLD;
    case TILE_MOUNTAIN:
    case TILE_STONE:
        return ZONE_HIGH_ALTITUDE;
    default:
        break;
    }

    /* Check elevation-based zone from world position */
    float nx = pos.x / (WORLD_TILES_X * TILE_SIZE);
    float ny = pos.y / (WORLD_TILES_Y * TILE_SIZE);
    float dx = nx - 0.5f;
    float dy = ny - 0.5f;
    float edge_dist = sqrtf(dx*dx + dy*dy);

    /* Edges of map are colder (mountain border) */
    if (edge_dist > 0.4f) return ZONE_COLD;

    return ZONE_NORMAL;
}

void environment_update(Game *game, float dt)
{
    Entity *player = &game->entities[game->player_id];
    EnvironmentZone zone = environment_get_zone(game, player->pos);
    player->environment.current_zone = zone;

    /* Temperature based on zone + time of day */
    float base_temp = 20.0f;
    float day_mod = (game->world.day_time > 10.0f && game->world.day_time < 16.0f) ? 5.0f : -5.0f;

    switch (zone) {
    case ZONE_HOT:
        base_temp = 45.0f + day_mod;
        break;
    case ZONE_COLD:
    case ZONE_HIGH_ALTITUDE:
        base_temp = -10.0f + day_mod;
        break;
    case ZONE_UNDERWATER:
        base_temp = 12.0f;
        break;
    case ZONE_TOXIC:
        base_temp = 30.0f;
        break;
    default:
        base_temp = 20.0f + day_mod;
        break;
    }

    /* Weather affects temperature */
    if (game->world.weather.current == WEATHER_RAIN) base_temp -= 5.0f;
    if (game->world.weather.current == WEATHER_SNOW) base_temp -= 15.0f;
    if (game->world.weather.current == WEATHER_STORM) base_temp -= 8.0f;

    /* Smoothly transition temperature */
    player->environment.temperature += (base_temp - player->environment.temperature) * dt * 0.5f;

    /* === HEAT DAMAGE === */
    if (player->environment.temperature > 40.0f) {
        player->environment.heat_damage_timer -= dt;
        if (player->environment.heat_damage_timer <= 0) {
            int dmg = (int)((player->environment.temperature - 40.0f) * 0.5f);
            if (dmg < 1) dmg = 1;
            player->stats.hp -= dmg;
            player->stats.thirst -= 2.0f;
            player->environment.heat_damage_timer = 2.0f;
        }
        player->environment.cold_slow_factor = 1.0f;
    }
    /* === COLD SLOWDOWN === */
    else if (player->environment.temperature < 0.0f) {
        float cold_severity = fabsf(player->environment.temperature) / 30.0f;
        if (cold_severity > 1.0f) cold_severity = 1.0f;
        player->environment.cold_slow_factor = 1.0f - cold_severity * 0.5f;

        /* Extreme cold damages */
        if (player->environment.temperature < -20.0f) {
            player->environment.heat_damage_timer -= dt;
            if (player->environment.heat_damage_timer <= 0) {
                player->stats.hp -= 3;
                player->environment.heat_damage_timer = 3.0f;
            }
        }
    } else {
        player->environment.cold_slow_factor = 1.0f;
    }

    /* === UNDERWATER OXYGEN === */
    if (zone == ZONE_UNDERWATER) {
        player->environment.is_underwater = true;
        player->environment.oxygen -= player->environment.oxygen_drain * dt;
        if (player->environment.oxygen <= 0) {
            player->environment.oxygen = 0;
            /* Drowning damage */
            player->stats.hp -= (int)(15.0f * dt);
        }
    } else {
        player->environment.is_underwater = false;
        /* Recover oxygen on land */
        if (player->environment.oxygen < player->environment.oxygen_max) {
            player->environment.oxygen += 20.0f * dt;
            if (player->environment.oxygen > player->environment.oxygen_max)
                player->environment.oxygen = player->environment.oxygen_max;
        }
    }

    /* === HIGH ALTITUDE === */
    if (zone == ZONE_HIGH_ALTITUDE) {
        /* Stamina drains faster, fatigue drops */
        player->stats.fatigue -= dt * 2.0f;
        if (player->stats.fatigue < 0) player->stats.fatigue = 0;
    }
}

void environment_draw_overlay(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    EnvironmentZone zone = player->environment.current_zone;

    switch (zone) {
    case ZONE_UNDERWATER: {
        /* Blue-green underwater tint */
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      (Color){ 20, 60, 100, 80 });
        /* Caustic light patterns */
        float t = game->game_time;
        for (int i = 0; i < 8; i++) {
            float x = (sinf(t * 0.5f + i * 1.2f) + 1.0f) * SCREEN_WIDTH * 0.5f;
            float y = (cosf(t * 0.7f + i * 0.8f) + 1.0f) * SCREEN_HEIGHT * 0.5f;
            DrawCircle((int)x, (int)y, 40 + sinf(t + i) * 15,
                       (Color){ 100, 200, 255, 15 });
        }
        break;
    }
    case ZONE_HOT: {
        /* Heat shimmer / orange tint */
        float shimmer = sinf(game->game_time * 3.0f) * 0.2f + 0.3f;
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      (Color){ 255, 120, 30, (unsigned char)(40 * shimmer) });
        break;
    }
    case ZONE_COLD:
    case ZONE_HIGH_ALTITUDE: {
        /* Frost vignette */
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      (Color){ 180, 220, 255, 30 });
        /* Corner frost */
        DrawRectangle(0, 0, 60, 60, (Color){ 200, 230, 255, 50 });
        DrawRectangle(SCREEN_WIDTH - 60, 0, 60, 60, (Color){ 200, 230, 255, 50 });
        DrawRectangle(0, SCREEN_HEIGHT - 60, 60, 60, (Color){ 200, 230, 255, 50 });
        DrawRectangle(SCREEN_WIDTH - 60, SCREEN_HEIGHT - 60, 60, 60, (Color){ 200, 230, 255, 50 });
        break;
    }
    case ZONE_TOXIC: {
        float pulse = sinf(game->game_time * 2.0f) * 0.3f + 0.5f;
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      (Color){ 80, 180, 30, (unsigned char)(30 * pulse) });
        break;
    }
    default:
        break;
    }
}

void environment_draw_hud(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];

    /* Temperature indicator — top center */
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f°C", player->environment.temperature);
    Color tc;
    if (player->environment.temperature > 35.0f)
        tc = (Color){ 255, 100, 50, 255 };
    else if (player->environment.temperature < 0.0f)
        tc = (Color){ 100, 180, 255, 255 };
    else
        tc = (Color){ 200, 200, 200, 200 };
    DrawText(buf, SCREEN_WIDTH / 2 + 60, 12, 12, tc);

    /* Zone name */
    if (player->environment.current_zone != ZONE_NORMAL) {
        const char *zn = zone_name(player->environment.current_zone);
        DrawText(zn, SCREEN_WIDTH / 2 + 60, 26, 10, tc);
    }

    /* Oxygen bar when underwater */
    if (player->environment.is_underwater) {
        float ratio = player->environment.oxygen / player->environment.oxygen_max;
        int bw = 120, bh = 10;
        int bx = SCREEN_WIDTH / 2 - bw / 2;
        int by = SCREEN_HEIGHT - 100;
        DrawRectangle(bx, by, bw, bh, (Color){ 10, 10, 10, 180 });
        Color oc = ratio > 0.3f ? (Color){ 50, 180, 255, 255 } : (Color){ 255, 50, 50, 255 };
        DrawRectangle(bx, by, (int)(bw * ratio), bh, oc);
        DrawRectangleLines(bx, by, bw, bh, (Color){ 80, 160, 255, 200 });
        DrawText("O₂", bx - 20, by - 1, 12, (Color){ 100, 200, 255, 255 });
    }
}
