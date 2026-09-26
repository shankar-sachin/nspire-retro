#include "save.h"
#include <stdio.h>
#include <string.h>
#define SAVE_VERSION 1u
#define SAVE_CAPACITY 2048
/* Every scalar is explicitly serialized, independent of compiler padding and ABI. */
#define GAME_FIELDS(X) \
 X(phase) X(selected_play) X(result) X(down) X(spot) X(line_to_gain) \
 X(score) X(drive) X(turnovers) X(last_gain) X(target) X(caught_receiver) \
 X(ticks) X(flight_ticks) X(in_flight) X(passed) X(new_drive) X(opponent_score) \
 X(quarter) X(clock_ticks) X(quarter_seconds) X(difficulty) X(home_team) X(away_team) \
 X(opponent_rating) X(energy) X(flight_duration) X(throw_cooldown) X(animation) \
 X(pass_attempts) X(completions) X(passing_yards) X(rushing_yards) X(touchdowns) \
 X(cpu_spot) X(cpu_down) X(cpu_line) X(cpu_timer) X(cpu_event) X(cpu_gain) \
 X(opponent_start) X(cpu_done) X(lob) X(resume_phase) X(rng)
#define SEASON_FIELDS(X) X(team) X(year) X(week) X(credits) X(trophies) X(career_wins) X(career_losses) X(rng)
typedef struct { unsigned char data[SAVE_CAPACITY]; size_t pos, size; bool ok; } Buffer;
static void put(Buffer *b, uint32_t n) {
    if (b->pos + 4 > SAVE_CAPACITY) { b->ok = false; return; }
    for (int i = 0; i < 4; ++i) b->data[b->pos++] = (unsigned char)(n >> (i * 8));
}
static uint32_t get(Buffer *b) {
    if (b->pos + 4 > b->size) { b->ok = false; return 0; }
    uint32_t n = 0;
    for (int i = 0; i < 4; ++i) n |= (uint32_t)b->data[b->pos++] << (i * 8);
    return n;
}
static uint32_t checksum(const unsigned char *data, size_t length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i) hash = (hash ^ data[i]) * 16777619u;
    return hash;
}
static void actor_put(Buffer *b, Actor a) { put(b, (uint32_t)a.x); put(b, (uint32_t)a.y); }
static Actor actor_get(Buffer *b) { Actor a; a.x = (int32_t)get(b); a.y = (int32_t)get(b); return a; }
static void encode(Buffer *b, const App *a, uint32_t sequence) {
    const Season *s = &a->season; const Game *g = &a->game;
    put(b, 0x4e535052u); put(b, SAVE_VERSION); put(b, sequence);
    put(b, a->has_career); put(b, a->match_active);
    put(b, (uint32_t)a->settings.difficulty); put(b, (uint32_t)a->settings.quarter_seconds); put(b, (uint32_t)a->settings.animations);
#define WRITE_S(f) put(b, (uint32_t)s->f);
    SEASON_FIELDS(WRITE_S)
#undef WRITE_S
    for (int i = 0; i < ROSTER_COUNT; ++i) {
        put(b, (uint32_t)s->roster[i].name); put(b, (uint32_t)s->roster[i].rating);
        put(b, (uint32_t)s->roster[i].xp); put(b, (uint32_t)s->roster[i].condition);
    }
    for (int i = 0; i < TEAM_COUNT; ++i) {
        const Standing *t = &s->table[i];
        put(b, (uint32_t)t->wins); put(b, (uint32_t)t->losses); put(b, (uint32_t)t->ties);
        put(b, (uint32_t)t->points_for); put(b, (uint32_t)t->points_against);
    }
    for (int i = 0; i < SEASON_WEEKS; ++i) for (int j = 0; j < 2; ++j) put(b, (uint32_t)s->results[i][j]);
#define WRITE_G(f) put(b, (uint32_t)g->f);
    GAME_FIELDS(WRITE_G)
#undef WRITE_G
    actor_put(b, g->carrier); actor_put(b, g->ball); actor_put(b, g->throw_start); actor_put(b, g->throw_target);
    for (int i = 0; i < RECEIVER_COUNT; ++i) actor_put(b, g->receivers[i]);
    for (int i = 0; i < DEFENDER_COUNT; ++i) { actor_put(b, g->defenders[i]); put(b, (uint32_t)g->blocked[i]); }
    for (int i = 0; i < BLOCKER_COUNT; ++i) actor_put(b, g->blockers[i]);
    for (int i = 0; i < 6; ++i) put(b, (uint32_t)g->ratings[i]);
    uint32_t hash = checksum(b->data, b->pos); put(b, hash);
}
static bool range(int n, int lo, int hi) { return n >= lo && n <= hi; }
static bool valid_actor(Actor a) { return range(a.x, -100 * FP, 800 * FP) && range(a.y, -100 * FP, 250 * FP); }
static bool valid(const App *a) {
    const Season *s = &a->season; const Game *g = &a->game;
    if (!range(a->settings.difficulty, 0, 2) || (a->settings.quarter_seconds != 60 && a->settings.quarter_seconds != 90 && a->settings.quarter_seconds != 120) || !range(a->settings.animations, 0, 1)) return false;
    if (!range(s->team, 0, TEAM_COUNT - 1) || !range(s->week, 0, SEASON_WEEKS) || !range(s->year, 1, 999) || !range(s->credits, 0, 9999) || !range(s->trophies, 0, 100000) || !range(s->career_wins, 0, 100000) || !range(s->career_losses, 0, 100000)) return false;
    for (int i = 0; i < 6; ++i) {
        const Player *p = &s->roster[i];
        if (!range(p->name, 0, 11) || !range(p->rating, 30, 95) || !range(p->xp, 0, 9) || !range(p->condition, 0, 100)) return false;
        if (!range(g->ratings[i], 20, 95)) return false;
    }
    for (int i = 0; i < SEASON_WEEKS; ++i) for (int j = 0; j < 2; ++j)
        if (i < s->week ? !range(s->results[i][j], 0, 1000) : s->results[i][j] != -1) return false;
    for (int i = 0; i < TEAM_COUNT; ++i) {
        const Standing *t = &s->table[i];
        if (!range(t->wins, 0, SEASON_WEEKS) || !range(t->losses, 0, SEASON_WEEKS) || !range(t->ties, 0, SEASON_WEEKS) || t->wins + t->losses + t->ties != s->week || !range(t->points_for, 0, 5000) || !range(t->points_against, 0, 5000)) return false;
    }
    if (a->match_active && (!a->has_career || s->week >= SEASON_WEEKS || g->home_team != s->team || g->away_team != season_opponent(s->team, s->week))) return false;
    if (!range(g->phase, PHASE_CALL, PHASE_FINAL) || !range(g->selected_play, 0, 3) || !range(g->result, RESULT_NONE, RESULT_MISSED_KICK) || !range(g->resume_phase, PHASE_CALL, PHASE_FINAL)) return false;
    if (!range(g->quarter, 1, 4) || !range(g->quarter_seconds, 30, 180) || !range(g->clock_ticks, 0, g->quarter_seconds * GAME_HZ) || !range(g->down, 1, 4) || !range(g->difficulty, 0, 2)) return false;
    if (!range(g->home_team, 0, TEAM_COUNT - 1) || !range(g->away_team, 0, TEAM_COUNT - 1) || !range(g->opponent_rating, 30, 95)) return false;
    if (!range(g->score, 0, 1000) || !range(g->opponent_score, 0, 1000) || !range(g->spot, 0, FIELD_LENGTH * FP) || !range(g->line_to_gain, 0, FIELD_LENGTH * FP)) return false;
    if (!range(g->target, 0, 1) || !range(g->caught_receiver, -1, 1) || !range(g->ticks, 0, PLAY_LIMIT_TICKS) || !range(g->flight_duration, 16, 24) || !range(g->flight_ticks, 0, g->flight_duration) || !range(g->energy, 0, 100) || !range(g->animation, 0, 119)) return false;
    if (!range(g->drive, 1, 1000) || !range(g->turnovers, 0, 1000) || !range(g->last_gain, -100, 100) || !range(g->throw_cooldown, 0, 1000) || !range(g->pass_attempts, 0, 1000) || !range(g->completions, 0, g->pass_attempts) || !range(g->passing_yards, -10000, 10000) || !range(g->rushing_yards, -10000, 10000) || !range(g->touchdowns, 0, 1000)) return false;
    if (!range(g->cpu_spot, 0, 100) || !range(g->cpu_down, 0, 4) || !range(g->cpu_line, 0, 100) || !range(g->cpu_timer, 0, 45) || !range(g->cpu_event, 0, 7) || !range(g->cpu_gain, -4, 34) || !range(g->opponent_start, 0, 100)) return false;
    if (!valid_actor(g->carrier) || !valid_actor(g->ball) || !valid_actor(g->throw_start) || !valid_actor(g->throw_target)) return false;
    for (int i = 0; i < RECEIVER_COUNT; ++i) if (!valid_actor(g->receivers[i])) return false;
    for (int i = 0; i < BLOCKER_COUNT; ++i) if (!valid_actor(g->blockers[i])) return false;
    for (int i = 0; i < DEFENDER_COUNT; ++i) if (!valid_actor(g->defenders[i]) || !range(g->blocked[i], 0, 50)) return false;
    return true;
}
static bool decode(Buffer *b, App *a) {
    if (b->size < 16 || b->size % 4) return false;
    size_t end = b->size - 4;
    b->pos = end; uint32_t hash = get(b); b->pos = 0;
    if (hash != checksum(b->data, end) || get(b) != 0x4e535052u || get(b) != SAVE_VERSION) return false;
    app_init(a); a->save_sequence = get(b);
    uint32_t career = get(b), active = get(b);
    if (career > 1 || active > 1) return false;
    a->has_career = career != 0; a->match_active = active != 0;
    a->settings.difficulty = (int32_t)get(b); a->settings.quarter_seconds = (int32_t)get(b); a->settings.animations = (int32_t)get(b);
    Season *s = &a->season; Game *g = &a->game;
#define READ_S(f) s->f = (int32_t)get(b);
    SEASON_FIELDS(READ_S)
#undef READ_S
    for (int i = 0; i < ROSTER_COUNT; ++i) {
        s->roster[i].name = (int32_t)get(b); s->roster[i].rating = (int32_t)get(b);
        s->roster[i].xp = (int32_t)get(b); s->roster[i].condition = (int32_t)get(b);
    }
    for (int i = 0; i < TEAM_COUNT; ++i) {
        Standing *t = &s->table[i];
        t->wins = (int32_t)get(b); t->losses = (int32_t)get(b); t->ties = (int32_t)get(b);
        t->points_for = (int32_t)get(b); t->points_against = (int32_t)get(b);
    }
    for (int i = 0; i < SEASON_WEEKS; ++i) for (int j = 0; j < 2; ++j) s->results[i][j] = (int32_t)get(b);
#define READ_G(f) g->f = (int32_t)get(b);
    GAME_FIELDS(READ_G)
#undef READ_G
    g->carrier = actor_get(b); g->ball = actor_get(b); g->throw_start = actor_get(b); g->throw_target = actor_get(b);
    for (int i = 0; i < RECEIVER_COUNT; ++i) g->receivers[i] = actor_get(b);
    for (int i = 0; i < DEFENDER_COUNT; ++i) { g->defenders[i] = actor_get(b); g->blocked[i] = (int32_t)get(b); }
    for (int i = 0; i < BLOCKER_COUNT; ++i) g->blockers[i] = actor_get(b);
    for (int i = 0; i < 6; ++i) g->ratings[i] = (int32_t)get(b);
    return b->ok && b->pos == end && valid(a);
}
static bool path_for(char path[512], const char *prefix, unsigned slot) {
    int n = snprintf(path, 512, "%s%u.tns", prefix, slot);
    return n > 0 && n < 512;
}
static bool read_slot(App *a, const char *prefix, unsigned slot) {
    char path[512]; Buffer b = {{0}, 0, 0, true};
    if (!path_for(path, prefix, slot)) return false;
    FILE *f = fopen(path, "rb"); if (!f) return false;
    b.size = fread(b.data, 1, SAVE_CAPACITY, f);
    bool ok = !ferror(f) && feof(f); fclose(f);
    return ok && decode(&b, a);
}
bool save_load(App *a, const char *prefix) {
    App first, second;
    bool v0 = read_slot(&first, prefix, 0), v1 = read_slot(&second, prefix, 1);
    if (!v0 && !v1) return false;
    if (v0 && (!v1 || (int32_t)(first.save_sequence - second.save_sequence) > 0)) *a = first;
    else *a = second;
    return true;
}
bool save_write(App *a, const char *prefix) {
    if (!valid(a)) return false;
    uint32_t sequence = a->save_sequence + 1;
    Buffer b = {{0}, 0, 0, true}; char path[512];
    encode(&b, a, sequence);
    if (!b.ok || !path_for(path, prefix, sequence & 1u)) return false;
    FILE *f = fopen(path, "wb"); if (!f) return false;
    bool ok = fwrite(b.data, 1, b.pos, f) == b.pos;
    if (fflush(f) != 0) ok = false;
    if (fclose(f) != 0) ok = false;
    if (ok) a->save_sequence = sequence;
    return ok;
}
