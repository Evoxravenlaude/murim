/*
 * save.c — Save/Load System
 * Heavenly Demon: Murim Chronicles v4.0
 *
 * Binary serialization of the full game state.
 * Saves: player entity, tamed beasts, shadow army, bestiary,
 *        quests, faction reputations, world events, alchemy resources,
 *        dungeon gate progress, play time.
 */
#include "save.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ─── Save header ─────────────────────────────────────── */
typedef struct {
    unsigned int magic;
    unsigned int version;
    float        play_time;
    int          total_kills;
    int          total_dungeons;
    int          bestiary_count;
    int          recipe_count;
    int          resource_count;
} SaveHeader;

/* ─── Player save data (subset of Entity) ────────────── */
typedef struct {
    Vec2             pos;
    Stats            stats;
    CultivationState cultivation;
    HunterRank       hunter_rank;
    int              gold;
    int              experience;
    int              kills;
    int              dungeons_cleared;
    bool             has_shadow_power;
    bool             can_extract_shadow;
    ElementType      elemental_affinity;
    int              active_technique;
    int              num_techniques;
    Technique        techniques[MAX_TECHNIQUES];
    InventorySlot    inventory[MAX_ITEMS];
    int              num_items;
    BountyState      bounty;
    FactionID        faction;
} PlayerSave;

/* ─── Resource inventory (alchemy) ───────────────────── */
/* We can't access the static inside alchemy.c, so we use
   the public API. Store counts externally here. */

bool save_exists(void)
{
    FILE *f = fopen(SAVE_FILE_PATH, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

void save_delete(void)
{
    remove(SAVE_FILE_PATH);
}

bool save_game(const Game *game)
{
    FILE *f = fopen(SAVE_FILE_PATH, "wb");
    if (!f) return false;

    /* Header */
    SaveHeader hdr;
    hdr.magic           = SAVE_MAGIC;
    hdr.version         = SAVE_VERSION;
    hdr.play_time       = game->play_time;
    hdr.total_kills     = game->total_kills;
    hdr.total_dungeons  = game->total_dungeons;
    hdr.bestiary_count  = game->bestiary_count;
    hdr.recipe_count    = game->recipe_count;
    hdr.resource_count  = game->resource_count;
    fwrite(&hdr, sizeof(hdr), 1, f);

    /* Player data */
    const Entity *p = &game->entities[game->player_id];
    PlayerSave ps;
    memset(&ps, 0, sizeof(ps));
    ps.pos               = p->pos;
    ps.stats             = p->stats;
    ps.cultivation       = p->cultivation;
    ps.hunter_rank       = p->hunter_rank;
    ps.gold              = p->gold;
    ps.experience        = p->experience;
    ps.kills             = p->kills;
    ps.dungeons_cleared  = p->dungeons_cleared;
    ps.has_shadow_power  = game->has_shadow_power;
    ps.can_extract_shadow= p->can_extract_shadow;
    ps.elemental_affinity= p->elemental.affinity;
    ps.active_technique  = p->active_technique;
    ps.num_techniques    = p->num_techniques;
    ps.num_items         = p->num_items;
    ps.bounty            = p->bounty;
    ps.faction           = p->faction;
    memcpy(ps.techniques, p->techniques, sizeof(p->techniques));
    memcpy(ps.inventory,  p->inventory,  sizeof(p->inventory));
    fwrite(&ps, sizeof(ps), 1, f);

    /* Tamed beasts */
    fwrite(game->tamed_beasts, sizeof(TamedBeast), MAX_TAMED_BEASTS, f);

    /* Shadow soldiers */
    fwrite(game->shadows, sizeof(ShadowSoldier), MAX_SHADOWS, f);

    /* Bestiary */
    fwrite(game->bestiary, sizeof(BestiaryEntry), MAX_BESTIARY, f);
    fwrite(&game->bestiary_count, sizeof(int), 1, f);

    /* Recipes (unlock state) */
    for (int i = 0; i < MAX_RECIPES; i++) {
        fwrite(&game->recipes[i].unlocked, sizeof(bool), 1, f);
    }

    /* Factions */
    fwrite(game->factions, sizeof(FactionState), MAX_FACTIONS, f);

    /* World events (completion state) */
    for (int i = 0; i < MAX_WORLD_EVENTS; i++) {
        fwrite(&game->world_events[i].completed, sizeof(bool), 1, f);
    }

    /* Quests */
    fwrite(game->quests, sizeof(Quest), MAX_QUESTS, f);

    /* World seed & time */
    fwrite(&game->world.day_time,  sizeof(float), 1, f);
    fwrite(&game->game_time,       sizeof(float), 1, f);

    fclose(f);
    return true;
}

bool load_game(Game *game)
{
    FILE *f = fopen(SAVE_FILE_PATH, "rb");
    if (!f) return false;

    /* Validate header */
    SaveHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) { fclose(f); return false; }
    if (hdr.magic != SAVE_MAGIC || hdr.version != SAVE_VERSION) {
        fclose(f); return false;
    }

    game->play_time      = hdr.play_time;
    game->total_kills    = hdr.total_kills;
    game->total_dungeons = hdr.total_dungeons;

    /* Player data */
    PlayerSave ps;
    if (fread(&ps, sizeof(ps), 1, f) != 1) { fclose(f); return false; }

    Entity *p = &game->entities[game->player_id];
    p->pos              = ps.pos;
    p->stats            = ps.stats;
    p->cultivation      = ps.cultivation;
    p->hunter_rank      = ps.hunter_rank;
    p->gold             = ps.gold;
    p->experience       = ps.experience;
    p->kills            = ps.kills;
    p->dungeons_cleared = ps.dungeons_cleared;
    game->has_shadow_power = ps.has_shadow_power;
    p->can_extract_shadow  = ps.can_extract_shadow;
    p->elemental.affinity  = ps.elemental_affinity;
    p->active_technique = ps.active_technique;
    p->num_techniques   = ps.num_techniques;
    p->num_items        = ps.num_items;
    p->bounty           = ps.bounty;
    p->faction          = ps.faction;
    memcpy(p->techniques, ps.techniques, sizeof(p->techniques));
    memcpy(p->inventory,  ps.inventory,  sizeof(p->inventory));

    /* Tamed beasts */
    fread(game->tamed_beasts, sizeof(TamedBeast), MAX_TAMED_BEASTS, f);

    /* Shadow soldiers */
    fread(game->shadows, sizeof(ShadowSoldier), MAX_SHADOWS, f);

    /* Bestiary */
    fread(game->bestiary, sizeof(BestiaryEntry), MAX_BESTIARY, f);
    fread(&game->bestiary_count, sizeof(int), 1, f);

    /* Recipe unlock states */
    for (int i = 0; i < MAX_RECIPES; i++) {
        bool unlocked;
        if (fread(&unlocked, sizeof(bool), 1, f) == 1)
            game->recipes[i].unlocked = unlocked;
    }

    /* Factions */
    fread(game->factions, sizeof(FactionState), MAX_FACTIONS, f);

    /* World events */
    for (int i = 0; i < MAX_WORLD_EVENTS; i++) {
        bool completed;
        if (fread(&completed, sizeof(bool), 1, f) == 1)
            game->world_events[i].completed = completed;
    }

    /* Quests */
    fread(game->quests, sizeof(Quest), MAX_QUESTS, f);

    /* World time */
    fread(&game->world.day_time, sizeof(float), 1, f);
    fread(&game->game_time,      sizeof(float), 1, f);

    fclose(f);
    return true;
}
