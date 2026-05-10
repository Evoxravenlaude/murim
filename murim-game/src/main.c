/*
 * main.c — Entry point for Heavenly Demon: Murim Chronicles
 *
 * A 2D open-world murim/anime-inspired game
 * Built with C + Raylib
 *
 * Controls:
 *   WASD / Arrows   — Move
 *   SPACE           — Attack (melee)
 *   Q               — Use Qi Technique
 *   E               — Cultivate / Interact
 *   TAB             — Stats Screen
 *   I               — Inventory
 *   K               — Skill Tree
 *   1-5             — Switch Technique
 *   ESC             — Pause
 *   F3              — Debug Info
 *   R               — Restart (from pause/gameover)
 */

#include "engine/camera.h"
#include "engine/particles.h"
#include "engine/renderer.h"
#include "engine/types.h"
#include "engine/weather.h"
#include "game/alchemy.h"
#include "game/audio_sys.h"
#include "game/bestiary.h"
#include "game/bounty.h"
#include "game/combat.h"
#include "game/cultivation.h"
#include "game/dungeon.h"
#include "game/elements.h"
#include "game/environment.h"
#include "game/events.h"
#include "game/exploration.h"
#include "game/fishing.h"
#include "game/mounts.h"
#include "game/npc.h"
#include "game/quests.h"
#include "game/reputation.h"
#include "game/save.h"
#include "game/standoff.h"
#include "game/taming.h"
#include "game/world.h"
#include "raylib.h"
#include "ui/hud.h"
#include "ui/system_ui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─── Global game instance ────────────────────────────── */
static Game game;

/* ─── Forward declarations ────────────────────────────── */
static void game_init(void);
static void game_update(float dt);
static void game_draw(void);
static void game_restart(void);
static void player_update(float dt);
static void start_dialogue(const char *speaker, const char **lines, int count);
static void update_dialogue(float dt);
static void stat_alloc_update(void);
static void stat_alloc_draw(const Game *game);

/* ─── Stat allocation screen state ───────────────────── */
static bool  s_stat_alloc_open = false;

/* ─── Quest log state ─────────────────────────────────── */
static bool  s_quest_log_open  = false;

/* ─── Dialogue lines ──────────────────────────────────── */
static const char *ELDER_DIALOGUE[] = {
    "Young one, you have potential. I can see it in your qi.",
    "Continue to cultivate and you will reach the heavens.",
    "Remember: the path of the martial artist is long and arduous.",
    "Go forth and make your sect proud!"};

static const char *MERCHANT_DIALOGUE[] = {
    "Welcome, fellow cultivator! I have wares for sale.",
    "Spirit stones, healing pills, technique scrolls...",
    "Come back when you have more gold!"};

static const char *FRIENDLY_DIALOGUE[] = {
    "The world is vast. Have you seen the bamboo groves to the east?",
    "Be careful of the rogue cultivators in the mountains.",
    "Safe travels, friend!"};

/* ═══════════════════════════════════════════════════════ */
/*                      INIT                              */
/* ═══════════════════════════════════════════════════════ */
static void game_init(void) {
  memset(&game, 0, sizeof(Game));
  game.state = STATE_TITLE;

  /* Generate world */
  unsigned int seed = (unsigned int)time(NULL);
  world_generate(&game.world, seed);

  /* Find a valid spawn position near center */
  Vec2 spawn = {WORLD_TILES_X * TILE_SIZE / 2.0f,
                WORLD_TILES_Y * TILE_SIZE / 2.0f};
  for (int attempt = 0; attempt < 100; attempt++) {
    float test_x = spawn.x + (rand() % 200 - 100);
    float test_y = spawn.y + (rand() % 200 - 100);
    if (world_pos_walkable(&game.world, test_x, test_y)) {
      spawn.x = test_x;
      spawn.y = test_y;
      break;
    }
  }

  /* Spawn player */
  npc_spawn_player(&game, spawn);

  /* Populate world with NPCs */
  /* Spread NPCs across the world */
  for (int i = 0; i < 12; i++) {
    float angle = ((float)i / 12.0f) * 6.2831f;
    float dist = 200.0f + (float)(rand() % 600);
    Vec2 area = {spawn.x + cosf(angle) * dist, spawn.y + sinf(angle) * dist};
    npc_populate_area(&game, area, 150.0f, 5 + rand() % 5);
  }

  /* Spawn some nearby friendlies */
  for (int i = 0; i < 3; i++) {
    Vec2 npc_pos = {spawn.x + (rand() % 100 - 50),
                    spawn.y + (rand() % 100 - 50)};
    if (world_pos_walkable(&game.world, npc_pos.x, npc_pos.y)) {
      if (i == 0)
        npc_spawn(&game, ENTITY_NPC_ELDER, npc_pos, "Master Chen");
      else if (i == 1)
        npc_spawn(&game, ENTITY_NPC_MERCHANT, npc_pos, "Trader Lin");
      else
        npc_spawn(&game, ENTITY_NPC_FRIENDLY, npc_pos, "Disciple Wei");
    }
  }

  /* Initialize camera */
  camera_init(&game.camera, spawn);

  renderer_init();
  system_ui_init(&game);
  weather_init(&game.world);

  /* v3.0 AAA system initialization */
  bestiary_init(&game);
  bounty_init(&game.entities[game.player_id]);
  environment_init(&game.entities[game.player_id]);
  exploration_init(&game.entities[game.player_id]);
  mounts_init(&game.entities[game.player_id]);
  fishing_init(&game.entities[game.player_id]);
  alchemy_init(&game);
  alchemy_spawn_resource_nodes(&game);
  taming_init(&game);
  dungeon_init(&game);
  dungeon_spawn_gates(&game);
  events_init(&game);
  reputation_init(&game);
    quests_init(&game);
    game.has_shadow_power = true;  /* Player starts with ARISE ability */
    elements_init_entity(&game.entities[game.player_id], ELEMENT_SHADOW);

    /* Give some enemies elemental affinities */
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game.entities[i];
        if (!e->active || i == game.player_id) continue;
        if (e->type == ENTITY_BEAST || e->type == ENTITY_NPC_HOSTILE) {
            ElementType el = (ElementType)(1 + rand() % 7);
            elements_init_entity(e, el);
        }
    }

    /* Give player some starting herbs for early survival */
    Entity *p = &game.entities[game.player_id];
    p->inventory[0].type     = ITEM_HERB;
    p->inventory[0].quantity = 4;
    p->inventory[0].name     = "Wild Herb";
    p->inventory[1].type     = ITEM_HEALING_PILL;
    p->inventory[1].quantity = 2;
    p->inventory[1].name     = "Minor Healing Pill";
    p->inventory[2].type     = ITEM_CAPTURE_ORB;
    p->inventory[2].quantity = 2;
    p->inventory[2].name     = "Capture Orb";
    p->num_items = 3;

    /* Allocate fog of war */
    game.fog = (FogOfWar *)calloc(1, sizeof(FogOfWar));
    if (game.fog) game.fog->vision_range = 8;

    /* Try to load save */
    if (save_exists()) {
        load_game(&game);
        system_notify(&game, NOTIFY_INFO, "[ Save Loaded ]", "Welcome back, cultivator.");
    }

    audio_init();
}

