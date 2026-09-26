#include "app.h"
#include <string.h>
static void navigate(App *a, Screen screen) { a->screen = screen; a->selection = 0; }
static void menu(App *a, const Input *in, int count) {
    if (in->up_pressed) a->selection = (a->selection + count - 1) % count;
    if (in->down_pressed) a->selection = (a->selection + 1) % count;
}
static void updated(App *a, bool ok) {
    a->notice = ok ? 4 : 3; a->notice_ticks = 90;
    if (ok) a->save_requested = true;
}
void app_init(App *a) {
    memset(a, 0, sizeof(*a));
    a->settings = (Settings){1, 90, 1};
    season_init(&a->season, 0); game_init(&a->game);
    a->screen = SCREEN_TITLE;
}
void app_start_match(App *a) {
    if (a->season.week >= SEASON_WEEKS) return;
    int ratings[ROSTER_COUNT];
    for (int i = 0; i < ROSTER_COUNT; ++i) ratings[i] = season_effective_rating(&a->season, i);
    int opponent = season_opponent(a->season.team, a->season.week);
    game_start(&a->game, a->settings.quarter_seconds, a->settings.difficulty, ratings,
        season_team_rating(opponent, a->season.year), a->season.rng ^ (uint32_t)(a->season.week + a->season.year * 100));
    a->game.home_team = a->season.team; a->game.away_team = opponent;
    a->match_active = true; a->save_requested = true; navigate(a, SCREEN_MATCH);
}
void app_save_result(App *a, bool success) {
    a->save_requested = false; a->notice = success ? 1 : 2; a->notice_ticks = 120;
    if (a->exit_after_save && success) a->exit_requested = true;
    a->exit_after_save = false; /* Failure leaves the user in the app with retry available. */
}
void app_update(App *a, const Input *in) {
    a->frame = (a->frame + 1) % 120;
    if (a->notice_ticks > 0) --a->notice_ticks;
    switch (a->screen) {
    case SCREEN_TITLE:
        menu(a, in, 6);
        if (in->action_pressed) {
            if (a->selection == 0 && a->has_career) navigate(a, a->match_active ? SCREEN_MATCH : SCREEN_HUB);
            if (a->selection == 1) navigate(a, a->has_career ? SCREEN_CONFIRM : SCREEN_TEAM);
            if (a->selection == 2) { a->return_screen = SCREEN_TITLE; navigate(a, SCREEN_SETTINGS); }
            if (a->selection == 3) { a->return_screen = SCREEN_TITLE; navigate(a, SCREEN_HELP); }
            if (a->selection == 4) { a->save_requested = true; a->exit_after_save = true; }
            if (a->selection == 5) a->exit_requested = true;
        }
        break;
    case SCREEN_CONFIRM:
        menu(a, in, 2);
        if (in->quit || (in->action_pressed && a->selection == 0)) navigate(a, SCREEN_TITLE);
        else if (in->action_pressed) navigate(a, SCREEN_TEAM);
        break;
    case SCREEN_TEAM:
        menu(a, in, TEAM_COUNT); a->team_selection = a->selection;
        if (in->quit) navigate(a, SCREEN_TITLE);
        else if (in->action_pressed) {
            season_init(&a->season, a->selection); a->has_career = true; a->match_active = false;
            a->save_requested = true; navigate(a, SCREEN_HUB);
        }
        break;
    case SCREEN_HUB:
        menu(a, in, 6);
        if (in->quit) navigate(a, SCREEN_TITLE);
        else if (in->action_pressed) switch (a->selection) {
        case 0:
            if (a->match_active) navigate(a, SCREEN_MATCH);
            else if (a->season.week == SEASON_WEEKS) { season_next(&a->season); updated(a, true); }
            else app_start_match(a);
            break;
        case 1: a->roster_selection = 0; navigate(a, SCREEN_ROSTER); break;
        case 2: navigate(a, SCREEN_SCHEDULE); break;
        case 3: a->return_screen = SCREEN_HUB; navigate(a, SCREEN_SETTINGS); break;
        case 4: a->save_requested = true; a->exit_after_save = true; break;
        case 5: navigate(a, SCREEN_TITLE); break;
        }
        break;
    case SCREEN_ROSTER:
        if (in->quit) { navigate(a, SCREEN_HUB); break; }
        menu(a, in, ROSTER_COUNT); a->roster_selection = a->selection;
        if (a->match_active) break; /* A suspended game's roster stays fixed. */
        if (in->action_pressed) updated(a, season_train(&a->season, a->selection));
        else if (in->target_pressed) updated(a, season_recruit(&a->season, a->selection));
        else if (in->right_pressed) updated(a, season_recover(&a->season));
        break;
    case SCREEN_SCHEDULE:
        if (in->quit) navigate(a, SCREEN_HUB);
        if (in->left_pressed || in->right_pressed || in->action_pressed) a->selection = !a->selection;
        break;
    case SCREEN_SETTINGS:
        menu(a, in, 3);
        if (in->quit) { navigate(a, a->return_screen); break; }
        if (in->left_pressed || in->right_pressed || in->action_pressed) {
            int delta = in->left_pressed ? -1 : 1;
            if (a->selection == 0) a->settings.difficulty = (a->settings.difficulty + delta + 3) % 3;
            if (a->selection == 1) {
                int idx = a->settings.quarter_seconds / 30 - 2;
                a->settings.quarter_seconds = (((idx + delta + 3) % 3) + 2) * 30;
            }
            if (a->selection == 2) a->settings.animations = !a->settings.animations;
            a->save_requested = true;
        }
        break;
    case SCREEN_HELP:
        if (in->quit || in->action_pressed) navigate(a, a->return_screen);
        break;
    case SCREEN_MATCH:
        if (in->quit) { navigate(a, SCREEN_PAUSE); break; }
        if (a->game.phase == PHASE_FINAL && in->action_pressed) {
            season_record(&a->season, a->game.score, a->game.opponent_score);
            a->match_active = false; a->save_requested = true; navigate(a, SCREEN_HUB);
        } else {
            Phase old = a->game.phase;
            game_update(&a->game, in);
            if (old != a->game.phase) a->save_requested = true;
        }
        break;
    case SCREEN_PAUSE:
        menu(a, in, 6);
        if (in->quit) navigate(a, SCREEN_MATCH);
        else if (in->action_pressed) switch (a->selection) {
        case 0: navigate(a, SCREEN_MATCH); break;
        case 1: a->save_requested = true; a->exit_after_save = true; break;
        case 2: a->return_screen = SCREEN_PAUSE; navigate(a, SCREEN_SETTINGS); break;
        case 3: a->return_screen = SCREEN_PAUSE; navigate(a, SCREEN_HELP); break;
        case 4: a->save_requested = true; navigate(a, SCREEN_HUB); break;
        case 5: a->exit_requested = true; break;
        }
        break;
    }
}
