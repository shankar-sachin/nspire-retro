#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libndls.h>
#include "game.h"
#include "render.h"
static bool keys[12];
static bool lcd_active;
static const char *capture;
static int frames;
bool isKeyPressed(int key) { return keys[key]; }
bool lcd_init(scr_type_t type) { lcd_active = type != SCR_TYPE_INVALID; return true; }
void lcd_blit(void *buffer, scr_type_t type) {
    assert(lcd_active && type == SCR_320x240_565);
    ++frames;
    if (capture) {
        FILE *file = fopen(capture, "wb"); assert(file);
        fputs("P6\n320 240\n255\n", file);
        for (int i = 0; i < 320 * 240; ++i) {
            uint16_t pixel = ((uint16_t *)buffer)[i];
            unsigned char rgb[3] = {(unsigned char)(((pixel >> 11) & 31) * 255 / 31),
                (unsigned char)(((pixel >> 5) & 63) * 255 / 63), (unsigned char)((pixel & 31) * 255 / 31)};
            assert(fwrite(rgb, 1, 3, file) == 3);
        }
        fclose(file); capture = NULL;
    }
}
int main(void) {
    Input in;
    input_init(); keys[KEY_NSPIRE_ENTER] = true;
    in = input_poll(); assert(!in.action_pressed); /* launch key is suppressed */
    keys[KEY_NSPIRE_ENTER] = false; input_poll();
    keys[KEY_NSPIRE_ENTER] = true; in = input_poll(); assert(in.action_pressed);
    in = input_poll(); assert(!in.action_pressed);
    keys[KEY_NSPIRE_UPRIGHT] = true; in = input_poll(); assert(in.dx == 1 && in.dy == -1);
    keys[KEY_NSPIRE_CTRL] = true; in = input_poll(); assert(in.target_pressed);
    in = input_poll(); assert(!in.target_pressed);
    keys[KEY_NSPIRE_ESC] = true; in = input_poll(); assert(in.quit);
    in = input_poll(); assert(!in.quit);
    keys[KEY_NSPIRE_SHIFT] = true; in = input_poll(); assert(in.boost);
    memset(keys, 0, sizeof(keys));
    Game g; game_init(&g); assert(render_init());
    capture = "build/play-call.ppm"; render_frame(&g);
    in = (Input){.action_pressed = true}; g.selected_play = PASS_SLANT; game_update(&g, &in);
    capture = "build/field.ppm"; render_frame(&g);
    for (int tick = 0; tick < 500; ++tick) {
        in = (Input){.dx = 1, .dy = tick % 80 < 40 ? 1 : -1, .action_pressed = tick % 25 == 0};
        game_update(&g, &in); render_frame(&g);
    }
    g.phase = PHASE_RESULT; g.result = RESULT_TOUCHDOWN; g.score = 6; g.new_drive = true;
    capture = "build/result.ppm"; render_frame(&g);
    /* Exercise clipping at both ends and actor edges under the sanitizers. */
    for (int i = -24; i < FIELD_LENGTH + 24; ++i) {
        g.carrier.x = i * FP; render_frame(&g);
    }
    App a; app_init(&a);
    capture = "build/title.ppm"; render_app(&a);
    a.screen = SCREEN_TEAM; a.selection = 2;
    capture = "build/teams.ppm"; render_app(&a);
    a.has_career = true; season_init(&a.season, 2); a.screen = SCREEN_HUB;
    capture = "build/clubhouse.ppm"; render_app(&a);
    a.screen = SCREEN_ROSTER; a.selection = 0;
    capture = "build/roster.ppm"; render_app(&a);
    a.screen = SCREEN_SCHEDULE; a.selection = 1;
    capture = "build/standings.ppm"; render_app(&a);
    for (int screen = SCREEN_TITLE; screen <= SCREEN_CONFIRM; ++screen) {
        a.screen = (Screen)screen; a.selection = 0;
        render_app(&a);
    }
    app_start_match(&a); a.game.phase = PHASE_OPPONENT;
    capture = "build/opponent.ppm"; render_app(&a);
    a.game.phase = PHASE_FINAL; a.game.quarter = 4; a.game.clock_ticks = 0; a.game.score = 21; a.game.opponent_score = 14;
    capture = "build/final.ppm"; render_app(&a);
    a.screen = SCREEN_PAUSE; a.selection = 5;
    capture = "build/pause.ppm"; render_app(&a);
    a.settings.animations = 0; render_app(&a);
    render_shutdown(); assert(!lcd_active && frames > 1100);
    puts("Input edges and rendering passed; previews written to build/*.ppm.");
    return 0;
}
