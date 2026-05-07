/*
 * combat.c — Real-time martial arts combat system
 * Heavenly Demon: Murim Chronicles
 */
#include "combat.h"
#include "bestiary.h"
#include "bounty.h"
#include "elements.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include "../ui/system_ui.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* ─── Technique definitions ───────────────────────────── */
static const Technique TECHNIQUE_DEFS[] = {
    [TECH_BASIC_PUNCH] = {
        TECH_BASIC_PUNCH, "Basic Fist", 0, 5, 0.3f, 0, REALM_MORTAL, 30.0f
    },
    [TECH_BASIC_SLASH] = {
        TECH_BASIC_SLASH, "Sword Slash", 0, 8, 0.4f, 0, REALM_MORTAL, 35.0f
    },
    [TECH_QI_BLAST] = {
        TECH_QI_BLAST, "Qi Blast", 15, 20, 1.0f, 0, REALM_QI_GATHERING, 60.0f
    },
    [TECH_HEAVENLY_STRIKE] = {
        TECH_HEAVENLY_STRIKE, "Heavenly Strike", 30, 45, 1.5f, 0, REALM_FOUNDATION, 40.0f
    },
    [TECH_SHADOW_STEP] = {
        TECH_SHADOW_STEP, "Shadow Step", 10, 0, 2.0f, 0, REALM_QI_GATHERING, 0.0f
    },
    [TECH_IRON_BODY] = {
        TECH_IRON_BODY, "Iron Body", 20, 0, 5.0f, 0, REALM_FOUNDATION, 0.0f
    },
    [TECH_DRAGON_FIST] = {
        TECH_DRAGON_FIST, "Dragon Fist", 50, 80, 2.0f, 0, REALM_CORE_FORMATION, 45.0f
    },
    [TECH_SWORD_RAIN] = {
        TECH_SWORD_RAIN, "Sword Rain", 60, 60, 3.0f, 0, REALM_NASCENT_SOUL, 80.0f
    },
};

void combat_init_player_techniques(Entity *player)
{
    player->techniques[0] = TECHNIQUE_DEFS[TECH_BASIC_SLASH];
    player->techniques[1] = TECHNIQUE_DEFS[TECH_QI_BLAST];
    player->techniques[2] = TECHNIQUE_DEFS[TECH_HEAVENLY_STRIKE];
    player->techniques[3] = TECHNIQUE_DEFS[TECH_DRAGON_FIST];
    player->techniques[4] = TECHNIQUE_DEFS[TECH_SWORD_RAIN];
    player->num_techniques = 5;
    player->active_technique = 0;

    /* Init poise */
    player->poise.max_poise    = 80.0f;
    player->poise.poise        = 80.0f;
    player->poise.poise_regen  = 8.0f;
    player->poise.is_staggered = false;
    player->poise.stagger_timer = 0;
}

void combat_use_item(Game *game)
{
    Entity *player = &game->entities[game->player_id];
    /* Find first usable item */
    for (int i = 0; i < MAX_ITEMS; i++) {
        InventorySlot *slot = &player->inventory[i];
        if (slot->type == ITEM_NONE || slot->quantity <= 0) continue;
        bool used = false;
        switch (slot->type) {
        case ITEM_HEALING_PILL:
            player->stats.hp += 50;
            if (player->stats.hp > player->stats.max_hp) player->stats.hp = player->stats.max_hp;
            system_notify(game, NOTIFY_SUCCESS, "[ Item ]", "Healing Elixir: +50 HP");
            used = true; break;
        case ITEM_QI_PILL:
            player->stats.qi += 40;
            if (player->stats.qi > player->stats.max_qi) player->stats.qi = player->stats.max_qi;
            system_notify(game, NOTIFY_INFO, "[ Item ]", "Qi Restoration: +40 QI");
            used = true; break;
        case ITEM_BUFF_POTION_ATK:
            for (int b = 0; b < MAX_BUFFS; b++) {
                if (!player->buffs[b].active) {
                    player->buffs[b].active = true;
                    player->buffs[b].type = BUFF_ATK_UP;
                    player->buffs[b].duration = 60.0f;
                    player->buffs[b].remaining = 60.0f;
                    player->buffs[b].magnitude = (int)(player->stats.attack * 0.3f);
                    player->stats.attack += player->buffs[b].magnitude;
                    break;
                }
            }
            system_notify(game, NOTIFY_SUCCESS, "[ Item ]", "Beast-Bane Oil: +30% ATK!");
            used = true; break;
        case ITEM_BUFF_POTION_DEF:
            for (int b = 0; b < MAX_BUFFS; b++) {
                if (!player->buffs[b].active) {
                    player->buffs[b].active = true;
                    player->buffs[b].type = BUFF_DEF_UP;
                    player->buffs[b].duration = 45.0f;
                    player->buffs[b].remaining = 45.0f;
                    player->buffs[b].magnitude = (int)(player->stats.defense * 0.5f);
                    player->stats.defense += player->buffs[b].magnitude;
                    break;
                }
            }
            system_notify(game, NOTIFY_SUCCESS, "[ Item ]", "Iron Skin Elixir: +50% DEF!");
            used = true; break;
        default: break;
        }
        if (used) {
            slot->quantity--;
            if (slot->quantity <= 0) { slot->type = ITEM_NONE; player->num_items--; }
            return;
        }
    }
    system_notify(game, NOTIFY_WARNING, "[ Item ]", "No usable items!");
}

