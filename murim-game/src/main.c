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
#include "game/reputation.h"
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
  elements_init_entity(&game.entities[game.player_id], ELEMENT_SHADOW);

  /* Give some enemies elemental affinities */
  for (int i = 0; i < MAX_ENTITIES; i++) {
    Entity *e = &game.entities[i];
    if (!e->active || i == game.player_id)
      continue;
    if (e->type == ENTITY_BEAST || e->type == ENTITY_NPC_HOSTILE) {
      ElementType el = (ElementType)(1 + rand() % 7);
      elements_init_entity(e, el);
    }
  }
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

  /* ARISE shadow extraction — F key */
  if (IsKeyPressed(KEY_F)) {
    taming_do_arise(&game);
  }

  /* Use item — X key */
  if (IsKeyPressed(KEY_X)) {
    combat_use_item(&game);
  }

  /* Check if player died */
  if (player->stats.hp <= 0) {
    game.state = STATE_GAME_OVER;
  }
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

    /* Pause */
    if (IsKeyPressed(KEY_ESCAPE)) {
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

  case STATE_STANDOFF:
    standoff_update(&game, dt);
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
    fishing_draw(&game);
    EndMode2D();
    fishing_draw(&game);
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
  CloseWindow();

  return 0;
}