/* ═══════════════════════════════════════════════════════ */
/*                    UPDATE                              */
/* ═══════════════════════════════════════════════════════ */
static void player_update(float dt) {
  Entity *player = &game.entities[game.player_id];

  /* Don't move while cultivating */
  if (player->cultivation.is_cultivating) {
    player->vel.x = 0;
    player->vel.y = 0;
    return;
  }

  /* Dash / Dodge Update (Elden Ring style) */
  if (player->dash.dash_cooldown > 0) {
    player->dash.dash_cooldown -= dt;
  }
  if (player->dash.iframe_timer > 0) {
    player->dash.iframe_timer -= dt;
  }

  if (player->dash.is_dashing) {
    player->dash.dash_timer -= dt;
    player->vel.x = player->dash.dash_dir.x * player->dash.dash_speed;
    player->vel.y = player->dash.dash_dir.y * player->dash.dash_speed;

    /* Create dash trail effect */
    Color dash_color = {200, 200, 255, 100};
    particle_burst(&game, player->pos, dash_color, 1, 0.0f, 0.2f, 1.0f);

    if (player->dash.dash_timer <= 0) {
      player->dash.is_dashing = false;
    }
  } else {
    /* Normal Movement */
    float speed = (float)player->stats.speed;
    float dx = 0, dy = 0;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
      dy -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
      dy += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
      dx -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
      dx += 1;

    /* Normalize diagonal movement */
    if (dx != 0 && dy != 0) {
      float inv_sqrt2 = 0.7071f;
      dx *= inv_sqrt2;
      dy *= inv_sqrt2;
    }

    player->vel.x = dx * speed;
    player->vel.y = dy * speed;

    /* Check for Dash input (SHIFT or Double tap could work, let's use SHIFT) */
    if ((IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) &&
        player->dash.dash_cooldown <= 0 && (dx != 0 || dy != 0)) {
      player->dash.is_dashing = true;
      player->dash.dash_timer = 0.2f;    /* 200ms dash */
      player->dash.dash_cooldown = 1.0f; /* 1 sec cooldown */
      player->dash.iframe_timer = 0.25f; /* 250ms invincibility */
      player->dash.dash_speed = speed * 3.5f;
      player->dash.dash_dir.x = dx;
      player->dash.dash_dir.y = dy;

      /* Dash sound/visual feedback */
      camera_shake(&game, 2.0f, 0.1f);

      /* Override velocity immediately */
      player->vel.x = dx * player->dash.dash_speed;
      player->vel.y = dy * player->dash.dash_speed;
    }

    /* Update facing direction */
    if (dx > 0 && dy > 0)
      player->dir = DIR_DOWN_RIGHT;
    else if (dx < 0 && dy > 0)
      player->dir = DIR_DOWN_LEFT;
    else if (dx > 0 && dy < 0)
      player->dir = DIR_UP_RIGHT;
    else if (dx < 0 && dy < 0)
      player->dir = DIR_UP_LEFT;
    else if (dx > 0)
      player->dir = DIR_RIGHT;
    else if (dx < 0)
      player->dir = DIR_LEFT;
    else if (dy > 0)
      player->dir = DIR_DOWN;
    else if (dy < 0)
      player->dir = DIR_UP;
  }

  /* Apply velocity with collision */
  float new_x = player->pos.x + player->vel.x * dt;
  float new_y = player->pos.y + player->vel.y * dt;

  if (world_pos_walkable(&game.world, new_x, player->pos.y))
    player->pos.x = new_x;
  if (world_pos_walkable(&game.world, player->pos.x, new_y))
    player->pos.y = new_y;

  /* RDR2 Survival Cores & Skyrim Progression Update */
  player->stats.hunger -= dt * 0.05f; /* Base drain */
  player->stats.thirst -= dt * 0.1f;

  if (player->vel.x != 0 || player->vel.y != 0) {
    if (player->dash.is_dashing) {
      player->stats.fatigue -= dt * 5.0f;
      player->stats.athletics += dt * 0.05f;
      player->stats.thirst -= dt * 0.2f;
    } else {
      player->stats.fatigue -= dt * 0.5f;
      player->stats.athletics += dt * 0.01f;
    }
  } else {
    player->stats.fatigue += dt * 2.0f; /* Recover when resting */
  }

  /* Clamp cores */
  if (player->stats.hunger < 0)
    player->stats.hunger = 0;
  if (player->stats.thirst < 0)
    player->stats.thirst = 0;
  if (player->stats.fatigue < 0)
    player->stats.fatigue = 0;
  if (player->stats.fatigue > 100.0f)
    player->stats.fatigue = 100.0f;

  /* Level up athletics check */
  if (player->stats.athletics >= 10.0f) {
    player->stats.athletics = 1.0f; /* Reset and boost speed */
    player->stats.speed += 5;
    system_notify(&game, NOTIFY_SUCCESS, "[ Skill Leveled Up ]",
                  "Athletics increased!");
  }

  /* Clamp to world bounds */
  if (player->pos.x < TILE_SIZE)
    player->pos.x = TILE_SIZE;
  if (player->pos.y < TILE_SIZE)
    player->pos.y = TILE_SIZE;
  if (player->pos.x > WORLD_TILES_X * TILE_SIZE - TILE_SIZE)
    player->pos.x = WORLD_TILES_X * TILE_SIZE - TILE_SIZE;
  if (player->pos.y > WORLD_TILES_Y * TILE_SIZE - TILE_SIZE)
    player->pos.y = WORLD_TILES_Y * TILE_SIZE - TILE_SIZE;

  /* Attack */
  if (IsKeyPressed(KEY_SPACE)) {
    combat_player_attack(&game);
  }

  /* Qi Technique */
  if (IsKeyPressed(KEY_Q)) {
    combat_player_technique(&game);
  }

  /* Switch techniques (1-5) */
  for (int i = 0; i < 5; i++) {
    if (IsKeyPressed(KEY_ONE + i) && i < player->num_techniques) {
      player->active_technique = i;
    }
  }

  /* Cultivate / Interact / Enter Gate */
  if (IsKeyPressed(KEY_E)) {
    /* Priority 1: nearby gate */
    int gid = dungeon_find_nearby_gate(&game, 60.0f);
    if (gid >= 0 && game.active_dungeon < 0) {
      dungeon_enter(&game, gid);
      return;
    }
    if (player->cultivation.is_cultivating) {
      player->cultivation.is_cultivating = false;
      game.state = STATE_PLAYING;
    } else {
      int npc_id = npc_find_nearby_friendly(&game, 40.0f);
      if (npc_id >= 0) {
        Entity *npc = &game.entities[npc_id];
        switch (npc->type) {
        case ENTITY_NPC_ELDER:    start_dialogue(npc->name, ELDER_DIALOGUE, 4); break;
        case ENTITY_NPC_MERCHANT: start_dialogue(npc->name, MERCHANT_DIALOGUE, 3); break;
        default: start_dialogue(npc->name, FRIENDLY_DIALOGUE, 3); break;
        }
      } else {
        player->cultivation.is_cultivating = true;
        game.state = STATE_CULTIVATING;
      }
    }
  }

  /* Standoff — G key */
  if (IsKeyPressed(KEY_G)) {
    standoff_initiate(&game);
  }

  /* Mount / Dismount — M key */
  if (IsKeyPressed(KEY_M)) {
    mounts_try_mount(&game);
  }

  /* Fishing — H key near water */
  if (IsKeyPressed(KEY_H)) {
    fishing_start(&game);
  }

  /* Use Capture Orb on nearest weakened beast — Z key */
  if (IsKeyPressed(KEY_Z)) {
    float best_dist = 9999.0f; int best_id = -1;
    for (int i = 0; i < MAX_ENTITIES; i++) {
      Entity *e = &game.entities[i];
      if (!e->active || i == game.player_id) continue;
      if (e->type != ENTITY_BEAST) continue;
      float dx=e->pos.x-player->pos.x, dy=e->pos.y-player->pos.y;
      float d=sqrtf(dx*dx+dy*dy);
      if (d < best_dist && d < 60.0f) { best_dist=d; best_id=i; }
    }
    if (best_id >= 0) taming_try_capture(&game, best_id);
    else system_notify(&game, NOTIFY_WARNING, "[ Taming ]", "No beast in range!");
  }

  /* ARISE shadow extraction — F key (only when ARISE prompt is active) */
  if (IsKeyPressed(KEY_F)) {
    taming_do_arise(&game);
  }

  /* Beast roster menu — T key */
  if (IsKeyPressed(KEY_T)) {
    game.state = STATE_BEAST_MENU;
  }

  /* Shadow army menu — Y key */
  if (IsKeyPressed(KEY_Y)) {
    game.state = STATE_SHADOW_MENU;
  }

  /* Use item — X key */
  if (IsKeyPressed(KEY_X)) {
    combat_use_item(&game);
  }

  /* Eat — N key near water or with food */
  if (IsKeyPressed(KEY_N)) {
    environment_eat(&game);
    audio_play(SFX_ITEM_USE);
  }
  /* Drink — U key near water */
  if (IsKeyPressed(KEY_U)) {
    environment_drink(&game);
    audio_play(SFX_ITEM_USE);
  }

  /* Check if player died */
  if (player->stats.hp <= 0) {
    game.state = STATE_GAME_OVER;
  }
}