bool combat_in_range(const Entity *a, const Entity *b, float range)
{
    float dx = b->pos.x - a->pos.x;
    float dy = b->pos.y - a->pos.y;
    return (dx * dx + dy * dy) <= (range * range);
}

/* Get directional offset for attack area */
static Vec2 get_attack_offset(Direction dir, float distance)
{
    Vec2 offset = {0, 0};
    switch (dir) {
    case DIR_UP:         offset.y = -distance; break;
    case DIR_DOWN:       offset.y = distance; break;
    case DIR_LEFT:       offset.x = -distance; break;
    case DIR_RIGHT:      offset.x = distance; break;
    case DIR_UP_LEFT:    offset.x = -distance * 0.7f; offset.y = -distance * 0.7f; break;
    case DIR_UP_RIGHT:   offset.x = distance * 0.7f; offset.y = -distance * 0.7f; break;
    case DIR_DOWN_LEFT:  offset.x = -distance * 0.7f; offset.y = distance * 0.7f; break;
    case DIR_DOWN_RIGHT: offset.x = distance * 0.7f; offset.y = distance * 0.7f; break;
    default: break;
    }
    return offset;
}

void combat_apply_damage(Game *game, Entity *attacker, Entity *defender)
{
    if (defender->invincible_timer > 0) return;

    /* Check if defender is staggered → Execution window */
    bool is_execution = (defender->poise.is_staggered && attacker->type == ENTITY_PLAYER);

    int damage = attacker->stats.attack - defender->stats.defense / 2;
    if (damage < 1) damage = 1;

    /* Execution multiplier (Elden Ring Heavenly Execution) */
    if (is_execution) { damage *= 3; }

    /* Random variance ±20% */
    float variance = 0.8f + (float)(rand() % 40) / 100.0f;
    damage = (int)(damage * variance);
    if (damage < 1) damage = 1;

    /* Critical hit chance */
    bool crit = (rand() % 100 < 5 + attacker->cultivation.realm * 3);
    if (crit && !is_execution) damage *= 2;

    /* Stance damage bonus */
    if (attacker->type == ENTITY_PLAYER && attacker->stance.current != STANCE_NEUTRAL) {
        damage = (int)(damage * (1.0f + attacker->stance.stance_bonus_dmg));
    }

    defender->stats.hp -= damage;
    defender->invincible_timer = is_execution ? 0.0f : 0.5f;
    defender->flash_timer = 0.3f;

    /* Poise damage on hit */
    if (!is_execution) {
        float poise_dmg = (float)attacker->stats.attack * 0.3f;
        defender->poise.poise -= poise_dmg;
        if (defender->poise.poise <= 0.0f && !defender->poise.is_staggered) {
            defender->poise.is_staggered = true;
            defender->poise.stagger_timer = 1.8f;
            defender->poise.poise = 0.0f;
            /* Stagger visual */
            particle_burst(game, defender->pos, (Color){255,200,100,200}, 10, 60.0f, 0.4f, 3.0f);
        }
    } else {
        /* Execution resets poise */
        defender->poise.is_staggered = false;
        defender->poise.poise = defender->poise.max_poise;
        particle_burst(game, defender->pos, (Color){255,220,50,255}, 30, 180.0f, 0.8f, 6.0f);
        camera_shake(game, 15.0f, 0.4f);
        system_notify(game, NOTIFY_SUCCESS, "[ Heavenly Execution ]", "Poise Broken!");
    }

    /* v3.0: Apply elemental reaction ON HIT (not just on kill) */
    if (attacker->elemental.affinity != ELEMENT_NONE) {
        elements_apply(game, defender, attacker->elemental.affinity, damage);
    }

    particle_hit_effect(game, defender->pos);
    if (!is_execution) camera_shake(game, 3.0f + damage * 0.2f, 0.15f);

    if (defender->stats.hp <= 0) {
        defender->stats.hp = 0;
        defender->active = false;

        if (attacker->type == ENTITY_PLAYER) {
            int xp = 10 + defender->stats.max_hp / 2;
            attacker->experience += xp;
            attacker->gold += rand() % 10 + 5;
            attacker->cultivation.qi_absorbed += 5.0f + defender->stats.max_hp * 0.1f;
            bestiary_log_kill(game, defender);
            bounty_on_kill(game, defender);
            attacker->kills++;
            game->total_kills++;

            /* Spawn loot drop */
            for (int i = 0; i < MAX_LOOT_DROPS; i++) {
                LootDrop *ld = &game->loot_drops[i];
                if (!ld->active) {
                    ld->active   = true;
                    ld->pos      = defender->pos;
                    ld->item     = (rand()%3==0) ? ITEM_HEALING_PILL : ITEM_SPIRIT_STONE;
                    ld->rarity   = (ItemRarity)(rand() % 3);
                    ld->quantity = 1 + rand() % 2;
                    ld->timer    = 30.0f;
                    ld->bob_timer = 0;
                    break;
                }
            }
        }

        Color death_color = defender->elemental.affinity != ELEMENT_NONE ?
                            element_color(defender->elemental.affinity) : defender->color;
        particle_burst(game, defender->pos, death_color, 20, 100.0f, 0.6f, 4.0f);
    }
}

