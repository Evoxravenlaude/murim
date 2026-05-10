/*
 * types.h — Common type definitions for the Murim game engine
 * Heavenly Demon: Murim Chronicles v2.0
 *
 * Expanded with: Hunter Ranks, Dungeon Gates, Beast Taming,
 * System UI, Shadow Army, Weather, Quests, Inventory
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

#define MAX_ENTITIES        512
#define MAX_PARTICLES       1024
#define MAX_ITEMS           64
#define MAX_TECHNIQUES      16
#define MAX_DIALOGUES       32
#define MAX_NOTIFICATIONS   16
#define MAX_QUESTS          32
#define MAX_DUNGEON_FLOORS  10
#define MAX_TAMED_BEASTS    20
#define MAX_ACTIVE_BEASTS   3
#define MAX_SHADOWS         10
#define MAX_LOOT_DROPS      32
#define MAX_BUFFS           8
#define MAX_COMBO_HITS      10
#define MAX_GATES           8

/* ─── v3.0 AAA System Constants ───────────────────────── */
#define MAX_BESTIARY        64
#define MAX_RECIPES         32
#define MAX_INGREDIENTS     16
#define MAX_RESOURCE_NODES  128
#define MAX_WORLD_EVENTS    8
#define MAX_FACTIONS        8
#define MAX_FISH_TYPES      12
#define MAX_STANDING_EFFECTS 4
#define BOUNTY_MAX_LEVEL    5
#define MAX_ELEMENTAL_REACTIONS 16
#define MAX_STANCE_COMBOS   5

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
    /* v2.0 tiles */
    TILE_VILLAGE_FLOOR,
    TILE_VILLAGE_WALL,
    TILE_RUIN_FLOOR,
    TILE_RUIN_WALL,
    TILE_TEMPLE_FLOOR,
    TILE_LAVA,
    TILE_ICE,
    TILE_DUNGEON_FLOOR,
    TILE_DUNGEON_WALL,
    TILE_DUNGEON_DOOR,
    TILE_GATE_PORTAL,
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

/* ─── Hunter Rank (Solo Leveling) ─────────────────────── */
typedef enum {
    RANK_E = 0,
    RANK_D,
    RANK_C,
    RANK_B,
    RANK_A,
    RANK_S,
    RANK_SS,
    RANK_SSS,
    RANK_NATIONAL,
    RANK_MONARCH,
    RANK_COUNT
} HunterRank;

/* ─── Item Rarity ─────────────────────────────────────── */
typedef enum {
    RARITY_COMMON = 0,
    RARITY_UNCOMMON,
    RARITY_RARE,
    RARITY_EPIC,
    RARITY_LEGENDARY,
    RARITY_MYTHIC,
    RARITY_COUNT
} ItemRarity;

/* ─── Dungeon Gate Rank ───────────────────────────────── */
typedef enum {
    GATE_RANK_E = 0,    /* Blue */
    GATE_RANK_D,        /* Green */
    GATE_RANK_C,        /* Yellow */
    GATE_RANK_B,        /* Orange */
    GATE_RANK_A,        /* Red */
    GATE_RANK_S,        /* Purple */
    GATE_RANK_COUNT
} GateRank;

