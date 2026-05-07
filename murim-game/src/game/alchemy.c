/*
 * alchemy.c — Crafting, Potions & Weapon Oils (Witcher 3 / Zelda)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#include "alchemy.h"
#include "../ui/system_ui.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Player resource inventory (simple counts) ───────── */
static int player_resources[RESOURCE_COUNT] = {0};

/* ─── Recipe definitions ──────────────────────────────── */
static void register_recipes(Game *game) {
  int idx = 0;

  /* Healing Potion */
  game->recipes[idx].unlocked = true;
  strncpy(game->recipes[idx].name, "Healing Elixir", 31);
  game->recipes[idx].category = RECIPE_POTION;
  game->recipes[idx].result = ITEM_HEALING_PILL;
  game->recipes[idx].result_rarity = RARITY_COMMON;
  game->recipes[idx].ingredients[0] = RESOURCE_HERB_RED;
  game->recipes[idx].ingredient_counts[0] = 3;
  game->recipes[idx].num_ingredients = 1;
  strncpy(game->recipes[idx].description, "Restores 50 HP", 63);
  idx++;

  /* Qi Restoration */
  game->recipes[idx].unlocked = true;
  strncpy(game->recipes[idx].name, "Qi Restoration Pill", 31);
  game->recipes[idx].category = RECIPE_POTION;
  game->recipes[idx].result = ITEM_QI_PILL;
  game->recipes[idx].result_rarity = RARITY_UNCOMMON;
  game->recipes[idx].ingredients[0] = RESOURCE_HERB_BLUE;
  game->recipes[idx].ingredient_counts[0] = 3;
  game->recipes[idx].num_ingredients = 1;
  strncpy(game->recipes[idx].description, "Restores 40 QI", 63);
  idx++;

  /* Attack Oil */
  game->recipes[idx].unlocked = true;
  strncpy(game->recipes[idx].name, "Beast-Bane Oil", 31);
  game->recipes[idx].category = RECIPE_OIL;
  game->recipes[idx].result = ITEM_BUFF_POTION_ATK;
  game->recipes[idx].result_rarity = RARITY_RARE;
  game->recipes[idx].ingredients[0] = RESOURCE_HERB_RED;
  game->recipes[idx].ingredient_counts[0] = 2;
  game->recipes[idx].ingredients[1] = RESOURCE_HERB_GOLD;
  game->recipes[idx].ingredient_counts[1] = 1;
  game->recipes[idx].num_ingredients = 2;
  strncpy(game->recipes[idx].description, "+30% ATK vs beasts for 60s", 63);
  idx++;

  /* Defense Elixir */
  game->recipes[idx].unlocked = true;
  strncpy(game->recipes[idx].name, "Iron Skin Elixir", 31);
  game->recipes[idx].category = RECIPE_ELIXIR;
  game->recipes[idx].result = ITEM_BUFF_POTION_DEF;
  game->recipes[idx].result_rarity = RARITY_RARE;
  game->recipes[idx].ingredients[0] = RESOURCE_ORE_IRON;
  game->recipes[idx].ingredient_counts[0] = 2;
  game->recipes[idx].ingredients[1] = RESOURCE_HERB_BLUE;
  game->recipes[idx].ingredient_counts[1] = 2;
  game->recipes[idx].num_ingredients = 2;
  strncpy(game->recipes[idx].description, "+50% DEF for 45s", 63);
  idx++;

  /* Spirit Bomb */
  game->recipes[idx].unlocked = false;
  strncpy(game->recipes[idx].name, "Spirit Bomb", 31);
  game->recipes[idx].category = RECIPE_BOMB;
  game->recipes[idx].result = ITEM_BUFF_POTION_ATK;
  game->recipes[idx].result_rarity = RARITY_EPIC;
  game->recipes[idx].ingredients[0] = RESOURCE_ORE_SPIRIT;
  game->recipes[idx].ingredient_counts[0] = 3;
  game->recipes[idx].ingredients[1] = RESOURCE_CRYSTAL;
  game->recipes[idx].ingredient_counts[1] = 1;
  game->recipes[idx].num_ingredients = 2;
  strncpy(game->recipes[idx].description, "AoE explosion dealing 200 dmg", 63);
  idx++;

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
