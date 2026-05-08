/*
 * audio_sys.c — Procedural Audio System (no external files needed)
 * Heavenly Demon: Murim Chronicles v4.0
 *
 * Generates Wave objects on the fly using Raylib's audio API.
 * Each sound is synthesized from simple waveforms.
 */
#include "audio_sys.h"
#include "world.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE  44100
#define CHANNELS     1

static Sound s_sounds[SFX_COUNT];
static bool  s_initialized = false;
static float s_master_vol  = 0.7f;

/* ─── PCM synthesis helpers ───────────────────────────── */
static Wave make_wave(float duration, float freq, float freq_end,
                      float volume, int wave_type)
{
    int samples = (int)(SAMPLE_RATE * duration);
    short *data = (short *)MemAlloc(samples * sizeof(short));

    for (int i = 0; i < samples; i++) {
        float t    = (float)i / SAMPLE_RATE;
        float prog = (float)i / samples;
        float f    = freq + (freq_end - freq) * prog;
        float env  = (prog < 0.05f) ? prog / 0.05f :
                     (prog > 0.7f)  ? (1.0f - prog) / 0.3f : 1.0f;
        float s    = 0;
        if (wave_type == 0) s = sinf(2.0f * 3.14159f * f * t);          /* sine   */
        else if (wave_type == 1) s = (fmodf(f * t, 1.0f) < 0.5f) ? 1.0f : -1.0f; /* square */
        else s = (2.0f * fmodf(f * t, 1.0f)) - 1.0f;                    /* saw    */
        /* add noise for percussive sounds */
        if (wave_type == 3) s = ((float)(rand() % 2000 - 1000)) / 1000.0f;
        data[i] = (short)(s * env * volume * 32767.0f);
    }

    Wave w;
    w.frameCount = samples;
    w.sampleRate = SAMPLE_RATE;
    w.sampleSize = 16;
    w.channels   = CHANNELS;
    w.data       = data;
    return w;
}

static Sound load_sfx(float dur, float f0, float f1, float vol, int type)
{
    Wave w = make_wave(dur, f0, f1, vol, type);
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}

void audio_init(void)
{
    if (s_initialized) return;
    InitAudioDevice();
    s_initialized = true;

    /* Build all sounds procedurally */
    s_sounds[SFX_SWORD_SWING]      = load_sfx(0.18f, 800,  200,  0.5f, 1);
    s_sounds[SFX_SWORD_HIT]        = load_sfx(0.12f, 300,  100,  0.8f, 3);
    s_sounds[SFX_QI_BLAST]         = load_sfx(0.35f, 400,  150,  0.7f, 0);
    s_sounds[SFX_HEAVENLY_STRIKE]  = load_sfx(0.45f, 600,   80,  0.9f, 1);
    s_sounds[SFX_DRAGON_FIST]      = load_sfx(0.40f, 250,   60,  1.0f, 3);
    s_sounds[SFX_STEP_GRASS]       = load_sfx(0.06f, 180,  100,  0.2f, 3);
    s_sounds[SFX_STEP_STONE]       = load_sfx(0.05f, 300,  200,  0.3f, 3);
    s_sounds[SFX_CULTIVATION_HUM]  = load_sfx(1.00f, 110,  115,  0.3f, 0);
    s_sounds[SFX_BREAKTHROUGH]     = load_sfx(1.20f, 200,  800,  1.0f, 0);
    s_sounds[SFX_LEVEL_UP]         = load_sfx(0.80f, 330,  660,  0.8f, 0);
    s_sounds[SFX_RANK_UP]          = load_sfx(1.00f, 220,  880,  0.9f, 0);
    s_sounds[SFX_ITEM_PICKUP]      = load_sfx(0.15f, 600,  900,  0.5f, 0);
    s_sounds[SFX_ITEM_USE]         = load_sfx(0.20f, 440,  550,  0.5f, 0);
    s_sounds[SFX_DASH]             = load_sfx(0.12f, 900,  300,  0.4f, 1);
    s_sounds[SFX_STAGGER]          = load_sfx(0.25f, 150,   50,  0.8f, 3);
    s_sounds[SFX_EXECUTION]        = load_sfx(0.55f, 100,  400,  1.0f, 1);
    s_sounds[SFX_GATE_OPEN]        = load_sfx(0.90f, 80,   300,  0.9f, 0);
    s_sounds[SFX_BOSS_APPEAR]      = load_sfx(1.50f, 55,   110,  1.0f, 1);
    s_sounds[SFX_GATE_BREAK]       = load_sfx(1.00f, 200,   30,  1.0f, 3);
    s_sounds[SFX_ARISE]            = load_sfx(1.20f, 60,   220,  1.0f, 0);
    s_sounds[SFX_CAPTURE_ORB]      = load_sfx(0.40f, 500,  1000, 0.6f, 0);
    s_sounds[SFX_EVOLUTION]        = load_sfx(1.00f, 220,  880,  0.9f, 0);
    s_sounds[SFX_FISH_BITE]        = load_sfx(0.10f, 700,  500,  0.5f, 0);
    s_sounds[SFX_FISH_CAUGHT]      = load_sfx(0.60f, 440,  880,  0.7f, 0);
    s_sounds[SFX_SYSTEM_NOTIFY]    = load_sfx(0.30f, 880, 1100,  0.4f, 0);
    s_sounds[SFX_DIALOGUE_BLIP]    = load_sfx(0.04f, 500,  500,  0.3f, 0);
    s_sounds[SFX_THUNDER]          = load_sfx(0.80f, 80,    20,  1.0f, 3);
    s_sounds[SFX_WIND_GUST]        = load_sfx(0.60f, 200,  100,  0.3f, 3);
    s_sounds[SFX_WATER_AMBIENT]    = load_sfx(0.50f, 150,  160,  0.2f, 3);
    s_sounds[SFX_MEDITATION_CHIME] = load_sfx(1.50f, 528,  264,  0.4f, 0);

    SetMasterVolume(s_master_vol);
}

