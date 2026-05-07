/*
 * elements.h — Elemental Affinity & Reaction System (Genshin Impact)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "../engine/types.h"

void elements_init_entity(Entity *e, ElementType affinity);
ReactionType elements_check_reaction(ElementType a, ElementType b);
float elements_reaction_multiplier(ReactionType r);
void elements_apply(Game *game, Entity *target, ElementType element, int base_damage);
void elements_update(Game *game, float dt);
void elements_draw_aura(const Entity *e, float time);

#endif /* ELEMENTS_H */
