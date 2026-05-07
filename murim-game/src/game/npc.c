/*
 * npc.c — NPC AI, spawning, and behavior
 * Heavenly Demon: Murim Chronicles
 */
#include "npc.h"
#include "world.h"
#include "combat.h"
#include "cultivation.h"
#include "../engine/particles.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ─── Entity colors by type ───────────────────────────── */
static const Color ENTITY_COLORS[] = {
    [ENTITY_NONE]         = { 0, 0, 0, 0 },
    [ENTITY_PLAYER]       = { 60, 60, 140, 255 },     /* blue robes */
    [ENTITY_NPC_FRIENDLY] = { 60, 140, 60, 255 },     /* green robes */
    [ENTITY_NPC_HOSTILE]  = { 160, 50, 50, 255 },     /* red robes */
    [ENTITY_NPC_MERCHANT] = { 180, 140, 50, 255 },    /* gold robes */
    [ENTITY_NPC_ELDER]    = { 200, 200, 220, 255 },   /* white/silver robes */
    [ENTITY_BEAST]        = { 100, 60, 40, 255 },     /* brown fur */
    [ENTITY_BOSS]         = { 120, 20, 120, 255 },    /* purple */
    [ENTITY_ITEM_DROP]    = { 255, 215, 0, 255 },     /* gold */
};

/* ─── NPC name pools ──────────────────────────────────── */
static const char *HOSTILE_NAMES[] = {
    "Rogue Disciple", "Dark Sect Member", "Bandit", "Corrupt Elder",
    "Shadow Assassin", "Demon Cultivator", "Fallen Warrior", "Blood Sect Disciple"
};
static const int NUM_HOSTILE_NAMES = 8;

static const char *BEAST_NAMES[] = {
    "Spirit Wolf", "Iron Boar", "Flame Fox", "Jade Serpent",
    "Thunder Hawk", "Stone Bear", "Shadow Cat", "Wind Tiger"
};
static const int NUM_BEAST_NAMES = 8;

static const char *FRIENDLY_NAMES[] = {
    "Sect Disciple", "Wandering Monk", "Pill Alchemist", "Formation Master",
    "Young Hero", "Traveling Scholar", "Medicine Woman", "Woodcutter"
};
static const int NUM_FRIENDLY_NAMES = 8;

/* ─── Helper: find free entity slot ───────────────────── */
static int find_free_entity(Game *game)
{
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!game->entities[i].active)
            return i;
    }
    return -1;
}

/* ─── Spawn functions ─────────────────────────────────── */
int npc_spawn_player(Game *game, Vec2 pos)
{
    int id = 0; /* Player always at index 0 */
    Entity *e = &game->entities[id];
    memset(e, 0, sizeof(Entity));

    e->active = true;
    e->type = ENTITY_PLAYER;
    e->pos = pos;
    e->dir = DIR_DOWN;
    e->color = ENTITY_COLORS[ENTITY_PLAYER];
    e->name = "Protagonist";
    e->detection_range = 0;
    
    e->hunter_rank = RANK_E;
    e->stats.level = 1;
    e->stats.exp = 0;
    e->stats.exp_to_next = 100;
    e->stats.strength = 10;
    e->stats.agility = 10;
    e->stats.vitality = 10;
    e->stats.intelligence = 10;
    e->stats.sense = 10;
    e->stats.luck = 10;
    e->stats.stat_points = 0;
    
    e->stats.hunger = 100.0f;
    e->stats.thirst = 100.0f;
    e->stats.fatigue = 100.0f;
    e->stats.sword_mastery = 1.0f;
    e->stats.fist_mastery = 1.0f;
    e->stats.athletics = 1.0f;

    cultivation_init(e, REALM_MORTAL);
    e->stats.hp = e->stats.max_hp;
    e->stats.qi = e->stats.max_qi;

    combat_init_player_techniques(e);

    e->gold = 50;
    e->experience = 0;

    game->player_id = id;
    return id;
}

