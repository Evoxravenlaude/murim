/*
 * quests.c — Quest System with 10 built-in quests
 * Heavenly Demon: Murim Chronicles v4.0
 */
#include "quests.h"
#include "../ui/system_ui.h"
#include "alchemy.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ─── Quest definitions ───────────────────────────────── */
typedef struct {
    const char    *title;
    const char    *description;
    const char    *objective_text;
    ObjectiveType  obj_type;
    int            obj_target;
    int            reward_gold;
    int            reward_exp;
    ItemType       reward_item;
    bool           is_daily;
} QuestDef;

static const QuestDef QUEST_DEFS[] = {
    {
        "First Blood",
        "Defeat 5 hostile cultivators in the world.",
        "Kill hostile cultivators: 0/5",
        OBJ_KILL_HOSTILE, 5, 100, 200, ITEM_HEALING_PILL, false
    },
    {
        "Beast Hunter",
        "Prove yourself by slaying 8 spirit beasts.",
        "Kill spirit beasts: 0/8",
        OBJ_KILL_BEAST, 8, 150, 300, ITEM_QI_PILL, false
    },
    {
        "Herbalist's Path",
        "Gather 6 red herbs for the sect alchemist.",
        "Gather Red Herbs: 0/6",
        OBJ_HARVEST_HERB, 6, 80, 150, ITEM_BUFF_POTION_ATK, true
    },
    {
        "Iron Will",
        "Mine 4 iron ore deposits.",
        "Mine iron ore: 0/4",
        OBJ_MINE_ORE, 4, 60, 100, ITEM_SPIRIT_STONE, true
    },
    {
        "Into the Abyss",
        "Dare to enter a Dungeon Gate.",
        "Enter a Dungeon Gate: 0/1",
        OBJ_ENTER_DUNGEON, 1, 200, 500, ITEM_GATE_KEY, false
    },
    {
        "Gate Breaker",
        "Clear a Dungeon Gate completely.",
        "Clear Dungeon Gates: 0/1",
        OBJ_CLEAR_DUNGEON, 1, 500, 1000, ITEM_EVOLUTION_STONE, false
    },
    {
        "Beast Tamer Initiate",
        "Capture and tame your first spirit beast.",
        "Tame a beast: 0/1",
        OBJ_TAME_BEAST, 1, 300, 600, ITEM_CAPTURE_ORB, false
    },
    {
        "Foundation Seeker",
        "Cultivate to the Foundation Realm.",
        "Reach Foundation Establishment",
        OBJ_REACH_LEVEL, (int)REALM_FOUNDATION, 400, 800, ITEM_QI_PILL, false
    },
    {
        "Fisher of Souls",
        "Catch 3 fish from the rivers of the world.",
        "Catch fish: 0/3",
        OBJ_CATCH_FISH, 3, 120, 200, ITEM_HEALING_PILL, true
    },
    {
        "Survivor",
        "Survive for 5 minutes without dying.",
        "Survive 300 seconds",
        OBJ_SURVIVE_SECONDS, 300, 250, 400, ITEM_SPIRIT_STONE, false
    },
};
#define NUM_QUESTS_DEFS (int)(sizeof(QUEST_DEFS)/sizeof(QUEST_DEFS[0]))

/* runtime counters that survive across the update loop */
static float s_survive_timer = 0;

static void assign_quest(Game *game, int quest_slot, int def_idx)
{
    if (quest_slot < 0 || quest_slot >= MAX_QUESTS) return;
    if (def_idx < 0 || def_idx >= NUM_QUESTS_DEFS) return;

    const QuestDef *d = &QUEST_DEFS[def_idx];
    Quest *q = &game->quests[quest_slot];

    q->active             = true;
    q->state              = QUEST_ACTIVE;
    q->objective_current  = 0;
    q->objective_target   = d->obj_target;
    q->reward_gold        = d->reward_gold;
    q->reward_exp         = d->reward_exp;
    q->reward_item        = d->reward_item;
    q->is_daily           = d->is_daily;
    strncpy(q->title,          d->title,          sizeof(q->title)-1);
    strncpy(q->description,    d->description,    sizeof(q->description)-1);
    strncpy(q->objective_text, d->objective_text, sizeof(q->objective_text)-1);
}

void quests_init(Game *game)
{
    memset(game->quests, 0, sizeof(game->quests));
    s_survive_timer = 0;

    /* Assign the first 5 quests at game start */
    for (int i = 0; i < 5 && i < NUM_QUESTS_DEFS; i++)
        assign_quest(game, i, i);
}

