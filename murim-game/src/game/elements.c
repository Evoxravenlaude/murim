/*
 * elements.c — Elemental Affinity & Reaction System (Genshin Impact)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * 7 elements with 10+ reaction combos.
 * Elements are applied via attacks and decay over time.
 * When two different elements meet on a target, a reaction triggers.
 */
#include "elements.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include <math.h>
#include <stdio.h>

void elements_init_entity(Entity *e, ElementType affinity)
{
    e->elemental.affinity = affinity;
    e->elemental.applied = ELEMENT_NONE;
    e->elemental.applied_timer = 0;
    e->elemental.reaction_cd = 0;
    e->elemental.reaction_dmg = 0;
    e->elemental.last_reaction = REACTION_NONE;
}

ReactionType elements_check_reaction(ElementType a, ElementType b)
{
    if (a == ELEMENT_NONE || b == ELEMENT_NONE) return REACTION_NONE;
    if (a == b) return REACTION_NONE;

    /* Fire reactions */
    if ((a == ELEMENT_FIRE && b == ELEMENT_WATER) ||
        (a == ELEMENT_WATER && b == ELEMENT_FIRE))
        return REACTION_VAPORIZE;

    if ((a == ELEMENT_FIRE && b == ELEMENT_ICE) ||
        (a == ELEMENT_ICE && b == ELEMENT_FIRE))
        return REACTION_MELT;

    if ((a == ELEMENT_FIRE && b == ELEMENT_LIGHTNING) ||
        (a == ELEMENT_LIGHTNING && b == ELEMENT_FIRE))
        return REACTION_OVERLOAD;

    /* Ice reactions */
    if ((a == ELEMENT_ICE && b == ELEMENT_LIGHTNING) ||
        (a == ELEMENT_LIGHTNING && b == ELEMENT_ICE))
        return REACTION_SUPERCONDUCT;

    if ((a == ELEMENT_WATER && b == ELEMENT_ICE) ||
        (a == ELEMENT_ICE && b == ELEMENT_WATER))
        return REACTION_FROZEN;

    /* Water + Lightning */
    if ((a == ELEMENT_WATER && b == ELEMENT_LIGHTNING) ||
        (a == ELEMENT_LIGHTNING && b == ELEMENT_WATER))
        return REACTION_ELECTROCHARGE;

    /* Wind swirls with anything */
    if (a == ELEMENT_WIND || b == ELEMENT_WIND)
        return REACTION_SWIRL;

    /* Earth crystallizes with anything */
    if (a == ELEMENT_EARTH || b == ELEMENT_EARTH)
        return REACTION_CRYSTALLIZE;

    /* Shadow combos */
    if ((a == ELEMENT_SHADOW && b == ELEMENT_FIRE) ||
        (a == ELEMENT_FIRE && b == ELEMENT_SHADOW))
        return REACTION_SHADOWBURN;

    if ((a == ELEMENT_SHADOW && b == ELEMENT_ICE) ||
        (a == ELEMENT_ICE && b == ELEMENT_SHADOW))
        return REACTION_SHADOWFREEZE;

    return REACTION_NONE;
}

float elements_reaction_multiplier(ReactionType r)
{
    switch (r) {
    case REACTION_VAPORIZE:      return 2.0f;
    case REACTION_MELT:          return 1.5f;
    case REACTION_OVERLOAD:      return 1.8f;
    case REACTION_SUPERCONDUCT:  return 1.3f;
    case REACTION_ELECTROCHARGE: return 1.4f;
    case REACTION_FROZEN:        return 1.0f; /* No extra dmg, but freeze */
    case REACTION_SWIRL:         return 1.2f;
    case REACTION_CRYSTALLIZE:   return 0.8f; /* Less dmg, but shield */
    case REACTION_SHADOWBURN:    return 1.6f;
    case REACTION_SHADOWFREEZE:  return 1.4f;
    default: return 1.0f;
    }
}

