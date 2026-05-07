/*
 * bounty.h — Karma & Sect Bounty System (GTA5)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef BOUNTY_H
#define BOUNTY_H

#include "../engine/types.h"

void bounty_init(Entity *player);
void bounty_add_karma(Game *game, int amount);
void bounty_on_kill(Game *game, const Entity *killed);
void bounty_update(Game *game, float dt);
void bounty_draw_stars(const Game *game);

#endif /* BOUNTY_H */
