/*
 * renderer.c — Rendering implementation
 * Heavenly Demon: Murim Chronicles
 *
 * Handles all visual rendering: tiles, entities, particles, lighting, effects
 */
#include "renderer.h"
#include <math.h>
#include <stdio.h>

/* ─── Tile Colors ─────────────────────────────────────── */
static const Color TILE_COLORS[] = {
    [TILE_GRASS]       = { 76, 153, 76, 255 },     /* green grass */
    [TILE_GRASS_DARK]  = { 56, 128, 56, 255 },     /* darker grass */
    [TILE_DIRT]        = { 139, 110, 72, 255 },     /* brown dirt */
    [TILE_STONE]       = { 128, 128, 128, 255 },    /* gray stone */
    [TILE_WATER]       = { 64, 120, 200, 255 },     /* blue water */
    [TILE_SAND]        = { 210, 190, 140, 255 },    /* sandy yellow */
    [TILE_TREE]        = { 34, 100, 34, 255 },      /* dark green tree */
    [TILE_MOUNTAIN]    = { 100, 90, 80, 255 },      /* dark brown mountain */
    [TILE_WALL]        = { 80, 70, 60, 255 },       /* dark wall */
    [TILE_FLOOR_WOOD]  = { 160, 120, 70, 255 },     /* wooden floor */
    [TILE_FLOOR_STONE] = { 150, 145, 140, 255 },    /* stone floor */
    [TILE_PATH]        = { 170, 150, 110, 255 },    /* dirt path */
    [TILE_BRIDGE]      = { 140, 100, 50, 255 },     /* wooden bridge */
    [TILE_FLOWER_RED]  = { 76, 153, 76, 255 },      /* grass (flower drawn on top) */
    [TILE_FLOWER_BLUE] = { 76, 153, 76, 255 },      /* grass (flower drawn on top) */
    [TILE_BAMBOO]      = { 56, 128, 56, 255 },      /* dark grass (bamboo on top) */
    [TILE_SECT_GATE]   = { 140, 50, 50, 255 },      /* red sect gate */
    [TILE_SECT_FLOOR]  = { 180, 160, 120, 255 },    /* polished floor */
};

/* ─── Realm Colors (for auras) ────────────────────────── */
static const Color REALM_COLORS[] = {
    [REALM_MORTAL]           = { 200, 200, 200, 80 },
    [REALM_QI_GATHERING]     = { 120, 200, 255, 100 },
    [REALM_FOUNDATION]       = { 100, 255, 150, 100 },
    [REALM_CORE_FORMATION]   = { 255, 215, 0, 120 },
    [REALM_NASCENT_SOUL]     = { 200, 100, 255, 120 },
    [REALM_SPIRIT_SEVERING]  = { 255, 80, 80, 140 },
    [REALM_TRANSCENDENCE]    = { 255, 255, 255, 160 },
};

void renderer_init(void)
{
    /* Future: load sprite sheets, shader resources */
}

void renderer_cleanup(void)
{
    /* Future: unload textures */
}

Color tile_get_color(TileType type)
{
    if (type >= 0 && type < TILE_COUNT) {
        return TILE_COLORS[type];
    }
    return MAGENTA; /* error color */
}