void elements_apply(Game *game, Entity *target, ElementType element, int base_damage)
{
    if (element == ELEMENT_NONE) return;
    if (target->elemental.reaction_cd > 0) {
        /* Just apply the element, no reaction yet */
        target->elemental.applied = element;
        target->elemental.applied_timer = 8.0f;
        return;
    }

    /* Check for reaction with existing applied element */
    ReactionType reaction = elements_check_reaction(target->elemental.applied, element);

    if (reaction != REACTION_NONE) {
        float mult = elements_reaction_multiplier(reaction);
        int reaction_dmg = (int)(base_damage * mult);

        target->stats.hp -= reaction_dmg;
        target->elemental.reaction_dmg = reaction_dmg;
        target->elemental.last_reaction = reaction;
        target->elemental.reaction_cd = 2.0f; /* 2 second ICD */
        target->invincible_timer = 0.3f;

        /* Visual feedback */
        Color rc = element_color(element);
        particle_burst(game, target->pos, rc, 25, 150.0f, 0.8f, 5.0f);
        camera_shake(game, 8.0f + reaction_dmg * 0.1f, 0.3f);

        /* Special reaction effects */
        switch (reaction) {
        case REACTION_FROZEN:
            /* Freeze: stop movement */
            target->vel.x = 0;
            target->vel.y = 0;
            target->invincible_timer = 0; /* Can still be hit */
            /* Apply freeze debuff */
            for (int i = 0; i < MAX_BUFFS; i++) {
                if (!target->buffs[i].active) {
                    target->buffs[i].active = true;
                    target->buffs[i].type = DEBUFF_FREEZE;
                    target->buffs[i].duration = 3.0f;
                    target->buffs[i].remaining = 3.0f;
                    break;
                }
            }
            break;
        case REACTION_OVERLOAD:
            /* AoE explosion — damage nearby enemies too */
            for (int i = 0; i < MAX_ENTITIES; i++) {
                Entity *e = &game->entities[i];
                if (!e->active || e == target) continue;
                float dx = e->pos.x - target->pos.x;
                float dy = e->pos.y - target->pos.y;
                if (dx*dx + dy*dy < 80.0f*80.0f) {
                    e->stats.hp -= reaction_dmg / 2;
                    e->invincible_timer = 0.2f;
                }
            }
            particle_burst(game, target->pos, (Color){255,150,50,255}, 30, 200.0f, 1.0f, 6.0f);
            break;
        case REACTION_SUPERCONDUCT:
            /* DEF shred */
            target->stats.defense = (int)(target->stats.defense * 0.6f);
            break;
        default:
            break;
        }

        /* Clear applied element after reaction */
        target->elemental.applied = ELEMENT_NONE;
        target->elemental.applied_timer = 0;
    } else {
        /* No reaction — just apply element */
        target->elemental.applied = element;
        target->elemental.applied_timer = 8.0f;
    }
}

void elements_update(Game *game, float dt)
{
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (!e->active) continue;

        /* Decay applied element */
        if (e->elemental.applied_timer > 0) {
            e->elemental.applied_timer -= dt;
            if (e->elemental.applied_timer <= 0) {
                e->elemental.applied = ELEMENT_NONE;
            }
        }

        /* Reaction cooldown */
        if (e->elemental.reaction_cd > 0) {
            e->elemental.reaction_cd -= dt;
        }

        /* Frozen check — immobilize */
        for (int b = 0; b < MAX_BUFFS; b++) {
            if (e->buffs[b].active && e->buffs[b].type == DEBUFF_FREEZE) {
                e->vel.x = 0;
                e->vel.y = 0;
            }
        }
    }
}

void elements_draw_aura(const Entity *e, float time)
{
    if (e->elemental.applied == ELEMENT_NONE) return;

    Color c = element_color(e->elemental.applied);
    float pulse = sinf(time * 6.0f) * 0.3f + 0.7f;
    c.a = (unsigned char)(120 * pulse);

    DrawCircle((int)e->pos.x, (int)e->pos.y, 18.0f * pulse, c);

    /* Small orbiting element particles */
    for (int i = 0; i < 3; i++) {
        float angle = time * 3.0f + i * 2.094f;
        float ox = cosf(angle) * 14.0f;
        float oy = sinf(angle) * 14.0f;
        Color pc = c;
        pc.a = (unsigned char)(200 * pulse);
        DrawCircle((int)(e->pos.x + ox), (int)(e->pos.y + oy), 2.5f, pc);
    }
}
