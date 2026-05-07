/*
 * cultivation.c — Cultivation / Qi progression system
 * Heavenly Demon: Murim Chronicles
 *
 * Realms:
 *   Mortal → Qi Gathering → Foundation → Core Formation
 *   → Nascent Soul → Spirit Severing → Transcendence
 *
 * Each realm has 9 sub-levels. Breaking through to a new realm
 * requires accumulating enough qi and sitting in meditation.
 */
#include "cultivation.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include <math.h>
#include <stdio.h>

/* ─── Realm names ─────────────────────────────────────── */
static const char *REALM_NAMES[] = {
    "Mortal",
    "Qi Gathering",
    "Foundation Establishment",
    "Core Formation",
    "Nascent Soul",
    "Spirit Severing",
    "Transcendence"
};

/* ─── Base stats per realm ────────────────────────────── */
typedef struct {
    int hp_base;
    int qi_base;
    int attack_base;
    int defense_base;
    int speed_base;
    int qi_regen_base;
} RealmStats;

static const RealmStats REALM_BASE_STATS[] = {
    { 100,  20,  8,  3, 100,  1 },  /* Mortal */
    { 150,  60, 15,  6, 110,  3 },  /* Qi Gathering */
    { 250, 120, 25, 12, 120,  5 },  /* Foundation */
    { 400, 200, 40, 20, 135,  8 },  /* Core Formation */
    { 600, 350, 65, 35, 150, 12 },  /* Nascent Soul */
    { 900, 500, 100, 55, 170, 18 }, /* Spirit Severing */
    { 1500, 800, 160, 90, 200, 30 }, /* Transcendence */
};

void cultivation_init(Entity *entity, CultivationRealm starting_realm)
{
    entity->cultivation.realm = starting_realm;
    entity->cultivation.sub_level = 1;
    entity->cultivation.progress = 0.0f;
    entity->cultivation.qi_absorbed = 0.0f;
    entity->cultivation.is_cultivating = false;
    entity->cultivation.cultivation_timer = 0.0f;

    cultivation_apply_bonuses(entity);
}

float cultivation_qi_required(CultivationRealm realm, int sub_level)
{
    /* Exponential scaling */
    float base = 50.0f * powf(2.5f, (float)realm);
    return base * (1.0f + sub_level * 0.3f);
}

void cultivation_apply_bonuses(Entity *entity)
{
    CultivationRealm r = entity->cultivation.realm;
    int sl = entity->cultivation.sub_level;

    if (r >= REALM_COUNT) r = REALM_COUNT - 1;

    const RealmStats *base = &REALM_BASE_STATS[r];
    float sub_mult = 1.0f + (sl - 1) * 0.08f; /* 8% per sub-level */

    entity->stats.max_hp   = (int)(base->hp_base * sub_mult);
    entity->stats.max_qi   = (int)(base->qi_base * sub_mult);
    entity->stats.attack   = (int)(base->attack_base * sub_mult);
    entity->stats.defense  = (int)(base->defense_base * sub_mult);
    entity->stats.speed    = (int)(base->speed_base * sub_mult);
    entity->stats.qi_regen = (int)(base->qi_regen_base * sub_mult);

    /* Keep current HP/QI if they're valid */
    if (entity->stats.hp > entity->stats.max_hp)
        entity->stats.hp = entity->stats.max_hp;
    if (entity->stats.qi > entity->stats.max_qi)
        entity->stats.qi = entity->stats.max_qi;
}

bool cultivation_check_breakthrough(Game *game, Entity *entity)
{
    CultivationState *cult = &entity->cultivation;
    float required = cultivation_qi_required(cult->realm, cult->sub_level);

    if (cult->qi_absorbed >= required) {
        cult->qi_absorbed -= required;
        cult->sub_level++;

        if (cult->sub_level > 9) {
            /* Realm breakthrough! */
            if (cult->realm < REALM_TRANSCENDENCE) {
                cult->realm++;
                cult->sub_level = 1;

                /* Major breakthrough effects */
                particle_burst(game, entity->pos,
                    (Color){255, 255, 255, 255}, 40, 200.0f, 1.0f, 6.0f);
                camera_shake(game, 15.0f, 0.5f);

                /* Full heal on breakthrough */
                cultivation_apply_bonuses(entity);
                entity->stats.hp = entity->stats.max_hp;
                entity->stats.qi = entity->stats.max_qi;
            } else {
                cult->sub_level = 9; /* Max */
            }
        } else {
            /* Sub-level up */
            particle_burst(game, entity->pos,
                (Color){200, 230, 255, 200}, 15, 100.0f, 0.5f, 3.0f);

            cultivation_apply_bonuses(entity);
            entity->stats.hp = entity->stats.max_hp;
            entity->stats.qi = entity->stats.max_qi;
        }

        return true;
    }

    return false;
}

void cultivation_meditate(Game *game, float dt)
{
    Entity *player = &game->entities[game->player_id];
    CultivationState *cult = &player->cultivation;

    if (!cult->is_cultivating) return;

    cult->cultivation_timer += dt;

    /* Absorb qi over time */
    float absorb_rate = 10.0f + cult->realm * 5.0f + cult->sub_level * 2.0f;
    cult->qi_absorbed += absorb_rate * dt;
    cult->progress = cult->qi_absorbed / cultivation_qi_required(cult->realm, cult->sub_level);
    if (cult->progress > 1.0f) cult->progress = 1.0f;

    /* Heal slowly while meditating */
    if (player->stats.hp < player->stats.max_hp) {
        player->stats.hp += (int)(5.0f * dt);
        if (player->stats.hp > player->stats.max_hp)
            player->stats.hp = player->stats.max_hp;
    }

    /* Restore qi while meditating */
    if (player->stats.qi < player->stats.max_qi) {
        player->stats.qi += (int)(player->stats.qi_regen * 3 * dt);
        if (player->stats.qi > player->stats.max_qi)
            player->stats.qi = player->stats.max_qi;
    }

    /* Qi particles during meditation */
    if ((int)(cult->cultivation_timer * 5) % 2 == 0) {
        particle_qi_effect(game, player->pos, cult->realm);
    }

    /* Check for breakthrough */
    cultivation_check_breakthrough(game, player);
}

const char *cultivation_realm_name(CultivationRealm realm)
{
    if (realm >= 0 && realm < REALM_COUNT)
        return REALM_NAMES[realm];
    return "Unknown";
}

void cultivation_update(Game *game, float dt)
{
    cultivation_meditate(game, dt);
}
