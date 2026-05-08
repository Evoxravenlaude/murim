/*
 * hud.c — HUD rendering: bars, minimap, menus, dialogue
 * Heavenly Demon: Murim Chronicles
 */
#include "hud.h"
#include "../engine/renderer.h"
#include "../game/cultivation.h"
#include "../game/combat.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ─── Colors ──────────────────────────────────────────── */
#define HUD_BG          (Color){ 20, 15, 10, 200 }
#define HUD_BORDER      (Color){ 140, 110, 60, 255 }
#define HUD_HP_COLOR    (Color){ 200, 50, 50, 255 }
#define HUD_QI_COLOR    (Color){ 80, 160, 255, 255 }
#define HUD_XP_COLOR    (Color){ 100, 200, 80, 255 }
#define HUD_TEXT         (Color){ 230, 220, 190, 255 }
#define HUD_TEXT_DIM     (Color){ 160, 150, 120, 200 }
#define HUD_GOLD_COLOR   (Color){ 255, 215, 0, 255 }

/* ─── Helper: draw a styled bar ───────────────────────── */
static void draw_bar(int x, int y, int w, int h, float ratio, Color color, const char *label)
{
    /* Background */
    DrawRectangle(x, y, w, h, (Color){10, 10, 10, 180});
    /* Fill */
    int fill_w = (int)(w * ratio);
    if (fill_w < 0) fill_w = 0;
    DrawRectangle(x, y, fill_w, h, color);
    /* Shine effect */
    DrawRectangle(x, y, fill_w, h / 3, (Color){255, 255, 255, 30});
    /* Border */
    DrawRectangleLines(x, y, w, h, HUD_BORDER);
    /* Label */
    if (label) {
        DrawText(label, x + 4, y + (h - 10) / 2, 10, WHITE);
    }
}

/* ─── Helper: draw panel ──────────────────────────────── */
static void draw_panel(int x, int y, int w, int h)
{
    DrawRectangle(x, y, w, h, HUD_BG);
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, 2, HUD_BORDER);
    /* Corner ornaments */
    int cs = 6;
    DrawRectangle(x, y, cs, cs, HUD_GOLD_COLOR);
    DrawRectangle(x + w - cs, y, cs, cs, HUD_GOLD_COLOR);
    DrawRectangle(x, y + h - cs, cs, cs, HUD_GOLD_COLOR);
    DrawRectangle(x + w - cs, y + h - cs, cs, cs, HUD_GOLD_COLOR);
}

