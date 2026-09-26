#ifndef NSPIRE_RETRO_RENDER_H
#define NSPIRE_RETRO_RENDER_H
#include <stdbool.h>
#include "app.h"
bool render_init(void);
void render_app(const App *a);
void render_frame(const Game *g);
void render_shutdown(void);
#endif
