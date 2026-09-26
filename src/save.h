#ifndef NSPIRE_RETRO_SAVE_H
#define NSPIRE_RETRO_SAVE_H
#include "app.h"
/* Alternating checksummed slots. The last valid slot survives interrupted writes. */
bool save_load(App *a, const char *prefix);
bool save_write(App *a, const char *prefix);
#endif
