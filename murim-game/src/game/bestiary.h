/*
 * bestiary.h — Monster Encyclopedia & Qi Sense (Witcher 3)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef BESTIARY_H
#define BESTIARY_H

#include "../engine/types.h"

void bestiary_init(Game *game);
void bestiary_log_kill(Game *game, const Entity *killed);
void bestiary_update(Game *game, float dt);
void bestiary_draw_qi_sense(const Game *game, float time);
void bestiary_draw_menu(const Game *game);
const BestiaryEntry* bestiary_find(const Game *game, const char *name);

#endif /* BESTIARY_H */
