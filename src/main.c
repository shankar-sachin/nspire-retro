#include <libndls.h>
#include "app.h"
#include "save.h"
#include "input.h"
#include "render.h"
int main(int argc, char **argv) {
    static App app;
    assert_ndless_rev(2004);
    if (!has_colors) return 1;
    bool storage_ready = argc > 0 && argv && argv[0] && enable_relative_paths(argv) == 0;
    app_init(&app);
    if (storage_ready) save_load(&app, "nspire-retro-save");
    if (!render_init()) { lcd_init(SCR_TYPE_INVALID); return 1; }
    if (!storage_ready) { app.notice = 2; app.notice_ticks = 300; }
    input_init();
    while (!app.exit_requested) {
        Input input = input_poll();
        app_update(&app, &input);
        if (app.save_requested) app_save_result(&app, storage_ready && save_write(&app, "nspire-retro-save"));
        render_app(&app);
        /* About 30 fps plus render time; Ndless clock() is not a frame timer. */
        msleep(1000 / GAME_HZ);
    }
    render_shutdown(); wait_no_key_pressed();
    return 0;
}