/* ─── Weather Type ────────────────────────────────────── */
typedef enum {
    WEATHER_CLEAR = 0,
    WEATHER_CLOUDY,
    WEATHER_RAIN,
    WEATHER_STORM,
    WEATHER_SNOW,
    WEATHER_FOG,
    WEATHER_SANDSTORM,
    WEATHER_COUNT
} WeatherType;

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
    /* v2.0 entities */
    ENTITY_TAMED_BEAST,
    ENTITY_SHADOW_SOLDIER,
    ENTITY_DUNGEON_MONSTER,
    ENTITY_DUNGEON_BOSS,
    ENTITY_GATE_PORTAL,
    ENTITY_VILLAGER,
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
    /* v2.0 */
    AI_FOLLOW_PLAYER,
    AI_GUARD_POSITION,
    AI_TAMED_PASSIVE,
    AI_SHADOW_COMMAND,
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
    /* v2.0 items */
    ITEM_GATE_KEY,
    ITEM_CAPTURE_ORB,
    ITEM_EVOLUTION_STONE,
    ITEM_BUFF_POTION_ATK,
    ITEM_BUFF_POTION_DEF,
    ITEM_BUFF_POTION_SPD,
    ITEM_SHADOW_CRYSTAL,
    ITEM_WEAPON_SWORD_IRON,
    ITEM_WEAPON_SWORD_STEEL,
    ITEM_WEAPON_SWORD_SPIRIT,
    ITEM_WEAPON_SPEAR,
    ITEM_WEAPON_STAFF,
    ITEM_ARMOR_LEATHER,
    ITEM_ARMOR_CHAIN,
    ITEM_ARMOR_PLATE,
    ITEM_ACCESSORY_RING,
    ITEM_ACCESSORY_AMULET,
    /* v3.0 hunting items */
    ITEM_MEAT,
    ITEM_PELT,
    ITEM_ANCIENT_SCROLL,
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
    /* v2.0 techniques */
    TECH_DASH,
    TECH_SHADOW_EXTRACTION,
    TECH_BEAST_CAPTURE,
    TECH_RULER_AUTHORITY,
    TECH_DOMAIN_EXPANSION,
    TECH_ARISE,            /* Shadow extraction! */
    TECH_COUNT
} TechniqueType;

/* ─── Buff Type ───────────────────────────────────────── */
typedef enum {
    BUFF_NONE = 0,
    BUFF_ATK_UP,
    BUFF_DEF_UP,
    BUFF_SPD_UP,
    BUFF_REGEN,
    BUFF_IRON_BODY,
    DEBUFF_POISON,
    DEBUFF_BURN,
    DEBUFF_FREEZE,
    DEBUFF_SLOW,
    BUFF_TYPE_COUNT
} BuffType;

/* ─── Beast Evolution Stage ───────────────────────────── */
typedef enum {
    BEAST_STAGE_JUVENILE = 0,
    BEAST_STAGE_ADULT,
    BEAST_STAGE_ELDER,
    BEAST_STAGE_KING,
    BEAST_STAGE_DIVINE,
    BEAST_STAGE_COUNT
} BeastStage;

/* ─── Quest State ─────────────────────────────────────── */
typedef enum {
    QUEST_INACTIVE = 0,
    QUEST_ACTIVE,
    QUEST_COMPLETE,
    QUEST_FAILED,
    QUEST_CLAIMED,
} QuestState;

/* ─── Quest Type ──────────────────────────────────────── */
typedef enum {
    QUEST_TYPE_DAILY = 0,
    QUEST_TYPE_STORY,
    QUEST_TYPE_DUNGEON,
    QUEST_TYPE_TAMING,
    QUEST_TYPE_HUNTING,
} QuestType;

/* ─── Notification Priority ──────────────────────────── */
typedef enum {
    NOTIFY_INFO = 0,
    NOTIFY_SUCCESS,
    NOTIFY_WARNING,
    NOTIFY_LEVEL_UP,
    NOTIFY_RANK_UP,
    NOTIFY_QUEST,
    NOTIFY_SYSTEM,
    NOTIFY_SHADOW,
    NOTIFY_LOOT,
} NotifyPriority;

/* ═══════════════════════════════════════════════════════ */
/*                    STRUCTURES                          */
/* ═══════════════════════════════════════════════════════ */

typedef struct {
    float x, y;
} Vec2;

/* ─── RPG Stats (expanded) ────────────────────────────── */
typedef struct {
    int hp;
    int max_hp;
    int qi;         /* mana/mp */
    int max_qi;
    int attack;     /* STR */
    int defense;    /* VIT */
    int speed;      /* AGI */
    int qi_regen;
    /* v2.0 stats */
    int strength;   /* raw STR for display */
    int agility;    /* raw AGI */
    int vitality;   /* raw VIT */
    int intelligence; /* affects qi/magic damage */
    int sense;      /* detection, crit rate */
    int luck;       /* drop rates, crit */
    int level;
    int exp;
    int exp_to_next;
    int stat_points;  /* unallocated stat points */
    
    /* v2.0 RDR2 Survival Cores */
    float hunger;     /* 0 to 100 */
    float thirst;     /* 0 to 100 */
    float fatigue;    /* 0 to 100 */
    
    /* v2.0 Skyrim Mastery Skills */
    float sword_mastery;
    float fist_mastery;
    float athletics;
} Stats;