/* ─── Completion helper ───────────────────────────────── */
static void complete_quest(Game *game, int qi)
{
    Quest *q = &game->quests[qi];
    if (q->state != QUEST_ACTIVE) return;

    q->state = QUEST_COMPLETE;
    Entity *player = &game->entities[game->player_id];
    player->gold       += q->reward_gold;
    player->experience += q->reward_exp;

    /* Add reward item */
    if (q->reward_item != ITEM_NONE) {
        for (int i = 0; i < MAX_ITEMS; i++) {
            if (player->inventory[i].type == ITEM_NONE) {
                player->inventory[i].type     = q->reward_item;
                player->inventory[i].quantity = 1;
                player->inventory[i].rarity   = RARITY_UNCOMMON;
                player->num_items++;
                break;
            }
        }
    }

    system_notify_quest_complete(game, q->title);

    /* Unlock next quest if available */
    for (int d = 0; d < NUM_QUESTS_DEFS; d++) {
        bool already = false;
        for (int j = 0; j < MAX_QUESTS; j++) {
            if (game->quests[j].active &&
                strncmp(game->quests[j].title, QUEST_DEFS[d].title, 32) == 0) {
                already = true; break;
            }
        }
        if (!already) {
            /* find empty slot */
            for (int j = 0; j < MAX_QUESTS; j++) {
                if (!game->quests[j].active) {
                    assign_quest(game, j, d);
                    goto next_unlock;
                }
            }
        }
    }
    next_unlock:;
}

static void advance_quest(Game *game, ObjectiveType obj_type, int amount)
{
    for (int i = 0; i < MAX_QUESTS; i++) {
        Quest *q = &game->quests[i];
        if (!q->active || q->state != QUEST_ACTIVE) continue;

        /* Map quest definition to objective type by checking title */
        const QuestDef *def = NULL;
        for (int d = 0; d < NUM_QUESTS_DEFS; d++) {
            if (strncmp(QUEST_DEFS[d].title, q->title, 32) == 0) {
                def = &QUEST_DEFS[d]; break;
            }
        }
        if (!def || def->obj_type != obj_type) continue;

        q->objective_current += amount;
        if (q->objective_current >= q->objective_target) {
            q->objective_current = q->objective_target;
            complete_quest(game, i);
        }
    }
}

void quests_on_kill(Game *game, const Entity *killed)
{
    if (killed->type == ENTITY_NPC_HOSTILE || killed->type == ENTITY_DUNGEON_MONSTER)
        advance_quest(game, OBJ_KILL_HOSTILE, 1);
    if (killed->type == ENTITY_BEAST)
        advance_quest(game, OBJ_KILL_BEAST, 1);
}

void quests_on_harvest(Game *game, ResourceType res)
{
    if (res == RESOURCE_HERB_RED || res == RESOURCE_HERB_BLUE || res == RESOURCE_HERB_GOLD)
        advance_quest(game, OBJ_HARVEST_HERB, 1);
    if (res == RESOURCE_ORE_IRON || res == RESOURCE_ORE_SPIRIT)
        advance_quest(game, OBJ_MINE_ORE, 1);
}

void quests_on_tame(Game *game)
{
    advance_quest(game, OBJ_TAME_BEAST, 1);
}

void quests_on_dungeon_enter(Game *game)
{
    advance_quest(game, OBJ_ENTER_DUNGEON, 1);
}

void quests_on_dungeon_clear(Game *game)
{
    advance_quest(game, OBJ_CLEAR_DUNGEON, 1);
}

void quests_on_fish(Game *game)
{
    advance_quest(game, OBJ_CATCH_FISH, 1);
}

void quests_update(Game *game, float dt)
{
    /* Survival quest */
    Entity *player = &game->entities[game->player_id];
    if (player->stats.hp > 0) {
        s_survive_timer += dt;
        for (int i = 0; i < MAX_QUESTS; i++) {
            Quest *q = &game->quests[i];
            if (!q->active || q->state != QUEST_ACTIVE) continue;
            if (strncmp(q->title, "Survivor", 8) == 0) {
                q->objective_current = (int)s_survive_timer;
                if (q->objective_current >= q->objective_target)
                    complete_quest(game, i);
            }
        }
    }

    /* Realm quest */
    CultivationRealm realm = player->cultivation.realm;
    for (int i = 0; i < MAX_QUESTS; i++) {
        Quest *q = &game->quests[i];
        if (!q->active || q->state != QUEST_ACTIVE) continue;
        if (strncmp(q->title, "Foundation Seeker", 17) == 0) {
            q->objective_current = (int)realm;
            if ((int)realm >= q->objective_target)
                complete_quest(game, i);
        }
    }
}