/* ─── Stat allocation update ──────────────────────────── */
static void stat_alloc_update(void)
{
    Entity *player = &game.entities[game.player_id];
    if (player->stats.stat_points <= 0) { s_stat_alloc_open = false; return; }

    if (IsKeyPressed(KEY_ONE)) { /* STR */
        player->stats.strength += 2; player->stats.attack += 2;
        player->stats.stat_points--;
        system_notify(&game, NOTIFY_SUCCESS, "[ Stat Allocated ]", "Strength +2");
        audio_play(SFX_LEVEL_UP);
    } else if (IsKeyPressed(KEY_TWO)) { /* VIT */
        player->stats.vitality += 2; player->stats.max_hp += 20;
        player->stats.hp += 20; player->stats.stat_points--;
        system_notify(&game, NOTIFY_SUCCESS, "[ Stat Allocated ]", "Vitality +2, Max HP +20");
        audio_play(SFX_LEVEL_UP);
    } else if (IsKeyPressed(KEY_THREE)) { /* AGI */
        player->stats.agility += 2; player->stats.speed += 5;
        player->stats.stat_points--;
        system_notify(&game, NOTIFY_SUCCESS, "[ Stat Allocated ]", "Agility +2, Speed +5");
        audio_play(SFX_LEVEL_UP);
    } else if (IsKeyPressed(KEY_FOUR)) { /* INT */
        player->stats.intelligence += 2; player->stats.max_qi += 15;
        player->stats.stat_points--;
        system_notify(&game, NOTIFY_SUCCESS, "[ Stat Allocated ]", "Intelligence +2, Max QI +15");
        audio_play(SFX_LEVEL_UP);
    } else if (IsKeyPressed(KEY_FIVE)) { /* SENSE */
        player->stats.sense += 2; player->stats.luck += 1;
        player->stats.stat_points--;
        system_notify(&game, NOTIFY_SUCCESS, "[ Stat Allocated ]", "Sense +2, Luck +1");
        audio_play(SFX_LEVEL_UP);
    }
    if (IsKeyPressed(KEY_ESCAPE)) s_stat_alloc_open = false;
}