void hud_draw(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    char buf[128];

    /* ─── Top-left: HP and QI bars ──────────────────── */
    draw_panel(10, 10, 260, 70);

    float hp_ratio = (float)player->stats.hp / player->stats.max_hp;
    snprintf(buf, sizeof(buf), "HP %d/%d", player->stats.hp, player->stats.max_hp);
    draw_bar(20, 20, 200, 16, hp_ratio, HUD_HP_COLOR, buf);

    float qi_ratio = (float)player->stats.qi / player->stats.max_qi;
    snprintf(buf, sizeof(buf), "QI %d/%d", player->stats.qi, player->stats.max_qi);
    draw_bar(20, 40, 200, 16, qi_ratio, HUD_QI_COLOR, buf);

    /* Cultivation realm */
    snprintf(buf, sizeof(buf), "%s Lv.%d",
             cultivation_realm_name(player->cultivation.realm),
             player->cultivation.sub_level);
    DrawText(buf, 20, 60, 10, HUD_GOLD_COLOR);

    /* Cultivation progress */
    float cult_progress = player->cultivation.progress;
    draw_bar(130, 60, 90, 8, cult_progress, (Color){180, 120, 255, 200}, NULL);

    /* ─── Top-right: Time and gold ──────────────────── */
    draw_panel(SCREEN_WIDTH - 170, 10, 160, 50);

    int hour = (int)game->world.day_time;
    int minute = (int)((game->world.day_time - hour) * 60);
    const char *period = hour >= 12 ? "PM" : "AM";
    int display_hour = hour % 12;
    if (display_hour == 0) display_hour = 12;
    snprintf(buf, sizeof(buf), "%02d:%02d %s", display_hour, minute, period);
    DrawText(buf, SCREEN_WIDTH - 155, 20, 14, HUD_TEXT);

    snprintf(buf, sizeof(buf), "Gold: %d", player->gold);
    DrawText(buf, SCREEN_WIDTH - 155, 40, 12, HUD_GOLD_COLOR);

    /* ─── Bottom: Active technique ──────────────────── */
    if (player->num_techniques > 0) {
        draw_panel(10, SCREEN_HEIGHT - 50, 250, 40);

        const Technique *tech = &player->techniques[player->active_technique];
        snprintf(buf, sizeof(buf), "[Q] %s  (QI: %d)", tech->name, tech->qi_cost);
        DrawText(buf, 20, SCREEN_HEIGHT - 42, 12, HUD_TEXT);

        if (tech->current_cooldown > 0) {
            snprintf(buf, sizeof(buf), "CD: %.1fs", tech->current_cooldown);
            DrawText(buf, 20, SCREEN_HEIGHT - 28, 10, (Color){255, 100, 100, 200});
        } else {
            DrawText("READY", 200, SCREEN_HEIGHT - 28, 10, (Color){100, 255, 100, 200});
        }

        /* Technique cycle hint */
        DrawText("[1-5] Switch", 20, SCREEN_HEIGHT - 16, 8, HUD_TEXT_DIM);
    }

    /* ─── Bottom-right: Controls hint ────────────── */
    draw_panel(SCREEN_WIDTH - 210, SCREEN_HEIGHT - 110, 200, 100);
    DrawText("WASD: Move  SPACE: Attack", SCREEN_WIDTH - 202, SCREEN_HEIGHT - 102, 8, HUD_TEXT_DIM);
    DrawText("E: Cultivate  Q: Technique", SCREEN_WIDTH - 202, SCREEN_HEIGHT - 90, 8, HUD_TEXT_DIM);
    DrawText("TAB: Stats    ESC: Pause",   SCREEN_WIDTH - 202, SCREEN_HEIGHT - 78, 8, HUD_TEXT_DIM);
    DrawText("F: Harvest    H: Fish",      SCREEN_WIDTH - 202, SCREEN_HEIGHT - 66, 8, HUD_TEXT_DIM);
    DrawText("N: Eat        U: Drink",     SCREEN_WIDTH - 202, SCREEN_HEIGHT - 54, 8, HUD_TEXT_DIM);
    DrawText("J: Quests     P: Stats(+)",  SCREEN_WIDTH - 202, SCREEN_HEIGHT - 42, 8, HUD_TEXT_DIM);
    DrawText("F3: Debug     F10: Save",    SCREEN_WIDTH - 202, SCREEN_HEIGHT - 30, 8, HUD_TEXT_DIM);

    /* ─── Debug info ────────────────────────────────── */
    if (game->show_debug) {
        snprintf(buf, sizeof(buf), "FPS: %d", GetFPS());
        DrawText(buf, SCREEN_WIDTH / 2 - 30, 10, 14, GREEN);

        snprintf(buf, sizeof(buf), "Pos: %.0f, %.0f", player->pos.x, player->pos.y);
        DrawText(buf, SCREEN_WIDTH / 2 - 50, 26, 10, GREEN);

        snprintf(buf, sizeof(buf), "Tile: %d, %d",
                 (int)(player->pos.x / TILE_SIZE),
                 (int)(player->pos.y / TILE_SIZE));
        DrawText(buf, SCREEN_WIDTH / 2 - 50, 38, 10, GREEN);

        int entity_count = 0;
        for (int i = 0; i < MAX_ENTITIES; i++) {
            if (game->entities[i].active) entity_count++;
        }
        snprintf(buf, sizeof(buf), "Entities: %d", entity_count);
        DrawText(buf, SCREEN_WIDTH / 2 - 50, 50, 10, GREEN);
    }

    /* ─── Cultivation mode indicator ────────────────── */
    if (player->cultivation.is_cultivating) {
        const char *msg = "~ Cultivating ~";
        int tw = MeasureText(msg, 20);
        float pulse = sinf(game->game_time * 3.0f) * 0.3f + 0.7f;
        Color glow = { 180, 120, 255, (unsigned char)(pulse * 255) };
        DrawText(msg, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 + 50, 20, glow);
    }

    /* ─── Minimap ───────────────────────────────────── */
    hud_draw_minimap(game);

    /* ─── NPC interaction prompt ────────────────────── */
    /* (drawn by main game loop when near NPC) */
}

