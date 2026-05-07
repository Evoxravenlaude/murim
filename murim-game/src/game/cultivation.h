/*
 * cultivation.h — Cultivation/leveling system
 */
#ifndef CULTIVATION_H
#define CULTIVATION_H

#include "../engine/types.h"

/* Initialize cultivation state for an entity */
void cultivation_init(Entity *entity, CultivationRealm starting_realm);

/* Process cultivation (meditation) for player */
void cultivation_meditate(Game *game, float dt);

/* Check and process realm breakthrough */
bool cultivation_check_breakthrough(Game *game, Entity *entity);

/* Get realm name string */
const char *cultivation_realm_name(CultivationRealm realm);

/* Get qi required for next sub-level */
float cultivation_qi_required(CultivationRealm realm, int sub_level);

/* Apply realm bonuses to stats */
void cultivation_apply_bonuses(Entity *entity);

/* Update cultivation state */
void cultivation_update(Game *game, float dt);

#endif /* CULTIVATION_H */
