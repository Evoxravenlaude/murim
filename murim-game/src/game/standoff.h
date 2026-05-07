/*
 * standoff.h — Cinematic 1vN Standoff Duels (Ghost of Tsushima)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef STANDOFF_H
#define STANDOFF_H

#include "../engine/types.h"

void standoff_initiate(Game *game);
void standoff_player_strike(Game *game);
void standoff_update(Game *game, float dt);
void standoff_draw(const Game *game);
bool standoff_is_active(const Game *game);

#endif /* STANDOFF_H */
