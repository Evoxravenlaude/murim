/*
 * reputation.h — Faction Reputation & NPC Schedules (RDR2)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef REPUTATION_H
#define REPUTATION_H

#include "../engine/types.h"

void reputation_init(Game *game);
void reputation_modify(Game *game, FactionID faction, int amount);
int  reputation_get(const Game *game, FactionID faction);
bool reputation_is_hostile(const Game *game, FactionID faction);
void reputation_update(Game *game, float dt);
void reputation_draw_hud(const Game *game);

#endif /* REPUTATION_H */
