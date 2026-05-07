/*
 * mounts.h — Beast Mounting & Mounted Combat
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef MOUNTS_H
#define MOUNTS_H

#include "../engine/types.h"

void mounts_init(Entity *player);
bool mounts_try_mount(Game *game);
void mounts_dismount(Game *game);
void mounts_update(Game *game, float dt);
void mounts_draw_hud(const Game *game);

#endif /* MOUNTS_H */
