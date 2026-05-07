/*
 * taming.h — Beast Taming & Shadow Army (Solo Leveling ARISE)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef TAMING_H
#define TAMING_H

#include "../engine/types.h"

void taming_init(Game *game);
bool taming_try_capture(Game *game, int entity_id);
void taming_check_arise_prompt(Game *game, int killed_entity_id);
void taming_do_arise(Game *game);
int  taming_deploy_beast(Game *game, int beast_idx);
void taming_recall_all(Game *game);
void taming_update(Game *game, float dt);
void taming_draw_beast_menu(const Game *game);
void taming_draw_shadow_menu(const Game *game);
void taming_draw_arise_prompt(const Game *game);

#endif /* TAMING_H */