void hud_draw_title(const Game *game)
{
    (void)game;
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){10, 8, 5, 255});

    /* Title with glow effect */
    const char *title = "HEAVENLY DEMON";
    const char *subtitle = "Murim Chronicles";
    int title_size = 48;
    int sub_size = 24;
    int title_w = MeasureText(title, title_size);
    int sub_w = MeasureText(subtitle, sub_size);

    float time = (float)GetTime();
    float pulse = sinf(time * 2.0f) * 0.2f + 0.8f;

    /* Glow behind title */
    Color glow = { 180, 100, 40, (unsigned char)(pulse * 60) };
    DrawCircle(SCREEN_WIDTH / 2, 200, 200 * pulse, glow);

    DrawText(title, SCREEN_WIDTH / 2 - title_w / 2, 180,
             title_size, (Color){220, 180, 100, 255});
    DrawText(subtitle, SCREEN_WIDTH / 2 - sub_w / 2, 240,
             sub_size, (Color){180, 160, 120, 200});

    /* Menu options */
    float bob = sinf(time * 3.0f) * 3.0f;
    const char *start_text = "Press ENTER to Begin Your Journey";
    int start_w = MeasureText(start_text, 18);
    DrawText(start_text, SCREEN_WIDTH / 2 - start_w / 2,
             380 + (int)bob, 18, (Color){200, 190, 160, (unsigned char)(pulse * 255)});

    /* Decorative elements */
    DrawText("⚔", SCREEN_WIDTH / 2 - 80, 300, 30, (Color){140, 100, 50, 200});
    DrawText("⚔", SCREEN_WIDTH / 2 + 60, 300, 30, (Color){140, 100, 50, 200});

    /* Credits */
    DrawText("A Murim-Inspired Open World Adventure",
             SCREEN_WIDTH / 2 - 155, 500, 12, HUD_TEXT_DIM);
    DrawText("Built with C / Raylib", SCREEN_WIDTH / 2 - 70, 520, 10, HUD_TEXT_DIM);
}

void hud_draw_pause(const Game *game)
{
    (void)game;
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 150});

    draw_panel(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 100, 300, 200);

    const char *title = "PAUSED";
    int tw = MeasureText(title, 30);
    DrawText(title, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 - 80, 30, HUD_GOLD_COLOR);

    DrawText("Press ESC to Resume", SCREEN_WIDTH / 2 - 70,
             SCREEN_HEIGHT / 2 - 20, 14, HUD_TEXT);
    DrawText("Press R to Restart", SCREEN_WIDTH / 2 - 65,
             SCREEN_HEIGHT / 2 + 10, 14, HUD_TEXT);
}

void hud_draw_stats(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    char buf[128];

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});

    /* Main panel */
    draw_panel(100, 50, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 100);

    DrawText("Character Stats", 140, 70, 24, HUD_GOLD_COLOR);
    DrawText("────────────────────────────", 140, 96, 12, HUD_BORDER);

    int y = 120;
    int x = 140;

    snprintf(buf, sizeof(buf), "Name: %s", player->name);
    DrawText(buf, x, y, 14, HUD_TEXT); y += 24;

    snprintf(buf, sizeof(buf), "Realm: %s (Level %d)",
             cultivation_realm_name(player->cultivation.realm),
             player->cultivation.sub_level);
    DrawText(buf, x, y, 14, HUD_GOLD_COLOR); y += 24;

    snprintf(buf, sizeof(buf), "HP: %d / %d", player->stats.hp, player->stats.max_hp);
    DrawText(buf, x, y, 14, HUD_HP_COLOR); y += 20;

    snprintf(buf, sizeof(buf), "QI: %d / %d", player->stats.qi, player->stats.max_qi);
    DrawText(buf, x, y, 14, HUD_QI_COLOR); y += 20;

    snprintf(buf, sizeof(buf), "Attack: %d", player->stats.attack);
    DrawText(buf, x, y, 14, HUD_TEXT); y += 20;

    snprintf(buf, sizeof(buf), "Defense: %d", player->stats.defense);
    DrawText(buf, x, y, 14, HUD_TEXT); y += 20;

    snprintf(buf, sizeof(buf), "Speed: %d", player->stats.speed);
    DrawText(buf, x, y, 14, HUD_TEXT); y += 20;

    snprintf(buf, sizeof(buf), "QI Regen: %d/s", player->stats.qi_regen);
    DrawText(buf, x, y, 14, HUD_TEXT); y += 24;

    snprintf(buf, sizeof(buf), "Experience: %d", player->experience);
    DrawText(buf, x, y, 14, HUD_XP_COLOR); y += 20;

    snprintf(buf, sizeof(buf), "Gold: %d", player->gold);
    DrawText(buf, x, y, 14, HUD_GOLD_COLOR); y += 30;

    /* Techniques */
    DrawText("Techniques:", x, y, 16, HUD_GOLD_COLOR); y += 22;
    for (int i = 0; i < player->num_techniques; i++) {
        const Technique *tech = &player->techniques[i];
        bool available = player->cultivation.realm >= tech->min_realm;
        Color tc = available ? HUD_TEXT : (Color){100, 100, 100, 150};

        snprintf(buf, sizeof(buf), "[%d] %s  (ATK: %d  QI: %d  Realm: %s)%s",
                 i + 1, tech->name, tech->damage, tech->qi_cost,
                 cultivation_realm_name(tech->min_realm),
                 i == player->active_technique ? "  ◄" : "");
        DrawText(buf, x + 10, y, 11, tc);
        y += 18;
    }

    DrawText("Press TAB to close", SCREEN_WIDTH / 2 - 60,
             SCREEN_HEIGHT - 80, 12, HUD_TEXT_DIM);
}