typedef struct {
    CultivationRealm realm;
    int              sub_level;
    float            progress;
    float            qi_absorbed;
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
    ItemType  type;
    int       quantity;
    ItemRarity rarity;
    int       bonus_stat;     /* bonus provided */
    const char *name;
} InventorySlot;

/* ─── Buff/Debuff ─────────────────────────────────────── */
typedef struct {
    BuffType type;
    float    duration;
    float    remaining;
    int      magnitude;
    bool     active;
} Buff;

/* ─── Combo System ────────────────────────────────────── */
typedef struct {
    int    hit_count;
    float  timer;          /* time since last hit, resets on hit */
    float  window;         /* max time between hits */
    float  multiplier;     /* damage multiplier */
    bool   active;
} ComboState;

/* ─── System Notification (Solo Leveling) ─────────────── */
typedef struct {
    bool           active;
    NotifyPriority priority;
    char           title[64];
    char           body[256];
    float          timer;
    float          duration;
    float          alpha;
    float          slide_offset;
} SystemNotification;

/* ─── Quest ───────────────────────────────────────────── */
typedef struct {
    bool       active;
    QuestType  type;
    QuestState state;
    char       title[64];
    char       description[128];
    char       objective_text[64];
    int        objective_current;
    int        objective_target;
    int        reward_gold;
    int        reward_exp;
    ItemType   reward_item;
    bool       is_daily;
} Quest;

/* ─── Tamed Beast ─────────────────────────────────────── */
typedef struct {
    bool        active;
    char        name[32];
    char        species[32];
    BeastStage  stage;
    Stats       stats;
    Color       color;
    int         entity_id;     /* -1 if not deployed */
    int         loyalty;       /* 0-100 */
    float       capture_hp_pct; /* HP% when captured */
    bool        is_deployed;
    bool        is_mounted;
    int         evolution_xp;
    int         evolution_threshold;
    /* Beast abilities */
    TechniqueType ability;
    float         ability_cooldown;
} TamedBeast;

/* ─── Shadow Soldier (Solo Leveling ARISE) ────────────── */
typedef struct {
    bool        active;
    char        name[32];
    int         entity_id;     /* entity index when deployed */
    Stats       stats;
    Color       color;
    HunterRank  rank;
    bool        is_deployed;
    float       loyalty;       /* shadows are always loyal but display */
    TechniqueType ability;
} ShadowSoldier;

/* ─── Dungeon Gate ────────────────────────────────────── */
typedef struct {
    bool      active;
    Vec2      world_pos;       /* position in the overworld */
    GateRank  rank;
    int       num_floors;
    int       current_floor;
    bool      is_entered;
    bool      is_cleared;
    float     pulse_timer;     /* visual pulsing */
    float     time_limit;      /* time before gate break */
    float     time_remaining;
    /* Dungeon floor data */
    TileType  floor_tiles[64][64];
    int       floor_width;
    int       floor_height;
    int       floor_enemies_remaining;
    bool      boss_spawned;
    Vec2      player_entry_pos; /* where to return after dungeon */
} DungeonGate;

/* ─── Weather State ───────────────────────────────────── */
typedef struct {
    WeatherType current;
    WeatherType target;
    float       transition;    /* 0-1 blend between current and target */
    float       intensity;     /* 0-1 */
    float       change_timer;  /* time until next weather change */
    float       lightning_timer;
    bool        lightning_flash;
    float       wind_strength;
    float       wind_angle;
} WeatherState;

/* ─── Dash/Dodge ──────────────────────────────────────── */
typedef struct {
    bool   is_dashing;
    float  dash_timer;
    float  dash_cooldown;
    float  dash_cooldown_max;
    Vec2   dash_dir;
    float  dash_speed;
    int    dash_charges;
    int    dash_max_charges;
    float  iframe_timer;      /* invincibility frames during dash */
} DashState;

/* ─── Fog of War ──────────────────────────────────────── */
typedef struct {
    bool explored[WORLD_CHUNKS_Y * CHUNK_SIZE][WORLD_CHUNKS_X * CHUNK_SIZE];
    bool visible[WORLD_CHUNKS_Y * CHUNK_SIZE][WORLD_CHUNKS_X * CHUNK_SIZE];
    int  vision_range;
} FogOfWar;