/* Draw tile decorations (trees, flowers, bamboo, etc.) */
static void draw_tile_decoration(TileType type, int px, int py, float time)
{
    switch (type) {
    case TILE_TREE: {
        /* Draw trunk */
        DrawRectangle(px + 12, py + 16, 8, 16, (Color){100, 70, 40, 255});
        /* Draw canopy with slight sway */
        float sway = sinf(time * 1.5f + px * 0.1f) * 2.0f;
        DrawCircle(px + 16 + (int)sway, py + 12, 12, (Color){34, 120, 34, 255});
        DrawCircle(px + 12 + (int)sway, py + 8, 8, (Color){45, 140, 45, 255});
        DrawCircle(px + 20 + (int)sway, py + 10, 9, (Color){38, 130, 38, 255});
        break;
    }
    case TILE_FLOWER_RED: {
        float bob = sinf(time * 2.0f + px * 0.3f) * 1.5f;
        DrawCircle(px + 16, py + 16 + (int)bob, 3, RED);
        DrawCircle(px + 14, py + 14 + (int)bob, 2, (Color){255, 100, 100, 255});
        DrawRectangle(px + 15, py + 18 + (int)bob, 2, 6, (Color){34, 100, 34, 255});
        break;
    }
    case TILE_FLOWER_BLUE: {
        float bob = sinf(time * 2.0f + py * 0.3f) * 1.5f;
        DrawCircle(px + 16, py + 16 + (int)bob, 3, (Color){80, 130, 255, 255});
        DrawCircle(px + 18, py + 14 + (int)bob, 2, (Color){120, 160, 255, 255});
        DrawRectangle(px + 15, py + 18 + (int)bob, 2, 6, (Color){34, 100, 34, 255});
        break;
    }
    case TILE_BAMBOO: {
        /* Tall bamboo stalks */
        float sway = sinf(time * 1.2f + px * 0.2f) * 1.5f;
        for (int i = 0; i < 3; i++) {
            int bx = px + 6 + i * 10;
            DrawRectangle(bx + (int)(sway * 0.5f), py, 3, 32, (Color){80, 160, 50, 255});
            /* Bamboo nodes */
            DrawRectangle(bx - 1 + (int)(sway * 0.3f), py + 8, 5, 2, (Color){60, 140, 40, 255});
            DrawRectangle(bx - 1 + (int)(sway * 0.7f), py + 20, 5, 2, (Color){60, 140, 40, 255});
            /* Leaves */
            DrawTriangle(
                (Vector2){bx + (int)sway, py - 2},
                (Vector2){bx - 5 + (int)sway, py + 6},
                (Vector2){bx + 5 + (int)sway, py + 6},
                (Color){60, 180, 40, 200}
            );
        }
        break;
    }
    case TILE_MOUNTAIN: {
        /* Rugged mountain peak */
        DrawTriangle(
            (Vector2){px + 16, py + 2},
            (Vector2){px, py + 32},
            (Vector2){px + 32, py + 32},
            (Color){110, 100, 90, 255}
        );
        /* Snow cap */
        DrawTriangle(
            (Vector2){px + 16, py + 2},
            (Vector2){px + 10, py + 12},
            (Vector2){px + 22, py + 12},
            (Color){230, 230, 240, 255}
        );
        break;
    }
    case TILE_SECT_GATE: {
        /* Draw gate pillars */
        DrawRectangle(px + 2, py + 4, 6, 28, (Color){160, 60, 60, 255});
        DrawRectangle(px + 24, py + 4, 6, 28, (Color){160, 60, 60, 255});
        /* Top bar */
        DrawRectangle(px, py + 2, 32, 6, (Color){180, 70, 40, 255});
        /* Chinese-style roof curve */
        DrawRectangle(px - 2, py, 36, 3, (Color){120, 40, 30, 255});
        /* Gate emblem */
        DrawCircle(px + 16, py + 20, 4, (Color){255, 215, 0, 200});
        break;
    }
    case TILE_WATER: {
        /* Animated water ripples */
        float wave = sinf(time * 3.0f + px * 0.2f + py * 0.15f);
        int alpha = (int)(40 + wave * 20);
        Color ripple = { 120, 180, 255, (unsigned char)alpha };
        DrawRectangle(px + 4 + (int)(wave * 3), py + 8, 8, 2, ripple);
        DrawRectangle(px + 12 - (int)(wave * 2), py + 20, 10, 2, ripple);
        break;
    }
    default:
        break;
    }
}

