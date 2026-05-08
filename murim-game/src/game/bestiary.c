/*
 * bestiary.c — Monster Encyclopedia & Qi Sense (Witcher 3)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Auto-logs enemies on kill. Qi Sense highlights interactables.
 */
#include "bestiary.h"
#include "../ui/system_ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ─── Predefined bestiary templates (50 creatures) ────── */
static const struct {
    const char *name;
    const char *lore;
    EntityType  type;
    ElementType weakness;
    ElementType resistance;
} BESTIARY_TEMPLATES[] = {
    /* ── Hostile Cultivators ─── */
    { "Rogue Disciple",      "Fallen cultivators who turned to banditry after losing their path.", ENTITY_NPC_HOSTILE, ELEMENT_LIGHTNING, ELEMENT_NONE },
    { "Dark Sect Member",    "Devotees of the forbidden Dark Sect, cultivating cursed qi.", ENTITY_NPC_HOSTILE, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Shadow Assassin",     "Silent killers trained in the art of shadow-step execution.", ENTITY_NPC_HOSTILE, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Demon Cultivator",    "Cultivators who consumed demonic cores, twisting their qi.", ENTITY_NPC_HOSTILE, ELEMENT_ICE, ELEMENT_FIRE },
    { "Blood Sect Disciple", "Disciples who sacrificed their meridians for raw power.", ENTITY_NPC_HOSTILE, ELEMENT_WATER, ELEMENT_FIRE },
    { "Bandit",              "Common criminals preying on lone cultivators.", ENTITY_NPC_HOSTILE, ELEMENT_LIGHTNING, ELEMENT_NONE },
    { "Fallen Warrior",      "Once-proud warriors corrupted by greed and dark artifacts.", ENTITY_NPC_HOSTILE, ELEMENT_FIRE, ELEMENT_EARTH },
    { "Corrupt Elder",       "A sect elder whose long cultivation warped his mind.", ENTITY_NPC_HOSTILE, ELEMENT_SHADOW, ELEMENT_ICE },
    { "Sect Traitor",        "A disciple who sold cultivation secrets to enemies.", ENTITY_NPC_HOSTILE, ELEMENT_LIGHTNING, ELEMENT_WIND },
    { "Iron Fist Thug",      "Street brawlers trained in brutal unarmed combat.", ENTITY_NPC_HOSTILE, ELEMENT_WIND, ELEMENT_EARTH },
    { "Cursed Swordsman",    "A blade cultivator bound to a cursed spirit sword.", ENTITY_NPC_HOSTILE, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Poison Needle User",  "Masters of hidden weapons coated in spirit venom.", ENTITY_NPC_HOSTILE, ELEMENT_FIRE, ELEMENT_ICE },
    { "Demonic Archer",      "Long-range cultivators who fire qi-infused arrows.", ENTITY_NPC_HOSTILE, ELEMENT_LIGHTNING, ELEMENT_WIND },
    { "Blood Moon Cultist",  "Fanatics who worship the Blood Moon and seek chaos.", ENTITY_NPC_HOSTILE, ELEMENT_WATER, ELEMENT_SHADOW },
    { "Grave Robber",        "Thieves who plunder ancient tombs for artifacts.", ENTITY_NPC_HOSTILE, ELEMENT_LIGHTNING, ELEMENT_EARTH },
    /* ── Beasts ─── */
    { "Spirit Wolf",         "Wolves infused with ancient forest qi, eyes glow silver.", ENTITY_BEAST, ELEMENT_FIRE, ELEMENT_WIND },
    { "Iron Boar",           "Mountain boars whose hides have calcified to iron.", ENTITY_BEAST, ELEMENT_LIGHTNING, ELEMENT_EARTH },
    { "Flame Fox",           "Foxes that have absorbed volcanic qi and breathe embers.", ENTITY_BEAST, ELEMENT_WATER, ELEMENT_FIRE },
    { "Jade Serpent",        "Venomous serpents whose scales glow with jade meridians.", ENTITY_BEAST, ELEMENT_ICE, ELEMENT_EARTH },
    { "Thunder Hawk",        "Eagles that nest in storm clouds and strike with lightning.", ENTITY_BEAST, ELEMENT_EARTH, ELEMENT_LIGHTNING },
    { "Stone Bear",          "Ancient bears whose fur has crystallized into stone armor.", ENTITY_BEAST, ELEMENT_WATER, ELEMENT_EARTH },
    { "Shadow Cat",          "Felines that phase between light and shadow at will.", ENTITY_BEAST, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Wind Tiger",          "Tigers that move at gale speed, leaving afterimages.", ENTITY_BEAST, ELEMENT_EARTH, ELEMENT_WIND },
    { "Frost Deer",          "Deer from frozen peaks whose breath turns air to ice.", ENTITY_BEAST, ELEMENT_FIRE, ELEMENT_ICE },
    { "Lava Salamander",     "Amphibians born in lava vents with molten-rock skin.", ENTITY_BEAST, ELEMENT_WATER, ELEMENT_FIRE },
    { "Storm Crane",         "Sacred cranes that control weather and heal nearby beasts.", ENTITY_BEAST, ELEMENT_EARTH, ELEMENT_LIGHTNING },
    { "Tide Dragon",         "Young dragon-kin that command water currents.", ENTITY_BEAST, ELEMENT_LIGHTNING, ELEMENT_WATER },
    { "Void Spider",         "Spiders that weave webs of shadow qi to trap prey.", ENTITY_BEAST, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Mud Tortoise",        "Massive tortoises with qi-hardened shells.", ENTITY_BEAST, ELEMENT_LIGHTNING, ELEMENT_EARTH },
    { "Celestial Deer",      "Rare deer whose antlers channel heaven's qi.", ENTITY_BEAST, ELEMENT_SHADOW, ELEMENT_WIND },
    { "Blood Bat",           "Bats that drink cultivator qi instead of blood.", ENTITY_BEAST, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Glacier Bear",        "Polar bears that hibernate inside glaciers for centuries.", ENTITY_BEAST, ELEMENT_FIRE, ELEMENT_ICE },
    { "Phantom Snake",       "Translucent serpents invisible to normal sight.", ENTITY_BEAST, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Earth Rhino",         "Rhinoceroses whose horns can shatter stone mountains.", ENTITY_BEAST, ELEMENT_WIND, ELEMENT_EARTH },
    { "Heaven Butterfly",    "Butterflies whose wing dust induces cultivation visions.", ENTITY_BEAST, ELEMENT_SHADOW, ELEMENT_WIND },
    { "Magma Scorpion",      "Desert scorpions with stingers dripping with magma.", ENTITY_BEAST, ELEMENT_WATER, ELEMENT_FIRE },
    /* ── Bosses / Dungeon ─── */
    { "Gate Guardian",       "A powerful construct that guards dungeon gates.", ENTITY_DUNGEON_BOSS, ELEMENT_SHADOW, ELEMENT_EARTH },
    { "Dungeon Creature",    "A twisted beast warped by the gate's dimensional energy.", ENTITY_DUNGEON_MONSTER, ELEMENT_FIRE, ELEMENT_NONE },
    { "Sect Elder Guardian", "An ancient protector of a collapsed sect's final hall.", ENTITY_BOSS, ELEMENT_LIGHTNING, ELEMENT_ICE },
    { "Dragon Corpse King",  "The undead remains of a thousand-year dragon.", ENTITY_BOSS, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Heavenly Tribulation Beast","A beast descended from tribulation lightning clouds.", ENTITY_BOSS, ELEMENT_EARTH, ELEMENT_LIGHTNING },
    { "Blood Moon Apostle",  "The chosen avatar of the Blood Moon cult's god.", ENTITY_BOSS, ELEMENT_SHADOW, ELEMENT_WATER },
    { "Demon King's Shade",  "A shadow left by the Demon King after his sealing.", ENTITY_BOSS, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Ancient Stone Golem", "A golem left behind by a long-dead formation master.", ENTITY_BOSS, ELEMENT_LIGHTNING, ELEMENT_EARTH },
    { "Void Devourer",       "An entity that consumes dimensional rifts to grow.", ENTITY_BOSS, ELEMENT_FIRE, ELEMENT_SHADOW },
    { "Nine-Tailed Revenant","The spirit of a thousand-year fox that seeks vengeance.", ENTITY_BOSS, ELEMENT_FIRE, ELEMENT_WIND },
};
#define NUM_TEMPLATES (int)(sizeof(BESTIARY_TEMPLATES)/sizeof(BESTIARY_TEMPLATES[0]))

void bestiary_init(Game *game)
{
    memset(game->bestiary, 0, sizeof(game->bestiary));
    game->bestiary_count = 0;
}

static int bestiary_find_or_create(Game *game, const char *name)
{
    /* Search existing */
    for (int i = 0; i < game->bestiary_count; i++) {
        if (game->bestiary[i].discovered && strcmp(game->bestiary[i].name, name) == 0)
            return i;
    }

    /* Create new entry */
    if (game->bestiary_count >= MAX_BESTIARY) return -1;

    int idx = game->bestiary_count++;
    BestiaryEntry *entry = &game->bestiary[idx];
    entry->discovered = true;
    strncpy(entry->name, name, sizeof(entry->name) - 1);

    /* Fill from templates */
    for (int t = 0; t < NUM_TEMPLATES; t++) {
        if (strcmp(BESTIARY_TEMPLATES[t].name, name) == 0) {
            strncpy(entry->lore, BESTIARY_TEMPLATES[t].lore, sizeof(entry->lore) - 1);
            entry->entity_type = BESTIARY_TEMPLATES[t].type;
            entry->weakness = BESTIARY_TEMPLATES[t].weakness;
            entry->resistance = BESTIARY_TEMPLATES[t].resistance;
            break;
        }
    }

    return idx;
}

void bestiary_log_kill(Game *game, const Entity *killed)
{
    if (!killed->name) return;

    int idx = bestiary_find_or_create(game, killed->name);
    if (idx < 0) return;

    BestiaryEntry *entry = &game->bestiary[idx];
    entry->kill_count++;
    entry->avg_hp = (entry->avg_hp * (entry->kill_count - 1) + killed->stats.max_hp) / entry->kill_count;

    if (entry->kill_count >= 10 && !entry->fully_studied) {
        entry->fully_studied = true;
        system_notify(game, NOTIFY_SUCCESS, "[ Bestiary Complete ]", entry->name);
    }
}

void bestiary_update(Game *game, float dt)
{
    /* Qi Sense pulse animation */
    if (game->qi_sense_global) {
        game->qi_sense_pulse += dt * 2.0f;
        if (game->qi_sense_pulse > 6.2831f) game->qi_sense_pulse -= 6.2831f;
    }
}

void bestiary_draw_qi_sense(const Game *game, float time)
{
    if (!game->qi_sense_global) return;

    Entity *player = (Entity*)&game->entities[game->player_id];
    float pulse_r = 80.0f + sinf(time * 2.0f) * 20.0f;

    /* Qi Sense pulse ring */
    DrawCircleLines((int)player->pos.x, (int)player->pos.y, pulse_r,
                    (Color){ 80, 160, 255, 120 });
    DrawCircleLines((int)player->pos.x, (int)player->pos.y, pulse_r * 0.8f,
                    (Color){ 80, 160, 255, 60 });

    /* Highlight nearby interactables */
    for (int i = 0; i < MAX_ENTITIES; i++) {
        const Entity *e = &game->entities[i];
        if (!e->active || i == game->player_id) continue;

        float dx = e->pos.x - player->pos.x;
        float dy = e->pos.y - player->pos.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist > 200.0f) continue;

        Color highlight;
        if (e->type == ENTITY_NPC_HOSTILE || e->type == ENTITY_BEAST || e->type == ENTITY_BOSS) {
            highlight = (Color){ 255, 50, 50, 100 }; /* Red for enemies */
        } else if (e->type == ENTITY_NPC_MERCHANT) {
            highlight = (Color){ 255, 215, 0, 100 }; /* Gold for merchants */
        } else {
            highlight = (Color){ 80, 200, 255, 80 }; /* Blue for friendly */
        }

        float glow = sinf(time * 4.0f + i) * 0.3f + 0.7f;
        highlight.a = (unsigned char)(highlight.a * glow);
        DrawCircle((int)e->pos.x, (int)e->pos.y, 20.0f, highlight);

        /* Show name and weakness if bestiary entry exists */
        if (e->name) {
            const BestiaryEntry *entry = bestiary_find(game, e->name);
            if (entry && entry->fully_studied) {
                char buf[48];
                snprintf(buf, sizeof(buf), "Weak: %s", element_name(entry->weakness));
                int tw = MeasureText(buf, 8);
                DrawText(buf, (int)e->pos.x - tw/2, (int)e->pos.y - 35, 8,
                         element_color(entry->weakness));
            }
        }
    }

    /* Highlight resource nodes */
    for (int i = 0; i < game->resource_count; i++) {
        const ResourceNode *node = &game->resource_nodes[i];
        if (!node->active) continue;

        float dx = node->pos.x - player->pos.x;
        float dy = node->pos.y - player->pos.y;
        if (dx*dx + dy*dy > 200.0f*200.0f) continue;

        float glow = sinf(time * 3.0f + i * 0.5f) * 0.4f + 0.6f;
        DrawCircle((int)node->pos.x, (int)node->pos.y, 12.0f * glow,
                   (Color){ 100, 255, 150, (unsigned char)(120 * glow) });
    }

    /* Screen overlay tint */
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 30, 60, 120, 40 });
}

void bestiary_draw_menu(const Game *game)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 180 });

    int pw = 700, ph = 500;
    int px = SCREEN_WIDTH/2 - pw/2, py = SCREEN_HEIGHT/2 - ph/2;

    system_ui_draw_panel(px, py, pw, ph, 1.0f);
    DrawText("[ BESTIARY ]", px + 30, py + 20, 24, (Color){220,240,255,255});
    DrawLine(px+30, py+50, px+pw-30, py+50, (Color){60,120,255,200});

    int y = py + 65;
    int displayed = 0;
    for (int i = 0; i < game->bestiary_count && displayed < 12; i++) {
        const BestiaryEntry *e = &game->bestiary[i];
        if (!e->discovered) continue;

        char buf[128];
        Color nc = e->fully_studied ? (Color){80,255,120,255} : (Color){200,200,200,255};
        snprintf(buf, sizeof(buf), "%s  [Kills: %d]%s", e->name, e->kill_count,
                 e->fully_studied ? " ★" : "");
        DrawText(buf, px + 40, y, 14, nc);

        if (e->fully_studied) {
            snprintf(buf, sizeof(buf), "  Weak: %s  |  Resist: %s  |  Avg HP: %.0f",
                     element_name(e->weakness), element_name(e->resistance), e->avg_hp);
            DrawText(buf, px + 60, y + 16, 10, (Color){150,180,220,200});
            DrawText(e->lore, px + 60, y + 28, 9, (Color){120,140,170,180});
            y += 44;
        } else {
            DrawText("  Kill more to reveal full info...", px + 60, y + 16, 10, (Color){100,100,100,200});
            y += 32;
        }
        displayed++;
    }

    if (game->bestiary_count == 0) {
        DrawText("No creatures discovered yet.", px + 40, y, 14, (Color){100,100,100,255});
    }

    DrawText("[ ESC ] Close", px + pw/2 - 40, py + ph - 30, 14, (Color){100,180,255,200});
}

const BestiaryEntry* bestiary_find(const Game *game, const char *name)
{
    if (!name) return NULL;
    for (int i = 0; i < game->bestiary_count; i++) {
        if (game->bestiary[i].discovered && strcmp(game->bestiary[i].name, name) == 0)
            return &game->bestiary[i];
    }
    return NULL;
}