/* ─── v3.0 Enums needed before Entity ────────────────── */

/* Elemental Type */
typedef enum {
    ELEMENT_NONE = 0,
    ELEMENT_FIRE,
    ELEMENT_WATER,
    ELEMENT_LIGHTNING,
    ELEMENT_ICE,
    ELEMENT_WIND,
    ELEMENT_EARTH,
    ELEMENT_SHADOW,
    ELEMENT_COUNT
} ElementType;

/* Reaction Type */
typedef enum {
    REACTION_NONE = 0,
    REACTION_VAPORIZE,
    REACTION_MELT,
    REACTION_OVERLOAD,
    REACTION_SUPERCONDUCT,
    REACTION_ELECTROCHARGE,
    REACTION_FROZEN,
    REACTION_SHATTER,
    REACTION_SWIRL,
    REACTION_CRYSTALLIZE,
    REACTION_SHADOWBURN,
    REACTION_SHADOWFREEZE,
    REACTION_BLOOM,
    REACTION_COUNT
} ReactionType;

/* Combat Stance */
typedef enum {
    STANCE_NEUTRAL = 0,
    STANCE_STONE,
    STANCE_WATER,
    STANCE_WIND,
    STANCE_MOON,
    STANCE_COUNT
} CombatStance;

/* Standoff State */
typedef enum {
    STANDOFF_NONE = 0,
    STANDOFF_APPROACH,
    STANDOFF_READY,
    STANDOFF_STRIKE,
    STANDOFF_SUCCESS,
    STANDOFF_FAIL,
} StandoffState;

/* Environment Zone */
typedef enum {
    ZONE_NORMAL = 0,
    ZONE_HOT,
    ZONE_COLD,
    ZONE_UNDERWATER,
    ZONE_TOXIC,
    ZONE_HIGH_ALTITUDE,
    ZONE_COUNT
} EnvironmentZone;

/* Resource Type */
typedef enum {
    RESOURCE_NONE = 0,
    RESOURCE_ORE_IRON,
    RESOURCE_ORE_SPIRIT,
    RESOURCE_HERB_RED,
    RESOURCE_HERB_BLUE,
    RESOURCE_HERB_GOLD,
    RESOURCE_WOOD,
    RESOURCE_CRYSTAL,
    RESOURCE_FISH_SPOT,
    RESOURCE_COUNT
} ResourceType;

/* Faction ID */
typedef enum {
    FACTION_NONE = 0,
    FACTION_HEAVENLY_SECT,
    FACTION_DEMON_CULT,
    FACTION_MERCHANTS_GUILD,
    FACTION_BEAST_TAMERS,
    FACTION_SHADOW_GUILD,
    FACTION_VILLAGERS,
    FACTION_COUNT
} FactionID;

/* World Event Type */
typedef enum {
    EVENT_NONE = 0,
    EVENT_BEAST_STAMPEDE,
    EVENT_GATE_BREAK,
    EVENT_TRAVELING_MERCHANT,
    EVENT_HEAVENLY_TRIBULATION,
    EVENT_SECT_WAR,
    EVENT_TREASURE_RAIN,
    EVENT_BOUNTY_HUNTER,
    EVENT_COUNT
} WorldEventType;

/* Alchemy Recipe Category */
typedef enum {
    RECIPE_POTION = 0,
    RECIPE_OIL,
    RECIPE_ELIXIR,
    RECIPE_BOMB,
    RECIPE_COUNT
} RecipeCategory;

/* ─── v3.0 Structs (must be before Entity) ───────────── */

typedef struct {
    ElementType  affinity;
    ElementType  applied;
    float        applied_timer;
    float        reaction_cd;
    int          reaction_dmg;
    ReactionType last_reaction;
} ElementalState;

typedef struct {
    float poise;
    float max_poise;
    float poise_regen;
    float stagger_timer;
    bool  is_staggered;
    bool  is_blocking;
    float block_stamina;
} PoiseState;

typedef struct {
    StandoffState state;
    float         timer;
    float         window;
    int           target_id;
    int           kills;
    int           max_chain;
    bool          active;
} Standoff;