void renderer_draw_world(const Game *game)
{
    /* Calculate visible tile range based on camera */
    Vector2 cam_target = game->camera.target;
    float zoom = game->camera.zoom;
    int half_w = (int)(SCREEN_WIDTH / (2.0f * zoom)) + TILE_SIZE * 2;
    int half_h = (int)(SCREEN_HEIGHT / (2.0f * zoom)) + TILE_SIZE * 2;

    int min_tx = (int)((cam_target.x - half_w) / TILE_SIZE);
    int max_tx = (int)((cam_target.x + half_w) / TILE_SIZE);
    int min_ty = (int)((cam_target.y - half_h) / TILE_SIZE);
    int max_ty = (int)((cam_target.y + half_h) / TILE_SIZE);

    if (min_tx < 0) min_tx = 0;
    if (min_ty < 0) min_ty = 0;
    if (max_tx >= WORLD_TILES_X) max_tx = WORLD_TILES_X - 1;
    if (max_ty >= WORLD_TILES_Y) max_ty = WORLD_TILES_Y - 1;

    for (int ty = min_ty; ty <= max_ty; ty++) {
        for (int tx = min_tx; tx <= max_tx; tx++) {
            int cx = tx / CHUNK_SIZE;
            int cy = ty / CHUNK_SIZE;
            int lx = tx % CHUNK_SIZE;
            int ly = ty % CHUNK_SIZE;

            if (cx < 0 || cx >= WORLD_CHUNKS_X || cy < 0 || cy >= WORLD_CHUNKS_Y)
                continue;

            const Chunk *chunk = &game->world.chunks[cy][cx];
            if (!chunk->generated) continue;

            TileType type = chunk->tiles[ly][lx];
            int px = tx * TILE_SIZE;
            int py = ty * TILE_SIZE;

            /* Draw base tile color */
            DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, tile_get_color(type));

            /* Add subtle grid lines */
            DrawRectangleLines(px, py, TILE_SIZE, TILE_SIZE,
                              (Color){0, 0, 0, 15});

            /* Draw tile decorations */
            draw_tile_decoration(type, px, py, game->game_time);
        }
    }
}

