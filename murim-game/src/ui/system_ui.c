/*
 * system_ui.c — Solo Leveling-style System Interface
 * Heavenly Demon: Murim Chronicles v2.0
 */
#include "system_ui.h"
#include "../game/cultivation.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ─── Solo Leveling System Colors ─────────────────────── */
#define SYS_PANEL_BG        (Color){ 10, 15, 30, 220 }
#define SYS_PANEL_BORDER    (Color){ 60, 120, 255, 200 }
#define SYS_TEXT_PRIMARY    (Color){ 220, 240, 255, 255 }
#define SYS_TEXT_SECONDARY  (Color){ 100, 180, 255, 200 }
#define SYS_GLOW            (Color){ 40, 100, 255, 100 }
#define SYS_ALERT           (Color){ 255, 80, 80, 255 }
#define SYS_SUCCESS         (Color){ 80, 255, 120, 255 }
#define SYS_GOLD            (Color){ 255, 215, 0, 255 }

/* ─── Initialize ──────────────────────────────────────── */
void system_ui_init(Game *game)
{
    memset(game->notifications, 0, sizeof(game->notifications));
    game->system_ui_open = false;
    game->level_up_anim = 0.0f;
}

/* ─── Core Drawing Primitives ─────────────────────────── */

void system_ui_draw_panel(int x, int y, int w, int h, float alpha)
{
    Color bg = SYS_PANEL_BG;
    Color border = SYS_PANEL_BORDER;
    bg.a = (unsigned char)(bg.a * alpha);
    border.a = (unsigned char)(border.a * alpha);

    /* Background with slight gradient effect via multiple rects */
    DrawRectangle(x, y, w, h, bg);
    
    /* Top highlight */
    DrawRectangle(x, y, w, 2, (Color){ 120, 180, 255, (unsigned char)(150 * alpha) });
    
    /* Borders */
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, 1.5f, border);
    
    /* Corner brackets (tech UI feel) */
    int cb = 8; /* bracket size */
    DrawRectangle(x - 1, y - 1, cb, 3, border);
    DrawRectangle(x - 1, y - 1, 3, cb, border);
    
    DrawRectangle(x + w - cb + 1, y - 1, cb, 3, border);
    DrawRectangle(x + w - 2, y - 1, 3, cb, border);
    
    DrawRectangle(x - 1, y + h - 2, cb, 3, border);
    DrawRectangle(x - 1, y + h - cb + 1, 3, cb, border);
    
    DrawRectangle(x + w - cb + 1, y + h - 2, cb, 3, border);
    DrawRectangle(x + w - 2, y + h - cb + 1, 3, cb, border);

    /* Scanline effect */
    for (int sy = y + 2; sy < y + h - 2; sy += 4) {
        DrawRectangle(x + 2, sy, w - 4, 1, (Color){ 0, 50, 150, (unsigned char)(20 * alpha) });
    }
}

void system_ui_draw_glow_box(int x, int y, int w, int h, Color color, float glow_strength)
{
    color.a = (unsigned char)(color.a * glow_strength);
    DrawRectangle(x - 4, y - 4, w + 8, h + 8, (Color){ color.r, color.g, color.b, (unsigned char)(color.a * 0.2f) });
    DrawRectangle(x - 2, y - 2, w + 4, h + 4, (Color){ color.r, color.g, color.b, (unsigned char)(color.a * 0.4f) });
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, 1, color);
}

/* ─── Notifications ───────────────────────────────────── */

static int get_free_notification(Game *game)
{
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (!game->notifications[i].active) return i;
    }
    /* If full, overwrite the oldest one (index 0 usually, but let's just shift) */
    for (int i = 0; i < MAX_NOTIFICATIONS - 1; i++) {
        game->notifications[i] = game->notifications[i+1];
    }
    return MAX_NOTIFICATIONS - 1;
}

void system_notify(Game *game, NotifyPriority priority, const char *title, const char *body)
{
    int idx = get_free_notification(game);
    SystemNotification *n = &game->notifications[idx];
    
    n->active = true;
    n->priority = priority;
    strncpy(n->title, title, sizeof(n->title) - 1);
    strncpy(n->body, body, sizeof(n->body) - 1);
    n->timer = 0;
    
    /* Display duration based on priority */
    n->duration = (priority == NOTIFY_LEVEL_UP || priority == NOTIFY_RANK_UP) ? 5.0f : 3.0f;
    n->alpha = 0.0f;
    n->slide_offset = 50.0f;
}

void system_notify_level_up(Game *game, int new_level)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "You have reached Level %d", new_level);
    system_notify(game, NOTIFY_LEVEL_UP, "[ Level Up! ]", buf);
    game->level_up_anim = 2.0f;
}

