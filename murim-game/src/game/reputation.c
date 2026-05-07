/*
 * reputation.c — Faction Reputation & NPC Schedules (RDR2)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * 6 factions track your reputation (-100 to +100).
 * Below -50 = hostile. Above 75 = allied.
 * NPCs follow daily schedules based on in-game time of day.
 */
#include "reputation.h"
#include "npc.h"
#include "../ui/system_ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ─── Faction default reputations ─────────────────────── */
static const int DEFAULT_REP[] = { 0, 20, -20, 10, 0, 0, 40 };
static const Color FACTION_COLORS[] = {
    {150,150,150,255}, {255,215,0,255},   {180,30,30,255},
    {200,150,50,255},  {80,200,80,255},   {100,80,200,255}, {150,220,150,255}
};

static float s_notify_timer = 0; /* Throttle reputation change notifications */

void reputation_init(Game *game)
{
    for (int i = 0; i < MAX_FACTIONS; i++) {
        FactionState *f = &game->factions[i];
        f->id         = (FactionID)i;
        f->reputation = (i < 7) ? DEFAULT_REP[i] : 0;
        f->is_hostile = f->reputation < -50;
        f->is_allied  = f->reputation > 75;
        f->color      = (i < 7) ? FACTION_COLORS[i] : WHITE;
        strncpy(f->name, faction_name_str((FactionID)i), sizeof(f->name)-1);
    }
}

void reputation_modify(Game *game, FactionID faction, int amount)
{
    if (faction <= FACTION_NONE || faction >= FACTION_COUNT) return;
    FactionState *f = &game->factions[(int)faction];
    int old_rep = f->reputation;
    f->reputation += amount;
    if (f->reputation < -100) f->reputation = -100;
    if (f->reputation >  100) f->reputation = 100;

    bool was_hostile = f->is_hostile;
    bool was_allied  = f->is_allied;
    f->is_hostile = f->reputation < -50;
    f->is_allied  = f->reputation > 75;

    /* Notify on threshold crossings */
    if (!was_hostile && f->is_hostile) {
        char buf[64]; snprintf(buf, sizeof(buf), "%s now considers you an enemy!", f->name);
        system_notify(game, NOTIFY_WARNING, "[ Faction ]", buf);
    } else if (was_hostile && !f->is_hostile) {
        char buf[64]; snprintf(buf, sizeof(buf), "%s is no longer hostile.", f->name);
        system_notify(game, NOTIFY_SUCCESS, "[ Faction ]", buf);
    } else if (!was_allied && f->is_allied) {
        char buf[64]; snprintf(buf, sizeof(buf), "%s now considers you an ally!", f->name);
        system_notify(game, NOTIFY_SUCCESS, "[ Faction ]", buf);
    }

    /* Throttled rep change notify */
    if (s_notify_timer <= 0 && (f->reputation - old_rep != 0)) {
        char buf[64]; snprintf(buf, sizeof(buf), "%s: %+d rep (%d)", f->name, amount, f->reputation);
        system_notify(game, amount>0?NOTIFY_INFO:NOTIFY_WARNING, "[ Reputation ]", buf);
        s_notify_timer = 4.0f;
    }
}

int reputation_get(const Game *game, FactionID faction)
{
    if (faction <= FACTION_NONE || faction >= FACTION_COUNT) return 0;
    return game->factions[(int)faction].reputation;
}

bool reputation_is_hostile(const Game *game, FactionID faction)
{
    if (faction <= FACTION_NONE || faction >= FACTION_COUNT) return false;
    return game->factions[(int)faction].is_hostile;
}

void reputation_update(Game *game, float dt)
{
    s_notify_timer -= dt;
    if (s_notify_timer < 0) s_notify_timer = 0;

    float hour = game->world.day_time;

    /* ── NPC Schedule Update ─────────────────────────── */
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (!e->active || i == game->player_id) continue;
        if (e->type != ENTITY_NPC_FRIENDLY && e->type != ENTITY_NPC_MERCHANT
            && e->type != ENTITY_NPC_ELDER) continue;

        /* Morning: head to work area (wander near home) */
        if (hour >= 6.0f && hour < 8.0f) {
            if (e->ai_state == AI_IDLE) e->ai_state = AI_WANDER;
        }
        /* Daytime: normal wander / patrol */
        else if (hour >= 8.0f && hour < 18.0f) {
            if (e->ai_state != AI_WANDER && e->ai_state != AI_CHASE)
                e->ai_state = AI_WANDER;
        }
        /* Evening: return home */
        else if (hour >= 18.0f && hour < 20.0f) {
            e->ai_state = AI_RETURN;
            e->ai_target = e->ai_home;
        }
        /* Night: sleep at home */
        else if (hour >= 20.0f || hour < 6.0f) {
            if (e->ai_state != AI_IDLE) {
                /* Snap to home */
                float dx = e->ai_home.x - e->pos.x;
                float dy = e->ai_home.y - e->pos.y;
                if (sqrtf(dx*dx+dy*dy) < 30.0f)
                    e->ai_state = AI_IDLE;
                else
                    e->ai_state = AI_RETURN;
            }
        }

        /* Faction hostility: force hostile NPCs matching hostile faction to attack */
        if (e->faction != FACTION_NONE && reputation_is_hostile(game, e->faction)) {
            e->ai_state = AI_CHASE;
        }
    }

    /* Slowly recover reputation toward neutral over very long time */
    static float rep_decay_timer = 0;
    rep_decay_timer += dt;
    if (rep_decay_timer >= 120.0f) { /* Every 2 real minutes */
        rep_decay_timer = 0;
        for (int f = 1; f < MAX_FACTIONS && f < FACTION_COUNT; f++) {
            FactionState *fs = &game->factions[f];
            if (fs->reputation < -10) reputation_modify(game, (FactionID)f, 1);
            else if (fs->reputation > 10) reputation_modify(game, (FactionID)f, -1);
        }
    }
}

void reputation_draw_hud(const Game *game)
{
    /* Draw faction standings in STATUS window (compact) */
    int x = 20, y = SCREEN_HEIGHT - 220;
    DrawText("Factions:", x, y, 11, (Color){160,160,160,180});
    y += 14;
    for (int i = 1; i < MAX_FACTIONS && i < FACTION_COUNT; i++) {
        const FactionState *f = &game->factions[i];
        if (f->reputation == 0) continue; /* Skip neutral */
        char buf[48]; snprintf(buf, sizeof(buf), "%-14s %+d", f->name, f->reputation);
        Color c = f->is_hostile ? (Color){255,80,80,200} :
                  f->is_allied  ? (Color){80,255,120,200} :
                                  (Color){180,180,180,160};
        DrawText(buf, x, y, 9, c);
        y += 12;
        if (y > SCREEN_HEIGHT - 40) break;
    }
}