void hud_draw_cultivation(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    char buf[128];

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 120});

    draw_panel(SCREEN_WIDTH / 2 - 200, 100, 400, 300);

    DrawText("Cultivation", SCREEN_WIDTH / 2 - 55, 120, 24, HUD_GOLD_COLOR);

    float progress = player->cultivation.progress;
    float required = cultivation_qi_required(player->cultivation.realm,
                                             player->cultivation.sub_level);

    snprintf(buf, sizeof(buf), "Current: %s Level %d",
             cultivation_realm_name(player->cultivation.realm),
             player->cultivation.sub_level);
    DrawText(buf, SCREEN_WIDTH / 2 - 130, 170, 14, HUD_TEXT);

    /* Progress bar */
    draw_bar(SCREEN_WIDTH / 2 - 150, 200, 300, 20, progress,
             (Color){180, 120, 255, 220}, NULL);

    snprintf(buf, sizeof(buf), "Qi: %.0f / %.0f", player->cultivation.qi_absorbed, required);
    DrawText(buf, SCREEN_WIDTH / 2 - 50, 225, 12, HUD_TEXT);

    /* Next realm preview */
    if (player->cultivation.sub_level >= 9 &&
        player->cultivation.realm < REALM_TRANSCENDENCE) {
        snprintf(buf, sizeof(buf), "Next Realm: %s",
                 cultivation_realm_name(player->cultivation.realm + 1));
        DrawText(buf, SCREEN_WIDTH / 2 - 80, 260, 14, (Color){255, 215, 0, 200});
    }

    float pulse = sinf(game->game_time * 4.0f) * 0.3f + 0.7f;
    DrawText("Press E to stop cultivating", SCREEN_WIDTH / 2 - 95, 340, 12,
             (Color){200, 190, 160, (unsigned char)(pulse * 255)});
}

void hud_draw_game_over(const Game *game)
{
    (void)game;
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){40, 0, 0, 200});

    const char *title = "DEFEATED";
    int tw = MeasureText(title, 40);
    DrawText(title, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 - 60, 40,
             (Color){200, 50, 50, 255});

    DrawText("Your journey has ended... for now.",
             SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2, 14, HUD_TEXT);

    float time = (float)GetTime();
    float pulse = sinf(time * 3.0f) * 0.3f + 0.7f;
    DrawText("Press R to try again",
             SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 + 40, 14,
             (Color){200, 190, 160, (unsigned char)(pulse * 255)});
}