void audio_cleanup(void)
{
    if (!s_initialized) return;
    for (int i = 0; i < SFX_COUNT; i++)
        UnloadSound(s_sounds[i]);
    CloseAudioDevice();
    s_initialized = false;
}

void audio_play(SfxID id)
{
    if (!s_initialized || id < 0 || id >= SFX_COUNT) return;
    PlaySound(s_sounds[id]);
}

void audio_play_volume(SfxID id, float vol)
{
    if (!s_initialized || id < 0 || id >= SFX_COUNT) return;
    SetSoundVolume(s_sounds[id], vol * s_master_vol);
    PlaySound(s_sounds[id]);
}

void audio_set_master_volume(float vol)
{
    s_master_vol = vol;
    if (s_initialized) SetMasterVolume(vol);
}

/* Ambient sound timers */
static float s_step_timer   = 0;
static float s_ambient_timer = 0;

void audio_update(Game *game, float dt)
{
    if (!s_initialized) return;

    Entity *player = &game->entities[game->player_id];

    /* Footstep sounds */
    float speed_sq = player->vel.x * player->vel.x + player->vel.y * player->vel.y;
    if (speed_sq > 100.0f) {
        s_step_timer -= dt;
        if (s_step_timer <= 0) {
            s_step_timer = player->dash.is_dashing ? 0.12f : 0.28f;
            TileType tile = world_get_tile_at(&game->world, player->pos.x, player->pos.y);
            if (tile == TILE_STONE || tile == TILE_DUNGEON_FLOOR)
                audio_play_volume(SFX_STEP_STONE, 0.4f);
            else
                audio_play_volume(SFX_STEP_GRASS, 0.3f);
        }
    }

    /* Ambient water sound near water tiles */
    s_ambient_timer -= dt;
    if (s_ambient_timer <= 0) {
        s_ambient_timer = 4.0f;
        TileType tile = world_get_tile_at(&game->world, player->pos.x, player->pos.y);
        if (tile == TILE_WATER || player->environment.is_underwater)
            audio_play_volume(SFX_WATER_AMBIENT, 0.2f);
    }

    /* Cultivation meditation chime */
    if (player->cultivation.is_cultivating) {
        static float chime_t = 0;
        chime_t -= dt;
        if (chime_t <= 0) {
            chime_t = 8.0f;
            audio_play_volume(SFX_MEDITATION_CHIME, 0.3f);
        }
    }

    /* Thunder during storms */
    if (game->world.weather.lightning_flash) {
        audio_play(SFX_THUNDER);
    }
}