void system_notify_rank_up(Game *game, HunterRank new_rank)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "You have become an %s Hunter", rank_name(new_rank));
    system_notify(game, NOTIFY_RANK_UP, "[ Rank Advancement ]", buf);
}

void system_notify_quest_complete(Game *game, const char *quest_name)
{
    system_notify(game, NOTIFY_QUEST, "[ Quest Complete ]", quest_name);
}

void system_notify_item_drop(Game *game, const char *item_name, ItemRarity rarity)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "Acquired: %s (%s)", item_name, rarity_name(rarity));
    system_notify(game, NOTIFY_LOOT, "[ Item Acquired ]", buf);
}

void system_notify_shadow_arise(Game *game, const char *shadow_name)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "Shadow extraction successful: %s", shadow_name);
    system_notify(game, NOTIFY_SHADOW, "[ ARISE ]", buf);
}

void system_notify_realm_breakthrough(Game *game, CultivationRealm realm)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "Broken through to %s", cultivation_realm_name(realm));
    system_notify(game, NOTIFY_SUCCESS, "[ Cultivation Breakthrough ]", buf);
}

void system_notify_beast_captured(Game *game, const char *beast_name)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "Successfully tamed %s", beast_name);
    system_notify(game, NOTIFY_SUCCESS, "[ Taming Successful ]", buf);
}

void system_ui_update(Game *game, float dt)
{
    /* Update level up animation */
    if (game->level_up_anim > 0) {
        game->level_up_anim -= dt;
    }

    /* Update notifications */
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        SystemNotification *n = &game->notifications[i];
        if (!n->active) continue;
        
        n->timer += dt;
        
        /* Fade in and slide up */
        if (n->timer < 0.3f) {
            n->alpha = n->timer / 0.3f;
            n->slide_offset = 50.0f * (1.0f - n->alpha);
        } 
        /* Fade out */
        else if (n->timer > n->duration - 0.5f) {
            float fade = (n->duration - n->timer) / 0.5f;
            n->alpha = fade;
            if (n->alpha < 0) n->alpha = 0;
        } 
        /* Hold */
        else {
            n->alpha = 1.0f;
            n->slide_offset = 0;
        }
        
        if (n->timer >= n->duration) {
            n->active = false;
        }
    }
}

void system_ui_draw_notifications(const Game *game)
{
    int base_y = SCREEN_HEIGHT / 4;
    int draw_count = 0;
    
    for (int i = MAX_NOTIFICATIONS - 1; i >= 0; i--) {
        const SystemNotification *n = &game->notifications[i];
        if (!n->active) continue;
        
        int nw = 350;
        int nh = 70;
        int nx = SCREEN_WIDTH / 2 - nw / 2;
        int ny = base_y + (draw_count * (nh + 10)) + (int)n->slide_offset;
        
        system_ui_draw_panel(nx, ny, nw, nh, n->alpha);
        
        Color title_color = SYS_TEXT_PRIMARY;
        if (n->priority == NOTIFY_LEVEL_UP || n->priority == NOTIFY_RANK_UP) {
            title_color = SYS_GOLD;
        } else if (n->priority == NOTIFY_WARNING) {
            title_color = SYS_ALERT;
        } else if (n->priority == NOTIFY_SHADOW) {
            title_color = (Color){ 180, 80, 255, 255 }; /* Purple for shadow */
        }
        
        title_color.a = (unsigned char)(title_color.a * n->alpha);
        Color body_color = SYS_TEXT_SECONDARY;
        body_color.a = (unsigned char)(body_color.a * n->alpha);
        
        /* Typewriter effect for body based on timer */
        int chars_to_show = (int)((n->timer / 0.5f) * strlen(n->body));
        if (chars_to_show > (int)strlen(n->body)) chars_to_show = strlen(n->body);
        if (n->timer > 0.5f) chars_to_show = strlen(n->body);
        
        char display_body[256] = {0};
        strncpy(display_body, n->body, chars_to_show);
        
        int title_w = MeasureText(n->title, 16);
        DrawText(n->title, nx + nw/2 - title_w/2, ny + 15, 16, title_color);
        
        int body_w = MeasureText(display_body, 14);
        DrawText(display_body, nx + nw/2 - body_w/2, ny + 40, 14, body_color);
        
        draw_count++;
        if (draw_count > 4) break; /* Max 4 visible at once */
    }
}

