/*
 * dungeon.c — Dungeon Gates & Instanced Dungeons (Solo Leveling)
 * Heavenly Demon: Murim Chronicles v3.0
 */
#include "dungeon.h"
#include "npc.h"
#include "quests.h"
#include "../ui/system_ui.h"
#include "../engine/particles.h"
#include "../engine/camera.h"
#include "world.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const float GATE_TIME_LIMITS[] = { 600,480,360,300,240,180 };
static const int   GATE_ENEMY_COUNT[] = { 5,8,12,16,20,25 };
static const int   GATE_EXP_REWARD[]  = { 200,500,1000,2000,4000,8000 };
static const int   GATE_GOLD_REWARD[] = { 100,250,500,1000,2500,6000 };

static void fill_rect(DungeonGate *g, int x, int y, int w, int h, TileType t)
{
    for (int dy=0;dy<h;dy++) for (int dx=0;dx<w;dx++) {
        int tx=x+dx, ty=y+dy;
        if (tx>=0&&tx<64&&ty>=0&&ty<64) g->floor_tiles[ty][tx]=t;
    }
}

static void carve(DungeonGate *g, int x1,int y1,int x2,int y2)
{
    int cx=x1,cy=y1;
    while (cx!=x2) { if(cx>=0&&cx<64&&cy>=0&&cy<64) g->floor_tiles[cy][cx]=TILE_DUNGEON_FLOOR; cx+=(x2>x1)?1:-1; }
    while (cy!=y2) { if(cx>=0&&cx<64&&cy>=0&&cy<64) g->floor_tiles[cy][cx]=TILE_DUNGEON_FLOOR; cy+=(y2>y1)?1:-1; }
}

void dungeon_generate_floor(DungeonGate *gate, int rank, unsigned int seed)
{
    fill_rect(gate,0,0,64,64,TILE_DUNGEON_WALL);
    srand(seed + rank*7777);
    typedef struct { int x,y,w,h; } Room;
    Room rooms[10]; int rc=0;
    int nr=5+rank; if(nr>10)nr=10;
    for (int att=0; att<40&&rc<nr; att++) {
        int rw=6+rand()%6, rh=6+rand()%6;
        int rx=2+rand()%(60-rw), ry=2+rand()%(60-rh);
        bool ov=false;
        for (int j=0;j<rc;j++) if (rx<rooms[j].x+rooms[j].w+2&&rx+rw+2>rooms[j].x&&ry<rooms[j].y+rooms[j].h+2&&ry+rh+2>rooms[j].y){ov=true;break;}
        if (ov) continue;
        rooms[rc++]=(Room){rx,ry,rw,rh};
        fill_rect(gate,rx,ry,rw,rh,TILE_DUNGEON_FLOOR);
    }
    for (int i=1;i<rc;i++) carve(gate,rooms[i-1].x+rooms[i-1].w/2,rooms[i-1].y+rooms[i-1].h/2,rooms[i].x+rooms[i].w/2,rooms[i].y+rooms[i].h/2);
    if (rc>=2) { int lx=rooms[rc-1].x+rooms[rc-1].w/2,ly=rooms[rc-1].y; if(ly>=0&&ly<64&&lx>=0&&lx<64) gate->floor_tiles[ly][lx]=TILE_DUNGEON_DOOR; }
    gate->floor_width=64; gate->floor_height=64;
}

void dungeon_init(Game *game) { memset(game->gates,0,sizeof(game->gates)); game->active_dungeon=-1; }

void dungeon_spawn_gates(Game *game)
{
    Entity *player=&game->entities[game->player_id];
    int spawned=0, max=MAX_GATES<6?MAX_GATES:6;
    for (int att=0; att<200&&spawned<max; att++) {
        float ang=((float)(rand()%360))*DEG2RAD, dist=400.0f+(float)(rand()%1200);
        Vec2 pos={player->pos.x+cosf(ang)*dist,player->pos.y+sinf(ang)*dist};
        if (!world_pos_walkable(&game->world,pos.x,pos.y)) continue;
        DungeonGate *g=&game->gates[spawned];
        g->active=true; g->world_pos=pos;
        g->rank=(GateRank)(spawned%GATE_RANK_COUNT);
        g->time_limit=GATE_TIME_LIMITS[g->rank];
        g->time_remaining=g->time_limit;
        dungeon_generate_floor(g,(int)g->rank,(unsigned int)(rand()+spawned*1337));
        spawned++;
    }
}

