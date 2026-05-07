/*
 * alchemy.h — Crafting, Potions & Weapon Oils (Witcher 3)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef ALCHEMY_H
#define ALCHEMY_H

#include "../engine/types.h"

void alchemy_init(Game *game);
void alchemy_add_resource(Game *game, ResourceType type, int count);
int alchemy_get_resource(const Game *game, ResourceType type);
bool alchemy_can_craft(const Game *game, int recipe_idx);
void alchemy_craft(Game *game, int recipe_idx);
void alchemy_draw_menu(const Game *game);
void alchemy_spawn_resource_nodes(Game *game);
void alchemy_update_nodes(Game *game, float dt);
void alchemy_draw_nodes(const Game *game, float time);

#endif /* ALCHEMY_H */
