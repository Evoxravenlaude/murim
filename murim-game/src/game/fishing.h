/*
 * fishing.h — Fishing Minigame (Stardew / Zelda style)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef FISHING_H
#define FISHING_H

#include "../engine/types.h"

void fishing_init(Entity *player);
bool fishing_start(Game *game);
void fishing_update(Game *game, float dt);
void fishing_cancel(Game *game);
void fishing_draw(const Game *game);

#endif /* FISHING_H */
