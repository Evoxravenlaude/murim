/*
 * events.h — Dynamic World Events (open world feel)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef EVENTS_H
#define EVENTS_H

#include "../engine/types.h"

void events_init(Game *game);
void events_update(Game *game, float dt);
void events_spawn(Game *game, WorldEventType type, Vec2 pos);
void events_draw(const Game *game);

#endif /* EVENTS_H */
