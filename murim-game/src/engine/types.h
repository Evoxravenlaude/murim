/*
 * types.h — Common type definitions for the Murim game engine
 * Heavenly Demon: Murim Chronicles
 */
#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

/* ─── World Constants ─────────────────────────────────── */
#define TILE_SIZE           32
#define CHUNK_SIZE          16    /* tiles per chunk side */
#define CHUNK_PIXEL_SIZE    (CHUNK_SIZE * TILE_SIZE)
#define WORLD_CHUNKS_X      16
#define WORLD_CHUNKS_Y      16
#define WORLD_TILES_X       (WORLD_CHUNKS_X * CHUNK_SIZE)
#define WORLD_TILES_Y       (WORLD_CHUNKS_Y * CHUNK_SIZE)

#define MAX_ENTITIES        256
#define MAX_PARTICLES       512
#define MAX_ITEMS           64
#define MAX_TECHNIQUES      16
#define MAX_DIALOGUES       32

/* ─── Screen ──────────────────────────────────────────── */
#define SCREEN_WIDTH        1280
#define SCREEN_HEIGHT       720
#define TARGET_FPS          60

/* ─── Tile Types ──────────────────────────────────────── */
typedef enum {
    TILE_GRASS = 0,
    TILE_GRASS_DARK,
    TILE_DIRT,
    TILE_STONE,
    TILE_WATER,
    TILE_SAND,
    TILE_TREE,
    TILE_MOUNTAIN,
    TILE_WALL,
    TILE_FLOOR_WOOD,
    TILE_FLOOR_STONE,
    TILE_PATH,
    TILE_BRIDGE,
    TILE_FLOWER_RED,
    TILE_FLOWER_BLUE,
    TILE_BAMBOO,
    TILE_SECT_GATE,
    TILE_SECT_FLOOR,
    TILE_COUNT
} TileType;

/* ─── Cultivation Realms ──────────────────────────────── */
typedef enum {
    REALM_MORTAL = 0,
    REALM_QI_GATHERING,
    REALM_FOUNDATION,
    REALM_CORE_FORMATION,
    REALM_NASCENT_SOUL,
    REALM_SPIRIT_SEVERING,
    REALM_TRANSCENDENCE,
    REALM_COUNT
} CultivationRealm;

/* ─── Direction ───────────────────────────────────────── */
typedef enum {
    DIR_DOWN = 0,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN_LEFT,
    DIR_DOWN_RIGHT,
    DIR_UP_LEFT,
    DIR_UP_RIGHT,
    DIR_COUNT
} Direction;

/* ─── Entity Type ─────────────────────────────────────── */
typedef enum {
    ENTITY_NONE = 0,
    ENTITY_PLAYER,
    ENTITY_NPC_FRIENDLY,
    ENTITY_NPC_HOSTILE,
    ENTITY_NPC_MERCHANT,
    ENTITY_NPC_ELDER,
    ENTITY_BEAST,
    ENTITY_BOSS,
    ENTITY_ITEM_DROP,
} EntityType;

/* ─── AI State ────────────────────────────────────────── */
typedef enum {
    AI_IDLE = 0,
    AI_WANDER,
    AI_CHASE,
    AI_ATTACK,
    AI_FLEE,
    AI_PATROL,
    AI_RETURN,
} AIState;

/* ─── Item Type ───────────────────────────────────────── */
typedef enum {
    ITEM_NONE = 0,
    ITEM_HEALING_PILL,
    ITEM_QI_PILL,
    ITEM_SWORD,
    ITEM_FIST_MANUAL,
    ITEM_MOVEMENT_SCROLL,
    ITEM_SPIRIT_STONE,
    ITEM_HERB,
    ITEM_COUNT
} ItemType;

/* ─── Technique Type ──────────────────────────────────── */
typedef enum {
    TECH_NONE = 0,
    TECH_BASIC_PUNCH,
    TECH_BASIC_SLASH,
    TECH_QI_BLAST,
    TECH_HEAVENLY_STRIKE,
    TECH_SHADOW_STEP,
    TECH_IRON_BODY,
    TECH_DRAGON_FIST,
    TECH_SWORD_RAIN,
    TECH_COUNT
} TechniqueType;

/* ─── Structures ──────────────────────────────────────── */

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    int hp;
    int max_hp;
    int qi;
    int max_qi;
    int attack;
    int defense;
    int speed;
    int qi_regen;
} Stats;

typedef struct {
    CultivationRealm realm;
    int              sub_level;     /* 1-9 within each realm */
    float            progress;     /* 0.0 to 1.0 toward next sub-level */
    float            qi_absorbed;  /* total qi absorbed */
    bool             is_cultivating;
    float            cultivation_timer;
} CultivationState;

typedef struct {
    TechniqueType type;
    const char   *name;
    int           qi_cost;
    int           damage;
    float         cooldown;
    float         current_cooldown;
    CultivationRealm min_realm;
    float         range;
} Technique;

typedef struct {
    ItemType type;
    int      quantity;
} InventorySlot;

typedef struct {
    bool         active;
    EntityType   type;
    Vec2         pos;
    Vec2         vel;
    Direction    dir;
    Stats        stats;
    float        anim_timer;
    int          anim_frame;
    bool         is_attacking;
    float        attack_timer;
    float        invincible_timer;
    float        flash_timer;
    Color        color;
    float        detection_range;

    /* AI */
    AIState      ai_state;
    Vec2         ai_home;
    Vec2         ai_target;
    float        ai_timer;
    float        ai_action_timer;

    /* Player specific */
    CultivationState cultivation;
    Technique        techniques[MAX_TECHNIQUES];
    int              num_techniques;
    int              active_technique;
    InventorySlot    inventory[MAX_ITEMS];
    int              num_items;
    int              experience;
    int              gold;
    const char      *name;
} Entity;

typedef struct {
    bool    active;
    Vec2    pos;
    Vec2    vel;
    Color   color;
    float   life;
    float   max_life;
    float   size;
    float   rotation;
    float   rot_speed;
} Particle;

typedef struct {
    TileType tiles[CHUNK_SIZE][CHUNK_SIZE];
    bool     generated;
    int      chunk_x, chunk_y;
} Chunk;

typedef struct {
    Chunk chunks[WORLD_CHUNKS_Y][WORLD_CHUNKS_X];
    float day_time;        /* 0.0 to 24.0 hours */
    float day_speed;       /* hours per real second */
} World;

typedef struct {
    const char *lines[8];
    int         num_lines;
    int         current_line;
    bool        active;
    float       char_timer;
    int         chars_shown;
    const char *speaker;
} DialogueBox;

/* ─── Game State ──────────────────────────────────────── */
typedef enum {
    STATE_TITLE = 0,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_DIALOGUE,
    STATE_STATS,
    STATE_CULTIVATING,
    STATE_GAME_OVER,
} GameState;

typedef struct {
    GameState    state;
    Entity       entities[MAX_ENTITIES];
    int          player_id;
    World        world;
    Camera2D     camera;
    Particle     particles[MAX_PARTICLES];
    DialogueBox  dialogue;
    float        screen_shake;
    float        screen_shake_timer;
    bool         show_debug;
    float        game_time;
} Game;

#endif /* TYPES_H */