void hud_draw_dialogue(const Game *game)
{
    if (!game->dialogue.active) return;

    const DialogueBox *dlg = &game->dialogue;

    /* Dialogue panel at bottom of screen */
    draw_panel(50, SCREEN_HEIGHT - 160, SCREEN_WIDTH - 100, 140);

    /* Speaker name */
    if (dlg->speaker) {
        DrawRectangle(70, SCREEN_HEIGHT - 170, 120, 20, HUD_BG);
        DrawRectangleLinesEx((Rectangle){70, SCREEN_HEIGHT - 170, 120, 20}, 1, HUD_BORDER);
        DrawText(dlg->speaker, 80, SCREEN_HEIGHT - 166, 12, HUD_GOLD_COLOR);
    }

    /* Current line with typewriter effect */
    if (dlg->current_line < dlg->num_lines) {
        const char *line = dlg->lines[dlg->current_line];
        char display[256];
        int len = dlg->chars_shown;
        int line_len = (int)strlen(line);
        if (len > line_len) len = line_len;
        strncpy(display, line, len);
        display[len] = '\0';

        DrawText(display, 80, SCREEN_HEIGHT - 130, 14, HUD_TEXT);
    }

    /* Continue prompt */
    if (dlg->chars_shown >= (int)strlen(dlg->lines[dlg->current_line])) {
        float pulse = sinf(game->game_time * 5.0f) * 0.3f + 0.7f;
        DrawText("Press E to continue ▼", SCREEN_WIDTH - 230,
                 SCREEN_HEIGHT - 45, 10,
                 (Color){200, 190, 160, (unsigned char)(pulse * 255)});
    }
}

void hud_draw_minimap(const Game *game)
{
    int mm_size = 100;
    int mm_x = SCREEN_WIDTH - mm_size - 15;
    int mm_y = 70;

    DrawRectangle(mm_x - 2, mm_y - 2, mm_size + 4, mm_size + 4, (Color){0, 0, 0, 180});
    DrawRectangleLinesEx((Rectangle){mm_x - 2, mm_y - 2, mm_size + 4, mm_size + 4},
                         1, HUD_BORDER);

    const Entity *player = &game->entities[game->player_id];
    float scale = (float)mm_size / (WORLD_TILES_X * TILE_SIZE);

    /* Draw world overview */
    for (int cy = 0; cy < WORLD_CHUNKS_Y; cy++) {
        for (int cx = 0; cx < WORLD_CHUNKS_X; cx++) {
            const Chunk *chunk = &game->world.chunks[cy][cx];
            if (!chunk->generated) continue;

            /* Sample a few tiles per chunk for minimap */
            int r = 0, g = 0, b = 0;
            int samples = 0;
            for (int sy = 0; sy < CHUNK_SIZE; sy += 4) {
                for (int sx = 0; sx < CHUNK_SIZE; sx += 4) {
                    Color c = tile_get_color(chunk->tiles[sy][sx]);
                    r += c.r; g += c.g; b += c.b;
                    samples++;
                }
            }
            if (samples > 0) {
                Color avg = { r / samples, g / samples, b / samples, 200 };
                int px = mm_x + (int)(cx * CHUNK_SIZE * TILE_SIZE * scale);
                int py = mm_y + (int)(cy * CHUNK_SIZE * TILE_SIZE * scale);
                int pw = (int)(CHUNK_SIZE * TILE_SIZE * scale);
                if (pw < 1) pw = 1;
                DrawRectangle(px, py, pw, pw, avg);
            }
        }
    }

    /* Draw player position */
    int pp_x = mm_x + (int)(player->pos.x * scale);
    int pp_y = mm_y + (int)(player->pos.y * scale);
    DrawCircle(pp_x, pp_y, 3, WHITE);
    DrawCircle(pp_x, pp_y, 2, (Color){80, 160, 255, 255});

    /* Draw entity dots */
    for (int i = 0; i < MAX_ENTITIES; i++) {
        const Entity *e = &game->entities[i];
        if (!e->active || i == game->player_id) continue;

        int ex = mm_x + (int)(e->pos.x * scale);
        int ey = mm_y + (int)(e->pos.y * scale);

        if (ex < mm_x || ex > mm_x + mm_size || ey < mm_y || ey > mm_y + mm_size)
            continue;

        Color dot = GREEN;
        if (e->type == ENTITY_NPC_HOSTILE || e->type == ENTITY_BEAST)
            dot = RED;
        else if (e->type == ENTITY_NPC_MERCHANT)
            dot = YELLOW;
        else if (e->type == ENTITY_NPC_ELDER)
            dot = (Color){200, 200, 255, 255};

        DrawRectangle(ex, ey, 2, 2, dot);
    }
}
