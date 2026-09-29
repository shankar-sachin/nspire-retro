#include "app.h"
#include <string.h>
static void navigate(App *a, Screen screen) {
    a->screen = screen; a->selection = 0; a->page = 0;
    if (screen == SCREEN_MATCH) a->aim_rearm = a->game.aiming;
}
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
    a->career_seed = 0x712af901u;
    a->settings = (Settings){1, 90, 1};
    season_init(&a->season, 0); game_init(&a->game);
    a->screen = SCREEN_TITLE;
}
void app_start_match(App *a) {
    int opponent = season_current_opponent(&a->season);
    if (opponent < 0 || a->match_active) return;
    int ratings[ROSTER_COUNT];
    for (int i = 0; i < ROSTER_COUNT; ++i) ratings[i] = season_effective_rating(&a->season, i);
    game_start(&a->game, a->settings.quarter_seconds, a->settings.difficulty, ratings,
        season_team_rating(opponent, a->season.year), a->season.rng ^ (uint32_t)(a->season.week + a->season.year * 100));
    a->game.postseason = a->season.stage != STAGE_REGULAR;
    a->game.bowl_game = a->season.stage == STAGE_TI_BOWL;
    a->game.home_team = a->season.team; a->game.away_team = opponent;
    a->match_active = true; a->save_requested = true; navigate(a, SCREEN_MATCH);
}
void app_save_result(App *a, bool success) {
    a->save_requested = false; a->notice = success ? 1 : 2; a->notice_ticks = 120;
    if (a->exit_after_save && success) a->exit_requested = true;
    a->exit_after_save = false; /* Failure leaves the user in the app with retry available. */
}
void app_update(App *a, const Input *in) {
    a->career_seed = a->career_seed * 1664525u + 1013904223u;
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
            season_init_seed(&a->season, a->selection, a->career_seed); a->has_career = true; a->match_active = false;
            a->save_requested = true; navigate(a, SCREEN_HUB);
        }
        break;
    case SCREEN_HUB:
        menu(a, in, 7);
        if (in->quit) navigate(a, SCREEN_TITLE);
        else if (in->action_pressed) switch (a->selection) {
        case 0:
            if (a->match_active) navigate(a, SCREEN_MATCH);
            else if (a->season.stage == STAGE_COMPLETE) { season_next(&a->season); updated(a, true); navigate(a, SCREEN_DRAFT); }
            else if (a->season.draft_active) navigate(a, SCREEN_DRAFT);
            else if (season_current_opponent(&a->season) < 0) { season_simulate_round(&a->season); updated(a, true); }
            else app_start_match(a);
            break;
        case 1: a->roster_selection = 0; navigate(a, SCREEN_ROSTER); break;
        case 2: navigate(a, SCREEN_SCHEDULE); break;
        case 3: a->return_screen = SCREEN_HUB; navigate(a, SCREEN_SETTINGS); break;
        case 4: a->save_requested = true; a->exit_after_save = true; break;
        case 5: navigate(a, SCREEN_TITLE); break;
        case 6: navigate(a, SCREEN_FREE_AGENTS); break;
        }
        break;
    case SCREEN_ROSTER:
        if (in->target_pressed) { navigate(a, SCREEN_FREE_AGENTS); break; }
        if (in->quit) { navigate(a, SCREEN_HUB); break; }
        menu(a, in, ROSTER_COUNT); a->roster_selection = season_roster_role(a->selection);
        if (in->action_pressed) navigate(a, SCREEN_PLAYER);
        break;
    case SCREEN_PLAYER: {
        int role = a->roster_selection;
        if (in->quit) { navigate(a, SCREEN_ROSTER); a->selection = season_roster_row(role); break; }
        menu(a, in, 5);
        if (a->match_active || !in->action_pressed) break;
        if (a->selection == 0) updated(a, season_train(&a->season, role));
        if (a->selection == 1) { navigate(a, SCREEN_FREE_AGENTS); a->selection = season_roster_row(role); break; }
        else if (a->selection == 2) {
            if (!season_can_sign(&a->season, role, season_salary(a->season.roster[role].rating, role))) { a->notice = 6; a->notice_ticks = 120; }
            else updated(a, season_renew(&a->season, role));
        }
        if (a->selection == 3) { navigate(a, SCREEN_RELEASE); break; }
        if (a->selection == 4) updated(a, season_recover(&a->season));
        break;
    }
    case SCREEN_RELEASE:
        menu(a, in, 2);
        if (in->quit || (in->action_pressed && a->selection == 0)) navigate(a, SCREEN_PLAYER);
        else if (in->action_pressed && !a->match_active) {
            updated(a, season_release(&a->season, a->roster_selection)); navigate(a, SCREEN_PLAYER);
        }
        break;
    case SCREEN_FREE_AGENTS:
    case SCREEN_DRAFT: {
        bool draft = a->screen == SCREEN_DRAFT;
        if (in->quit) { navigate(a, SCREEN_HUB); break; }
        menu(a, in, ROSTER_COUNT + (draft ? 1 : 0));
        if (in->action_pressed && !a->match_active && (!draft || a->season.draft_active)) {
            a->roster_selection = a->selection == ROSTER_COUNT ? -1 : season_roster_role(a->selection);
            a->return_screen = a->screen; navigate(a, SCREEN_SIGN);
        }
        break;
    }
    case SCREEN_SIGN: {
        int role = a->roster_selection; bool draft = a->return_screen == SCREEN_DRAFT;
        menu(a, in, 2);
        if (in->quit || (in->action_pressed && a->selection == 0)) {
            navigate(a, a->return_screen); a->selection = role < 0 ? ROSTER_COUNT : season_roster_row(role);
        } else if (in->action_pressed && !a->match_active) {
            bool ok;
            if (role < 0) { ok = a->season.draft_active; season_skip_pick(&a->season); }
            else ok = draft ? season_draft(&a->season, role) : season_recruit(&a->season, role);
            updated(a, ok);
            if (ok) {
                navigate(a, draft && !a->season.draft_active ? SCREEN_HUB : a->return_screen);
                if (a->screen != SCREEN_HUB) a->selection = role < 0 ? ROSTER_COUNT : season_roster_row(role);
            }
        }
        break;
    }
    case SCREEN_SCHEDULE: {
        if (in->quit) { navigate(a, SCREEN_HUB); break; }
        if (in->left_pressed || in->right_pressed || in->action_pressed) {
            a->selection = (a->selection + (in->left_pressed ? 3 : 1)) % 4; a->page = 0;
        }
        int pages = a->selection == 0 ? (season_regular_weeks(&a->season) + 7) / 8 :
            (a->selection == 1 ? (a->season.league_size + 7) / 8 : (a->selection == 2 ? 2 : season_regular_weeks(&a->season)));
        if (in->up_pressed) a->page = (a->page + pages - 1) % pages;
        if (in->down_pressed) a->page = (a->page + 1) % pages;
        break;
    }
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
            Input play = *in;
            if (a->aim_rearm) {
                if (!in->action_pressed) break;
                a->aim_rearm = false; play.action_pressed = play.action_released = false;
            }
            Phase old = a->game.phase;
            game_update(&a->game, &play);
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