static void stat_alloc_draw(const Game *game)
{
    const Entity *p = &game->entities[game->player_id];
    int px = SCREEN_WIDTH/2-220, py = SCREEN_HEIGHT/2-160;
    DrawRectangle(px, py, 440, 320, (Color){5,10,25,230});
    DrawRectangle(px, py, 440, 2,   (Color){60,120,255,220});
    DrawRectangleLinesEx((Rectangle){px,py,440,320}, 1.5f, (Color){60,120,255,200});
    DrawText("[ ALLOCATE STAT POINTS ]", px+50, py+16, 18, (Color){220,240,255,255});
    char buf[64];
    snprintf(buf, sizeof(buf), "Available: %d points", p->stats.stat_points);
    DrawText(buf, px+130, py+42, 13, (Color){255,215,0,255});
    DrawLine(px+20, py+60, px+420, py+60, (Color){60,120,255,150});
    int y = py+74;
    snprintf(buf, sizeof(buf), "[1] Strength   (STR: %d)  +2 STR, +2 ATK",  p->stats.strength);  DrawText(buf,px+30,y,12,(Color){255,120,120,255}); y+=28;
    snprintf(buf, sizeof(buf), "[2] Vitality   (VIT: %d)  +2 VIT, +20 HP",  p->stats.vitality);  DrawText(buf,px+30,y,12,(Color){120,255,120,255}); y+=28;
    snprintf(buf, sizeof(buf), "[3] Agility    (AGI: %d)  +2 AGI, +5 SPD",  p->stats.agility);   DrawText(buf,px+30,y,12,(Color){120,200,255,255}); y+=28;
    snprintf(buf, sizeof(buf), "[4] Intelligence(INT: %d) +2 INT, +15 QI",  p->stats.intelligence);DrawText(buf,px+30,y,12,(Color){200,120,255,255}); y+=28;
    snprintf(buf, sizeof(buf), "[5] Sense      (SEN: %d)  +2 SEN, +1 LCK", p->stats.sense);      DrawText(buf,px+30,y,12,(Color){255,215,0,255}); y+=28;
    DrawText("[ESC] Close", px+170, py+288, 12, (Color){100,180,255,200});
}

static void start_dialogue(const char *speaker, const char **lines, int count) {
  game.dialogue.active = true;
  game.dialogue.speaker = speaker;
  game.dialogue.num_lines = count;
  game.dialogue.current_line = 0;
  game.dialogue.chars_shown = 0;
  game.dialogue.char_timer = 0;
  for (int i = 0; i < count && i < 8; i++) {
    game.dialogue.lines[i] = lines[i];
  }
  game.state = STATE_DIALOGUE;
}

static void update_dialogue(float dt) {
  DialogueBox *dlg = &game.dialogue;
  if (!dlg->active)
    return;

  /* Typewriter effect */
  dlg->char_timer += dt;
  if (dlg->char_timer >= 0.03f) {
    dlg->chars_shown++;
    dlg->char_timer = 0;
  }

  /* Advance or close */
  if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
    int line_len = (int)strlen(dlg->lines[dlg->current_line]);
    if (dlg->chars_shown < line_len) {
      /* Skip to full line */
      dlg->chars_shown = line_len;
    } else {
      /* Next line */
      dlg->current_line++;
      dlg->chars_shown = 0;
      if (dlg->current_line >= dlg->num_lines) {
        dlg->active = false;
        game.state = STATE_PLAYING;
      }
    }
  }
}

