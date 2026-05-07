/*
 * hud.h — Heads-Up Display (health, qi, cultivation, minimap)
 */
#ifndef HUD_H
#define HUD_H

#include "../engine/types.h"

/* Draw the full HUD overlay */
void hud_draw(const Game *game);

/* Draw the title screen */
void hud_draw_title(const Game *game);

/* Draw the pause menu */
void hud_draw_pause(const Game *game);

/* Draw the stats/inventory screen */
void hud_draw_stats(const Game *game);

/* Draw the cultivation meditation screen */
void hud_draw_cultivation(const Game *game);

/* Draw the game over screen */
void hud_draw_game_over(const Game *game);

/* Draw dialogue box */
void hud_draw_dialogue(const Game *game);

/* Draw minimap */
void hud_draw_minimap(const Game *game);

#endif /* HUD_H */
