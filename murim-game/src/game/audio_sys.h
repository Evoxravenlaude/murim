/*
 * audio_sys.h — Audio/Sound System
 * Heavenly Demon: Murim Chronicles v4.0
 *
 * Wraps Raylib audio. Procedurally generates all sounds
 * so no external audio files are required.
 */
#ifndef AUDIO_SYS_H
#define AUDIO_SYS_H

#include "../engine/types.h"

/* ─── Sound IDs ───────────────────────────────────────── */
typedef enum {
    SFX_SWORD_SWING = 0,
    SFX_SWORD_HIT,
    SFX_QI_BLAST,
    SFX_HEAVENLY_STRIKE,
    SFX_DRAGON_FIST,
    SFX_STEP_GRASS,
    SFX_STEP_STONE,
    SFX_CULTIVATION_HUM,
    SFX_BREAKTHROUGH,
    SFX_LEVEL_UP,
    SFX_RANK_UP,
    SFX_ITEM_PICKUP,
    SFX_ITEM_USE,
    SFX_DASH,
    SFX_STAGGER,
    SFX_EXECUTION,
    SFX_GATE_OPEN,
    SFX_BOSS_APPEAR,
    SFX_GATE_BREAK,
    SFX_ARISE,
    SFX_CAPTURE_ORB,
    SFX_EVOLUTION,
    SFX_FISH_BITE,
    SFX_FISH_CAUGHT,
    SFX_SYSTEM_NOTIFY,
    SFX_DIALOGUE_BLIP,
    SFX_THUNDER,
    SFX_WIND_GUST,
    SFX_WATER_AMBIENT,
    SFX_MEDITATION_CHIME,
    SFX_COUNT
} SfxID;

void audio_init(void);
void audio_cleanup(void);
void audio_play(SfxID id);
void audio_play_volume(SfxID id, float volume);
void audio_set_master_volume(float vol);
void audio_update(Game *game, float dt); /* play ambient sounds */

#endif /* AUDIO_SYS_H */