void system_ui_draw_level_up(const Game *game)
{
    if (game->level_up_anim <= 0) return;
    
    /* Draw column of light on player (requires world coordinates, so this should ideally be in game_draw with camera active)
       But we can draw a screen overlay here */
    
    float alpha = 1.0f;
    if (game->level_up_anim > 1.5f) {
        alpha = (2.0f - game->level_up_anim) / 0.5f;
    } else if (game->level_up_anim < 0.5f) {
        alpha = game->level_up_anim / 0.5f;
    }
    
    Color glow = { 100, 200, 255, (unsigned char)(80 * alpha) };
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, glow);
}

void system_ui_draw_rank_badge(const Game *game, int x, int y)
{
    const Entity *player = &game->entities[game->player_id];
    HunterRank rank = player->hunter_rank;
    
    Color rc = rank_color(rank);
    const char *rn = rank_name(rank);
    
    /* Shield shape for badge */
    Vector2 p1 = { x, y };
    Vector2 p2 = { x + 40, y };
    Vector2 p3 = { x + 40, y + 40 };
    Vector2 p4 = { x + 20, y + 55 };
    Vector2 p5 = { x, y + 40 };
    
    /* Glow */
    DrawTriangle(p1, p5, p4, (Color){ rc.r, rc.g, rc.b, 100 });
    DrawTriangle(p1, p4, p2, (Color){ rc.r, rc.g, rc.b, 100 });
    DrawTriangle(p2, p4, p3, (Color){ rc.r, rc.g, rc.b, 100 });
    
    /* Outline */
    DrawLineEx(p1, p2, 2, rc);
    DrawLineEx(p2, p3, 2, rc);
    DrawLineEx(p3, p4, 2, rc);
    DrawLineEx(p4, p5, 2, rc);
    DrawLineEx(p5, p1, 2, rc);
    
    /* Rank letter */
    char letter[4] = {0};
    if (rank <= RANK_S) {
        letter[0] = rn[0];
    } else if (rank == RANK_SS) {
        strcpy(letter, "SS");
    } else if (rank == RANK_SSS) {
        strcpy(letter, "SSS");
    } else if (rank == RANK_NATIONAL) {
        strcpy(letter, "NL");
    } else {
        strcpy(letter, "M");
    }
    
    int lw = MeasureText(letter, 24);
    DrawText(letter, x + 20 - lw/2, y + 15, 24, WHITE);
    
    /* Little rank label below */
    DrawText("RANK", x + 8, y + 60, 10, SYS_TEXT_SECONDARY);
}

