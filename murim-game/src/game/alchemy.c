/*
 * alchemy.c — Crafting, Potions & Weapon Oils (Witcher 3 / Zelda)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#include "alchemy.h"
#include "audio_sys.h"
#include "quests.h"
#include "../ui/system_ui.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Player resource inventory (simple counts) ───────── */
static int player_resources[RESOURCE_COUNT] = {0};

/* ─── Recipe definitions ──────────────────────────────── */
static void def_recipe(Game *g, int *idx, const char *name, RecipeCategory cat,
    ItemType result, ItemRarity rar, const char *desc,
    ResourceType i0, int c0, ResourceType i1, int c1,
    ResourceType i2, int c2, bool unlocked)
{
    if (*idx >= MAX_RECIPES) return;
    AlchemyRecipe *r = &g->recipes[*idx];
    r->unlocked = unlocked;
    strncpy(r->name, name, 31);
    r->category = cat;
    r->result = result;
    r->result_rarity = rar;
    strncpy(r->description, desc, 63);
    r->num_ingredients = 0;
    if (i0 != RESOURCE_NONE) { r->ingredients[r->num_ingredients] = i0; r->ingredient_counts[r->num_ingredients++] = c0; }
    if (i1 != RESOURCE_NONE) { r->ingredients[r->num_ingredients] = i1; r->ingredient_counts[r->num_ingredients++] = c1; }
    if (i2 != RESOURCE_NONE) { r->ingredients[r->num_ingredients] = i2; r->ingredient_counts[r->num_ingredients++] = c2; }
    (*idx)++;
}