static void game_update(float dt) {
  game.game_time += dt;

  switch (game.state) {
  case STATE_TITLE:
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
      game.state = STATE_PLAYING;
    }
    break;

  case STATE_PLAYING:
    /* Toggle debug */
    if (IsKeyPressed(KEY_F3))
      game.show_debug = !game.show_debug;

    /* System UI Debug testing keys */
    if (IsKeyPressed(KEY_F4)) {
      game.entities[game.player_id].stats.level++;
      system_notify_level_up(&game, game.entities[game.player_id].stats.level);
    }
    if (IsKeyPressed(KEY_F5)) {
      if (game.entities[game.player_id].hunter_rank < RANK_MONARCH) {
        game.entities[game.player_id].hunter_rank++;
        system_notify_rank_up(&game, game.entities[game.player_id].hunter_rank);
      }
    }
    if (IsKeyPressed(KEY_F6)) {
      system_notify_item_drop(&game, "Demon King's Dagger", RARITY_MYTHIC);
    }

    /* Weather testing keys */
    if (IsKeyPressed(KEY_F7))
      weather_set(&game.world, WEATHER_RAIN);
    if (IsKeyPressed(KEY_F8))
      weather_set(&game.world, WEATHER_STORM);
    if (IsKeyPressed(KEY_F9))
      weather_set(&game.world, WEATHER_CLEAR);

    /* v3.0 AAA Keybinds */
    /* Qi Sense toggle (Witcher Sense) */
    if (IsKeyPressed(KEY_V)) {
      game.qi_sense_global = !game.qi_sense_global;
      if (game.qi_sense_global)
        system_notify(&game, NOTIFY_INFO, "[ Qi Sense ]", "Activated");
    }

    /* Bestiary menu */
    if (IsKeyPressed(KEY_B)) {
      game.state = STATE_BESTIARY;
      break;
    }

    /* Alchemy / Crafting menu */
    if (IsKeyPressed(KEY_C)) {
      game.state = STATE_ALCHEMY;
      break;
    }

    /* Inventory — I key */
    if (IsKeyPressed(KEY_I)) {
      game.state = STATE_INVENTORY;
      break;
    }

    /* Skill Tree — K key */
    if (IsKeyPressed(KEY_K)) {
      game.state = STATE_SKILL_TREE;
      break;
    }

    /* Combat Stance switch (R cycles stances) */
    if (IsKeyPressed(KEY_R) && game.state != STATE_PAUSED) {
      Entity *p = &game.entities[game.player_id];
      p->stance.current =
          (CombatStance)((p->stance.current + 1) % STANCE_COUNT);
      char buf[48];
      snprintf(buf, sizeof(buf), "Switched to %s Stance",
               stance_name(p->stance.current));
      system_notify(&game, NOTIFY_INFO, "[ Stance ]", buf);
    }

    /* Stat allocation screen */
    if (IsKeyPressed(KEY_P) && game.entities[game.player_id].stats.stat_points > 0) {
      s_stat_alloc_open = !s_stat_alloc_open;
    }
    if (s_stat_alloc_open) { stat_alloc_update(); }

    /* Quest log */
    if (IsKeyPressed(KEY_J)) {
      s_quest_log_open = !s_quest_log_open;
    }

    /* Save game — F10 */
    if (IsKeyPressed(KEY_F10)) {
      save_game(&game);
      system_notify(&game, NOTIFY_SUCCESS, "[ SAVED ]", "Game saved successfully.");
    }

    /* Pause */
    if (IsKeyPressed(KEY_ESCAPE) && !s_stat_alloc_open && !s_quest_log_open) {
      game.state = STATE_PAUSED;
      break;
    }

    /* Stats screen */
    if (IsKeyPressed(KEY_TAB)) {
      game.state = STATE_STATS;
      break;
    }

    /* Camera zoom */
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      game.camera.zoom += wheel * 0.2f;
      if (game.camera.zoom < 1.0f)
        game.camera.zoom = 1.0f;
      if (game.camera.zoom > 4.0f)
        game.camera.zoom = 4.0f;
    }

    player_update(dt);
    npc_update_ai(&game, dt);
    combat_update(&game, dt);
    cultivation_update(&game, dt);
    world_update(&game.world, dt);
    weather_update(&game, dt);
    particles_update(&game, dt);
    system_ui_update(&game, dt);
    audio_update(&game, dt);
    quests_update(&game, dt);

    /* v3.0 system updates */
    elements_update(&game, dt);
    bestiary_update(&game, dt);
    bounty_update(&game, dt);
    environment_update(&game, dt);
    alchemy_update_nodes(&game, dt);
    taming_update(&game, dt);
    dungeon_update(&game, dt);
    events_update(&game, dt);
    reputation_update(&game, dt);
    exploration_update(&game, dt);
    mounts_update(&game, dt);

    /* Fog of war reveal around player */
    if (game.fog) {
        Entity *fp = &game.entities[game.player_id];
        int tx = (int)(fp->pos.x / TILE_SIZE);
        int ty = (int)(fp->pos.y / TILE_SIZE);
        int vr = game.fog->vision_range;

        /* Clear visibility each frame (explored persists) */
        memset(game.fog->visible, 0, sizeof(game.fog->visible));

        for (int dy2 = -vr; dy2 <= vr; dy2++)
            for (int dx2 = -vr; dx2 <= vr; dx2++) {
                /* Circular vision range */
                if (dx2*dx2 + dy2*dy2 > vr*vr) continue;
                int ex = tx+dx2, ey = ty+dy2;
                if (ex>=0&&ex<WORLD_TILES_X&&ey>=0&&ey<WORLD_TILES_Y) {
                    game.fog->explored[ey][ex] = true;
                    game.fog->visible[ey][ex]  = true;
                }
            }
    }

    /* Loot drop auto-pickup */
    {
        Entity *lp = &game.entities[game.player_id];
        for (int i = 0; i < MAX_LOOT_DROPS; i++) {
            LootDrop *ld = &game.loot_drops[i];
            if (!ld->active) continue;
            ld->bob_timer += dt;
            ld->timer -= dt;
            float ldx = ld->pos.x - lp->pos.x, ldy = ld->pos.y - lp->pos.y;
            if (ldx*ldx+ldy*ldy < 20.0f*20.0f) {
                /* Auto-pickup */
                for (int s = 0; s < MAX_ITEMS; s++) {
                    if (lp->inventory[s].type == ITEM_NONE) {
                        lp->inventory[s].type     = ld->item;
                        lp->inventory[s].quantity = ld->quantity;
                        lp->inventory[s].rarity   = ld->rarity;
                        lp->inventory[s].name     = "Item";
                        lp->num_items++;
                        audio_play(SFX_ITEM_PICKUP);
                        system_notify_item_drop(&game, "Item", ld->rarity);
                        break;
                    }
                }
                ld->active = false;
            }
            if (ld->timer <= 0) ld->active = false;
        }
    }

    /* Iron Body mastery leveling (on taking damage) */
    {
        Entity *ibp = &game.entities[game.player_id];
        if (ibp->flash_timer > 0) {
            ibp->iron_body_mastery += 0.01f;
            if (ibp->iron_body_mastery >= 10.0f) {
                ibp->iron_body_mastery = 1.0f;
                ibp->stats.defense += 3;
                system_notify(&game, NOTIFY_SUCCESS, "[ Skill Leveled Up ]", "Iron Body increased!");
            }
        }
    }

    /* Camera follow */
    Entity *player = &game.entities[game.player_id];
    camera_update(&game.camera, player->pos, dt);
    camera_update_shake(&game, dt);
    break;

  case STATE_PAUSED:
    if (IsKeyPressed(KEY_ESCAPE))
      game.state = STATE_PLAYING;
    if (IsKeyPressed(KEY_R))
      game_restart();
    break;

  case STATE_STATS:
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_ESCAPE))
      game.state = STATE_PLAYING;
    break;

  case STATE_CULTIVATING:
    cultivation_update(&game, dt);
    particles_update(&game, dt);
    world_update(&game.world, dt);

    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ESCAPE)) {
      game.entities[game.player_id].cultivation.is_cultivating = false;
      game.state = STATE_PLAYING;
    }
    break;

  case STATE_DIALOGUE:
    update_dialogue(dt);
    break;

  case STATE_GAME_OVER:
    if (IsKeyPressed(KEY_R))
      game_restart();
    break;

  /* v3.0 state handlers */
  case STATE_BESTIARY:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_B))
      game.state = STATE_PLAYING;
    break;

  case STATE_ALCHEMY:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_C))
      game.state = STATE_PLAYING;
    /* Craft with number keys */
    for (int i = 0; i < 5; i++) {
      if (IsKeyPressed(KEY_ONE + i) && i < game.recipe_count) {
        alchemy_craft(&game, i);
      }
    }
    break;

  case STATE_INVENTORY:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_I))
      game.state = STATE_PLAYING;
    /* Use item with number keys */
    if (IsKeyPressed(KEY_X))
      combat_use_item(&game);
    break;

  case STATE_SKILL_TREE:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_K))
      game.state = STATE_PLAYING;
    break;

  case STATE_STANDOFF:
    standoff_update(&game, dt);
    if (IsKeyPressed(KEY_SPACE)) standoff_player_strike(&game);
    particles_update(&game, dt);
    system_ui_update(&game, dt);
    break;

  case STATE_BEAST_MENU:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_T))
      game.state = STATE_PLAYING;
    /* Deploy beast with number keys */
    for (int i = 0; i < 8; i++) {
      if (IsKeyPressed(KEY_ONE + i))
        taming_deploy_beast(&game, i);
    }
    break;

  case STATE_SHADOW_MENU:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Y))
      game.state = STATE_PLAYING;
    /* Deploy shadow with number keys */
    for (int i = 0; i < 8; i++) {
      if (IsKeyPressed(KEY_ONE + i)) {
        if (i < MAX_SHADOWS && game.shadows[i].active && !game.shadows[i].is_deployed) {
          Entity *player = &game.entities[game.player_id];
          float ang = ((float)(rand()%360)) * DEG2RAD;
          Vec2 pos = { player->pos.x + cosf(ang)*60.0f, player->pos.y + sinf(ang)*60.0f };
          int eid = npc_spawn(&game, ENTITY_SHADOW_SOLDIER, pos, game.shadows[i].name);
          if (eid >= 0) {
            game.entities[eid].stats = game.shadows[i].stats;
            game.entities[eid].color = game.shadows[i].color;
            game.entities[eid].ai_state = AI_SHADOW_COMMAND;
            game.shadows[i].is_deployed = true;
            game.shadows[i].entity_id = eid;
            system_notify(&game, NOTIFY_SHADOW, "[ Shadow Deployed ]", game.shadows[i].name);
          }
        }
      }
    }
    break;

  case STATE_DUNGEON:
    if (IsKeyPressed(KEY_ESCAPE)) dungeon_exit(&game);
    player_update(dt);
    npc_update_ai(&game, dt);
    combat_update(&game, dt);
    cultivation_update(&game, dt);
    particles_update(&game, dt);
    system_ui_update(&game, dt);
    taming_update(&game, dt);
    dungeon_update(&game, dt);
    elements_update(&game, dt);
    bounty_update(&game, dt);
    {
      Entity *player = &game.entities[game.player_id];
      camera_update(&game.camera, player->pos, dt);
      camera_update_shake(&game, dt);
    }
    break;

  case STATE_FISHING:
    fishing_update(&game, dt);
    if (IsKeyPressed(KEY_ESCAPE)) fishing_cancel(&game);
    break;

  default:
    break;
  }
}