void system_ui_draw_status_window(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });
    
    int pw = 800;
    int ph = 500;
    int px = SCREEN_WIDTH/2 - pw/2;
    int py = SCREEN_HEIGHT/2 - ph/2;
    
    system_ui_draw_panel(px, py, pw, ph, 1.0f);
    
    /* Title */
    DrawText("[ STATUS WINDOW ]", px + 30, py + 30, 24, SYS_TEXT_PRIMARY);
    DrawLine(px + 30, py + 60, px + pw - 30, py + 60, SYS_PANEL_BORDER);
    
    /* Left Column: Basic Info & Portrait */
    int lx = px + 40;
    int ly = py + 80;
    
    /* Portrait placeholder */
    system_ui_draw_glow_box(lx, ly, 150, 150, SYS_PANEL_BORDER, 0.5f);
    DrawText("PLAYER", lx + 45, ly + 65, 16, SYS_TEXT_SECONDARY);
    
    ly += 170;
    char buf[128];
    snprintf(buf, sizeof(buf), "Name:  %s", player->name);
    DrawText(buf, lx, ly, 18, SYS_TEXT_PRIMARY); ly += 30;
    
    snprintf(buf, sizeof(buf), "Level: %d", player->stats.level);
    DrawText(buf, lx, ly, 18, SYS_GOLD); ly += 30;
    
    snprintf(buf, sizeof(buf), "Class: %s", "Necromancer / Cultivator");
    DrawText(buf, lx, ly, 18, SYS_TEXT_PRIMARY); ly += 30;
    
    snprintf(buf, sizeof(buf), "Title: %s", "Demon Hunter");
    DrawText(buf, lx, ly, 18, SYS_TEXT_PRIMARY); ly += 30;
    
    /* Middle Column: Stats */
    int mx = px + 250;
    int my = py + 80;
    
    DrawText("■ ATTRIBUTES", mx, my, 20, SYS_TEXT_SECONDARY); my += 30;
    
    snprintf(buf, sizeof(buf), "HP: %d / %d", player->stats.hp, player->stats.max_hp);
    DrawText(buf, mx, my, 18, SYS_SUCCESS); my += 30;
    
    snprintf(buf, sizeof(buf), "MP: %d / %d", player->stats.qi, player->stats.max_qi);
    DrawText(buf, mx, my, 18, (Color){100, 150, 255, 255}); my += 40;
    
    snprintf(buf, sizeof(buf), "STR:   %d", player->stats.strength);
    DrawText(buf, mx, my, 18, SYS_TEXT_PRIMARY); my += 30;
    
    snprintf(buf, sizeof(buf), "VIT:   %d", player->stats.vitality);
    DrawText(buf, mx, my, 18, SYS_TEXT_PRIMARY); my += 30;
    
    snprintf(buf, sizeof(buf), "AGI:   %d", player->stats.agility);
    DrawText(buf, mx, my, 18, SYS_TEXT_PRIMARY); my += 30;
    
    snprintf(buf, sizeof(buf), "INT:   %d", player->stats.intelligence);
    DrawText(buf, mx, my, 18, SYS_TEXT_PRIMARY); my += 30;
    
    snprintf(buf, sizeof(buf), "SENSE: %d", player->stats.sense);
    DrawText(buf, mx, my, 18, SYS_TEXT_PRIMARY); my += 30;
    
    if (player->stats.stat_points > 0) {
        snprintf(buf, sizeof(buf), "Available Points: %d", player->stats.stat_points);
        DrawText(buf, mx, my + 10, 18, SYS_GOLD);
    }
    
    /* Right Column: Other Info */
    int rx = px + 520;
    int ry = py + 80;
    
    DrawText("■ PROGRESSION", rx, ry, 20, SYS_TEXT_SECONDARY); ry += 30;
    
    system_ui_draw_rank_badge(game, rx, ry);
    ry += 80;
    
    snprintf(buf, sizeof(buf), "Gold: %d G", player->gold);
    DrawText(buf, rx, ry, 18, SYS_GOLD); ry += 30;
    
    snprintf(buf, sizeof(buf), "EXP:  %d / %d", player->stats.exp, player->stats.exp_to_next);
    DrawText(buf, rx, ry, 16, SYS_TEXT_PRIMARY);
    /* EXP bar */
    float xp_ratio = (float)player->stats.exp / player->stats.exp_to_next;
    DrawRectangle(rx, ry + 20, 200, 10, (Color){ 50, 50, 50, 255 });
    DrawRectangle(rx, ry + 20, (int)(200 * xp_ratio), 10, SYS_TEXT_SECONDARY);
    DrawRectangleLines(rx, ry + 20, 200, 10, SYS_PANEL_BORDER);
    ry += 40;
    
    /* v2.0 RDR2 Cores & Skyrim Skills */
    DrawText("■ SURVIVAL & SKILLS", rx, ry, 20, SYS_TEXT_SECONDARY); ry += 30;
    
    snprintf(buf, sizeof(buf), "Hunger:  %.0f%%", player->stats.hunger);
    DrawText(buf, rx, ry, 14, (Color){200, 150, 50, 255}); ry += 20;
    snprintf(buf, sizeof(buf), "Thirst:  %.0f%%", player->stats.thirst);
    DrawText(buf, rx, ry, 14, (Color){50, 150, 255, 255}); ry += 20;
    snprintf(buf, sizeof(buf), "Fatigue: %.0f%%", player->stats.fatigue);
    DrawText(buf, rx, ry, 14, (Color){150, 200, 50, 255}); ry += 25;
    
    snprintf(buf, sizeof(buf), "Sword Mastery: Lv. %.1f", player->stats.sword_mastery);
    DrawText(buf, rx, ry, 14, SYS_TEXT_PRIMARY); ry += 20;
    snprintf(buf, sizeof(buf), "Fist Mastery:  Lv. %.1f", player->stats.fist_mastery);
    DrawText(buf, rx, ry, 14, SYS_TEXT_PRIMARY); ry += 20;
    snprintf(buf, sizeof(buf), "Athletics:     Lv. %.1f", player->stats.athletics);
    DrawText(buf, rx, ry, 14, SYS_TEXT_PRIMARY); ry += 30;
    
    DrawText("■ ACTIVE BUFFS", rx, ry, 20, SYS_TEXT_SECONDARY); ry += 30;
    int buff_count = 0;
    for (int i = 0; i < MAX_BUFFS; i++) {
        if (player->buffs[i].active) {
            snprintf(buf, sizeof(buf), "Buff %d (%.0fs)", player->buffs[i].type, player->buffs[i].remaining);
            DrawText(buf, rx, ry, 16, SYS_SUCCESS);
            ry += 20;
            buff_count++;
        }
    }
    if (buff_count == 0) {
        DrawText("None", rx, ry, 16, (Color){150, 150, 150, 255});
    }
    
    /* Footer */
    DrawText("[ ESC ] Close", px + pw / 2 - 40, py + ph - 30, 14, SYS_TEXT_SECONDARY);
}