int npc_spawn(Game *game, EntityType type, Vec2 pos, const char *name)
{
    int id = find_free_entity(game);
    if (id < 0) return -1;

    Entity *e = &game->entities[id];
    memset(e, 0, sizeof(Entity));

    e->active = true;
    e->type = type;
    e->pos = pos;
    e->ai_home = pos;
    e->dir = (Direction)(rand() % 4);
    e->color = ENTITY_COLORS[type];
    e->name = name;

    /* Stats based on type */
    switch (type) {
    case ENTITY_NPC_HOSTILE:
        cultivation_init(e, REALM_QI_GATHERING + rand() % 2);
        e->detection_range = 80.0f;
        e->ai_state = AI_WANDER;
        break;
    case ENTITY_BEAST:
        cultivation_init(e, REALM_MORTAL + rand() % 2);
        e->stats.attack += 5;
        e->detection_range = 100.0f;
        e->ai_state = AI_WANDER;
        break;
    case ENTITY_BOSS:
        cultivation_init(e, REALM_CORE_FORMATION);
        e->stats.max_hp *= 3;
        e->stats.hp = e->stats.max_hp;
        e->stats.attack *= 2;
        e->detection_range = 120.0f;
        e->ai_state = AI_PATROL;
        break;
    case ENTITY_NPC_FRIENDLY:
    case ENTITY_NPC_MERCHANT:
    case ENTITY_NPC_ELDER:
        cultivation_init(e, REALM_FOUNDATION + rand() % 3);
        e->detection_range = 0;
        e->ai_state = AI_IDLE;
        break;
    default:
        cultivation_init(e, REALM_MORTAL);
        e->ai_state = AI_IDLE;
        break;
    }

    e->stats.hp = e->stats.max_hp;
    e->stats.qi = e->stats.max_qi;

    return id;
}

void npc_populate_area(Game *game, Vec2 center, float radius, int count)
{
    for (int i = 0; i < count; i++) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        float dist = (float)(rand() % (int)radius);
        Vec2 pos = {
            center.x + cosf(angle) * dist,
            center.y + sinf(angle) * dist
        };

        /* Check if position is walkable */
        if (!world_pos_walkable(&game->world, pos.x, pos.y)) continue;

        /* Random type */
        int r = rand() % 100;
        if (r < 35) {
            /* Hostile NPC */
            npc_spawn(game, ENTITY_NPC_HOSTILE, pos,
                      HOSTILE_NAMES[rand() % NUM_HOSTILE_NAMES]);
        } else if (r < 60) {
            /* Beast */
            npc_spawn(game, ENTITY_BEAST, pos,
                      BEAST_NAMES[rand() % NUM_BEAST_NAMES]);
        } else if (r < 80) {
            /* Friendly NPC */
            npc_spawn(game, ENTITY_NPC_FRIENDLY, pos,
                      FRIENDLY_NAMES[rand() % NUM_FRIENDLY_NAMES]);
        } else if (r < 92) {
            /* Merchant */
            npc_spawn(game, ENTITY_NPC_MERCHANT, pos, "Merchant");
        } else {
            /* Elder */
            npc_spawn(game, ENTITY_NPC_ELDER, pos, "Sect Elder");
        }
    }
}

/* ─── AI Behavior ─────────────────────────────────────── */
static float dist_to_player(const Game *game, const Entity *e)
{
    const Entity *player = &game->entities[game->player_id];
    float dx = player->pos.x - e->pos.x;
    float dy = player->pos.y - e->pos.y;
    return sqrtf(dx * dx + dy * dy);
}

static void ai_wander(Game *game, Entity *e, float dt)
{
    e->ai_timer -= dt;

    if (e->ai_timer <= 0) {
        /* Pick a new random direction */
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        float speed = (float)e->stats.speed * 0.3f;
        e->vel.x = cosf(angle) * speed;
        e->vel.y = sinf(angle) * speed;
        e->ai_timer = 1.0f + (float)(rand() % 30) / 10.0f;

        /* Sometimes stop */
        if (rand() % 3 == 0) {
            e->vel.x = 0;
            e->vel.y = 0;
            e->ai_timer = 2.0f + (float)(rand() % 20) / 10.0f;
        }
    }

    /* Check for player proximity (hostile only) */
    if (e->type == ENTITY_NPC_HOSTILE || e->type == ENTITY_BEAST) {
        float dist = dist_to_player(game, e);
        if (dist < e->detection_range) {
            e->ai_state = AI_CHASE;
            e->ai_timer = 0;
        }
    }
}

static void ai_chase(Game *game, Entity *e, float dt)
{
    (void)dt;
    const Entity *player = &game->entities[game->player_id];
    float dx = player->pos.x - e->pos.x;
    float dy = player->pos.y - e->pos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist > e->detection_range * 2.0f) {
        /* Lost the player, return home */
        e->ai_state = AI_RETURN;
        e->ai_timer = 0;
        return;
    }

    if (dist < 25.0f) {
        /* Close enough to attack */
        e->ai_state = AI_ATTACK;
        e->ai_timer = 0;
        e->vel.x = 0;
        e->vel.y = 0;
        return;
    }

    /* Move toward player */
    float speed = (float)e->stats.speed * 0.7f;
    if (dist > 0) {
        e->vel.x = (dx / dist) * speed;
        e->vel.y = (dy / dist) * speed;
    }

    /* Update facing direction */
    if (fabsf(dx) > fabsf(dy)) {
        e->dir = dx > 0 ? DIR_RIGHT : DIR_LEFT;
    } else {
        e->dir = dy > 0 ? DIR_DOWN : DIR_UP;
    }
}

