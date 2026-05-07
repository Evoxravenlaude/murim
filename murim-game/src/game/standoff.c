/*
 * standoff.c — Cinematic 1vN Standoff Duels (Ghost of Tsushima)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Press G near enemies to initiate. A timing circle closes in—
 * hit SPACE in the window for an instant kill chain.
 */
#include "standoff.h"
#include "combat.h"
#include "../ui/system_ui.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* ─── Static standoff state ───────────────────────────── */
static Standoff s_sd;
static float    s_ring_progress = 0; /* 0→1, strike when between 0.4-0.7 */
static float    s_ring_speed    = 0.35f;
static bool     s_strike_window = false;

static int find_nearest_hostile(const Game *game)
{
    const Entity *player = &game->entities[game->player_id];
    float best = 9999; int best_id = -1;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (!e->active || i == game->player_id) continue;
        if (e->type != ENTITY_NPC_HOSTILE && e->type != ENTITY_BEAST) continue;
        float dx = e->pos.x-player->pos.x, dy = e->pos.y-player->pos.y;
        float d = sqrtf(dx*dx+dy*dy);
        if (d < best && d < 200.0f) { best=d; best_id=i; }
    }
    return best_id;
}

void standoff_initiate(Game *game)
{
    int tid = find_nearest_hostile(game);
    if (tid < 0) {
        system_notify(game, NOTIFY_WARNING, "[ Standoff ]", "No opponent nearby.");
        return;
    }
    s_sd.active     = true;
    s_sd.state      = STANDOFF_APPROACH;
    s_sd.target_id  = tid;
    s_sd.timer      = 2.0f;   /* approach phase */
    s_sd.window     = 0.3f;
    s_sd.kills      = 0;
    s_sd.max_chain  = 1 + (int)game->entities[game->player_id].cultivation.realm / 2;
    s_ring_progress = 0;
    s_ring_speed    = 0.25f + (float)game->entities[tid].cultivation.realm * 0.04f;
    s_strike_window = false;
    game->state     = STATE_STANDOFF;
    camera_shake(game, 3.0f, 0.2f);
}

void standoff_player_strike(Game *game)
{
    if (!s_sd.active || s_sd.state != STANDOFF_READY) return;
    /* Check if ring is in valid window (0.38 – 0.68) */
    if (s_ring_progress >= 0.38f && s_ring_progress <= 0.68f) {
        s_sd.state = STANDOFF_SUCCESS;
        s_sd.timer = 1.2f;
    } else {
        s_sd.state = STANDOFF_FAIL;
        s_sd.timer = 0.8f;
    }
}

void standoff_update(Game *game, float dt)
{
    if (!s_sd.active) return;
    Entity *player = &game->entities[game->player_id];
    s_sd.timer -= dt;

    switch (s_sd.state) {
    case STANDOFF_APPROACH:
        if (s_sd.timer <= 0) {
            s_sd.state  = STANDOFF_READY;
            s_sd.timer  = 4.0f;
            s_ring_progress = 0;
        }
        break;

    case STANDOFF_READY:
        s_ring_progress += s_ring_speed * dt;
        if (s_ring_progress >= 0.38f && s_ring_progress <= 0.68f) s_strike_window = true;
        else s_strike_window = false;

        if (s_ring_progress > 1.0f) {
            /* Missed: enemy strikes */
            s_sd.state = STANDOFF_FAIL;
            s_sd.timer = 0.8f;
        }
        if (s_sd.timer <= 0) {
            s_sd.state = STANDOFF_FAIL;
            s_sd.timer = 0.8f;
        }
        break;

    case STANDOFF_SUCCESS: {
        /* Instant-kill target */
        if (s_sd.target_id >= 0 && s_sd.target_id < MAX_ENTITIES) {
            Entity *t = &game->entities[s_sd.target_id];
            if (t->active) {
                particle_burst(game, t->pos, (Color){255,220,100,255}, 25, 150.0f, 0.8f, 5.0f);
                t->active = false;
                player->experience += 20 + t->stats.max_hp / 3;
                s_sd.kills++;
                system_notify(game, NOTIFY_SUCCESS, "[ Standoff ]", "Perfect strike!");
                camera_shake(game, 10.0f, 0.3f);
            }
        }
        if (s_sd.timer <= 0) {
            /* Chain to next enemy? */
            if (s_sd.kills < s_sd.max_chain) {
                int next = find_nearest_hostile(game);
                if (next >= 0) {
                    s_sd.target_id  = next;
                    s_sd.state      = STANDOFF_READY;
                    s_sd.timer      = 4.0f;
                    s_ring_progress = 0;
                    s_strike_window = false;
                    break;
                }
            }
            /* End standoff */
            s_sd.active = false;
            game->state = STATE_PLAYING;
        }
        break;
    }

    case STANDOFF_FAIL: {
        if (s_sd.timer <= 0 || s_sd.timer > 0) {
            /* Enemy hits player once */
            if (s_sd.target_id >= 0) {
                Entity *t = &game->entities[s_sd.target_id];
                if (t->active) {
                    player->stats.hp -= t->stats.attack;
                    particle_burst(game, player->pos, (Color){255,50,50,255}, 15, 100.0f, 0.5f, 3.0f);
                    camera_shake(game, 12.0f, 0.4f);
                }
            }
            system_notify(game, NOTIFY_WARNING, "[ Standoff ]", "You were hit!");
            s_sd.active = false;
            game->state = STATE_PLAYING;
        }
        break;
    }
    default: s_sd.active=false; game->state=STATE_PLAYING; break;
    }
}

