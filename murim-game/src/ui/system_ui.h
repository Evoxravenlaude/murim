/*
 * system_ui.h — Solo Leveling-style System Interface
 * Heavenly Demon: Murim Chronicles v2.0
 *
 * The iconic blue holographic windows, notifications, and status displays
 */
#ifndef SYSTEM_UI_H
#define SYSTEM_UI_H

#include "../engine/types.h"

/* Initialize the system UI */
void system_ui_init(Game *game);

/* Push a notification to the queue */
void system_notify(Game *game, NotifyPriority priority,
                   const char *title, const char *body);

/* Convenience notification helpers */
void system_notify_level_up(Game *game, int new_level);
void system_notify_rank_up(Game *game, HunterRank new_rank);
void system_notify_quest_complete(Game *game, const char *quest_name);
void system_notify_item_drop(Game *game, const char *item_name, ItemRarity rarity);
void system_notify_shadow_arise(Game *game, const char *shadow_name);
void system_notify_realm_breakthrough(Game *game, CultivationRealm realm);
void system_notify_beast_captured(Game *game, const char *beast_name);

/* Update notification timers */
void system_ui_update(Game *game, float dt);

/* Draw the Solo Leveling-style status window */
void system_ui_draw_status_window(const Game *game);

/* Draw active notifications (floating system messages) */
void system_ui_draw_notifications(const Game *game);

/* Draw the level-up animation (column of light + system window) */
void system_ui_draw_level_up(const Game *game);

/* Draw the hunter rank badge on HUD */
void system_ui_draw_rank_badge(const Game *game, int x, int y);

/* Draw Solo Leveling-style blue panel */
void system_ui_draw_panel(int x, int y, int w, int h, float alpha);

/* Draw a glowing bordered box */
void system_ui_draw_glow_box(int x, int y, int w, int h, Color color, float glow_strength);

#endif /* SYSTEM_UI_H */