void dungeon_update(Game *game, float dt)
{
    Entity *player=&game->entities[game->player_id];
    for (int i=0;i<MAX_GATES;i++) {
        DungeonGate *g=&game->gates[i];
        if (!g->active) continue;
        g->pulse_timer+=dt;
        if (game->active_dungeon!=i) continue;
        g->time_remaining-=dt;
        if (g->time_remaining<=0&&!g->is_cleared) {
            g->time_remaining=0; dungeon_exit(game); g->active=false;
            int bc=4+(int)g->rank*2;
            for (int j=0;j<bc;j++) {
                float ang=((float)(rand()%360))*DEG2RAD;
                Vec2 p={g->world_pos.x+cosf(ang)*60.0f,g->world_pos.y+sinf(ang)*60.0f};
                npc_spawn(game,ENTITY_NPC_HOSTILE,p,"Gate Monster");
            }
            system_notify(game,NOTIFY_WARNING,"[ GATE BREAK ]","Monsters have poured into the world!");
            camera_shake(game,25.0f,1.0f); continue;
        }
        int alive=0;
        for (int j=0;j<MAX_ENTITIES;j++) {
            Entity *e=&game->entities[j];
            if (!e->active||j==game->player_id) continue;
            if (e->type==ENTITY_DUNGEON_MONSTER||e->type==ENTITY_DUNGEON_BOSS) alive++;
        }
        g->floor_enemies_remaining=alive;
        if (alive==0&&!g->boss_spawned) {
            g->boss_spawned=true;
            Vec2 bp={player->pos.x+100.0f,player->pos.y+80.0f};
            int bid=npc_spawn(game,ENTITY_DUNGEON_BOSS,bp,"Gate Guardian");
            if (bid>=0) {
                Entity *b=&game->entities[bid];
                b->stats.max_hp*=(2+(int)g->rank); b->stats.hp=b->stats.max_hp;
                b->stats.attack*=(1+(int)g->rank/2); b->color=(Color){160,30,200,255};
                b->detection_range=200.0f;
            }
            particle_burst(game,player->pos,(Color){200,50,255,255},40,200.0f,1.5f,6.0f);
            camera_shake(game,20.0f,0.6f);
            system_notify(game,NOTIFY_SYSTEM,"[ BOSS APPEARED ]","A Gate Guardian has emerged!");
        }
        if (g->boss_spawned&&alive==0) {
            g->is_cleared=true;
            player->experience+=GATE_EXP_REWARD[g->rank];
            player->gold+=GATE_GOLD_REWARD[g->rank];
            player->dungeons_cleared++; game->total_dungeons++;
            char buf[64]; snprintf(buf,sizeof(buf),"+%d EXP  +%d Gold",GATE_EXP_REWARD[g->rank],GATE_GOLD_REWARD[g->rank]);
            system_notify(game,NOTIFY_SUCCESS,"[ DUNGEON CLEARED ]",buf);
            quests_on_dungeon_clear(game);
            dungeon_exit(game); g->active=false;
        }
    }
}

void dungeon_draw_gates(const Game *game)
{
    const Entity *player=&game->entities[game->player_id];
    for (int i=0;i<MAX_GATES;i++) {
        const DungeonGate *g=&game->gates[i];
        if (!g->active||game->active_dungeon==i) continue;
        float dx=g->world_pos.x-player->pos.x, dy=g->world_pos.y-player->pos.y;
        if (dx*dx+dy*dy>600.0f*600.0f) continue;
        Color gc=gate_color(g->rank);
        float pulse=sinf(game->game_time*3.0f+i)*0.4f+0.6f;
        Color inner=gc; inner.a=(unsigned char)(180*pulse);
        Color outer=gc; outer.a=(unsigned char)(80*pulse);
        DrawCircle((int)g->world_pos.x,(int)g->world_pos.y,32.0f*pulse,outer);
        DrawCircle((int)g->world_pos.x,(int)g->world_pos.y,20.0f*pulse,inner);
        DrawCircleLines((int)g->world_pos.x,(int)g->world_pos.y,36.0f*pulse,gc);
        const char *rn=gate_rank_name(g->rank);
        DrawText(rn,(int)g->world_pos.x-MeasureText(rn,9)/2,(int)g->world_pos.y-44,9,gc);
        if (dx*dx+dy*dy<60.0f*60.0f) {
            DrawText("[E] Enter Gate",(int)g->world_pos.x-50,(int)g->world_pos.y-58,10,(Color){255,255,200,220});
            char tb[16]; snprintf(tb,sizeof(tb),"%.0fs",g->time_remaining);
            DrawText(tb,(int)g->world_pos.x-10,(int)g->world_pos.y-70,9,g->time_remaining<60?RED:(Color){200,200,200,180});
        }
    }
}