/* ─── Draw: quest log screen ──────────────────────────── */
void quests_draw_log(const Game *game)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0,0,0,180});

    int pw = 740, ph = 520;
    int px = SCREEN_WIDTH/2 - pw/2, py = SCREEN_HEIGHT/2 - ph/2;

    /* Panel */
    DrawRectangle(px, py, pw, ph, (Color){8,12,25,230});
    DrawRectangle(px, py, pw, 2, (Color){60,120,255,200});
    DrawRectangleLinesEx((Rectangle){px,py,pw,ph}, 1.5f, (Color){60,120,255,200});

    DrawText("[ QUEST LOG ]", px+30, py+18, 22, (Color){220,240,255,255});
    DrawLine(px+30, py+48, px+pw-30, py+48, (Color){60,120,255,150});

    int y = py + 60;
    int active_shown = 0, done_shown = 0;

    DrawText("■ ACTIVE", px+40, y, 14, (Color){100,200,255,255}); y += 22;

    for (int i = 0; i < MAX_QUESTS && active_shown < 8; i++) {
        const Quest *q = &game->quests[i];
        if (!q->active || q->state != QUEST_ACTIVE) continue;
        char buf[128];
        snprintf(buf, sizeof(buf), "%s", q->title);
        DrawText(buf, px+50, y, 13, (Color){220,220,200,255});
        snprintf(buf, sizeof(buf), "  %s  [%d/%d]",
                 q->objective_text, q->objective_current, q->objective_target);
        DrawText(buf, px+60, y+16, 10, (Color){120,160,210,200});
        snprintf(buf, sizeof(buf), "  Reward: %dG  +%dEXP", q->reward_gold, q->reward_exp);
        DrawText(buf, px+60, y+28, 10, (Color){255,215,0,180});
        y += 44;
        active_shown++;
    }

    if (active_shown == 0)
        DrawText("  No active quests.", px+50, y, 12, (Color){100,100,100,255});

    y += 14;
    DrawText("■ COMPLETED", px+40, y, 14, (Color){80,220,120,255}); y += 22;

    for (int i = 0; i < MAX_QUESTS && done_shown < 4; i++) {
        const Quest *q = &game->quests[i];
        if (!q->active || q->state != QUEST_COMPLETE) continue;
        char buf[64];
        snprintf(buf, sizeof(buf), "  ✓ %s", q->title);
        DrawText(buf, px+50, y, 11, (Color){80,200,100,180});
        y += 18;
        done_shown++;
    }

    DrawText("[ ESC / J ] Close", px+pw/2-65, py+ph-28, 12, (Color){100,180,255,200});
}

/* ─── Draw: HUD tracker (top right, shows first active quest) */
void quests_draw_tracker(const Game *game)
{
    for (int i = 0; i < MAX_QUESTS; i++) {
        const Quest *q = &game->quests[i];
        if (!q->active || q->state != QUEST_ACTIVE) continue;

        int bx = SCREEN_WIDTH - 280, by = 200;
        DrawRectangle(bx-4, by-4, 264, 60, (Color){5,8,20,180});
        DrawRectangle(bx-4, by-4, 2, 60, (Color){60,120,255,200});

        char title[48];
        snprintf(title, sizeof(title), "Quest: %s", q->title);
        DrawText(title, bx, by+2, 11, (Color){220,240,255,220});

        char prog[64];
        snprintf(prog, sizeof(prog), "%s  [%d/%d]",
                 q->objective_text, q->objective_current, q->objective_target);
        DrawText(prog, bx, by+18, 9, (Color){120,160,210,200});

        /* Progress bar */
        float ratio = (q->objective_target > 0)
                    ? (float)q->objective_current / q->objective_target : 0;
        DrawRectangle(bx, by+34, 240, 5, (Color){20,20,40,200});
        DrawRectangle(bx, by+34, (int)(240*ratio), 5, (Color){60,200,120,255});

        break; /* Only show first active quest */
    }
}