void combat_player_attack(Game *game)
{
    Entity *player = &game->entities[game->player_id];
    if (player->is_attacking || player->attack_timer > 0) return;

    Technique *tech = &player->techniques[player->active_technique];
    if (tech->current_cooldown > 0) return;

    player->is_attacking = true;
    player->attack_timer = 0.3f;
    
    /* Skyrim progression */
    if (tech->type == TECH_BASIC_SLASH) {
        player->stats.sword_mastery += 0.05f;
    } else {
        player->stats.fist_mastery += 0.05f;
    }

    /* Check all enemies in range */
    Vec2 attack_pos = player->pos;
    Vec2 offset = get_attack_offset(player->dir, 20.0f);
    attack_pos.x += offset.x;
    attack_pos.y += offset.y;

    /* Attack particles */
    Color slash_color = { 200, 200, 255, 200 };
    particle_burst(game, attack_pos, slash_color, 5, 60.0f, 0.3f, 2.5f);

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (!e->active || i == game->player_id) continue;
        if (e->type != ENTITY_NPC_HOSTILE && e->type != ENTITY_BEAST) continue;

        if (combat_in_range(player, e, tech->range)) {
            /* Check facing direction */
            float dx = e->pos.x - player->pos.x;
            float dy = e->pos.y - player->pos.y;
            bool in_front = false;

            switch (player->dir) {
            case DIR_RIGHT: case DIR_DOWN_RIGHT: case DIR_UP_RIGHT:
                in_front = dx > 0; break;
            case DIR_LEFT: case DIR_DOWN_LEFT: case DIR_UP_LEFT:
                in_front = dx < 0; break;
            case DIR_UP:
                in_front = dy < 0; break;
            case DIR_DOWN:
                in_front = dy > 0; break;
            default:
                in_front = true; break;
            }

            if (in_front) {
                int orig_attack = player->stats.attack;
                player->stats.attack += tech->damage;
                combat_apply_damage(game, player, e);
                player->stats.attack = orig_attack;
            }
        }
    }
}