void dungeon_draw_floor(const Game *game)
{
    if (game->active_dungeon<0) return;
    const DungeonGate *g=&game->gates[game->active_dungeon];
    Vector2 cam=game->camera.target; float zoom=game->camera.zoom;
    int hw=(int)(SCREEN_WIDTH/(2.0f*zoom))+TILE_SIZE*2, hh=(int)(SCREEN_HEIGHT/(2.0f*zoom))+TILE_SIZE*2;
    int minx=((int)(cam.x-hw))/TILE_SIZE, maxx=((int)(cam.x+hw))/TILE_SIZE;
    int miny=((int)(cam.y-hh))/TILE_SIZE, maxy=((int)(cam.y+hh))/TILE_SIZE;
    if (minx<0)minx=0; if (miny<0)miny=0;
    if (maxx>=g->floor_width)maxx=g->floor_width-1;
    if (maxy>=g->floor_height)maxy=g->floor_height-1;
    for (int ty=miny;ty<=maxy;ty++) for (int tx=minx;tx<=maxx;tx++) {
        TileType t=g->floor_tiles[ty][tx];
        Color c; switch(t){
        case TILE_DUNGEON_FLOOR: c=(Color){50,45,60,255}; break;
        case TILE_DUNGEON_WALL:  c=(Color){25,20,30,255}; break;
        case TILE_DUNGEON_DOOR:  c=gate_color(g->rank);   break;
        default:                 c=(Color){10,8,15,255};  break; }
        DrawRectangle(tx*TILE_SIZE,ty*TILE_SIZE,TILE_SIZE,TILE_SIZE,c);
        DrawRectangleLines(tx*TILE_SIZE,ty*TILE_SIZE,TILE_SIZE,TILE_SIZE,(Color){0,0,0,20});
    }
    char tb[32]; int m=(int)(g->time_remaining/60),s=(int)g->time_remaining%60;
    snprintf(tb,sizeof(tb),"Time: %02d:%02d",m,s);
    DrawText(tb,SCREEN_WIDTH/2-40,8,16,g->time_remaining<30?RED:(Color){200,200,200,220});
    snprintf(tb,sizeof(tb),"Enemies: %d",g->floor_enemies_remaining);
    DrawText(tb,SCREEN_WIDTH/2-40,28,12,(Color){200,200,200,200});
    DrawRectangle(0,0,6,SCREEN_HEIGHT,gate_color(g->rank));
}

void dungeon_enter(Game *game, int gate_idx)
{
    if (gate_idx<0||gate_idx>=MAX_GATES) return;
    DungeonGate *g=&game->gates[gate_idx];
    if (!g->active||g->is_cleared) return;
    Entity *player=&game->entities[game->player_id];
    g->player_entry_pos=player->pos;
    player->pos.x=8*TILE_SIZE+TILE_SIZE/2.0f;
    player->pos.y=8*TILE_SIZE+TILE_SIZE/2.0f;
    for (int ty=0;ty<g->floor_height;ty++) for (int tx=0;tx<g->floor_width;tx++)
        if (g->floor_tiles[ty][tx]==TILE_DUNGEON_FLOOR) {
            player->pos.x=tx*TILE_SIZE+TILE_SIZE/2.0f;
            player->pos.y=ty*TILE_SIZE+TILE_SIZE/2.0f; goto sp; }
    sp:;
    int ec=GATE_ENEMY_COUNT[g->rank];
    for (int i=0;i<ec;i++) {
        float ang=((float)(rand()%360))*DEG2RAD, dist=100.0f+(float)(rand()%200);
        Vec2 ep={player->pos.x+cosf(ang)*dist,player->pos.y+sinf(ang)*dist};
        int etx=(int)(ep.x/TILE_SIZE), ety=(int)(ep.y/TILE_SIZE);
        if(etx<0)etx=1; if(ety<0)ety=1;
        if(etx>=g->floor_width)etx=g->floor_width-2;
        if(ety>=g->floor_height)ety=g->floor_height-2;
        if (g->floor_tiles[ety][etx]==TILE_DUNGEON_FLOOR) {
            int eid=npc_spawn(game,ENTITY_DUNGEON_MONSTER,ep,"Dungeon Creature");
            if (eid>=0) {
                Entity *e=&game->entities[eid];
                e->stats.attack*=(1+(int)g->rank/2); e->stats.max_hp*=(1+(int)g->rank/3);
                e->stats.hp=e->stats.max_hp; e->color=(Color){100+rand()%80,20,80+rand()%60,255};
            }
        }
    }
    g->floor_enemies_remaining=ec; g->boss_spawned=false; g->is_entered=true;
    game->active_dungeon=gate_idx; game->state=STATE_DUNGEON;
    system_notify(game,NOTIFY_SYSTEM,"[ DUNGEON ENTERED ]",gate_rank_name(g->rank));
    particle_burst(game,player->pos,gate_color(g->rank),30,150.0f,1.0f,5.0f);
    quests_on_dungeon_enter(game);
}

void dungeon_exit(Game *game)
{
    if (game->active_dungeon<0) return;
    DungeonGate *g=&game->gates[game->active_dungeon];
    for (int i=0;i<MAX_ENTITIES;i++) {
        Entity *e=&game->entities[i];
        if (!e->active||i==game->player_id) continue;
        if (e->type==ENTITY_DUNGEON_MONSTER||e->type==ENTITY_DUNGEON_BOSS) e->active=false;
    }
    game->entities[game->player_id].pos=g->player_entry_pos;
    game->active_dungeon=-1; game->state=STATE_PLAYING;
}

int dungeon_find_nearby_gate(const Game *game, float range)
{
    const Entity *p=&game->entities[game->player_id];
    for (int i=0;i<MAX_GATES;i++) {
        const DungeonGate *g=&game->gates[i];
        if (!g->active||g->is_cleared) continue;
        float dx=g->world_pos.x-p->pos.x, dy=g->world_pos.y-p->pos.y;
        if (dx*dx+dy*dy<range*range) return i;
    }
    return -1;
}
