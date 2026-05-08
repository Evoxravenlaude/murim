/*
 * save.h — Save/Load System
 * Heavenly Demon: Murim Chronicles v4.0
 */
#ifndef SAVE_H
#define SAVE_H

#include "../engine/types.h"
#include <stdbool.h>

#define SAVE_FILE_PATH  "murim_save.dat"
#define SAVE_MAGIC      0x4D524D47  /* 'MRMG' */
#define SAVE_VERSION    4

bool save_game(const Game *game);
bool load_game(Game *game);
bool save_exists(void);
void save_delete(void);

#endif /* SAVE_H */