static void ai_attack(Game *game, Entity *e, float dt)
{
    const Entity *player = &game->entities[game->player_id];
    float dist = dist_to_player(game, e);

    if (dist > 30.0f) {
        e->ai_state = AI_CHASE;
        return;
    }

    e->ai_action_timer -= dt;
    if (e->ai_action_timer <= 0) {
        /* Attack the player */
        if (!e->is_attacking && e->attack_timer <= 0) {
            e->is_attacking = true;
            e->attack_timer = 0.4f;

            /* Face the player */
            float dx = player->pos.x - e->pos.x;
            float dy = player->pos.y - e->pos.y;
            if (fabsf(dx) > fabsf(dy))
                e->dir = dx > 0 ? DIR_RIGHT : DIR_LEFT;
            else
                e->dir = dy > 0 ? DIR_DOWN : DIR_UP;

            /* Deal damage */
            if (combat_in_range(e, player, 30.0f)) {
                combat_apply_damage(game, e, &game->entities[game->player_id]);
            }
        }
        e->ai_action_timer = 0.8f + (float)(rand() % 10) / 10.0f;
    }

    e->vel.x = 0;
    e->vel.y = 0;
}

static void ai_return(Game *game, Entity *e, float dt)
{
    (void)game;
    (void)dt;
    float dx = e->ai_home.x - e->pos.x;
    float dy = e->ai_home.y - e->pos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist < 10.0f) {
        e->ai_state = AI_WANDER;
        e->vel.x = 0;
        e->vel.y = 0;
        return;
    }

    float speed = (float)e->stats.speed * 0.5f;
    if (dist > 0) {
        e->vel.x = (dx / dist) * speed;
        e->vel.y = (dy / dist) * speed;
    }
}

void npc_update_ai(Game *game, float dt)
{
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (!e->active || e->type == ENTITY_PLAYER) continue;

        /* Update AI based on state */
        switch (e->ai_state) {
        case AI_IDLE:
            e->vel.x = 0;
            e->vel.y = 0;
            break;
        case AI_WANDER:
            ai_wander(game, e, dt);
            break;
        case AI_CHASE:
            ai_chase(game, e, dt);
            break;
        case AI_ATTACK:
            ai_attack(game, e, dt);
            break;
        case AI_RETURN:
            ai_return(game, e, dt);
            break;
        default:
            break;
        }

        /* Apply velocity with collision */
        float new_x = e->pos.x + e->vel.x * dt;
        float new_y = e->pos.y + e->vel.y * dt;

        if (world_pos_walkable(&game->world, new_x, e->pos.y))
            e->pos.x = new_x;
        if (world_pos_walkable(&game->world, e->pos.x, new_y))
            e->pos.y = new_y;

        /* Keep within world bounds */
        if (e->pos.x < TILE_SIZE) e->pos.x = TILE_SIZE;
        if (e->pos.y < TILE_SIZE) e->pos.y = TILE_SIZE;
        if (e->pos.x > WORLD_TILES_X * TILE_SIZE - TILE_SIZE)
            e->pos.x = WORLD_TILES_X * TILE_SIZE - TILE_SIZE;
        if (e->pos.y > WORLD_TILES_Y * TILE_SIZE - TILE_SIZE)
            e->pos.y = WORLD_TILES_Y * TILE_SIZE - TILE_SIZE;

        /* NPC natural HP regen */
        if (e->stats.hp < e->stats.max_hp && e->ai_state != AI_CHASE && e->ai_state != AI_ATTACK) {
            e->stats.hp += (int)(2.0f * dt);
            if (e->stats.hp > e->stats.max_hp)
                e->stats.hp = e->stats.max_hp;
        }
    }
}

int npc_find_nearby_friendly(Game *game, float range)
{
    const Entity *player = &game->entities[game->player_id];

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (!e->active || i == game->player_id) continue;
        if (e->type != ENTITY_NPC_FRIENDLY && e->type != ENTITY_NPC_MERCHANT
            && e->type != ENTITY_NPC_ELDER) continue;

        float dx = e->pos.x - player->pos.x;
        float dy = e->pos.y - player->pos.y;
        if (sqrtf(dx * dx + dy * dy) < range)
            return i;
    }
    return -1;
}