typedef struct {
    EnvironmentZone current_zone;
    float temperature;
    float altitude;
    float oxygen;
    float oxygen_max;
    float oxygen_drain;
    bool  is_underwater;
    bool  is_climbing;
    float climb_stamina_drain;
    float swim_speed_mult;
    float heat_damage_timer;
    float cold_slow_factor;
} EnvironmentState;

typedef struct {
    bool  is_mounted;
    int   mount_beast_idx;
    float mount_speed_mult;
    float mount_stamina;
    float mount_max_stamina;
    bool  can_mount_combat;
} MountState;

typedef struct {
    bool  active;
    float pulse_timer;
    float pulse_radius;
    float max_radius;
    Color highlight_color;
} QiSenseState;

typedef struct {
    CombatStance current;
    float        switch_cooldown;
    float        stance_bonus_dmg;
    float        stance_bonus_speed;
    int          combo_step;
    float        combo_timer;
} StanceState;

typedef struct {
    int   karma;
    int   bounty_level;
    float bounty_timer;
    float hunter_spawn_timer;
    int   hunters_dispatched;
    bool  heavens_angry;
    int   innocents_killed;
    int   sects_robbed;
} BountyState;

typedef struct {
    bool  active;
    bool  fish_hooked;
    float cast_distance;
    float tension;
    float reel_progress;
    float fish_fight_timer;
    int   fish_type;
    int   fish_quality;
    float bobber_x, bobber_y;
    float wait_timer;
} FishingState;

/* ─── Entity (expanded) ──────────────────────────────── */
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

    /* v2.0 additions */
    HunterRank       hunter_rank;
    DashState        dash;
    ComboState       combo;
    Buff             buffs[MAX_BUFFS];
    int              kills;
    int              dungeons_cleared;
    bool             can_extract_shadow;
    GateRank         highest_gate_cleared;

    /* Beast taming */
    bool             is_tameable;
    float            tame_difficulty;
    int              owner_id;

    /* v3.0 AAA additions */
    ElementalState   elemental;
    PoiseState       poise;
    StanceState      stance;
    Standoff         standoff;
    EnvironmentState environment;
    MountState       mount;
    QiSenseState     qi_sense;
    BountyState      bounty;
    FactionID        faction;
    FishingState     fishing;
    float            iron_body_mastery;
    int              bestiary_kills[MAX_BESTIARY];
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
    /* v2.0 */
    bool    has_trail;
    Vec2    trail_positions[4];
    int     trail_index;
} Particle;

typedef struct {
    TileType tiles[CHUNK_SIZE][CHUNK_SIZE];
    bool     generated;
    int      chunk_x, chunk_y;
} Chunk;

