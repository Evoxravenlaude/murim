/*
 * combat.h — Combat system with martial arts techniques
 */
#ifndef COMBAT_H
#define COMBAT_H

#include "../engine/types.h"

/* Initialize techniques for the player */
void combat_init_player_techniques(Entity *player);

/* Process player attack input */
void combat_player_attack(Game *game);

/* Process player qi technique */
void combat_player_technique(Game *game);

/* Apply damage from attacker to defender */
void combat_apply_damage(Game *game, Entity *attacker, Entity *defender);

/* Update all combat timers */
void combat_update(Game *game, float dt);

/* Check if entity a is in range to hit entity b */
bool combat_in_range(const Entity *a, const Entity *b, float range);

/* Get technique name string */
const char *combat_technique_name(TechniqueType type);

/* Use first available consumable item */
void combat_use_item(Game *game);

#endif /* COMBAT_H */