void combat_player_technique(Game *game)
{
    Entity *player = &game->entities[game->player_id];
    if (player->active_technique >= player->num_techniques) return;

    Technique *tech = &player->techniques[player->active_technique];
    if (tech->current_cooldown > 0) return;
    if (player->stats.qi < tech->qi_cost) return;
    if (tech->min_realm > player->cultivation.realm) return;

    player->stats.qi -= tech->qi_cost;
    tech->current_cooldown = tech->cooldown;

    /* Special technique effects */
    Vec2 attack_pos = player->pos;
    Vec2 offset = get_attack_offset(player->dir, 30.0f);
    attack_pos.x += offset.x;
    attack_pos.y += offset.y;

    switch (tech->type) {
    case TECH_QI_BLAST: {
        Color qi_color = { 120, 200, 255, 230 };
        particle_burst(game, attack_pos, qi_color, 15, 120.0f, 0.5f, 4.0f);
        camera_shake(game, 5.0f, 0.2f);

        /* Damage in area */
        for (int i = 0; i < MAX_ENTITIES; i++) {
            Entity *e = &game->entities[i];
            if (!e->active || i == game->player_id) continue;
            if (e->type != ENTITY_NPC_HOSTILE && e->type != ENTITY_BEAST) continue;
            if (combat_in_range(player, e, tech->range)) {
                int orig = player->stats.attack;
                player->stats.attack = tech->damage;
                combat_apply_damage(game, player, e);
                player->stats.attack = orig;
            }
        }
        break;
    }
    case TECH_HEAVENLY_STRIKE: {
        Color gold = { 255, 215, 0, 230 };
        particle_burst(game, attack_pos, gold, 20, 150.0f, 0.6f, 5.0f);
        particle_burst(game, player->pos, (Color){255, 255, 200, 200}, 10, 80.0f, 0.4f, 3.0f);
        camera_shake(game, 8.0f, 0.3f);

        for (int i = 0; i < MAX_ENTITIES; i++) {
            Entity *e = &game->entities[i];
            if (!e->active || i == game->player_id) continue;
            if (e->type != ENTITY_NPC_HOSTILE && e->type != ENTITY_BEAST) continue;
            if (combat_in_range(player, e, tech->range)) {
                int orig = player->stats.attack;
                player->stats.attack = tech->damage;
                combat_apply_damage(game, player, e);
                player->stats.attack = orig;
            }
        }
        break;
    }
    case TECH_DRAGON_FIST: {
        Color dragon = { 255, 100, 50, 230 };
        for (int j = 0; j < 5; j++) {
            Vec2 p = { attack_pos.x + (rand() % 40 - 20),
                       attack_pos.y + (rand() % 40 - 20) };
            particle_burst(game, p, dragon, 8, 100.0f, 0.5f, 4.0f);
        }
        camera_shake(game, 12.0f, 0.4f);

        for (int i = 0; i < MAX_ENTITIES; i++) {
            Entity *e = &game->entities[i];
            if (!e->active || i == game->player_id) continue;
            if (e->type != ENTITY_NPC_HOSTILE && e->type != ENTITY_BEAST) continue;
            if (combat_in_range(player, e, tech->range)) {
                int orig = player->stats.attack;
                player->stats.attack = tech->damage;
                combat_apply_damage(game, player, e);
                player->stats.attack = orig;
            }
        }
        break;
    }
    default:
        break;
    }

    player->is_attacking = true;
    player->attack_timer = 0.4f;
}

void combat_update(Game *game, float dt)
{
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (!e->active) continue;

        if (e->attack_timer > 0) {
            e->attack_timer -= dt;
            if (e->attack_timer <= 0) { e->is_attacking = false; e->attack_timer = 0; }
        }
        if (e->invincible_timer > 0) e->invincible_timer -= dt;
        if (e->flash_timer > 0)      e->flash_timer      -= dt;

        /* Poise regen when not staggered */
        if (!e->poise.is_staggered && e->poise.poise < e->poise.max_poise) {
            e->poise.poise += e->poise.poise_regen * dt;
            if (e->poise.poise > e->poise.max_poise) e->poise.poise = e->poise.max_poise;
        }
        /* Stagger timer countdown */
        if (e->poise.is_staggered) {
            e->poise.stagger_timer -= dt;
            if (e->poise.stagger_timer <= 0) {
                e->poise.is_staggered = false;
                e->poise.poise = e->poise.max_poise * 0.3f; /* Partially restore */
            }
            /* Stagger: cancel velocity */
            e->vel.x *= 0.1f; e->vel.y *= 0.1f;
        }

        /* Buff timers */
        for (int b = 0; b < MAX_BUFFS; b++) {
            if (!e->buffs[b].active) continue;
            e->buffs[b].remaining -= dt;
            if (e->buffs[b].remaining <= 0) {
                /* Remove buff effect */
                if (e->buffs[b].type == BUFF_ATK_UP)
                    e->stats.attack  -= e->buffs[b].magnitude;
                if (e->buffs[b].type == BUFF_DEF_UP)
                    e->stats.defense -= e->buffs[b].magnitude;
                e->buffs[b].active = false;
            }
        }

        /* Update technique cooldowns */
        for (int t = 0; t < e->num_techniques; t++) {
            if (e->techniques[t].current_cooldown > 0)
                e->techniques[t].current_cooldown -= dt;
        }

        /* Qi regeneration */
        if (e->stats.qi < e->stats.max_qi) {
            e->stats.qi += (int)(e->stats.qi_regen * dt);
            if (e->stats.qi > e->stats.max_qi) e->stats.qi = e->stats.max_qi;
        }
    }
}

const char *combat_technique_name(TechniqueType type)
{
    if (type >= 0 && type < TECH_COUNT)
        return TECHNIQUE_DEFS[type].name;
    return "Unknown";
}
