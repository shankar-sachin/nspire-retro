#include "game.h"
#include "season.h"
#include <string.h>
#define YARD (PX_PER_YARD * FP)
#define GOAL (100 * YARD)
static int abs_i(int v) { return v < 0 ? -v : v; }
static int clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static uint32_t random_u(Game *g) { g->rng = g->rng * 1664525u + 1013904223u; return g->rng; }
bool game_is_pass(Play play) { return play == PASS_SLANT || play == PASS_CROSS || play >= PASS_GO; }
int game_defender_count(const Game *g) { return g->legacy_units ? 4 : DEFENDER_COUNT; }
int game_receiver_count(const Game *g) { return g->legacy_units ? 2 : RECEIVER_COUNT; }
int game_blocker_count(const Game *g) { return g->legacy_units ? 2 : BLOCKER_COUNT; }
int game_receiver_role(int receiver) { return receiver < 2 ? ROLE_WR1 + receiver : ROLE_TE; }
static bool pass_play(const Game *g) { return game_is_pass(g->selected_play); }
static bool close_to(Actor a, Actor b, int radius) {
    int dx = (a.x - b.x) / FP, dy = (a.y - b.y) / FP;
    return dx * dx + dy * dy <= radius * radius;
}
static void pursue(Actor *a, Actor b, int speed) {
    int dx = b.x - a->x, dy = b.y - a->y;
    int ax = abs_i(dx), ay = abs_i(dy);
    int norm = (ax > ay ? ax : ay) + (ax > ay ? ay : ax) / 2;
    if (norm <= speed) { *a = b; return; }
    a->x += dx * speed / norm; a->y += dy * speed / norm;
}
static void fresh_drive(Game *g, int yard) {
    g->spot = clamp(yard, 5, 95) * YARD;
    g->line_to_gain = clamp(g->spot + 10 * YARD, 0, GOAL);
    g->down = 1; g->new_drive = false;
}
static void formation(Game *g) {
    g->carrier = (Actor){g->spot - (pass_play(g) ? 3 * YARD : YARD), 76 * FP};
    if (g->selected_play == RUN_SWEEP) g->carrier.y = 106 * FP;
    if (g->selected_play == RUN_COUNTER) g->carrier.y = 46 * FP;
    if (g->selected_play == RUN_DRAW) g->carrier.x = g->spot - 3 * YARD;
    for (int i = 0; i < game_receiver_count(g); ++i)
        g->receivers[i] = (Actor){g->spot - 2 * FP, (i == 0 ? 24 : (i == 1 ? 128 : 56)) * FP};
    if (g->selected_play == PASS_SCREEN) for (int i = 0; i < game_receiver_count(g); ++i)
        g->receivers[i] = (Actor){g->spot - 22 * FP, (i == 0 ? 46 : (i == 1 ? 106 : 76)) * FP};
    for (int i = 0; i < game_defender_count(g); ++i) {
        if (g->legacy_units) g->defenders[i] = (Actor){g->spot + (18 + i * 13) * FP, (26 + i * 33) * FP};
        else if (i < 4) g->defenders[i] = (Actor){g->spot + 10 * FP, (31 + i * 30) * FP};
        else if (i < 7) g->defenders[i] = (Actor){g->spot + 34 * FP, (40 + (i - 4) * 36) * FP};
        else if (i < 10) g->defenders[i] = (Actor){g->spot + 62 * FP, (i == 7 ? 24 : (i == 8 ? 128 : 56)) * FP};
        else g->defenders[i] = (Actor){g->spot + 104 * FP, 76 * FP};
        g->blocked[i] = 0;
    }
    if (!pass_play(g)) {
        int middle = g->legacy_units ? 2 : 5;
        g->defenders[middle] = (Actor){g->spot + (18 + (2 - g->difficulty) * 4) * FP, 76 * FP};
    }
    for (int i = 0; i < game_blocker_count(g); ++i)
        g->blockers[i] = (Actor){g->spot - 4 * FP, (g->legacy_units ? (i ? 94 : 58) : 34 + i * 21) * FP};
    g->support[0] = (Actor){g->spot - (pass_play(g) ? 7 : 18) * FP, (pass_play(g) ? 100 : 76) * FP};
    g->support[1] = (Actor){g->spot - 14 * FP, 52 * FP};
    g->ball = g->carrier;
    g->ticks = g->flight_ticks = 0;
    g->in_flight = g->passed = g->lob = false;
    g->caught_receiver = -1; g->target = 0;
    g->energy = 100; g->throw_cooldown = 0; g->flight_duration = PASS_TICKS;
}
void game_start(Game *g, int seconds, int difficulty, const int ratings[ROSTER_COUNT], int opponent_rating, uint32_t seed) {
    memset(g, 0, sizeof(*g));
    g->phase = PHASE_CALL; g->drive = 1; g->quarter = 1;
    g->quarter_seconds = clamp(seconds, 30, 180); g->clock_ticks = g->quarter_seconds * GAME_HZ;
    g->difficulty = clamp(difficulty, 0, 2); g->opponent_rating = clamp(opponent_rating, 30, 95);
    for (int i = 0; i < ROSTER_COUNT; ++i) g->ratings[i] = clamp(ratings[i], 20, 95);
    g->kick_direction = 1;
    g->rng = seed ? seed : 1; g->away_team = 1;
    fresh_drive(g, 25); formation(g);
}
void game_init(Game *g) {
    const int ratings[ROSTER_COUNT] = {65,65,65,65,65,65,65,65,65,65,65,65};
    game_start(g, 90, 1, ratings, 65, 0x12345678u);
}
int game_yards_to_go(const Game *g) {
    int d = g->line_to_gain - g->spot;
    return d > 0 ? (d + YARD - 1) / YARD : 0;
}
static void finish(Game *g, Result reason, bool incomplete) {
    int end = incomplete ? g->spot : clamp(g->carrier.x, 0, GOAL);
    g->last_gain = (end - g->spot) / YARD;
    if (g->caught_receiver >= 0) g->passing_yards += g->last_gain;
    else if (!incomplete) g->rushing_yards += g->last_gain;
    g->result = reason; g->phase = PHASE_RESULT; g->in_flight = false;
    /* Compact huddle/runoff keeps human and simulated possessions on comparable clocks.
       The clock stops after incompletions, scores and out-of-bounds plays. */
    if (!incomplete && reason != RESULT_BOUNDS && end > 0 && end < GOAL)
        g->clock_ticks = clamp(g->clock_ticks - 5 * GAME_HZ, 0, g->quarter_seconds * GAME_HZ);
    g->opponent_start = clamp(100 - end / YARD, 5, 95);
    if (reason == RESULT_INTERCEPTION) {
        g->opponent_start = clamp(100 - g->ball.x / YARD, 5, 95);
        g->new_drive = true; ++g->turnovers;
    } else if (!incomplete && end >= GOAL) {
        g->score += TOUCHDOWN_POINTS; ++g->touchdowns; g->pending_pat = true;
        g->result = RESULT_TOUCHDOWN; g->new_drive = true; g->opponent_start = 25;
    } else if (!incomplete && end <= 0) {
        g->result = RESULT_SAFETY; g->new_drive = true; ++g->turnovers;
        g->opponent_score += 2; g->opponent_start = 40;
    } else if (end >= g->line_to_gain) {
        g->down = 1; g->line_to_gain = clamp(end + 10 * YARD, 0, GOAL);
        g->result = RESULT_FIRST_DOWN;
    } else if (g->down == 4) {
        g->result = RESULT_DOWNS; g->new_drive = true; ++g->turnovers;
    } else ++g->down;
    g->spot = end;
}
static void begin_cpu(Game *g, int spot) {
    g->phase = PHASE_OPPONENT; g->cpu_spot = spot; g->cpu_down = 1;
    g->cpu_line = clamp(spot + 10, 0, 100); g->cpu_timer = 0;
    g->cpu_event = 0; g->cpu_gain = 0; g->cpu_done = false; g->new_drive = false;
}
static void start_kick(Game *g, int kind);
static void boundary(Game *g) {
    if (g->clock_ticks > 0) return;
    if (g->quarter == 4) {
        if (g->postseason && g->score == g->opponent_score) {
            g->shootout_active = true; g->shootout_round = 1; g->spot = 80 * YARD;
            start_kick(g, KICK_FIELD_GOAL);
        } else g->phase = PHASE_FINAL;
        return;
    }
    g->resume_phase = g->phase; g->phase = PHASE_BREAK;
}
static void next_user(Game *g, int spot) {
    ++g->drive; fresh_drive(g, spot); formation(g); g->phase = PHASE_CALL;
}
/* One simulated opponent snap, with the same down/field-position constraints. */
static void cpu_play(Game *g) {
    g->cpu_pass = (random_u(g) % 100) >= 45;
    int front = (g->ratings[ROLE_DEF] + g->ratings[ROLE_DL2]) / 2;
    int defense = g->cpu_pass ? (front + g->ratings[ROLE_LB] + 2 * g->ratings[ROLE_DB]) / 4
        : (2 * front + g->ratings[ROLE_LB]) / 3;
    int strength = g->opponent_rating - defense + (g->difficulty - 1) * 12;
    int roll = (int)(random_u(g) % 100);
    g->clock_ticks = clamp(g->clock_ticks - (4 + (int)(random_u(g) % 5)) * GAME_HZ, 0, 180 * GAME_HZ);
    g->cpu_timer = 0;
    if (g->cpu_down == 4) {
        if (g->cpu_spot >= 60) {
            bool made = roll < clamp(50 + (g->cpu_spot - 60) + strength / 2, 25, 95);
            g->cpu_event = made ? 3 : 4;
            if (made) g->opponent_score += 3;
            g->opponent_start = made ? 25 : 100 - g->cpu_spot;
        } else {
            g->cpu_event = 5; g->opponent_start = clamp(100 - g->cpu_spot - 35, 10, 90);
        }
        g->cpu_done = true; return;
    }
    if (roll < clamp(9 - strength / 8, 3, 18)) {
        g->cpu_event = 6; g->cpu_done = true; g->opponent_start = 100 - g->cpu_spot; return;
    }
    int gain = clamp((int)(random_u(g) % 18) - 2 + strength / 10, -4, 22);
    if (roll > 94) gain += 12;
    g->cpu_gain = gain; g->cpu_spot = clamp(g->cpu_spot + gain, 0, 100); g->cpu_event = 1;
    if (g->cpu_spot >= 100) {
        g->opponent_score += 7; g->cpu_event = 2; g->cpu_done = true; g->opponent_start = 25;
    } else if (g->cpu_spot <= 0) {
        g->score += 2; g->cpu_event = 7; g->cpu_done = true; g->opponent_start = 40;
    } else if (g->cpu_spot >= g->cpu_line) {
        g->cpu_down = 1; g->cpu_line = clamp(g->cpu_spot + 10, 0, 100);
    } else ++g->cpu_down;
}
static void start_kick(Game *g, int kind) {
    g->kick_kind = kind; g->kick_meter = 0; g->kick_direction = 1; g->kick_ticks = 0;
    g->kick_return = 0; g->kick_touchback = false;
    g->kick_distance = kind == KICK_PAT ? 33 : (kind == KICK_FIELD_GOAL ? 117 - g->spot / YARD : 0);
    g->phase = PHASE_KICK;
}
void game_kick(Game *g, bool field_goal) {
    if ((g->phase != PHASE_CALL && g->phase != PHASE_SPECIAL) || !g->clock_ticks) return;
    start_kick(g, field_goal ? KICK_FIELD_GOAL : KICK_PUNT);
}
static void resolve_kick(Game *g) {
    int quality = 100 - abs_i(g->kick_meter - 50) * 2;
    int rating = g->ratings[ROLE_K], spot = g->spot / YARD;
    g->new_drive = true; g->last_gain = 0; g->phase = PHASE_RESULT;
    if (g->kick_kind != KICK_PAT) g->clock_ticks = clamp(g->clock_ticks - 5 * GAME_HZ, 0, 180 * GAME_HZ);
    if (g->kick_kind == KICK_PUNT) {
        g->kick_distance = 25 + rating / 4 + quality / 5;
        int landing = spot + g->kick_distance;
        g->kick_touchback = landing >= 100;
        g->kick_return = g->kick_touchback ? 0 : (100 - quality) / 10 + (95 - rating) / 15;
        g->opponent_start = g->kick_touchback ? 20 : clamp(100 - landing + g->kick_return, 1, 99);
        g->result = RESULT_PUNT;
    } else {
        bool made = g->kick_distance <= 30 + rating / 3 + quality / 8 &&
            quality >= 45 + g->kick_distance / 3 - rating / 8;
        if (g->kick_kind == KICK_PAT) {
            if (made) ++g->score;
            g->pending_pat = false; g->opponent_start = 25;
            g->result = made ? RESULT_EXTRA_POINT : RESULT_EXTRA_MISSED;
        } else {
            if (made) g->score += 3;
            if (g->shootout_active && (int)(random_u(g) % 100) < 65 + g->difficulty * 6 + (g->opponent_rating - 65) / 4)
                g->opponent_score += 3;
            g->opponent_start = made ? 25 : clamp(107 - spot, 20, 99);
            g->result = made ? RESULT_FIELD_GOAL : RESULT_MISSED_KICK;
        }
    }
}
static void route_step(const Game *g, Actor *a, int index) {
    int dy = 0, speed = RECEIVER_SPEED + (g->ratings[game_receiver_role(index)] - 65) * 2;
    if (g->selected_play == PASS_SLANT && a->x < g->spot + 90 * FP) dy = index ? -190 : 190;
    if (g->selected_play == PASS_CROSS && a->x > g->spot + 18 * FP) dy = index ? -256 : 256;
    if (g->selected_play == PASS_GO) speed += 32;
    if (g->selected_play == PASS_OUT && a->x > g->spot + 18 * FP) dy = index ? 320 : -320;
    if (g->selected_play == PASS_POST && a->x > g->spot + 48 * FP) dy = index ? -240 : 240;
    if (g->selected_play == PASS_SCREEN && a->x < g->spot + 10 * FP) speed = 192;
    a->x = clamp(a->x + speed, 0, GOAL + 8 * FP);
    a->y = clamp(a->y + dy, 10 * FP, (FIELD_WIDTH - 10) * FP);
}
static void throw_ball(Game *g, bool lob) {
    g->passed = g->in_flight = true; g->lob = lob; ++g->pass_attempts;
    g->flight_ticks = 0; g->flight_duration = lob ? 24 : PASS_TICKS;
    g->throw_start = g->carrier; g->ball = g->throw_start;
    g->throw_target = g->receivers[g->target];
    for (int i = 0; i < g->flight_duration; ++i) route_step(g, &g->throw_target, g->target);
    /* Arm strength limits range. Beyond it, the throw falls short. */
    int range = (22 + g->ratings[ROLE_QB] / 3) * YARD;
    int distance = abs_i(g->throw_target.x - g->throw_start.x) + abs_i(g->throw_target.y - g->throw_start.y) / 2;
    if (distance > range) {
        g->throw_target.x = g->throw_start.x + (int)((int64_t)(g->throw_target.x - g->throw_start.x) * range / distance);
        g->throw_target.y = g->throw_start.y + (int)((int64_t)(g->throw_target.y - g->throw_start.y) * range / distance);
    }
    /* Pressure creates a visible, deterministic error rather than random drops. */
    for (int i = 0; i < game_defender_count(g); ++i)
        if (close_to(g->carrier, g->defenders[i], 18)) {
            g->throw_target.y += (g->target ? 1 : -1) * (20 - g->ratings[ROLE_QB] / 8) * FP; break;
        }
}
static bool crosses(Actor from, Actor to, Actor defender) {
    int vx = (to.x - from.x) / FP, vy = (to.y - from.y) / FP;
    int wx = (defender.x - from.x) / FP, wy = (defender.y - from.y) / FP;
    int len = vx * vx + vy * vy, dot = wx * vx + wy * vy;
    Actor nearest = from;
    if (len && dot > 0) {
        if (dot >= len) nearest = to;
        else { nearest.x += vx * dot * FP / len; nearest.y += vy * dot * FP / len; }
    }
    return close_to(nearest, defender, 6);
}
static void defense_step(Game *g) {
    int speed = DEFENDER_SPEED + (g->difficulty - 1) * 28 + (g->opponent_rating - 65);
    for (int b = 0; b < game_blocker_count(g); ++b) {
        int d = b;
        if (g->ticks < (g->selected_play == RUN_DRAW ? 120 : 90)) {
            pursue(&g->blockers[b], g->defenders[d], 320);
            if (close_to(g->blockers[b], g->defenders[d], 10))
                g->blocked[d] = 10 + g->ratings[b < 3 ? ROLE_OL : ROLE_OL2] / 3;
        }
    }
    if (!g->legacy_units && g->ticks < 90) {
        pursue(&g->support[1], g->defenders[6], 288);
        if (close_to(g->support[1], g->defenders[6], 9)) g->blocked[6] = 12;
    }
    for (int d = 0; d < game_defender_count(g); ++d) {
        Actor target = g->carrier;
        if (pass_play(g) && !g->passed && g->carrier.x <= g->spot && d >= (g->legacy_units ? 2 : 7) && d < (g->legacy_units ? 4 : 10)) {
            target = g->receivers[d - (g->legacy_units ? 2 : 7)]; target.x += 14 * FP; /* Outside leverage. */
        } else if (g->in_flight) target = g->throw_target;
        else { target.x += 5 * FP; } /* Pursuit angle, not just a trailing conga line. */
        int move = speed;
        if (g->blocked[d] > 0) { --g->blocked[d]; move /= 4; }
        if (g->ticks > 8 + d * 3) pursue(&g->defenders[d], target, move);
    }
}
void game_update(Game *g, const Input *in) {
    g->animation = (g->animation + 1) % 120;
    if (g->phase == PHASE_FINAL) return;
    if (g->phase == PHASE_SPECIAL) {
        if (in->target_pressed) { g->phase = PHASE_CALL; return; }
        if (in->up_pressed || in->down_pressed) g->kick_kind = !g->kick_kind;
        if (in->action_pressed) game_kick(g, g->kick_kind == KICK_FIELD_GOAL);
        return;
    }
    if (g->phase == PHASE_KICK) {
        if (in->action_pressed || ++g->kick_ticks >= 150) { resolve_kick(g); return; }
        g->kick_meter += g->kick_direction * (3 + g->difficulty);
        if (g->kick_meter >= 100) { g->kick_meter = 100; g->kick_direction = -1; }
        if (g->kick_meter <= 0) { g->kick_meter = 0; g->kick_direction = 1; }
        return;
    }
    if (g->phase == PHASE_BREAK) {
        if (in->action_pressed) {
            ++g->quarter; g->clock_ticks = g->quarter_seconds * GAME_HZ;
            g->phase = g->resume_phase;
            if (g->quarter == 3) begin_cpu(g, 25); /* Opponent receives second-half kickoff. */
        }
        return;
    }
    if (g->phase == PHASE_OPPONENT) {
        if (g->cpu_done || g->clock_ticks == 0) {
            if (in->action_pressed) {
                if (g->cpu_done) next_user(g, g->opponent_start);
                boundary(g);
            }
        } else if (in->action_pressed || ++g->cpu_timer >= 45) cpu_play(g);
        return;
    }
    if (g->phase == PHASE_CALL) {
        if (in->up_pressed) g->selected_play = (Play)((g->selected_play + PLAY_COUNT - 1) % PLAY_COUNT);
        if (in->down_pressed) g->selected_play = (Play)((g->selected_play + 1) % PLAY_COUNT);
        formation(g);
        if (in->target_pressed) { g->phase = PHASE_SPECIAL; g->kick_kind = KICK_PUNT; return; }
        if (in->action_pressed) { g->phase = PHASE_LIVE; g->result = RESULT_NONE; }
        return;
    }
    if (g->phase == PHASE_RESULT) {
        if (in->action_pressed) {
            if (g->shootout_active) {
                if (g->score != g->opponent_score) { g->shootout_active = false; g->phase = PHASE_FINAL; }
                else { ++g->shootout_round; start_kick(g, KICK_FIELD_GOAL); }
                return;
            }
            if (g->pending_pat) { start_kick(g, KICK_PAT); return; }
            if (g->new_drive) begin_cpu(g, g->opponent_start);
            else { g->phase = PHASE_CALL; formation(g); }
            boundary(g);
        }
        return;
    }
    ++g->ticks;
    if (g->clock_ticks > 0) --g->clock_ticks;
    if (!g->in_flight) {
        int dx = clamp(in->dx, -1, 1), dy = clamp(in->dy, -1, 1);
        int role = g->caught_receiver >= 0 ? game_receiver_role(g->caught_receiver) : (pass_play(g) ? ROLE_QB : ROLE_RB);
        int speed = PLAYER_SPEED + (g->ratings[role] - 65) * 2;
        if (in->boost && g->energy >= 3 && (dx || dy)) { speed += 128; g->energy -= 3; }
        else if (!in->boost) g->energy = clamp(g->energy + 1, 0, 100);
        if (dx && dy) speed = speed * 181 / 256;
        g->carrier.x += dx * speed; g->carrier.y += dy * speed;
        if (g->carrier.x >= GOAL || g->carrier.x <= 0) { finish(g, RESULT_TACKLE, false); return; }
        if (g->carrier.y < 4 * FP || g->carrier.y > (FIELD_WIDTH - 4) * FP) { finish(g, RESULT_BOUNDS, false); return; }
        for (int i = 0; i < game_defender_count(g); ++i)
            if (close_to(g->carrier, g->defenders[i], TACKLE_RADIUS)) { finish(g, RESULT_TACKLE, false); return; }
    }
    if (pass_play(g)) for (int i = 0; i < game_receiver_count(g); ++i)
        if (i != g->caught_receiver) route_step(g, &g->receivers[i], i);
    defense_step(g);
    bool launched = false;
    if (pass_play(g) && !g->passed) {
        if (in->target_pressed) g->target = (g->target + 1) % game_receiver_count(g);
        if (in->action_pressed && g->carrier.x <= g->spot) { throw_ball(g, in->boost); launched = true; }
    }
    if (g->in_flight && !launched) {
        Actor previous = g->ball;
        int t = ++g->flight_ticks;
        g->ball.x = g->throw_start.x + (g->throw_target.x - g->throw_start.x) * t / g->flight_duration;
        g->ball.y = g->throw_start.y + (g->throw_target.y - g->throw_start.y) * t / g->flight_duration;
        if (!g->lob || t * 4 >= g->flight_duration * 3)
            for (int d = 0; d < game_defender_count(g); ++d)
                if (crosses(previous, g->ball, g->defenders[d])) { finish(g, RESULT_INTERCEPTION, true); return; }
        if (t >= g->flight_duration) {
            int radius = 7 + g->ratings[game_receiver_role(g->target)] / 16;
            if (close_to(g->ball, g->receivers[g->target], radius)) {
                g->carrier = g->receivers[g->target]; g->caught_receiver = g->target;
                g->in_flight = false; ++g->completions;
                if (g->carrier.x >= GOAL) { finish(g, RESULT_TOUCHDOWN, false); return; }
            } else { finish(g, RESULT_INCOMPLETE, true); return; }
        }
    } else if (!g->in_flight) g->ball = g->carrier;
    if (!g->in_flight) for (int i = 0; i < game_defender_count(g); ++i)
        if (close_to(g->carrier, g->defenders[i], TACKLE_RADIUS)) { finish(g, RESULT_TACKLE, false); return; }
    if (g->ticks >= PLAY_LIMIT_TICKS) finish(g, RESULT_TIMEOUT, g->in_flight);
}
const char *game_play_name(Play play) {
    static const char *const names[PLAY_COUNT] = {"RUN  SPLIT", "RUN  SWEEP", "PASS SLANT", "PASS CROSS", "RUN  COUNTER", "RUN  DRAW", "PASS GO", "PASS OUT", "PASS POST", "PASS SCREEN"};
    return names[play];
}
const char *game_result_name(Result result) {
    static const char *const names[] = {"READY", "TACKLED", "OUT OF BOUNDS", "INCOMPLETE", "FIRST DOWN", "TOUCHDOWN",
        "TURNOVER ON DOWNS", "INTERCEPTED", "SAFETY", "PLAY CLOCK EXPIRED", "PUNT", "FIELD GOAL", "KICK MISSED", "EXTRA POINT GOOD", "EXTRA POINT MISSED"};
    return names[result];
}

const char *game_play_description(Play play) {
    static const char *const descriptions[PLAY_COUNT] = {"HIT THE MIDDLE", "ATTACK THE LOW LANE", "CUT INSIDE THEN UPFIELD", "CROSS THE MIDDLE", "ATTACK THE HIGH LANE", "DEEP START LONGER BLOCKS", "THREE VERTICAL ROUTES", "BREAK TO THE SIDELINES", "DEEP CUTS INSIDE", "SHORT CATCH BEHIND BLOCKS"};
    return descriptions[play];
}
