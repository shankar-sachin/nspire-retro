#include <assert.h>
#include <stdio.h>
#include "game.h"
#define YARD (PX_PER_YARD * FP)
static const Input idle = {0};
static const Input action = {.action_pressed = true};
static void start(Game *g, Play play) {
    game_init(g); g->selected_play = play; game_update(g, &action);
    assert(g->phase == PHASE_LIVE);
}
static void isolate(Game *g) {
    for (int i = 0; i < DEFENDER_COUNT; ++i)
        g->defenders[i] = (Actor){-1000 * FP, -1000 * FP};
}
static void tackle_at(Game *g, int x) {
    g->carrier.x = x;
    g->defenders[0] = g->carrier;
    game_update(g, &idle);
    assert(g->phase == PHASE_RESULT);
}
static void test_rules(void) {
    Game g;
    start(&g, RUN_SPLIT);
    assert(g.down == 1 && game_yards_to_go(&g) == 10);
    tackle_at(&g, 29 * YARD);
    assert(g.down == 2 && game_yards_to_go(&g) == 6 && g.last_gain == 4);
    game_update(&g, &action); game_update(&g, &action);
    tackle_at(&g, 26 * YARD);
    assert(g.down == 3 && game_yards_to_go(&g) == 9 && g.last_gain == -3);
    game_update(&g, &action); game_update(&g, &action);
    g.down = 4; tackle_at(&g, 35 * YARD);
    assert(g.down == 1 && g.result == RESULT_FIRST_DOWN && !g.new_drive);
    assert(g.line_to_gain == 45 * YARD);
    start(&g, RUN_SPLIT); g.down = 4; tackle_at(&g, 34 * YARD);
    assert(g.result == RESULT_DOWNS && g.turnovers == 1 && g.new_drive);
    game_update(&g, &action);
    assert(g.phase == PHASE_OPPONENT && g.cpu_spot == 66);
    start(&g, RUN_SPLIT); g.spot = 96 * YARD; g.line_to_gain = 100 * YARD;
    g.down = 4; g.carrier.x = 100 * YARD; game_update(&g, &idle);
    assert(g.result == RESULT_TOUCHDOWN && g.score == TOUCHDOWN_POINTS);
    game_update(&g, &action);
    assert(g.score == TOUCHDOWN_POINTS && g.phase == PHASE_KICK);
    g.kick_meter = 50; game_update(&g, &action);
    assert(g.score == TOUCHDOWN_POINTS + 1 && g.result == RESULT_EXTRA_POINT);
    game_update(&g, &action); assert(g.phase == PHASE_OPPONENT);
    start(&g, RUN_SPLIT); g.carrier.x = 0; game_update(&g, &idle);
    assert(g.result == RESULT_SAFETY && g.new_drive);
    start(&g, RUN_SPLIT); g.carrier.y = 0; game_update(&g, &idle);
    assert(g.result == RESULT_BOUNDS && g.down == 2);
    start(&g, RUN_SPLIT); g.spot = 93 * YARD; g.line_to_gain = 94 * YARD;
    tackle_at(&g, 95 * YARD);
    assert(g.line_to_gain == 100 * YARD && game_yards_to_go(&g) == 5);
    start(&g, RUN_SPLIT); isolate(&g); g.ticks = PLAY_LIMIT_TICKS - 1;
    game_update(&g, &idle); assert(g.result == RESULT_TIMEOUT);
}
static void aim_receiver(Game *g, int target, bool lob) {
    game_update(g, &(Input){.action_pressed=true,.action_held=true});
    assert(g->aiming && !g->passed);
    Game future = *g; future.aiming = false; future.passed = true;
    for (int i = 0; i <= (lob ? 24 : PASS_TICKS); ++i) game_update(&future, &idle);
    g->aim = future.receivers[target];
    game_update(g, &(Input){.action_released=true,.boost=lob});
    assert(g->in_flight && !g->aiming);
}
static void test_passes(void) {
    Game g;
    for (int play = 0; play < PLAY_COUNT; ++play) {
        if (!game_is_pass((Play)play)) continue;
        for (int target = 0; target < RECEIVER_COUNT; ++target) {
            start(&g, (Play)play); isolate(&g); g.target = (target + 1) % RECEIVER_COUNT;
            aim_receiver(&g, target, false); /* Catch depends on location, not old target ID. */
            Actor landing = g.throw_target;
            for (int i = 0; i < PASS_TICKS; ++i) game_update(&g, &idle);
            assert(!g.in_flight && g.caught_receiver == target && g.phase == PHASE_LIVE);
            assert(g.throw_target.x == landing.x && g.throw_target.y == landing.y);
            int x = g.carrier.x; game_update(&g, &(Input){.dx=1}); assert(g.carrier.x > x);
        }
    }
    start(&g, PASS_SLANT); isolate(&g);
    Actor before = g.carrier; game_update(&g, &(Input){.dx=-1}); assert(g.carrier.x < before.x);
    game_update(&g, &(Input){.action_pressed=true,.action_held=true});
    before = g.carrier; int tick = g.ticks, x = g.aim.x, y = g.aim.y;
    for (int i = 0; i < 8; ++i) game_update(&g, &(Input){.dx=1,.dy=-1,.action_held=true});
    assert(g.carrier.x == before.x && g.carrier.y == before.y && g.ticks == tick + 2);
    assert(g.aim.x > x && g.aim.y < y && !g.passed);
    game_update(&g, &(Input){.target_pressed=true,.action_held=true}); assert(!g.aiming && !g.passed);
    game_update(&g, &(Input){.action_released=true}); assert(!g.passed);
    g.carrier.x = g.spot + FP; game_update(&g, &action); assert(!g.aiming && !g.passed);
    start(&g, PASS_SLANT); isolate(&g); aim_receiver(&g, 0, false);
    g.defenders[0] = g.ball; game_update(&g, &idle); assert(g.result == RESULT_INTERCEPTION);
    start(&g, PASS_GO); isolate(&g); aim_receiver(&g, 0, false);
    g.receivers[0].y += 50 * FP;
    for (int i = 0; i < PASS_TICKS; ++i) game_update(&g, &idle);
    assert(g.result == RESULT_INCOMPLETE && g.spot == 25 * YARD && g.down == 2);
    start(&g, PASS_GO); isolate(&g); g.down = 4; aim_receiver(&g, 0, false);
    g.receivers[0].y += 50 * FP;
    for (int i = 0; i < PASS_TICKS; ++i) game_update(&g, &idle);
    assert(g.result == RESULT_DOWNS);
    start(&g, PASS_GO); isolate(&g); game_update(&g, &action);
    g.aim = (Actor){90 * YARD,76*FP}; game_update(&g, &(Input){.action_released=true});
    assert(g.throw_target.x - g.throw_start.x <= game_throw_range(&g));
}
static void test_movement(void) {
    Game g; start(&g, RUN_SPLIT); isolate(&g);
    Actor before = g.carrier; Input move = {.dx = 1, .dy = 1};
    game_update(&g, &move);
    int dx = g.carrier.x - before.x, dy = g.carrier.y - before.y;
    assert(dx * dx + dy * dy <= PLAYER_SPEED * PLAYER_SPEED);
    start(&g, RUN_SPLIT); g.ticks = 30;
    int old = g.defenders[0].x; game_update(&g, &idle);
    assert(g.defenders[0].x < old && DEFENDER_SPEED < PLAYER_SPEED);
    game_init(&g); Input up = {.up_pressed = true}; game_update(&g, &up);
    assert(g.selected_play == PASS_SCREEN);
}
static void test_advanced_play(void) {
    Game normal, sprint, rookie, hard;
    start(&normal, RUN_SPLIT); isolate(&normal); sprint = normal;
    Input run = {.dx = 1}, boost = {.dx = 1, .boost = true};
    game_update(&normal, &run); game_update(&sprint, &boost);
    assert(sprint.carrier.x > normal.carrier.x && sprint.energy == 97);
    game_update(&sprint, &run); assert(sprint.energy == 98);
    start(&rookie, RUN_SPLIT); rookie.difficulty = 0; rookie.ticks = 30; hard = rookie; hard.difficulty = 2;
    game_update(&rookie, &idle); game_update(&hard, &idle);
    assert(hard.defenders[2].x < rookie.defenders[2].x);
    start(&normal, RUN_SPLIT); normal.ticks = 20;
    normal.blockers[0] = normal.defenders[0]; game_update(&normal, &idle);
    assert(normal.blocked[0] > 0);
    start(&normal, RUN_SPLIT); normal.difficulty = 2;
    /* Re-form after choosing the difficulty to include its defensive alignment. */
    normal.phase = PHASE_CALL; game_update(&normal, &action);
    for (int i = 0; i < 100 && normal.phase == PHASE_LIVE; ++i) game_update(&normal, &boost);
    assert(normal.phase == PHASE_RESULT && normal.last_gain < 10 && normal.result != RESULT_TOUCHDOWN);
    start(&normal, PASS_SLANT); isolate(&normal);
    aim_receiver(&normal, 0, true);
    assert(normal.lob && normal.flight_duration == 24);
    for (int i = 0; i < 24; ++i) game_update(&normal, &idle);
    assert(normal.completions == 1 && normal.caught_receiver == 0);
    start(&normal, PASS_SLANT); normal.ticks = 30;
    normal.defenders[7].y = 60 * FP;
    int old_y = normal.defenders[7].y; game_update(&normal, &idle);
    assert(normal.defenders[7].y < old_y); /* Coverage tracks the high receiver. */
}
static void test_long_session(void) {
    Game g; uint32_t random = 1; game_init(&g);
    for (int tick = 0; tick < 100000; ++tick) {
        random = random * 1664525u + 1013904223u;
        Input in = {.dx = (int)(random % 3) - 1, .dy = (int)((random >> 4) % 3) - 1,
            .action_pressed = (random & 31) == 0, .action_released = (random & 31) == 3, .target_pressed = (random & 63) == 1,
            .down_pressed = (random & 15) == 2};
        if (g.phase == PHASE_FINAL) game_init(&g);
        game_update(&g, &in);
        assert(g.down >= 1 && g.down <= 4);
        assert(g.spot >= 0 && g.spot <= 100 * YARD);
        assert(g.line_to_gain <= 100 * YARD);
        assert(g.target >= 0 && g.target < RECEIVER_COUNT);
        assert(g.score >= 0 && g.opponent_score >= 0);
    }
}
static void test_position_units(void) {
    Game g; start(&g, PASS_SLANT);
    assert(game_blocker_count(&g) == 5 && game_defender_count(&g) == 11 && game_receiver_count(&g) == 3);
    assert(game_receiver_role(2) == ROLE_TE);
    int run_effects = 0, pass_effects = 0;
    for (int seed = 1; seed <= 100; ++seed) {
        Game weak, strong; game_init(&weak); weak.phase = PHASE_OPPONENT;
        weak.cpu_spot = 30; weak.cpu_line = 40; weak.cpu_down = 1; weak.rng = (uint32_t)seed;
        weak.ratings[ROLE_DEF] = weak.ratings[ROLE_DL2] = weak.ratings[ROLE_LB] = weak.ratings[ROLE_DB] = 40;
        strong = weak;
        strong.ratings[ROLE_DEF] = strong.ratings[ROLE_DL2] = strong.ratings[ROLE_LB] = strong.ratings[ROLE_DB] = 95;
        game_update(&weak, &action); game_update(&strong, &action);
        assert(weak.cpu_pass == strong.cpu_pass);
        if (weak.cpu_spot != strong.cpu_spot || weak.cpu_event != strong.cpu_event) {
            if (weak.cpu_pass) ++pass_effects; else ++run_effects;
        }
    }
    assert(run_effects > 0 && pass_effects > 0);
    start(&g, RUN_DRAW); g.ticks = 100; g.blockers[3] = g.defenders[3]; g.ratings[ROLE_OL2] = 40;
    Game better = g; better.ratings[ROLE_OL2] = 95;
    game_update(&g, &idle); game_update(&better, &idle);
    assert(better.blocked[3] > g.blocked[3]);
}
int main(void) {
    test_position_units(); test_rules(); test_passes(); test_movement(); test_advanced_play(); test_long_session();
    puts("Game rules, passes, movement, and 100000 simulation ticks passed.");
    return 0;
}
