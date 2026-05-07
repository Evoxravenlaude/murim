/*
 * taming.c — Beast Taming & Shadow Army (Solo Leveling ARISE)
 * Heavenly Demon: Murim Chronicles v3.0
 *
 * Use Capture Orb on weakened beast (<30% HP) to tame it.
 * Shadow-power holders can whisper "ARISE" after killing a boss.
 */
#include "taming.h"
#include "../ui/system_ui.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include "npc.h"
#include "combat.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ─── Static state for ARISE prompt ──────────────────── */
static Vec2  s_arise_pos       = {0,0};
static float s_arise_timer     = 0;
static bool  s_can_arise       = false;
static char  s_arise_name[32]  = "";

void taming_init(Game *game)
{
    memset(game->tamed_beasts, 0, sizeof(game->tamed_beasts));
    memset(game->shadows,      0, sizeof(game->shadows));
    s_can_arise   = false;
    s_arise_timer = 0;
}

/* ─── Beast Capture ───────────────────────────────────── */
bool taming_try_capture(Game *game, int entity_id)
{
    if (entity_id < 0 || entity_id >= MAX_ENTITIES) return false;
    Entity *target = &game->entities[entity_id];
    Entity *player = &game->entities[game->player_id];

    if (!target->active) return false;
    if (target->type != ENTITY_BEAST) {
        system_notify(game, NOTIFY_WARNING, "[ Taming ]", "Only beasts can be captured!");
        return false;
    }
    float hp_pct = (float)target->stats.hp / (float)target->stats.max_hp;
    if (hp_pct > 0.30f) {
        system_notify(game, NOTIFY_WARNING, "[ Taming ]", "Weaken the beast below 30% HP first!");
        return false;
    }

    /* Check for capture orb in inventory */
    bool has_orb = false;
    int orb_slot = -1;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (player->inventory[i].type == ITEM_CAPTURE_ORB && player->inventory[i].quantity > 0) {
            has_orb = true; orb_slot = i; break;
        }
    }
    if (!has_orb) {
        system_notify(game, NOTIFY_WARNING, "[ Taming ]", "No Capture Orbs in inventory!");
        return false;
    }

    /* Find free tamed beast slot */
    int slot = -1;
    for (int i = 0; i < MAX_TAMED_BEASTS; i++) {
        if (!game->tamed_beasts[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        system_notify(game, NOTIFY_WARNING, "[ Taming ]", "Beast roster is full!");
        return false;
    }

    /* Capture roll: harder beasts at higher HP% are harder */
    float difficulty = target->is_tameable ? target->tame_difficulty : 0.7f;
    float roll = (float)(rand() % 100) / 100.0f;
    float success_chance = (1.0f - difficulty) * (1.0f - hp_pct) * 2.0f;
    if (success_chance > 0.95f) success_chance = 0.95f;

    /* Consume orb */
    player->inventory[orb_slot].quantity--;
    if (player->inventory[orb_slot].quantity <= 0)
        player->inventory[orb_slot].type = ITEM_NONE;

    particle_burst(game, target->pos, (Color){100,255,200,255}, 20, 120.0f, 0.8f, 4.0f);

    if (roll > success_chance) {
        system_notify(game, NOTIFY_WARNING, "[ Taming Failed ]", "The beast broke free!");
        return false;
    }

    /* Populate tamed beast entry */
    TamedBeast *tb = &game->tamed_beasts[slot];
    tb->active     = true;
    tb->stage      = BEAST_STAGE_JUVENILE;
    tb->stats      = target->stats;
    tb->stats.hp   = tb->stats.max_hp;
    tb->color      = target->color;
    tb->loyalty    = 60;
    tb->entity_id  = -1;
    tb->is_deployed = false;
    tb->is_mounted  = false;
    tb->evolution_xp        = 0;
    tb->evolution_threshold = 500;
    tb->ability     = TECH_BASIC_PUNCH;
    if (target->name)
        strncpy(tb->name, target->name, sizeof(tb->name)-1);
    else
        snprintf(tb->name, sizeof(tb->name), "Beast #%d", slot);
    strncpy(tb->species, tb->name, sizeof(tb->species)-1);

    /* Remove beast from world */
    target->active = false;

    camera_shake(game, 5.0f, 0.3f);
    system_notify(game, NOTIFY_SUCCESS, "[ Tamed! ]", tb->name);
    return true;
}

/* ─── ARISE Shadow Extraction ─────────────────────────── */
void taming_check_arise_prompt(Game *game, int killed_entity_id)
{
    if (!game->has_shadow_power) return;
    Entity *e = &game->entities[killed_entity_id];
    if (e->type != ENTITY_BOSS && e->type != ENTITY_DUNGEON_BOSS) return;

    s_can_arise   = true;
    s_arise_pos   = e->pos;
    s_arise_timer = 8.0f; /* 8 second window */
    if (e->name) strncpy(s_arise_name, e->name, sizeof(s_arise_name)-1);
    else snprintf(s_arise_name, sizeof(s_arise_name), "Shadow");
}

void taming_do_arise(Game *game)
{
    if (!s_can_arise || !game->has_shadow_power) return;

    int slot = -1;
    for (int i = 0; i < MAX_SHADOWS; i++) {
        if (!game->shadows[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        system_notify(game, NOTIFY_WARNING, "[ ARISE ]", "Shadow army is full!");
        return;
    }

    ShadowSoldier *sh = &game->shadows[slot];
    sh->active      = true;
    sh->entity_id   = -1;
    sh->is_deployed = false;
    sh->rank        = RANK_D;
    sh->loyalty     = 100.0f;
    sh->color       = (Color){80, 30, 180, 255};
    strncpy(sh->name, s_arise_name, sizeof(sh->name)-1);
    /* Shadows inherit partial stats of boss */
    sh->stats.attack  = 40;
    sh->stats.max_hp  = 200;
    sh->stats.hp      = 200;
    sh->stats.speed   = 110;
    sh->ability       = TECH_BASIC_SLASH;

    /* Visual: dark column of light */
    particle_burst(game, s_arise_pos, (Color){80,0,200,255}, 50, 200.0f, 2.0f, 8.0f);
    camera_shake(game, 15.0f, 0.8f);

    char buf[64]; snprintf(buf, sizeof(buf), "%s has answered your call!", sh->name);
    system_notify(game, NOTIFY_SHADOW, "[ ARISE ]", buf);

    s_can_arise   = false;
    s_arise_timer = 0;
}

/* ─── Deploy / Recall ─────────────────────────────────── */
int taming_deploy_beast(Game *game, int beast_idx)
{
    if (beast_idx < 0 || beast_idx >= MAX_TAMED_BEASTS) return -1;
    TamedBeast *tb = &game->tamed_beasts[beast_idx];
    if (!tb->active || tb->is_deployed) return -1;

    Entity *player = &game->entities[game->player_id];
    float ang = ((float)(rand()%360)) * DEG2RAD;
    Vec2 pos = { player->pos.x + cosf(ang)*60.0f, player->pos.y + sinf(ang)*60.0f };

    int eid = npc_spawn(game, ENTITY_TAMED_BEAST, pos, tb->name);
    if (eid < 0) return -1;

    Entity *e = &game->entities[eid];
    e->stats    = tb->stats;
    e->color    = tb->color;
    e->owner_id = game->player_id;
    e->ai_state = AI_FOLLOW_PLAYER;
    e->detection_range = 80.0f;

    tb->is_deployed = true;
    tb->entity_id   = eid;

    system_notify(game, NOTIFY_INFO, "[ Beast Deployed ]", tb->name);
    return eid;
}

void taming_recall_all(Game *game)
{
    for (int i = 0; i < MAX_TAMED_BEASTS; i++) {
        TamedBeast *tb = &game->tamed_beasts[i];
        if (!tb->active || !tb->is_deployed) continue;
        if (tb->entity_id >= 0 && tb->entity_id < MAX_ENTITIES)
            game->entities[tb->entity_id].active = false;
        tb->is_deployed = false;
        tb->entity_id   = -1;
    }
    system_notify(game, NOTIFY_INFO, "[ Recall ]", "All beasts recalled.");
}

/* ─── Update ──────────────────────────────────────────── */
void taming_update(Game *game, float dt)
{
    Entity *player = &game->entities[game->player_id];

    /* ARISE window countdown */
    if (s_can_arise) {
        s_arise_timer -= dt;
        if (s_arise_timer <= 0) { s_can_arise = false; s_arise_timer = 0; }
    }

    /* Deployed beast AI: follow player */
    for (int i = 0; i < MAX_TAMED_BEASTS; i++) {
        TamedBeast *tb = &game->tamed_beasts[i];
        if (!tb->active || !tb->is_deployed || tb->entity_id < 0) continue;
        Entity *e = &game->entities[tb->entity_id];
        if (!e->active) { tb->is_deployed = false; tb->entity_id = -1; continue; }

        /* Follow player */
        float dx = player->pos.x - e->pos.x, dy = player->pos.y - e->pos.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist > 60.0f) {
            float spd = (float)e->stats.speed * 0.8f;
            e->vel.x = (dx/dist)*spd; e->vel.y = (dy/dist)*spd;
        } else { e->vel.x = 0; e->vel.y = 0; }

        /* Attack nearby hostiles */
        if (e->attack_timer <= 0) {
            for (int j = 0; j < MAX_ENTITIES; j++) {
                Entity *t = &game->entities[j];
                if (!t->active || j == game->player_id || j == tb->entity_id) continue;
                if (t->type != ENTITY_NPC_HOSTILE && t->type != ENTITY_BEAST) continue;
                float ex = t->pos.x-e->pos.x, ey = t->pos.y-e->pos.y;
                if (ex*ex+ey*ey < 40.0f*40.0f) {
                    e->is_attacking = true; e->attack_timer = 0.6f;
                    combat_apply_damage(game, e, t); break;
                }
            }
        }

        /* Evolution XP from game kills (simplified: time-based) */
        tb->evolution_xp += (int)(dt * 2);
        if (tb->evolution_xp >= tb->evolution_threshold && tb->stage < BEAST_STAGE_DIVINE) {
            tb->stage++;
            tb->evolution_xp = 0;
            tb->evolution_threshold *= 3;
            tb->stats.attack += 10; tb->stats.max_hp += 50; tb->stats.hp = tb->stats.max_hp;
            e->stats = tb->stats;
            char buf[48]; snprintf(buf, sizeof(buf), "%s evolved!", tb->name);
            system_notify(game, NOTIFY_SUCCESS, "[ Evolution! ]", buf);
            particle_burst(game, e->pos, (Color){255,215,0,255}, 30, 150.0f, 1.0f, 5.0f);
        }
    }

    /* Deployed shadows: attack nearest enemy */
    for (int i = 0; i < MAX_SHADOWS; i++) {
        ShadowSoldier *sh = &game->shadows[i];
        if (!sh->active || !sh->is_deployed || sh->entity_id < 0) continue;
        Entity *e = &game->entities[sh->entity_id];
        if (!e->active) { sh->is_deployed = false; sh->entity_id = -1; continue; }

        /* Find nearest hostile */
        float best = 9999.0f; int best_id = -1;
        for (int j = 0; j < MAX_ENTITIES; j++) {
            Entity *t = &game->entities[j];
            if (!t->active||j==game->player_id||j==sh->entity_id) continue;
            if (t->type!=ENTITY_NPC_HOSTILE&&t->type!=ENTITY_BEAST&&t->type!=ENTITY_DUNGEON_MONSTER) continue;
            float dx=t->pos.x-e->pos.x, dy=t->pos.y-e->pos.y;
            float d=sqrtf(dx*dx+dy*dy);
            if (d<best) { best=d; best_id=j; }
        }
        if (best_id >= 0) {
            Entity *t = &game->entities[best_id];
            float dx=t->pos.x-e->pos.x, dy=t->pos.y-e->pos.y;
            if (best > 35.0f) {
                float spd=(float)e->stats.speed*0.9f;
                e->vel.x=(dx/best)*spd; e->vel.y=(dy/best)*spd;
            } else { e->vel.x=0; e->vel.y=0;
                if (e->attack_timer<=0) { e->is_attacking=true; e->attack_timer=0.5f; combat_apply_damage(game,e,t); }
            }
        } else {
            /* Follow player */
            float dx=player->pos.x-e->pos.x, dy=player->pos.y-e->pos.y;
            float d=sqrtf(dx*dx+dy*dy);
            if (d>80.0f) { float spd=(float)e->stats.speed; e->vel.x=(dx/d)*spd; e->vel.y=(dy/d)*spd; }
            else { e->vel.x=0; e->vel.y=0; }
        }
    }
}

/* ─── Draw Menus ──────────────────────────────────────── */
void taming_draw_arise_prompt(const Game *game)
{
    if (!s_can_arise) return;
    (void)game;
    float pulse = sinf((float)GetTime()*6.0f)*0.3f+0.7f;
    const char *msg = "[F] ARISE";
    int tw = MeasureText(msg, 20);
    DrawText(msg, SCREEN_WIDTH/2-tw/2, SCREEN_HEIGHT/2-60, 20,
             (Color){160,50,255,(unsigned char)(255*pulse)});
    /* Timer bar */
    float ratio = s_arise_timer / 8.0f;
    DrawRectangle(SCREEN_WIDTH/2-80, SCREEN_HEIGHT/2-35, 160, 8, (Color){30,30,30,200});
    DrawRectangle(SCREEN_WIDTH/2-80, SCREEN_HEIGHT/2-35, (int)(160*ratio), 8, (Color){140,40,220,255});
}

void taming_draw_beast_menu(const Game *game)
{
    DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,(Color){0,0,0,180});
    int pw=680,ph=480, px=SCREEN_WIDTH/2-pw/2, py=SCREEN_HEIGHT/2-ph/2;
    DrawRectangle(px,py,pw,ph,(Color){10,15,30,220});
    DrawRectangleLinesEx((Rectangle){px,py,pw,ph},2,(Color){60,120,255,200});
    DrawText("[ BEAST ROSTER ]",px+30,py+20,22,(Color){220,240,255,255});
    DrawLine(px+30,py+50,px+pw-30,py+50,(Color){60,120,255,200});
    int y=py+65; int shown=0;
    for (int i=0;i<MAX_TAMED_BEASTS&&shown<8;i++) {
        const TamedBeast *tb=&game->tamed_beasts[i];
        if (!tb->active) continue;
        static const char *stages[]={"Juvenile","Adult","Elder","King","Divine"};
        char buf[128];
        snprintf(buf,sizeof(buf),"[%d] %s  (%s) | ATK:%d HP:%d | Loyalty:%d%%",
                 i+1, tb->name, stages[tb->stage],
                 tb->stats.attack, tb->stats.max_hp, tb->loyalty);
        Color c=tb->is_deployed?(Color){80,255,120,255}:(Color){200,200,200,255};
        DrawText(buf,px+40,y,13,c);
        y+=22; shown++;
    }
    if (!shown) DrawText("No beasts tamed yet. Use Capture Orbs on weakened beasts.",px+40,y,12,(Color){120,120,120,255});
    DrawText("[1-8] Deploy  |  [ESC] Close",px+pw/2-90,py+ph-28,12,(Color){100,180,255,200});
}

void taming_draw_shadow_menu(const Game *game)
{
    DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,(Color){0,0,0,200});
    int pw=680,ph=480, px=SCREEN_WIDTH/2-pw/2, py=SCREEN_HEIGHT/2-ph/2;
    DrawRectangle(px,py,pw,ph,(Color){5,5,20,230});
    DrawRectangleLinesEx((Rectangle){px,py,pw,ph},2,(Color){100,40,220,200});
    float pulse=sinf((float)GetTime()*2.0f)*0.2f+0.8f;
    DrawText("[ SHADOW ARMY ]",px+30,py+20,22,(Color){(unsigned char)(160*pulse),40,(unsigned char)(255*pulse),255});
    DrawLine(px+30,py+50,px+pw-30,py+50,(Color){100,40,220,200});
    int y=py+65; int shown=0;
    for (int i=0;i<MAX_SHADOWS&&shown<8;i++) {
        const ShadowSoldier *sh=&game->shadows[i];
        if (!sh->active) continue;
        char buf[128];
        snprintf(buf,sizeof(buf),"[%d] %s  %s | ATK:%d HP:%d",
                 i+1, sh->name, rank_name(sh->rank), sh->stats.attack, sh->stats.max_hp);
        Color c=sh->is_deployed?(Color){160,80,255,255}:(Color){180,180,200,255};
        DrawText(buf,px+40,y,13,c);
        y+=22; shown++;
    }
    if (!shown) DrawText("No shadows extracted. Defeat bosses and ARISE them.",px+40,y,12,(Color){80,80,100,255});
    DrawText("[1-8] Deploy  |  [ESC] Close",px+pw/2-90,py+ph-28,12,(Color){100,80,200,200});
}
