/*
 * exploration.h — Climbing & Swimming Traversal (Zelda TotK)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef EXPLORATION_H
#define EXPLORATION_H

#include "../engine/types.h"

void exploration_init(Entity *player);
void exploration_update(Game *game, float dt);
bool exploration_is_near_climbable(const Game *game, Vec2 pos);
void exploration_draw_hud(const Game *game);

#endif /* EXPLORATION_H */