typedef struct {
    Chunk chunks[WORLD_CHUNKS_Y][WORLD_CHUNKS_X];
    float day_time;
    float day_speed;
    /* v2.0 */
    WeatherState weather;
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

/* ─── Loot Drop ───────────────────────────────────────── */
typedef struct {
    bool       active;
    Vec2       pos;
    ItemType   item;
    ItemRarity rarity;
    int        quantity;
    float      timer;        /* pickup window / despawn */
    float      bob_timer;    /* visual bobbing */
} LootDrop;

/* ─── Game State ──────────────────────────────────────── */
typedef enum {
    STATE_TITLE = 0,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_DIALOGUE,
    STATE_STATS,
    STATE_CULTIVATING,
    STATE_GAME_OVER,
    /* v2.0 states */
    STATE_DUNGEON,
    STATE_INVENTORY,
    STATE_SKILL_TREE,
    STATE_BEAST_MENU,
    STATE_SHADOW_MENU,
    STATE_QUEST_LOG,
    /* v3.0 states */
    STATE_BESTIARY,
    STATE_ALCHEMY,
    STATE_STANDOFF,
    STATE_FISHING,
} GameState;

/* ─── Bestiary Entry (Witcher 3) ──────────────────────── */
typedef struct {
    bool         discovered;
    char         name[32];
    char         lore[128];
    EntityType   entity_type;
    ElementType  weakness;
    ElementType  resistance;
    int          kill_count;
    int          drops[4];
    int          num_drops;
    float        avg_hp;
    bool         fully_studied;
} BestiaryEntry;

/* ─── Alchemy Recipe (Witcher 3) ──────────────────────── */
typedef struct {
    bool           unlocked;
    char           name[32];
    RecipeCategory category;
    ItemType       result;
    ItemRarity     result_rarity;
    ResourceType   ingredients[MAX_INGREDIENTS];
    int            ingredient_counts[MAX_INGREDIENTS];
    int            num_ingredients;
    int            crafting_time;
    char           description[64];
} AlchemyRecipe;

/* ─── Resource Node ───────────────────────────────────── */
typedef struct {
    bool         active;
    ResourceType type;
    Vec2         pos;
    int          amount;
    float        respawn_timer;
    float        harvest_time;
    bool         is_harvesting;
    float        harvest_progress;
} ResourceNode;

/* ─── Faction Reputation ──────────────────────────────── */
typedef struct {
    FactionID id;
    char      name[32];
    int       reputation;
    bool      is_hostile;
    bool      is_allied;
    Color     color;
} FactionState;

/* ─── World Event ─────────────────────────────────────── */
typedef struct {
    bool           active;
    WorldEventType type;
    Vec2           pos;
    float          timer;
    float          duration;
    char           description[64];
    bool           completed;
    int            reward_exp;
    int            reward_gold;
} WorldEvent;

/* ─── Global Game ─────────────────────────────────────── */
typedef struct {
    GameState            state;
    GameState            prev_state;  /* for returning from sub-menus */
    Entity               entities[MAX_ENTITIES];
    int                  player_id;
    World                world;
    Camera2D             camera;
    Particle             particles[MAX_PARTICLES];
    DialogueBox          dialogue;
    float                screen_shake;
    float                screen_shake_timer;
    bool                 show_debug;
    float                game_time;

    /* v2.0 systems */
    SystemNotification   notifications[MAX_NOTIFICATIONS];
    Quest                quests[MAX_QUESTS];
    DungeonGate          gates[MAX_GATES];
    int                  active_dungeon;   /* -1 if not in dungeon */
    TamedBeast           tamed_beasts[MAX_TAMED_BEASTS];
    ShadowSoldier        shadows[MAX_SHADOWS];
    LootDrop             loot_drops[MAX_LOOT_DROPS];
    FogOfWar            *fog;              /* allocated dynamically */
    bool                 system_ui_open;   /* Solo Leveling status window */
    float                level_up_anim;    /* animation timer for level up */
    int                  total_kills;
    int                  total_dungeons;
    float                play_time;
    bool                 has_shadow_power;  /* unlocked shadow extraction */

    /* v3.0 AAA systems */
    BestiaryEntry        bestiary[MAX_BESTIARY];
    int                  bestiary_count;
    AlchemyRecipe        recipes[MAX_RECIPES];
    int                  recipe_count;
    ResourceNode         resource_nodes[MAX_RESOURCE_NODES];
    int                  resource_count;
    FactionState         factions[MAX_FACTIONS];
    WorldEvent           world_events[MAX_WORLD_EVENTS];
    bool                 qi_sense_global;   /* Toggle for Qi Sense vision */
    float                qi_sense_pulse;
} Game;

/* ─── Rank/Rarity helper colors ───────────────────────── */
static inline Color rank_color(HunterRank rank) {
    switch (rank) {
    case RANK_E:        return (Color){ 150, 150, 150, 255 }; /* gray */
    case RANK_D:        return (Color){ 100, 200, 100, 255 }; /* green */
    case RANK_C:        return (Color){ 100, 180, 255, 255 }; /* blue */
    case RANK_B:        return (Color){ 255, 200, 50, 255 };  /* yellow */
    case RANK_A:        return (Color){ 255, 120, 50, 255 };  /* orange */
    case RANK_S:        return (Color){ 255, 50, 50, 255 };   /* red */
    case RANK_SS:       return (Color){ 200, 50, 255, 255 };  /* purple */
    case RANK_SSS:      return (Color){ 255, 50, 200, 255 };  /* pink */
    case RANK_NATIONAL: return (Color){ 255, 215, 0, 255 };   /* gold */
    case RANK_MONARCH:  return (Color){ 255, 255, 255, 255 }; /* white */
    default:            return WHITE;
    }
}

static inline Color rarity_color(ItemRarity rarity) {
    switch (rarity) {
    case RARITY_COMMON:    return (Color){ 200, 200, 200, 255 };
    case RARITY_UNCOMMON:  return (Color){ 80, 200, 80, 255 };
    case RARITY_RARE:      return (Color){ 80, 140, 255, 255 };
    case RARITY_EPIC:      return (Color){ 180, 80, 255, 255 };
    case RARITY_LEGENDARY: return (Color){ 255, 165, 0, 255 };
    case RARITY_MYTHIC:    return (Color){ 255, 50, 50, 255 };
    default:               return WHITE;
    }
}

static inline Color gate_color(GateRank rank) {
    switch (rank) {
    case GATE_RANK_E: return (Color){ 80, 140, 255, 255 };  /* blue */
    case GATE_RANK_D: return (Color){ 80, 220, 80, 255 };   /* green */
    case GATE_RANK_C: return (Color){ 255, 220, 50, 255 };  /* yellow */
    case GATE_RANK_B: return (Color){ 255, 140, 40, 255 };  /* orange */
    case GATE_RANK_A: return (Color){ 255, 50, 50, 255 };   /* red */
    case GATE_RANK_S: return (Color){ 160, 50, 255, 255 };  /* purple */
    default:          return WHITE;
    }
}

static inline const char *rank_name(HunterRank rank) {
    static const char *names[] = {
        "E-Rank", "D-Rank", "C-Rank", "B-Rank", "A-Rank",
        "S-Rank", "SS-Rank", "SSS-Rank", "National Level", "Monarch"
    };
    if (rank >= 0 && rank < RANK_COUNT) return names[rank];
    return "Unknown";
}

static inline const char *rarity_name(ItemRarity r) {
    static const char *names[] = {
        "Common", "Uncommon", "Rare", "Epic", "Legendary", "Mythic"
    };
    if (r >= 0 && r < RARITY_COUNT) return names[r];
    return "Unknown";
}

static inline const char *gate_rank_name(GateRank r) {
    static const char *names[] = {
        "E-Rank Gate", "D-Rank Gate", "C-Rank Gate",
        "B-Rank Gate", "A-Rank Gate", "S-Rank Gate"
    };
    if (r >= 0 && r < GATE_RANK_COUNT) return names[r];
    return "Unknown Gate";
}



static inline const char* element_name(ElementType e) {
    static const char *names[] = {
        "None", "Fire", "Water", "Lightning", "Ice",
        "Wind", "Earth", "Shadow"
    };
    if (e >= 0 && e < ELEMENT_COUNT) return names[e];
    return "Unknown";
}

static inline Color element_color(ElementType e) {
    switch (e) {
    case ELEMENT_FIRE:      return (Color){ 255, 80, 30, 255 };
    case ELEMENT_WATER:     return (Color){ 50, 140, 255, 255 };
    case ELEMENT_LIGHTNING: return (Color){ 255, 255, 80, 255 };
    case ELEMENT_ICE:       return (Color){ 150, 220, 255, 255 };
    case ELEMENT_WIND:      return (Color){ 130, 255, 180, 255 };
    case ELEMENT_EARTH:     return (Color){ 200, 160, 80, 255 };
    case ELEMENT_SHADOW:    return (Color){ 140, 50, 200, 255 };
    default:                return WHITE;
    }
}

static inline const char* zone_name(EnvironmentZone z) {
    static const char *names[] = {
        "Normal", "Scorching", "Freezing",
        "Underwater", "Toxic", "High Altitude"
    };
    if (z >= 0 && z < ZONE_COUNT) return names[z];
    return "Unknown";
}

static inline const char* faction_name_str(FactionID f) {
    static const char *names[] = {
        "None", "Heavenly Sword Sect", "Demon Cult",
        "Merchant Guild", "Beast Tamers Lodge",
        "Shadow Guild", "Villagers"
    };
    if (f >= 0 && f < FACTION_COUNT) return names[f];
    return "Unknown";
}

static inline const char* stance_name(CombatStance s) {
    static const char *names[] = {
        "Neutral", "Stone", "Water", "Wind", "Moon"
    };
    if (s >= 0 && s < STANCE_COUNT) return names[s];
    return "Unknown";
}

#endif /* TYPES_H */