void standoff_draw(const Game *game)
{
    if (!s_sd.active) return;
    float t = (float)GetTime();

    /* Dark cinematic bars */
    DrawRectangle(0, 0, SCREEN_WIDTH, 80, (Color){0,0,0,200});
    DrawRectangle(0, SCREEN_HEIGHT-80, SCREEN_WIDTH, 80, (Color){0,0,0,200});

    if (s_sd.state == STANDOFF_APPROACH) {
        const char *msg = "— STANDOFF —";
        float pulse = sinf(t*4.0f)*0.3f+0.7f;
        int tw = MeasureText(msg, 28);
        DrawText(msg, SCREEN_WIDTH/2-tw/2, SCREEN_HEIGHT/2-14, 28,
                 (Color){255,215,0,(unsigned char)(255*pulse)});
        DrawText("Wait for the moment...", SCREEN_WIDTH/2-90, SCREEN_HEIGHT/2+24, 14,
                 (Color){200,190,160,200});
        return;
    }

    if (s_sd.state == STANDOFF_READY) {
        /* Closing ring mechanic */
        int cx = SCREEN_WIDTH/2, cy = SCREEN_HEIGHT/2;
        float outer_r = 80.0f;
        float inner_r = outer_r * (1.0f - s_ring_progress); /* ring closes in */

        /* Outer fixed ring */
        DrawCircleLines(cx, cy, outer_r, (Color){200,200,200,120});

        /* Closing ring */
        Color rc = s_strike_window ?
                   (Color){100,255,100,255} : (Color){255,200,50,220};
        DrawCircleLines(cx, cy, inner_r, rc);
        DrawCircleLines(cx, cy, inner_r+2, rc);

        /* Window indicator marks */
        DrawText("├", cx+(int)(outer_r*0.38f)-4, cy-8, 16, (Color){100,255,100,200});
        DrawText("┤", cx+(int)(outer_r*0.68f)-4, cy-8, 16, (Color){100,255,100,200});

        /* Center button prompt */
        float pulse = sinf(t*8.0f)*0.4f+0.6f;
        const char *prompt = s_strike_window ? "[ SPACE ] NOW!" : "[ SPACE ]";
        int tw = MeasureText(prompt, 18);
        DrawText(prompt, cx-tw/2, cy-9, 18,
                 (Color){255,255,255,(unsigned char)(255*pulse)});

        /* Kills counter */
        if (s_sd.max_chain > 1) {
            char buf[32]; snprintf(buf, sizeof(buf), "Chain: %d / %d", s_sd.kills+1, s_sd.max_chain);
            DrawText(buf, cx-50, cy+55, 14, (Color){255,215,0,220});
        }
    }

    if (s_sd.state == STANDOFF_SUCCESS) {
        const char *msg = "PERFECT!";
        int tw = MeasureText(msg, 36);
        float alpha = s_sd.timer / 1.2f;
        DrawText(msg, SCREEN_WIDTH/2-tw/2, SCREEN_HEIGHT/2-18, 36,
                 (Color){255,220,50,(unsigned char)(255*alpha)});
    }

    if (s_sd.state == STANDOFF_FAIL) {
        const char *msg = "TOO SLOW!";
        int tw = MeasureText(msg, 32);
        DrawText(msg, SCREEN_WIDTH/2-tw/2, SCREEN_HEIGHT/2-16, 32,
                 (Color){255,50,50,200});
    }

    (void)game;
}

bool standoff_is_active(const Game *game)
{
    (void)game;
    return s_sd.active;
}
