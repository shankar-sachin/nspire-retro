#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app.h"
#include "save.h"
#define YARD (PX_PER_YARD * FP)
static const Input action = {.action_pressed = true};
static const Input idle = {0};
static void test_schedule(void) {
    for (int team = 0; team < TEAM_COUNT; ++team) {
        unsigned seen = 0;
        for (int week = 0; week < SEASON_WEEKS; ++week) {
            int opponent = season_opponent(team, week);
            assert(opponent != team && opponent >= 0 && opponent < TEAM_COUNT);
            assert(!(seen & (1u << opponent))); seen |= 1u << opponent;
            assert(season_opponent(opponent, week) == team);
        }
        assert(seen == ((1u << TEAM_COUNT) - 1u) - (1u << team));
    }
    assert(!strcmp(season_team_abbr(0), "NYG"));
    assert(!strcmp(season_team_abbr(1), "GB"));
    assert(!strcmp(season_team_abbr(2), "SEA"));
}
static void test_progression(void) {
    Season s; season_init(&s, 2);
    assert(season_train(&s, ROLE_QB)); assert(s.roster[0].rating == 62 && s.credits == 14);
    assert(season_recover(&s)); assert(s.credits == 9 && s.roster[0].condition == 100);
    assert(!season_recover(&s)); assert(!season_recruit(&s, ROLE_QB));
    s.credits = 100; assert(season_recruit(&s, ROLE_QB)); assert(s.roster[0].rating == season_recruit_rating(&s, ROLE_QB));
    for (int week = 0; week < SEASON_WEEKS; ++week) {
        season_record(&s, 28, 0);
        for (int t = 0; t < TEAM_COUNT; ++t)
            assert(s.table[t].wins + s.table[t].losses + s.table[t].ties == week + 1);
    }
    assert(s.week == SEASON_WEEKS && s.trophies == 1 && season_rank(&s, 2) == 1);
    assert(s.career_wins == SEASON_WEEKS && s.roster[ROLE_RB].rating > 63);
    int credits = s.credits; season_record(&s, 7, 0); assert(s.credits == credits);
    season_next(&s); assert(s.week == 0 && s.year == 2 && s.trophies == 1 && s.credits == credits);
    for (int i = 0; i < ROSTER_COUNT; ++i) assert(s.roster[i].condition == 100);
    season_record(&s, 7, 7); assert(s.table[2].ties == 1);
}
static void test_clock_and_cpu(void) {
    Game g; game_init(&g);
    int clock = g.clock_ticks; game_update(&g, &idle); assert(g.clock_ticks == clock);
    game_update(&g, &action); game_update(&g, &idle); assert(g.clock_ticks == clock - 1);
    g.clock_ticks = 1; g.carrier.y = 0; game_update(&g, &idle);
    assert(g.phase == PHASE_RESULT && g.clock_ticks == 0);
    game_update(&g, &action); assert(g.phase == PHASE_BREAK);
    int spot = g.spot, down = g.down;
    game_update(&g, &action); assert(g.quarter == 2 && g.clock_ticks == 90 * GAME_HZ && g.spot == spot && g.down == down);
    g.clock_ticks = 0; g.phase = PHASE_RESULT; game_update(&g, &action); game_update(&g, &action);
    assert(g.quarter == 3 && g.phase == PHASE_OPPONENT && g.cpu_spot == 25);
    game_update(&g, &action); assert(g.clock_ticks < 90 * GAME_HZ);
    g.clock_ticks = 0; g.quarter = 4; game_update(&g, &action); assert(g.phase == PHASE_FINAL);
    game_init(&g); game_kick(&g, false); assert(g.phase == PHASE_KICK);
    g.kick_meter = 50; game_update(&g, &action); assert(g.result == RESULT_PUNT && g.new_drive);
    game_update(&g, &action); assert(g.phase == PHASE_OPPONENT && g.cpu_spot == g.opponent_start);
    game_init(&g); game_kick(&g, true); g.kick_meter = 50; game_update(&g, &action); assert(g.result == RESULT_MISSED_KICK);
    game_init(&g);
    g.spot = 90 * YARD; game_kick(&g, true); g.kick_meter = 50; game_update(&g, &action); assert(g.result == RESULT_FIELD_GOAL);
    game_init(&g); g.phase = PHASE_LIVE; g.carrier.x = 0; game_update(&g, &idle);
    assert(g.opponent_score == 2 && g.result == RESULT_SAFETY);
}
static void test_menus_pause(void) {
    App a; app_init(&a); assert(!a.has_career && a.screen == SCREEN_TITLE);
    a.selection = 1; app_update(&a, &action); assert(a.screen == SCREEN_TEAM);
    a.selection = 2; app_update(&a, &action); assert(a.has_career && a.season.team == 2 && a.screen == SCREEN_HUB);
    app_update(&a, &action); assert(a.match_active && a.screen == SCREEN_MATCH);
    app_update(&a, &action); assert(a.game.phase == PHASE_LIVE);
    Input back = {.quit = true}; app_update(&a, &back); assert(a.screen == SCREEN_PAUSE);
    Game frozen = a.game;
    for (int i = 0; i < 100; ++i) app_update(&a, &idle);
    assert(!memcmp(&a.game, &frozen, sizeof(Game)));
    app_update(&a, &action); assert(a.screen == SCREEN_MATCH);
    a.screen = SCREEN_ROSTER; a.selection = 0;
    int rating = a.season.roster[0].rating;
    app_update(&a, &action); app_update(&a, &action); assert(a.season.roster[0].rating == rating);
    a.screen = SCREEN_PAUSE; a.selection = 1; app_update(&a, &action);
    assert(a.exit_after_save && a.save_requested); app_save_result(&a, false);
    assert(!a.exit_requested && a.notice == 2);
    app_update(&a, &action); app_save_result(&a, true); assert(a.exit_requested);
}
static void remove_saves(void) { remove("build/test-save0.tns"); remove("build/test-save1.tns"); }
static void test_saves(void) {
    App a, loaded; remove_saves(); app_init(&a);
    assert(!save_load(&a, "build/test-save"));
    a.has_career = true; app_start_match(&a);
    app_update(&a, &action); /* Exact live match, including timing and actor state. */
    for (int i = 0; i < 4; ++i) app_update(&a, &idle);
    assert(save_write(&a, "build/test-save")); assert(a.save_sequence == 1);
    assert(save_load(&loaded, "build/test-save"));
    assert(!memcmp(&a.game, &loaded.game, sizeof(Game)));
    assert(!memcmp(&a.season, &loaded.season, sizeof(Season)));
    assert(loaded.match_active && loaded.screen == SCREEN_TITLE);
    a.season.credits += 1; assert(save_write(&a, "build/test-save"));
    assert(save_load(&loaded, "build/test-save") && loaded.season.credits == a.season.credits);
    FILE *f = fopen("build/test-save0.tns", "wb"); assert(f); fputs("truncated", f); fclose(f);
    assert(save_load(&loaded, "build/test-save") && loaded.save_sequence == 1);
    assert(loaded.season.credits == a.season.credits - 1);
    f = fopen("build/test-save1.tns", "r+b"); assert(f); fputc(0, f); fclose(f);
    assert(!save_load(&loaded, "build/test-save"));
    assert(!save_write(&a, "build/no-such-dir/save"));
    a.game.selected_play = (Play)99; assert(!save_write(&a, "build/test-save"));
    remove_saves();
}
static void test_full_seasons(void) {
    App a; app_init(&a); a.has_career = true;
    /* Entire seasons via the public app state machine: no forced final scores. */
    for (int season = 0; season < 2; ++season) {
        for (int week = 0; week < SEASON_WEEKS; ++week) {
            app_start_match(&a);
            int steps = 0;
            while (a.game.phase != PHASE_FINAL && steps++ < 30000) {
                Input in = {.action_pressed = a.game.phase != PHASE_LIVE, .dx = 1};
                app_update(&a, &in);
                if (steps % 150 == 0) assert(save_write(&a, "build/test-save"));
            }
            assert(a.game.phase == PHASE_FINAL && a.game.quarter == 4);
            app_update(&a, &action);
            assert(a.screen == SCREEN_HUB && !a.match_active && a.season.week == week + 1);
            assert(save_write(&a, "build/test-save"));
        }
        assert(a.season.week == SEASON_WEEKS);
        app_update(&a, &action); assert(a.season.week == 0);
    }
    assert(a.season.year == 3);
    remove_saves();
}
static void test_contracts(void) {
    Season s; season_init(&s, 0); s.credits = 200;
    assert(season_payroll(&s) <= SALARY_CAP);
    for (int i = 0; i < ROSTER_COUNT; ++i) { s.roster[i].salary = 14; s.roster[i].years = 2; }
    s.roster[ROLE_K].salary = 16; /* Exactly 100M. */
    Season before = s;
    assert(!season_recruit(&s, ROLE_QB)); assert(!memcmp(&s, &before, sizeof(s)));
    s.roster[ROLE_QB].rating = 90; before = s;
    assert(!season_renew(&s, ROLE_QB)); assert(!memcmp(&s, &before, sizeof(s)));
    s.roster[ROLE_QB].rating = 60;
    assert(season_can_sign(&s, ROLE_QB, 14)); assert(!season_can_sign(&s, ROLE_QB, 15));
    assert(season_release(&s, ROLE_WR1)); assert(season_payroll(&s) == 86);
    assert(s.roster[ROLE_WR1].rating == 40 && !season_train(&s, ROLE_WR1));
    assert(season_recruit(&s, ROLE_QB)); assert(season_payroll(&s) <= SALARY_CAP);
    int salary = s.roster[ROLE_QB].salary;
    assert(season_train(&s, ROLE_QB)); assert(s.roster[ROLE_QB].salary == salary);
    s.roster[ROLE_QB].years = 1; assert(season_renew(&s, ROLE_QB)); assert(s.roster[ROLE_QB].years == 2);
    s.week = SEASON_WEEKS; s.roster[ROLE_K].years = 1;
    season_next(&s); assert(!s.roster[ROLE_K].years && s.roster[ROLE_K].salary == 0 && s.roster[ROLE_K].rating == 40);
    assert(season_effective_rating(&s, ROLE_K) == 40);
    assert(!season_renew(&s, ROLE_K));
}
static void test_special_teams(void) {
    Game g; game_init(&g); Input special = {.target_pressed = true};
    game_update(&g, &special); assert(g.phase == PHASE_SPECIAL);
    game_update(&g, &special); assert(g.phase == PHASE_CALL);
    game_update(&g, &special); Input down = {.down_pressed = true}; game_update(&g, &down);
    game_update(&g, &action); assert(g.phase == PHASE_KICK && g.kick_kind == KICK_FIELD_GOAL);
    g.kick_meter = 50; game_update(&g, &action); assert(g.result == RESULT_MISSED_KICK && g.opponent_start == 82);
    game_init(&g); g.spot = 65 * YARD; game_kick(&g, true); g.kick_meter = 50;
    game_update(&g, &action); assert(g.result == RESULT_FIELD_GOAL && g.score == 3);
    game_init(&g); g.spot = 65 * YARD; game_kick(&g, true); g.kick_meter = 0;
    game_update(&g, &action); assert(g.result == RESULT_MISSED_KICK && g.score == 0 && g.opponent_start == 42);
    game_init(&g); g.spot = 80 * YARD; game_kick(&g, false); g.kick_meter = 50;
    game_update(&g, &action); assert(g.kick_touchback && g.opponent_start == 20);
    game_init(&g); game_kick(&g, false); g.kick_meter = 0;
    game_update(&g, &action); assert(g.kick_return > 0 && !g.kick_touchback);
    game_init(&g); game_kick(&g, false);
    for (int i = 0; i < 150; ++i) game_update(&g, &idle);
    assert(g.phase == PHASE_RESULT);
    game_init(&g); g.quarter = 4; g.clock_ticks = 1; game_update(&g, &action);
    g.carrier.x = 100 * YARD; game_update(&g, &idle);
    assert(g.score == 6 && g.pending_pat && !g.clock_ticks);
    game_update(&g, &action); assert(g.phase == PHASE_KICK);
    g.kick_meter = 50; game_update(&g, &action); assert(g.score == 7 && !g.pending_pat);
    game_update(&g, &action); assert(g.phase == PHASE_FINAL);
    App a, loaded; app_init(&a); a.has_career = true; app_start_match(&a); game_kick(&a.game, false);
    for (int i = 0; i < 11; ++i) app_update(&a, &idle);
    Input pause = {.quit = true}; app_update(&a, &pause);
    Game frozen = a.game; app_update(&a, &idle); assert(!memcmp(&frozen, &a.game, sizeof(Game)));
    remove_saves(); assert(save_write(&a, "build/test-save")); assert(save_load(&loaded, "build/test-save"));
    assert(!memcmp(&a.game, &loaded.game, sizeof(Game))); remove_saves();
}
static void test_v1_migration(void) {
    remove_saves(); FILE *src = fopen("tests/fixtures/v1-flight.bin", "rb"); assert(src);
    FILE *dst = fopen("build/test-save1.tns", "wb"); assert(dst);
    int byte; while ((byte = fgetc(src)) != EOF) fputc(byte, dst);
    fclose(src); fclose(dst);
    App a, loaded; assert(save_load(&a, "build/test-save"));
    assert(a.season.team == 2 && a.match_active && a.game.in_flight && a.game.phase == PHASE_LIVE);
    assert(a.season.salary_cap == 100 && season_payroll(&a.season) == 68 && a.game.ratings[ROLE_K] == 60);
    assert(!a.game.pending_pat && a.season.roster[ROLE_QB].rating == 60);
    assert(save_write(&a, "build/test-save")); assert(save_load(&loaded, "build/test-save"));
    assert(!memcmp(&a.game, &loaded.game, sizeof(Game)) && !memcmp(&a.season, &loaded.season, sizeof(Season)));
    remove_saves();
    src = fopen("tests/fixtures/v1-touchdown.bin", "rb"); assert(src);
    dst = fopen("build/test-save1.tns", "wb"); assert(dst);
    while ((byte = fgetc(src)) != EOF) fputc(byte, dst);
    fclose(src); fclose(dst);
    assert(save_load(&a, "build/test-save"));
    assert(a.game.score == 7 && !a.game.pending_pat);
    app_update(&a, &action); /* Continue from title. */
    app_update(&a, &action); /* Advance the already-converted touchdown. */
    assert(a.game.phase == PHASE_OPPONENT && a.game.score == 7);
    remove_saves();
}
int main(void) {
    test_contracts(); test_special_teams(); test_v1_migration(); test_schedule(); test_progression(); test_clock_and_cpu(); test_menus_pause(); test_saves(); test_full_seasons();
    puts("Eight-team schedules, progression, match clocks, menus, save recovery, and two full seasons passed.");
    return 0;
}