static void game_restart(void) {
  renderer_cleanup();
  game_init();
  game.state = STATE_PLAYING;
}

/* ═══════════════════════════════════════════════════════ */
/*                      DRAW                              */
/* ═══════════════════════════════════════════════════════ */
static void game_draw(void) {
  BeginDrawing();
  ClearBackground((Color){20, 25, 15, 255});

  switch (game.state) {
  case STATE_TITLE:
    hud_draw_title(&game);
    break;

  case STATE_PLAYING:
  case STATE_CULTIVATING:
  case STATE_DIALOGUE: {
    BeginMode2D(game.camera);

    renderer_draw_world(&game);

    /* v3.0: Draw resource nodes in world space */
    alchemy_draw_nodes(&game, game.game_time);

    renderer_draw_entities(&game);
    renderer_draw_particles(&game);

    /* v3.0: Draw elemental auras on entities */
    for (int i = 0; i < MAX_ENTITIES; i++) {
      const Entity *e = &game.entities[i];
      if (e->active)
        elements_draw_aura(e, game.game_time);
    }

    /* Draw qi aura for cultivating player */
    Entity *player = &game.entities[game.player_id];
    if (player->cultivation.is_cultivating) {
      renderer_draw_qi_aura(player->pos, player->cultivation.realm,
                            game.game_time);
    }

    /* Draw health bars for damaged enemies */
    for (int i = 0; i < MAX_ENTITIES; i++) {
      const Entity *e = &game.entities[i];
      if (!e->active || i == game.player_id)
        continue;
      if (e->stats.hp < e->stats.max_hp) {
        Color bar_color = GREEN;
        if (e->type == ENTITY_NPC_HOSTILE || e->type == ENTITY_BEAST)
          bar_color = RED;
        renderer_draw_health_bar(e->pos, e->stats.hp, e->stats.max_hp,
                                 bar_color);
      }
    }

    /* NPC interaction prompt */
    int nearby_npc = npc_find_nearby_friendly(&game, 40.0f);
    if (nearby_npc >= 0 && !game.dialogue.active) {
      Entity *npc = &game.entities[nearby_npc];
      float pulse = sinf(game.game_time * 4.0f) * 0.3f + 0.7f;
      const char *prompt = "[E] Talk";
      int tw = MeasureText(prompt, 10);
      DrawText(prompt, (int)npc->pos.x - tw / 2, (int)npc->pos.y - 30, 10,
               (Color){255, 255, 200, (unsigned char)(pulse * 255)});
      /* Draw NPC name */
      if (npc->name) {
        int nw = MeasureText(npc->name, 8);
        DrawText(npc->name, (int)npc->pos.x - nw / 2, (int)npc->pos.y - 38, 8,
                 (Color){200, 200, 200, 200});
      }
    }

    /* v3.0: Qi Sense overlay (world space) */
    bestiary_draw_qi_sense(&game, game.game_time);

    /* v3.0 Gate draws (world space) */
    dungeon_draw_gates(&game);

    EndMode2D();

    /* Day/night overlay */
    renderer_draw_daynight(&game);

    /* Weather overlay (rain/fog) */
    weather_draw_overlay(&game);

    /* v3.0: Environment overlay (underwater/heat/cold) */
    environment_draw_overlay(&game);

    /* HUD */
    hud_draw(&game);

    /* v3.0: Bounty stars */
    bounty_draw_stars(&game);

    /* v3.0: Environment HUD (temperature, oxygen) */
    environment_draw_hud(&game);

    /* v3.0: Stance indicator + poise bar */
    {
      const Entity *p = &game.entities[game.player_id];
      if (p->stance.current != STANCE_NEUTRAL) {
        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "⚔ %s", stance_name(p->stance.current));
        DrawText(sbuf, 20, SCREEN_HEIGHT - 95, 11, (Color){200, 180, 100, 220});
      }
      /* Element indicator */
      if (p->elemental.affinity != ELEMENT_NONE) {
        Color ec = element_color(p->elemental.affinity);
        DrawCircle(275, 30, 6, ec);
        DrawText(element_name(p->elemental.affinity), 285, 25, 10, ec);
      }
      /* Poise bar when damaged */
      if (p->poise.poise < p->poise.max_poise) {
        float pr = p->poise.poise / p->poise.max_poise;
        int px2 = SCREEN_WIDTH/2+100, py2 = SCREEN_HEIGHT-44;
        DrawRectangle(px2, py2, 80, 6, (Color){20,20,20,180});
        DrawRectangle(px2, py2, (int)(80*pr), 6, p->poise.is_staggered?(Color){255,80,80,255}:(Color){200,150,50,255});
        DrawRectangleLines(px2, py2, 80, 6, (Color){160,120,40,200});
        DrawText("Poise", px2, py2-12, 9, (Color){160,130,60,200});
      }
      /* Stagger "EXECUTE" prompt */
      if (p->poise.is_staggered) {
        /* Show on staggered enemy near player */
      }
    }

    /* v3.0: Exploration HUD */
    exploration_draw_hud(&game);

    /* v3.0: Mount HUD */
    mounts_draw_hud(&game);

    /* v3.0: Reputation HUD */
    reputation_draw_hud(&game);

    /* v3.0: ARISE prompt */
    taming_draw_arise_prompt(&game);

    if (game.state == STATE_CULTIVATING)
      hud_draw_cultivation(&game);

    if (game.state == STATE_DIALOGUE)
      hud_draw_dialogue(&game);

    /* v3.0: World event indicators */
    events_draw(&game);

    system_ui_draw_level_up(&game);
    system_ui_draw_notifications(&game);

    /* Stat allocation overlay */
    if (s_stat_alloc_open) stat_alloc_draw(&game);
    /* Quest log overlay */
    if (s_quest_log_open) quests_draw_log(&game);
    /* Quest tracker (top right) */
    if (!s_quest_log_open) quests_draw_tracker(&game);

    /* Survival warning overlays */
    {
        const Entity *sw = &game.entities[game.player_id];
        float t2 = (float)GetTime();
        if (sw->stats.hunger < 20.0f) {
            float a = sinf(t2*4.0f)*0.4f+0.6f;
            const char *msg = "STARVING!";
            int tw2 = MeasureText(msg,16);
            DrawText(msg, SCREEN_WIDTH/2-tw2/2, SCREEN_HEIGHT/2+80, 16,
                     (Color){255,120,30,(unsigned char)(a*255)});
        }
        if (sw->stats.thirst < 20.0f) {
            float a = sinf(t2*4.0f+1.0f)*0.4f+0.6f;
            const char *msg = "DEHYDRATED!";
            int tw2 = MeasureText(msg,16);
            DrawText(msg, SCREEN_WIDTH/2-tw2/2, SCREEN_HEIGHT/2+100, 16,
                     (Color){80,160,255,(unsigned char)(a*255)});
        }
        /* Stat points available indicator */
        if (sw->stats.stat_points > 0) {
            float a = sinf(t2*3.0f)*0.3f+0.7f;
            char spbuf[48]; snprintf(spbuf,sizeof(spbuf),"[P] %d Stat Points Available!",sw->stats.stat_points);
            int spw = MeasureText(spbuf,13);
            DrawText(spbuf, SCREEN_WIDTH/2-spw/2, SCREEN_HEIGHT/2+124, 13,
                     (Color){255,215,0,(unsigned char)(a*220)});
        }
    }

    /* Poise execute prompt on staggered nearby enemy */
    {
        const Entity *pp = &game.entities[game.player_id];
        for (int pi = 0; pi < MAX_ENTITIES; pi++) {
            const Entity *pe = &game.entities[pi];
            if (!pe->active || pi == game.player_id) continue;
            if (!pe->poise.is_staggered) continue;
            float edx = pe->pos.x-pp->pos.x, edy = pe->pos.y-pp->pos.y;
            if (edx*edx+edy*edy > 50.0f*50.0f) continue;
            float ea = sinf((float)GetTime()*8.0f)*0.3f+0.7f;
            const char *exec_msg = "[ SPACE ] HEAVENLY EXECUTION";
            int emw = MeasureText(exec_msg, 14);
            DrawText(exec_msg, SCREEN_WIDTH/2-emw/2, SCREEN_HEIGHT/2-80, 14,
                     (Color){255,220,50,(unsigned char)(ea*255)});
            break;
        }
    }
    break;
  }

  case STATE_PAUSED:
    /* Draw game behind pause */
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    hud_draw_pause(&game);
    break;

  case STATE_STATS:
    /* Draw game behind stats */
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    system_ui_draw_status_window(&game);
    system_ui_draw_notifications(&game);
    break;

  /* v3.0 menu states */
  case STATE_BESTIARY:
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    bestiary_draw_menu(&game);
    break;

  case STATE_ALCHEMY:
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    alchemy_draw_menu(&game);
    break;

  case STATE_INVENTORY: {
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    /* Inventory panel */
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    int ipw = 600, iph = 440;
    int ipx = SCREEN_WIDTH/2 - ipw/2, ipy = SCREEN_HEIGHT/2 - iph/2;
    system_ui_draw_panel(ipx, ipy, ipw, iph, 1.0f);
    DrawText("[ INVENTORY ]", ipx + 30, ipy + 20, 22, (Color){220,240,255,255});
    DrawLine(ipx + 30, ipy + 50, ipx + ipw - 30, ipy + 50, (Color){60,120,255,200});
    const Entity *inv_p = &game.entities[game.player_id];
    int iy = ipy + 65;
    int shown_items = 0;
    for (int i = 0; i < MAX_ITEMS; i++) {
      const InventorySlot *sl = &inv_p->inventory[i];
      if (sl->type == ITEM_NONE || sl->quantity <= 0) continue;
      Color ic = (Color){200,200,200,255};
      if (sl->rarity == RARITY_RARE)   ic = (Color){80,180,255,255};
      if (sl->rarity == RARITY_EPIC)   ic = (Color){200,80,255,255};
      if (sl->rarity == RARITY_MYTHIC) ic = (Color){255,215,0,255};
      char ibuf[128];
      snprintf(ibuf, sizeof(ibuf), "%-20s  x%d   (%s)",
               sl->name ? sl->name : "Item", sl->quantity, rarity_name(sl->rarity));
      DrawText(ibuf, ipx + 40, iy, 14, ic);
      iy += 24;
      shown_items++;
    }
    if (!shown_items)
      DrawText("Your inventory is empty.", ipx + 40, iy, 14, (Color){120,120,120,255});
    char goldbuf[48];
    snprintf(goldbuf, sizeof(goldbuf), "Gold: %d G", inv_p->gold);
    DrawText(goldbuf, ipx + 40, ipy + iph - 60, 16, (Color){255,215,0,255});
    DrawText("[X] Use Item  |  [ESC] Close", ipx + ipw/2 - 100, ipy + iph - 30, 12, (Color){100,180,255,200});
    system_ui_draw_notifications(&game);
    break;
  }

  case STATE_SKILL_TREE: {
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    /* Skill tree panel */
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    int spw = 700, sph = 500;
    int spx = SCREEN_WIDTH/2 - spw/2, spy = SCREEN_HEIGHT/2 - sph/2;
    system_ui_draw_panel(spx, spy, spw, sph, 1.0f);
    DrawText("[ SKILL TREE ]", spx + 30, spy + 20, 22, (Color){220,240,255,255});
    DrawLine(spx + 30, spy + 50, spx + spw - 30, spy + 50, (Color){60,120,255,200});
    const Entity *sk_p = &game.entities[game.player_id];
    int sy2 = spy + 70;
    /* Combat Skills */
    DrawText("■ COMBAT ARTS", spx + 40, sy2, 18, (Color){255,120,120,255}); sy2 += 30;
    char sbuf[128];
    snprintf(sbuf, sizeof(sbuf), "Sword Mastery: %.1f", sk_p->stats.sword_mastery);
    DrawText(sbuf, spx + 60, sy2, 14, (Color){200,200,200,255});
    float sm_ratio = sk_p->stats.sword_mastery / 10.0f;
    DrawRectangle(spx + 280, sy2, 150, 10, (Color){30,30,30,200});
    DrawRectangle(spx + 280, sy2, (int)(150 * sm_ratio), 10, (Color){255,120,120,220});
    sy2 += 26;
    snprintf(sbuf, sizeof(sbuf), "Fist Mastery:  %.1f", sk_p->stats.fist_mastery);
    DrawText(sbuf, spx + 60, sy2, 14, (Color){200,200,200,255});
    float fm_ratio = sk_p->stats.fist_mastery / 10.0f;
    DrawRectangle(spx + 280, sy2, 150, 10, (Color){30,30,30,200});
    DrawRectangle(spx + 280, sy2, (int)(150 * fm_ratio), 10, (Color){255,180,80,220});
    sy2 += 26;
    snprintf(sbuf, sizeof(sbuf), "Iron Body:     %.1f", sk_p->iron_body_mastery);
    DrawText(sbuf, spx + 60, sy2, 14, (Color){200,200,200,255});
    float ib_ratio = sk_p->iron_body_mastery / 10.0f;
    DrawRectangle(spx + 280, sy2, 150, 10, (Color){30,30,30,200});
    DrawRectangle(spx + 280, sy2, (int)(150 * ib_ratio), 10, (Color){120,200,255,220});
    sy2 += 36;
    /* Physical Skills */
    DrawText("■ PHYSICAL ARTS", spx + 40, sy2, 18, (Color){120,255,120,255}); sy2 += 30;
    snprintf(sbuf, sizeof(sbuf), "Athletics:     %.1f", sk_p->stats.athletics);
    DrawText(sbuf, spx + 60, sy2, 14, (Color){200,200,200,255});
    float at_ratio = sk_p->stats.athletics / 10.0f;
    DrawRectangle(spx + 280, sy2, 150, 10, (Color){30,30,30,200});
    DrawRectangle(spx + 280, sy2, (int)(150 * at_ratio), 10, (Color){120,255,120,220});
    sy2 += 36;
    /* Techniques */
    DrawText("■ TECHNIQUES", spx + 40, sy2, 18, (Color){100,180,255,255}); sy2 += 30;
    for (int ti = 0; ti < sk_p->num_techniques && ti < 5; ti++) {
      const Technique *tech = &sk_p->techniques[ti];
      Color tc2 = ti == sk_p->active_technique ? (Color){255,215,0,255} : (Color){180,180,200,255};
      snprintf(sbuf, sizeof(sbuf), "[%d] %-18s  DMG:%d  QI:%d  CD:%.1fs",
               ti+1, tech->name, tech->damage, tech->qi_cost, tech->cooldown);
      DrawText(sbuf, spx + 60, sy2, 13, tc2);
      sy2 += 22;
    }
    sy2 += 14;
    /* Cultivation realm */
    snprintf(sbuf, sizeof(sbuf), "Cultivation: %s", cultivation_realm_name(sk_p->cultivation.realm));
    DrawText(sbuf, spx + 40, sy2, 16, (Color){200,150,255,255});
    DrawText("[ESC] Close", spx + spw/2 - 30, spy + sph - 30, 12, (Color){100,180,255,200});
    system_ui_draw_notifications(&game);
    break;
  }

  case STATE_STANDOFF:
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    renderer_draw_particles(&game);
    EndMode2D();
    standoff_draw(&game);
    system_ui_draw_notifications(&game);
    break;

  case STATE_DUNGEON:
    BeginMode2D(game.camera);
    dungeon_draw_floor(&game);
    renderer_draw_entities(&game);
    renderer_draw_particles(&game);
    EndMode2D();
    hud_draw(&game);
    taming_draw_arise_prompt(&game);
    system_ui_draw_level_up(&game);
    system_ui_draw_notifications(&game);
    break;

  case STATE_FISHING:
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    fishing_draw(&game); /* bobber + HUD (handles world & screen) */
    EndMode2D();
    hud_draw(&game);
    system_ui_draw_notifications(&game);
    break;

  case STATE_BEAST_MENU:
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    taming_draw_beast_menu(&game);
    break;

  case STATE_SHADOW_MENU:
    BeginMode2D(game.camera);
    renderer_draw_world(&game);
    renderer_draw_entities(&game);
    EndMode2D();
    taming_draw_shadow_menu(&game);
    break;

  case STATE_GAME_OVER:
    hud_draw_game_over(&game);
    break;

  default:
    break;
  }

  EndDrawing();
}

/* ═══════════════════════════════════════════════════════ */
/*                      MAIN                              */
/* ═══════════════════════════════════════════════════════ */
int main(void) {
  srand((unsigned int)time(NULL));

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Heavenly Demon: Murim Chronicles");
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_NULL); /* Don't exit on ESC */

  game_init();

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    if (dt > 0.05f)
      dt = 0.05f; /* cap delta time */

    game_update(dt);
    game_draw();
  }

  renderer_cleanup();
  audio_cleanup();
  if (game.fog) { free(game.fog); game.fog = NULL; }
  CloseWindow();

  return 0;
}
