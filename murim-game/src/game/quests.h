/*
 * quests.h — Quest System
 * Heavenly Demon: Murim Chronicles v4.0
 */
#ifndef QUESTS_H
#define QUESTS_H

#include "../engine/types.h"
#include <stdbool.h>

/* Quest objective types */
typedef enum {
    OBJ_KILL_HOSTILE = 0,  /* kill N hostile NPCs            */
    OBJ_KILL_BEAST,        /* kill N beasts                  */
    OBJ_HARVEST_HERB,      /* gather N herbs                 */
    OBJ_MINE_ORE,          /* mine N ore                     */
    OBJ_ENTER_DUNGEON,     /* enter a dungeon gate           */
    OBJ_CLEAR_DUNGEON,     /* clear a dungeon gate           */
    OBJ_TAME_BEAST,        /* tame a beast                   */
    OBJ_REACH_LEVEL,       /* reach cultivation sub-level N  */
    OBJ_CATCH_FISH,        /* catch N fish                   */
    OBJ_SURVIVE_SECONDS,   /* survive N seconds in the world */
} ObjectiveType;

void quests_init(Game *game);
void quests_update(Game *game, float dt);
void quests_on_kill(Game *game, const Entity *killed);
void quests_on_harvest(Game *game, ResourceType res);
void quests_on_tame(Game *game);
void quests_on_dungeon_enter(Game *game);
void quests_on_dungeon_clear(Game *game);
void quests_on_fish(Game *game);
void quests_draw_log(const Game *game);
void quests_draw_tracker(const Game *game); /* top-right active objective */

#endif /* QUESTS_H */