/* Draw character sprite (procedural — will be replaced with sprite sheets) */
static void draw_character_sprite(const Entity *entity, float time)
{
    int x = (int)entity->pos.x;
    int y = (int)entity->pos.y;
    int w = 24, h = 28;

    /* Flash white when invincible (hit) */
    Color body_color = entity->color;
    if (entity->invincible_timer > 0 && ((int)(entity->invincible_timer * 10) % 2)) {
        body_color = WHITE;
    }

    /* Body */
    DrawRectangle(x - w/2 + 2, y - h/2 + 6, w - 4, h - 10, body_color);

    /* Head */
    DrawCircle(x, y - h/2 + 4, 8, (Color){255, 220, 180, 255}); /* skin */

    /* Hair based on entity type */
    switch (entity->type) {
    case ENTITY_PLAYER:
        /* Anime-style spiky hair */
        DrawTriangle(
            (Vector2){x - 8, y - h/2 + 2},
            (Vector2){x - 3, y - h/2 - 8},
            (Vector2){x + 2, y - h/2 + 2},
            (Color){30, 30, 40, 255}
        );
        DrawTriangle(
            (Vector2){x - 3, y - h/2 + 2},
            (Vector2){x + 2, y - h/2 - 10},
            (Vector2){x + 7, y - h/2 + 2},
            (Color){30, 30, 40, 255}
        );
        DrawTriangle(
            (Vector2){x + 3, y - h/2 + 2},
            (Vector2){x + 8, y - h/2 - 6},
            (Vector2){x + 10, y - h/2 + 4},
            (Color){30, 30, 40, 255}
        );
        break;
    case ENTITY_NPC_ELDER:
        /* Long white hair/beard */
        DrawRectangle(x - 6, y - h/2 - 2, 12, 4, (Color){220, 220, 230, 255});
        DrawRectangle(x - 3, y - h/2 + 8, 6, 10, (Color){220, 220, 230, 255});
        break;
    case ENTITY_BEAST:
        /* Beast ears */
        DrawTriangle(
            (Vector2){x - 7, y - h/2},
            (Vector2){x - 4, y - h/2 - 8},
            (Vector2){x - 1, y - h/2},
            body_color
        );
        DrawTriangle(
            (Vector2){x + 1, y - h/2},
            (Vector2){x + 4, y - h/2 - 8},
            (Vector2){x + 7, y - h/2},
            body_color
        );
        break;
    default:
        /* Simple hair */
        DrawRectangle(x - 7, y - h/2 - 2, 14, 6, (Color){60, 40, 20, 255});
        break;
    }

    /* Eyes */
    int eye_offset = 0;
    if (entity->dir == DIR_LEFT || entity->dir == DIR_DOWN_LEFT || entity->dir == DIR_UP_LEFT)
        eye_offset = -2;
    else if (entity->dir == DIR_RIGHT || entity->dir == DIR_DOWN_RIGHT || entity->dir == DIR_UP_RIGHT)
        eye_offset = 2;

    if (entity->dir != DIR_UP && entity->dir != DIR_UP_LEFT && entity->dir != DIR_UP_RIGHT) {
        DrawRectangle(x - 4 + eye_offset, y - h/2 + 2, 2, 3, BLACK);
        DrawRectangle(x + 2 + eye_offset, y - h/2 + 2, 2, 3, BLACK);
    }

    /* Weapon / attack effect */
    if (entity->is_attacking) {
        float at = entity->attack_timer;
        int sword_len = 16;
        Color sword_color = { 200, 200, 220, 255 };
        Color slash_color = { 255, 255, 200, (unsigned char)(200 * at * 3) };

        switch (entity->dir) {
        case DIR_RIGHT: case DIR_DOWN_RIGHT: case DIR_UP_RIGHT:
            DrawRectangle(x + w/2, y - 2, sword_len, 3, sword_color);
            DrawCircle(x + w/2 + sword_len, y, 6 * (1.0f - at * 3), slash_color);
            break;
        case DIR_LEFT: case DIR_DOWN_LEFT: case DIR_UP_LEFT:
            DrawRectangle(x - w/2 - sword_len, y - 2, sword_len, 3, sword_color);
            DrawCircle(x - w/2 - sword_len, y, 6 * (1.0f - at * 3), slash_color);
            break;
        case DIR_UP:
            DrawRectangle(x - 1, y - h/2 - sword_len, 3, sword_len, sword_color);
            DrawCircle(x, y - h/2 - sword_len, 6 * (1.0f - at * 3), slash_color);
            break;
        case DIR_DOWN:
        default:
            DrawRectangle(x - 1, y + h/2, 3, sword_len, sword_color);
            DrawCircle(x, y + h/2 + sword_len, 6 * (1.0f - at * 3), slash_color);
            break;
        }
    }

    /* Walking animation — bobbing legs */
    float speed_sq = entity->vel.x * entity->vel.x + entity->vel.y * entity->vel.y;
    if (speed_sq > 0.1f) {
        float bob = sinf(time * 12.0f) * 3.0f;
        DrawRectangle(x - 5, y + h/2 - 6 + (int)bob, 4, 6, (Color){60, 50, 40, 255});
        DrawRectangle(x + 1, y + h/2 - 6 - (int)bob, 4, 6, (Color){60, 50, 40, 255});
    } else {
        DrawRectangle(x - 5, y + h/2 - 6, 4, 6, (Color){60, 50, 40, 255});
        DrawRectangle(x + 1, y + h/2 - 6, 4, 6, (Color){60, 50, 40, 255});
    }
}

void renderer_draw_entity(const Entity *entity, float time)
{
    if (!entity->active) return;
    draw_character_sprite(entity, time);
}

void renderer_draw_entities(const Game *game)
{
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (game->entities[i].active) {
            renderer_draw_entity(&game->entities[i], game->game_time);
        }
    }
}

void renderer_draw_particles(const Game *game)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        const Particle *p = &game->particles[i];
        if (!p->active) continue;

        /* Draw trail if enabled */
        if (p->has_trail) {
            for (int j = 0; j < 3; j++) {
                int idx1 = (p->trail_index - 1 - j + 4) % 4;
                int idx2 = (p->trail_index - 2 - j + 4) % 4;
                Vec2 p1 = p->trail_positions[idx1];
                Vec2 p2 = p->trail_positions[idx2];
                if (p1.x != 0 && p2.x != 0) {
                    Color trail_c = p->color;
                    trail_c.a = (unsigned char)(trail_c.a * (1.0f - (j/3.0f)));
                    DrawLineEx((Vector2){p1.x, p1.y}, (Vector2){p2.x, p2.y}, p->size, trail_c);
                }
            }
        }

        float alpha_mult = p->life / p->max_life;
        Color c = p->color;
        c.a = (unsigned char)(c.a * alpha_mult);
        float size = p->size * alpha_mult;

        DrawRectanglePro(
            (Rectangle){ p->pos.x, p->pos.y, size, size },
            (Vector2){ size / 2, size / 2 },
            p->rotation,
            c
        );
    }
}