static void register_recipes(Game *game) {
    int idx = 0;
    /* ─── POTIONS ─────────────────────────────────── */
    def_recipe(game, &idx, "Minor Healing Elixir",  RECIPE_POTION, ITEM_HEALING_PILL, RARITY_COMMON,   "Restores 50 HP",          RESOURCE_HERB_RED,  2, RESOURCE_NONE,0, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Healing Elixir",        RECIPE_POTION, ITEM_HEALING_PILL, RARITY_UNCOMMON, "Restores 120 HP",         RESOURCE_HERB_RED,  4, RESOURCE_HERB_GOLD,1, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Major Healing Elixir",  RECIPE_POTION, ITEM_HEALING_PILL, RARITY_RARE,     "Restores full HP",        RESOURCE_HERB_RED,  6, RESOURCE_HERB_GOLD,2, RESOURCE_CRYSTAL,1, false);
    def_recipe(game, &idx, "Minor Qi Pill",         RECIPE_POTION, ITEM_QI_PILL,      RARITY_COMMON,   "Restores 40 QI",          RESOURCE_HERB_BLUE, 2, RESOURCE_NONE,0, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Qi Restoration Pill",   RECIPE_POTION, ITEM_QI_PILL,      RARITY_UNCOMMON, "Restores 100 QI",         RESOURCE_HERB_BLUE, 4, RESOURCE_HERB_GOLD,1, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Grand Qi Elixir",       RECIPE_POTION, ITEM_QI_PILL,      RARITY_RARE,     "Restores full QI",        RESOURCE_HERB_BLUE, 6, RESOURCE_ORE_SPIRIT,1, RESOURCE_CRYSTAL,1, false);
    def_recipe(game, &idx, "Spirit Stone Pill",     RECIPE_POTION, ITEM_SPIRIT_STONE,  RARITY_UNCOMMON, "+200 cultivation qi",     RESOURCE_ORE_SPIRIT,2, RESOURCE_HERB_GOLD,2, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Capture Orb",           RECIPE_POTION, ITEM_CAPTURE_ORB,   RARITY_UNCOMMON, "Tame a weakened beast",   RESOURCE_CRYSTAL,   2, RESOURCE_HERB_BLUE,2, RESOURCE_ORE_IRON,1, true);
    /* ─── OILS ────────────────────────────────────── */
    def_recipe(game, &idx, "Beast-Bane Oil",        RECIPE_OIL,    ITEM_BUFF_POTION_ATK, RARITY_UNCOMMON, "+30% ATK for 60s",      RESOURCE_HERB_RED,  2, RESOURCE_HERB_GOLD,1, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Demon-Slayer Oil",      RECIPE_OIL,    ITEM_BUFF_POTION_ATK, RARITY_RARE,     "+50% ATK vs dark qi",   RESOURCE_HERB_RED,  3, RESOURCE_ORE_SPIRIT,1, RESOURCE_CRYSTAL,1, false);
    def_recipe(game, &idx, "Iron-Skin Oil",         RECIPE_OIL,    ITEM_BUFF_POTION_DEF, RARITY_UNCOMMON, "+30% DEF for 45s",      RESOURCE_ORE_IRON,  2, RESOURCE_HERB_BLUE,2, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Heaven-Shield Elixir",  RECIPE_OIL,    ITEM_BUFF_POTION_DEF, RARITY_RARE,     "+60% DEF for 90s",      RESOURCE_ORE_IRON,  3, RESOURCE_CRYSTAL,  2, RESOURCE_HERB_GOLD,2, false);
    def_recipe(game, &idx, "Wind-Step Oil",         RECIPE_OIL,    ITEM_BUFF_POTION_SPD, RARITY_UNCOMMON, "+40% SPD for 30s",      RESOURCE_HERB_BLUE, 2, RESOURCE_WOOD,     2, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Nine-Heaven Speed Pill",RECIPE_OIL,    ITEM_BUFF_POTION_SPD, RARITY_EPIC,     "+80% SPD for 60s",      RESOURCE_HERB_GOLD, 3, RESOURCE_ORE_SPIRIT,2, RESOURCE_CRYSTAL,2, false);
    /* ─── ELIXIRS ─────────────────────────────────── */
    def_recipe(game, &idx, "Iron Body Elixir",      RECIPE_ELIXIR, ITEM_BUFF_POTION_DEF, RARITY_RARE,     "Tempers body for 2min",  RESOURCE_ORE_IRON,  4, RESOURCE_HERB_RED, 2, RESOURCE_HERB_BLUE,2, false);
    def_recipe(game, &idx, "Dragon Blood Elixir",   RECIPE_ELIXIR, ITEM_BUFF_POTION_ATK, RARITY_EPIC,     "+100% ATK for 30s",      RESOURCE_CRYSTAL,   3, RESOURCE_ORE_SPIRIT,3, RESOURCE_HERB_GOLD,3, false);
    def_recipe(game, &idx, "Shadow Step Brew",      RECIPE_ELIXIR, ITEM_BUFF_POTION_SPD, RARITY_RARE,     "Reduces dash cooldown",  RESOURCE_HERB_BLUE, 3, RESOURCE_WOOD,     2, RESOURCE_CRYSTAL,  1, false);
    def_recipe(game, &idx, "Evolution Stone",       RECIPE_ELIXIR, ITEM_EVOLUTION_STONE, RARITY_EPIC,     "Evolve a tamed beast",   RESOURCE_CRYSTAL,   4, RESOURCE_ORE_SPIRIT,4, RESOURCE_HERB_GOLD,4, false);
    def_recipe(game, &idx, "Shadow Crystal Brew",   RECIPE_ELIXIR, ITEM_SHADOW_CRYSTAL,  RARITY_LEGENDARY,"Amplify shadow power",  RESOURCE_CRYSTAL,   5, RESOURCE_ORE_SPIRIT,5, RESOURCE_HERB_GOLD,5, false);
    /* ─── BOMBS ───────────────────────────────────── */
    def_recipe(game, &idx, "Qi Shockwave Bomb",     RECIPE_BOMB,   ITEM_BUFF_POTION_ATK, RARITY_UNCOMMON, "AoE 60 qi damage",       RESOURCE_ORE_IRON,  2, RESOURCE_HERB_RED, 2, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Flame Spirit Bomb",     RECIPE_BOMB,   ITEM_BUFF_POTION_ATK, RARITY_RARE,     "Fire AoE 150 damage",    RESOURCE_HERB_RED,  4, RESOURCE_CRYSTAL,  2, RESOURCE_NONE,0, false);
    def_recipe(game, &idx, "Frost Cage Bomb",       RECIPE_BOMB,   ITEM_BUFF_POTION_DEF, RARITY_RARE,     "Freezes enemies 3s",     RESOURCE_HERB_BLUE, 4, RESOURCE_ORE_IRON, 2, RESOURCE_NONE,0, false);
    def_recipe(game, &idx, "Thunder Talisman",      RECIPE_BOMB,   ITEM_BUFF_POTION_ATK, RARITY_EPIC,     "Lightning strike 200dmg",RESOURCE_ORE_SPIRIT,3, RESOURCE_CRYSTAL,  3, RESOURCE_HERB_GOLD,2, false);
    def_recipe(game, &idx, "Smoke Screen Bomb",     RECIPE_BOMB,   ITEM_BUFF_POTION_SPD, RARITY_UNCOMMON, "Escape and +SPD 20s",    RESOURCE_WOOD,      3, RESOURCE_HERB_BLUE,2, RESOURCE_NONE,0, true);
    def_recipe(game, &idx, "Shadow Fog Grenade",    RECIPE_BOMB,   ITEM_SHADOW_CRYSTAL,  RARITY_RARE,     "Shadow AoE, fear effect",RESOURCE_CRYSTAL,   2, RESOURCE_HERB_BLUE,3, RESOURCE_ORE_SPIRIT,1, false);
    def_recipe(game, &idx, "Heaven-Earth Pill",     RECIPE_ELIXIR, ITEM_HEALING_PILL,    RARITY_LEGENDARY,"Full heal + breakthrough",RESOURCE_CRYSTAL,  5, RESOURCE_ORE_SPIRIT,5, RESOURCE_HERB_GOLD,5, false);
    def_recipe(game, &idx, "Immortal Spring Tea",   RECIPE_POTION, ITEM_QI_PILL,         RARITY_LEGENDARY,"Max HP+QI, triple regen", RESOURCE_HERB_GOLD, 5, RESOURCE_CRYSTAL,  4, RESOURCE_ORE_SPIRIT,4, false);
    def_recipe(game, &idx, "Monarch's Elixir",      RECIPE_ELIXIR, ITEM_BUFF_POTION_ATK, RARITY_MYTHIC,   "All stats x2 for 60s",   RESOURCE_CRYSTAL,   8, RESOURCE_ORE_SPIRIT,8, RESOURCE_HERB_GOLD,8, false);
    game->recipe_count = idx;
}

void alchemy_init(Game *game) {
  memset(player_resources, 0, sizeof(player_resources));
  memset(game->recipes, 0, sizeof(game->recipes));
  memset(game->resource_nodes, 0, sizeof(game->resource_nodes));
  game->resource_count = 0;
  register_recipes(game);

  /* Give starting resources */
  player_resources[RESOURCE_HERB_RED] = 5;
  player_resources[RESOURCE_HERB_BLUE] = 3;
}

void alchemy_add_resource(Game *game, ResourceType type, int count) {
  (void)game;
  if (type > 0 && type < RESOURCE_COUNT) {
    player_resources[type] += count;
  }
}

int alchemy_get_resource(const Game *game, ResourceType type) {
  (void)game;
  if (type > 0 && type < RESOURCE_COUNT)
    return player_resources[type];
  return 0;
}

bool alchemy_can_craft(const Game *game, int recipe_idx) {
  if (recipe_idx < 0 || recipe_idx >= game->recipe_count)
    return false;
  const AlchemyRecipe *r = &game->recipes[recipe_idx];
  if (!r->unlocked)
    return false;

  for (int i = 0; i < r->num_ingredients; i++) {
    if (player_resources[r->ingredients[i]] < r->ingredient_counts[i])
      return false;
  }
  return true;
}

void alchemy_craft(Game *game, int recipe_idx) {
  if (!alchemy_can_craft(game, recipe_idx))
    return;
  const AlchemyRecipe *r = &game->recipes[recipe_idx];

  /* Consume ingredients */
  for (int i = 0; i < r->num_ingredients; i++) {
    player_resources[r->ingredients[i]] -= r->ingredient_counts[i];
  }

  /* Add result to player inventory */
  Entity *player = &game->entities[game->player_id];
  for (int i = 0; i < MAX_ITEMS; i++) {
    if (player->inventory[i].type == ITEM_NONE) {
      player->inventory[i].type = r->result;
      player->inventory[i].quantity = 1;
      player->inventory[i].rarity = r->result_rarity;
      player->inventory[i].name = r->name;
      player->num_items++;
      break;
    }
  }

  char buf[64];
  snprintf(buf, sizeof(buf), "Crafted: %s", r->name);
  system_notify(game, NOTIFY_SUCCESS, "[ Alchemy ]", buf);
}

void alchemy_spawn_resource_nodes(Game *game) {
  Entity *player = &game->entities[game->player_id];
  int count = 0;

  /* Scatter resources around spawn */
  for (int i = 0; i < 60 && count < MAX_RESOURCE_NODES; i++) {
    float angle = ((float)(rand() % 360)) * DEG2RAD;
    float dist = 100.0f + (float)(rand() % 800);
    Vec2 pos = {player->pos.x + cosf(angle) * dist,
                player->pos.y + sinf(angle) * dist};

    if (!world_pos_walkable(&game->world, pos.x, pos.y))
      continue;

    ResourceNode *node = &game->resource_nodes[count];
    node->active = true;
    node->pos = pos;
    node->amount = 2 + rand() % 4;
    node->respawn_timer = 0;
    node->harvest_time = 1.5f;
    node->is_harvesting = false;
    node->harvest_progress = 0;

    /* Type based on local tile */
    TileType tile = world_get_tile_at(&game->world, pos.x, pos.y);
    if (tile == TILE_FLOWER_RED)
      node->type = RESOURCE_HERB_RED;
    else if (tile == TILE_FLOWER_BLUE)
      node->type = RESOURCE_HERB_BLUE;
    else if (tile == TILE_STONE)
      node->type = RESOURCE_ORE_IRON;
    else if (tile == TILE_MOUNTAIN)
      node->type = RESOURCE_ORE_SPIRIT;
    else if (tile == TILE_BAMBOO || tile == TILE_TREE)
      node->type = RESOURCE_WOOD;
    else {
      int r = rand() % 5;
      ResourceType types[] = {RESOURCE_HERB_RED, RESOURCE_HERB_BLUE,
                              RESOURCE_HERB_GOLD, RESOURCE_ORE_IRON,
                              RESOURCE_CRYSTAL};
      node->type = types[r];
    }
    count++;
  }
  game->resource_count = count;
}

void alchemy_update_nodes(Game *game, float dt) {
  Entity *player = &game->entities[game->player_id];

  for (int i = 0; i < game->resource_count; i++) {
    ResourceNode *node = &game->resource_nodes[i];
    if (!node->active) {
      /* Respawn timer */
      if (node->respawn_timer > 0) {
        node->respawn_timer -= dt;
        if (node->respawn_timer <= 0) {
          node->active = true;
          node->amount = 2 + rand() % 3;
        }
      }
      continue;
    }

    /* Auto-harvest if player is close and pressing F */
    float dx = node->pos.x - player->pos.x;
    float dy = node->pos.y - player->pos.y;
    if (dx * dx + dy * dy < 30.0f * 30.0f && IsKeyDown(KEY_F)) {
      node->harvest_progress += dt;
      if (node->harvest_progress >= node->harvest_time) {
        /* Harvest! */
        alchemy_add_resource(game, node->type, 1);
        node->amount--;
        node->harvest_progress = 0;

        static const char *res_names[] = {"Nothing",  "Iron Ore",  "Spirit Ore",
                                          "Red Herb", "Blue Herb", "Gold Herb",
                                          "Wood",     "Crystal",   "Fish Spot"};
        char buf[48];
        snprintf(buf, sizeof(buf), "+1 %s", res_names[node->type]);
        system_notify(game, NOTIFY_INFO, "[ Gathered ]", buf);
        quests_on_harvest(game, node->type);
        audio_play(SFX_ITEM_PICKUP);

        if (node->amount <= 0) {
          node->active = false;
          node->respawn_timer = 60.0f;
        }
      }
    } else {
      node->harvest_progress = 0;
    }
  }
}

static const char *resource_name(ResourceType t) {
  static const char *names[] = {"-",        "Iron Ore",  "Spirit Ore",
                                "Red Herb", "Blue Herb", "Gold Herb",
                                "Wood",     "Crystal",   "Fish Spot"};
  if (t >= 0 && t < RESOURCE_COUNT)
    return names[t];
  return "?";
}

void alchemy_draw_nodes(const Game *game, float time) {
  const Entity *player = &game->entities[game->player_id];

  for (int i = 0; i < game->resource_count; i++) {
    const ResourceNode *node = &game->resource_nodes[i];
    if (!node->active)
      continue;

    float dx = node->pos.x - player->pos.x;
    float dy = node->pos.y - player->pos.y;
    if (dx * dx + dy * dy > 300.0f * 300.0f)
      continue;

    /* Draw node indicator */
    Color nc;
    switch (node->type) {
    case RESOURCE_HERB_RED:
      nc = (Color){255, 80, 80, 200};
      break;
    case RESOURCE_HERB_BLUE:
      nc = (Color){80, 120, 255, 200};
      break;
    case RESOURCE_HERB_GOLD:
      nc = (Color){255, 215, 0, 200};
      break;
    case RESOURCE_ORE_IRON:
      nc = (Color){160, 160, 160, 200};
      break;
    case RESOURCE_ORE_SPIRIT:
      nc = (Color){180, 120, 255, 200};
      break;
    case RESOURCE_CRYSTAL:
      nc = (Color){200, 240, 255, 200};
      break;
    case RESOURCE_WOOD:
      nc = (Color){140, 100, 50, 200};
      break;
    default:
      nc = WHITE;
      break;
    }

    float bob = sinf(time * 2.0f + i * 0.7f) * 2.0f;
    DrawCircle((int)node->pos.x, (int)(node->pos.y + bob), 5, nc);

    /* Sparkle */
    float sparkle = sinf(time * 5.0f + i) * 0.5f + 0.5f;
    nc.a = (unsigned char)(80 * sparkle);
    DrawCircle((int)node->pos.x, (int)(node->pos.y + bob), 8, nc);

    /* Harvest prompt if close */
    if (dx * dx + dy * dy < 30.0f * 30.0f) {
      const char *prompt = "[F] Gather";
      int tw = MeasureText(prompt, 9);
      DrawText(prompt, (int)node->pos.x - tw / 2, (int)node->pos.y - 20, 9,
               (Color){255, 255, 200, 200});

      /* Progress bar */
      if (node->harvest_progress > 0) {
        float ratio = node->harvest_progress / node->harvest_time;
        DrawRectangle((int)node->pos.x - 15, (int)node->pos.y - 28, 30, 4,
                      (Color){30, 30, 30, 180});
        DrawRectangle((int)node->pos.x - 15, (int)node->pos.y - 28,
                      (int)(30 * ratio), 4, (Color){100, 255, 100, 255});
      }
    }
  }
}

void alchemy_draw_menu(const Game *game) {
  DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});

  int pw = 750, ph = 500;
  int px = SCREEN_WIDTH / 2 - pw / 2, py = SCREEN_HEIGHT / 2 - ph / 2;

  system_ui_draw_panel(px, py, pw, ph, 1.0f);
  DrawText("[ ALCHEMY & CRAFTING ]", px + 30, py + 20, 24,
           (Color){220, 240, 255, 255});
  DrawLine(px + 30, py + 50, px + pw - 30, py + 50, (Color){60, 120, 255, 200});

  /* Resources inventory on the right */
  int rx = px + pw - 200;
  int ry = py + 70;
  DrawText("■ MATERIALS", rx, ry, 16, (Color){100, 180, 255, 200});
  ry += 25;
  for (int i = 1; i < RESOURCE_COUNT; i++) {
    int count = player_resources[i];
    if (count > 0) {
      char buf[48];
      snprintf(buf, sizeof(buf), "%s: %d", resource_name(i), count);
      DrawText(buf, rx, ry, 12, (Color){200, 200, 200, 255});
      ry += 16;
    }
  }

  /* Recipes on the left */
  int ly = py + 70;
  DrawText("■ RECIPES", px + 40, ly, 16, (Color){100, 180, 255, 200});
  ly += 25;

  for (int i = 0; i < game->recipe_count; i++) {
    const AlchemyRecipe *r = &game->recipes[i];
    if (!r->unlocked)
      continue;

    bool can = alchemy_can_craft(game, i);
    Color nc = can ? (Color){80, 255, 120, 255} : (Color){150, 150, 150, 200};

    char buf[96];
    snprintf(buf, sizeof(buf), "[%d] %s — %s", i + 1, r->name, r->description);
    DrawText(buf, px + 50, ly, 13, nc);

    /* Ingredient list */
    char ing[128] = "  Needs: ";
    for (int j = 0; j < r->num_ingredients; j++) {
      char part[32];
      snprintf(part, sizeof(part), "%dx %s  ", r->ingredient_counts[j],
               resource_name(r->ingredients[j]));
      strncat(ing, part, sizeof(ing) - strlen(ing) - 1);
    }
    DrawText(ing, px + 60, ly + 15, 10, (Color){120, 140, 170, 180});
    ly += 34;
  }

  DrawText("[ 1-5 ] Craft  |  [ ESC ] Close", px + pw / 2 - 100, py + ph - 30,
           12, (Color){100, 180, 255, 200});
}
