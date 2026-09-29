#ifndef NSPIRE_RETRO_APP_H
#define NSPIRE_RETRO_APP_H
#include "game.h"
#include "season.h"
typedef enum { SCREEN_TITLE, SCREEN_TEAM, SCREEN_HUB, SCREEN_ROSTER, SCREEN_SCHEDULE,
    SCREEN_SETTINGS, SCREEN_HELP, SCREEN_MATCH, SCREEN_PAUSE, SCREEN_CONFIRM, SCREEN_PLAYER, SCREEN_RELEASE, SCREEN_FREE_AGENTS, SCREEN_DRAFT, SCREEN_SIGN } Screen;
typedef struct { int difficulty, quarter_seconds, animations; } Settings;
typedef struct {
    Season season;
    Game game;
    Settings settings;
    Screen screen, return_screen;
    int page, selection, roster_selection, team_selection, frame;
    bool aim_rearm; /* A menu confirmation must never release a saved aimed pass. */
    bool has_career, match_active, save_requested, exit_requested, exit_after_save;
    int notice, notice_ticks; /* 1 saved, 2 failed, 3 funds/limit, 4 updated, 5 recovered */
    uint32_t save_sequence, career_seed;
} App;
void app_init(App *a);
void app_update(App *a, const Input *input);
void app_save_result(App *a, bool success);
void app_start_match(App *a);
#endif