void renderer_draw_daynight(const Game *game)
{
    float t = game->world.day_time;
    int alpha = 0;

    /* Night: 20:00 - 6:00 */
    if (t >= 20.0f) {
        alpha = (int)((t - 20.0f) / 4.0f * 120.0f);
        if (alpha > 120) alpha = 120;
    } else if (t < 6.0f) {
        alpha = 120;
    } else if (t < 8.0f) {
        alpha = (int)((1.0f - (t - 6.0f) / 2.0f) * 120.0f);
    }

    /* Dawn/dusk warm tint */
    if ((t >= 5.5f && t < 7.5f) || (t >= 17.5f && t < 19.5f)) {
        float warmth;
        if (t < 12.0f)
            warmth = 1.0f - fabsf(t - 6.5f) / 1.0f;
        else
            warmth = 1.0f - fabsf(t - 18.5f) / 1.0f;
        if (warmth < 0) warmth = 0;
        DrawRectangle(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2,
                      (Color){255, 140, 50, (unsigned char)(warmth * 40)});
    }

    if (alpha > 0) {
        DrawRectangle(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2,
                      (Color){10, 10, 40, (unsigned char)alpha});
    }
}

void renderer_draw_qi_aura(Vec2 pos, CultivationRealm realm, float time)
{
    if (realm <= REALM_MORTAL) return;

    Color aura = REALM_COLORS[realm];
    int num_circles = 3 + (int)realm;
    float base_radius = 18.0f + realm * 4.0f;

    for (int i = 0; i < num_circles; i++) {
        float offset = (float)i / num_circles * 6.2831f;
        float pulse = sinf(time * 3.0f + offset) * 0.3f + 0.7f;
        float r = base_radius * pulse;
        Color c = aura;
        c.a = (unsigned char)(aura.a * pulse * 0.5f);
        DrawCircle((int)pos.x, (int)pos.y, r, c);
    }

    /* Rising qi particles */
    for (int i = 0; i < realm + 2; i++) {
        float angle = time * 2.0f + i * 1.2f;
        float dist = 12.0f + sinf(time * 4.0f + i) * 6.0f;
        float px = pos.x + cosf(angle) * dist;
        float py = pos.y + sinf(angle) * dist - sinf(time * 3.0f + i * 0.7f) * 8.0f;
        Color c = aura;
        c.a = (unsigned char)(aura.a * 0.8f);
        DrawCircle((int)px, (int)py, 2.0f + realm * 0.5f, c);
    }
}

void renderer_draw_damage_number(Vec2 pos, int damage, float timer)
{
    if (timer <= 0) return;
    float alpha = timer * 2.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    float rise = (1.0f - timer) * 30.0f;

    char buf[16];
    snprintf(buf, sizeof(buf), "-%d", damage);
    Color c = { 255, 60, 60, (unsigned char)(alpha * 255) };
    DrawText(buf, (int)pos.x - 10, (int)(pos.y - 20 - rise), 16, c);
}

void renderer_draw_health_bar(Vec2 pos, int hp, int max_hp, Color color)
{
    if (hp >= max_hp) return; /* Don't show full health */

    int bar_w = 30;
    int bar_h = 4;
    int x = (int)pos.x - bar_w / 2;
    int y = (int)pos.y - 22;
    float ratio = (float)hp / max_hp;

    DrawRectangle(x - 1, y - 1, bar_w + 2, bar_h + 2, (Color){0, 0, 0, 150});
    DrawRectangle(x, y, (int)(bar_w * ratio), bar_h, color);
}
