#include "save.h"
#include <stdio.h>
#include <string.h>
#define SAVE_VERSION 3u
#define SAVE_CAPACITY 8192
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
    for (int i = 0; i < 6; ++i) {
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
    put(b, (uint32_t)s->salary_cap);
    const Player *k = &s->roster[ROLE_K];
    put(b, (uint32_t)k->name); put(b, (uint32_t)k->rating); put(b, (uint32_t)k->xp); put(b, (uint32_t)k->condition);
    for (int i = 0; i < ROSTER_COUNT; ++i) { put(b, (uint32_t)s->roster[i].salary); put(b, (uint32_t)s->roster[i].years); }
    put(b, (uint32_t)g->ratings[ROLE_K]);
    put(b, (uint32_t)g->kick_kind); put(b, (uint32_t)g->kick_meter); put(b, (uint32_t)g->kick_direction);
    put(b, (uint32_t)g->kick_ticks); put(b, (uint32_t)g->kick_distance); put(b, (uint32_t)g->kick_return);
    put(b, g->pending_pat); put(b, g->kick_touchback);
    put(b, (uint32_t)s->league_size); put(b, (uint32_t)s->stage); put(b, (uint32_t)s->champion);
    for (int i = 7; i < ROSTER_COUNT; ++i) {
        const Player *p = &s->roster[i]; put(b, (uint32_t)p->name); put(b, (uint32_t)p->rating); put(b, (uint32_t)p->xp); put(b, (uint32_t)p->condition);
        put(b, (uint32_t)g->ratings[i]);
    }
    for (int i = 0; i < 14; ++i) put(b, (uint32_t)s->playoff_teams[i]);
    for (int i = 0; i < 13; ++i) { put(b, (uint32_t)s->playoff_winners[i]); put(b, (uint32_t)s->playoff_scores[i][0]); put(b, (uint32_t)s->playoff_scores[i][1]); }
    for (int w = 0; w < SEASON_WEEKS; ++w) for (int t = 0; t < TEAM_COUNT; ++t) put(b, (uint32_t)s->week_scores[w][t]);
    actor_put(b, g->support[0]); actor_put(b, g->support[1]);
    put(b, g->postseason); put(b, g->bowl_game); put(b, g->shootout_active); put(b, g->legacy_units); put(b, g->cpu_pass); put(b, (uint32_t)g->shootout_round);
    uint32_t hash = checksum(b->data, b->pos); put(b, hash);
}
static bool range(int n, int lo, int hi) { return n >= lo && n <= hi; }
static bool valid_actor(Actor a) { return range(a.x, -100 * FP, 800 * FP) && range(a.y, -100 * FP, 250 * FP); }
static bool valid(const App *a) {
    const Season *s = &a->season; const Game *g = &a->game;
    if (!range(a->settings.difficulty, 0, 2) || (a->settings.quarter_seconds != 60 && a->settings.quarter_seconds != 90 && a->settings.quarter_seconds != 120) || !range(a->settings.animations, 0, 1)) return false;
    if (!range(s->team, 0, TEAM_COUNT - 1) || !range(s->week, 0, SEASON_WEEKS) || !range(s->year, 1, 999) || !range(s->credits, 0, 9999) || !range(s->trophies, 0, 100000) || !range(s->career_wins, 0, 100000) || !range(s->career_losses, 0, 100000)) return false;
    if ((s->league_size != 8 && s->league_size != TEAM_COUNT) || s->team >= s->league_size ||
        !range(s->stage, STAGE_REGULAR, STAGE_COMPLETE) || s->week > season_regular_weeks(s)) return false;
    if ((s->stage == STAGE_REGULAR) != (s->week < season_regular_weeks(s))) return false;
    if (s->league_size == 8 && s->stage != STAGE_REGULAR && s->stage != STAGE_COMPLETE) return false;
    if (s->salary_cap != SALARY_CAP) return false;
    for (int i = 0; i < ROSTER_COUNT; ++i) {
        const Player *p = &s->roster[i];
        if (!range(p->name, 0, 11) || !range(p->rating, 30, 95) || !range(p->xp, 0, 9) || !range(p->condition, 0, 100)) return false;
        if (!range(p->salary, 0, 40) || !range(p->years, 0, 2) || ((p->years == 0) != (p->salary == 0))) return false;
        if (!p->years && (p->rating != 40 || p->xp != 0)) return false;
        if (!range(g->ratings[i], 20, 95)) return false;
    }
    if (season_payroll(s) > s->salary_cap) return false;
    for (int i = 0; i < SEASON_WEEKS; ++i) for (int j = 0; j < 2; ++j)
        if (i < s->week ? !range(s->results[i][j], 0, 1000) : s->results[i][j] != -1) return false;
    for (int i = 0; i < TEAM_COUNT; ++i) {
        const Standing *t = &s->table[i];
        if (!range(t->wins, 0, SEASON_WEEKS) || !range(t->losses, 0, SEASON_WEEKS) || !range(t->ties, 0, SEASON_WEEKS) || t->wins + t->losses + t->ties != (i < s->league_size ? s->week : 0) || !range(t->points_for, 0, 17000) || !range(t->points_against, 0, 17000)) return false;
    }
    for (int w = 0; w < SEASON_WEEKS; ++w) for (int t = 0; t < TEAM_COUNT; ++t) {
        int score = s->week_scores[w][t];
        if (!range(score, -1, 1000) || ((w >= s->week || t >= s->league_size) && score != -1)) return false;
        if (s->league_size == TEAM_COUNT && w < s->week && score < 0) return false;
    }
    if (s->league_size == TEAM_COUNT && s->stage != STAGE_REGULAR) {
        uint32_t seen = 0;
        for (int i = 0; i < 14; ++i) {
            int team = s->playoff_teams[i];
            if (!range(team, 0, TEAM_COUNT - 1) || (seen & (1u << team)) || season_conference(team) != i / 7 || season_conference_rank(s, team) != i % 7 + 1) return false;
            seen |= 1u << team;
        }
        int finished = s->stage == STAGE_WILDCARD ? 0 : (s->stage == STAGE_DIVISIONAL ? 6 : (s->stage == STAGE_CONFERENCE ? 10 : (s->stage == STAGE_TI_BOWL ? 12 : 13)));
        for (int i = 0; i < 13; ++i) {
            int pa = s->playoff_scores[i][0], pb = s->playoff_scores[i][1];
            if (i < finished) {
                int ta, tb; season_bracket_pair(s, i, &ta, &tb);
                if (!range(pa, 0, 1000) || !range(pb, 0, 1000) || pa == pb || s->playoff_winners[i] != (pa > pb ? ta : tb)) return false;
            } else if (pa != -1 || pb != -1 || s->playoff_winners[i] != -1) return false;
        }
        if (s->champion != (s->stage == STAGE_COMPLETE ? s->playoff_winners[12] : -1)) return false;
    } else {
        for (int i = 0; i < 14; ++i) if (s->playoff_teams[i] != -1) return false;
        for (int i = 0; i < 13; ++i) if (s->playoff_winners[i] != -1 || s->playoff_scores[i][0] != -1 || s->playoff_scores[i][1] != -1) return false;
        if (s->stage == STAGE_COMPLETE ? !range(s->champion, 0, s->league_size - 1) : s->champion != -1) return false;
    }
    if (a->match_active && (!a->has_career || s->stage == STAGE_COMPLETE || g->home_team != s->team || g->away_team != season_current_opponent(s))) return false;
    if (!range(g->phase, PHASE_CALL, PHASE_KICK) || !range(g->selected_play, 0, PLAY_COUNT - 1) || !range(g->result, RESULT_NONE, RESULT_EXTRA_MISSED) || !range(g->resume_phase, PHASE_CALL, PHASE_FINAL)) return false;
    if (!range(g->quarter, 1, 4) || !range(g->quarter_seconds, 30, 180) || !range(g->clock_ticks, 0, g->quarter_seconds * GAME_HZ) || !range(g->down, 1, 4) || !range(g->difficulty, 0, 2)) return false;
    if (!range(g->home_team, 0, TEAM_COUNT - 1) || !range(g->away_team, 0, TEAM_COUNT - 1) || !range(g->opponent_rating, 30, 95)) return false;
    if (!range(g->score, 0, 1000) || !range(g->opponent_score, 0, 1000) || !range(g->spot, 0, FIELD_LENGTH * FP) || !range(g->line_to_gain, 0, FIELD_LENGTH * FP)) return false;
    if (!range(g->target, 0, game_receiver_count(g) - 1) || !range(g->caught_receiver, -1, game_receiver_count(g) - 1) || !range(g->ticks, 0, PLAY_LIMIT_TICKS) || !range(g->flight_duration, 16, 24) || !range(g->flight_ticks, 0, g->flight_duration) || !range(g->energy, 0, 100) || !range(g->animation, 0, 119)) return false;
    if (!range(g->drive, 1, 1000) || !range(g->turnovers, 0, 1000) || !range(g->last_gain, -100, 100) || !range(g->throw_cooldown, 0, 1000) || !range(g->pass_attempts, 0, 1000) || !range(g->completions, 0, g->pass_attempts) || !range(g->passing_yards, -10000, 10000) || !range(g->rushing_yards, -10000, 10000) || !range(g->touchdowns, 0, 1000)) return false;
    if (!range(g->cpu_spot, 0, 100) || !range(g->cpu_down, 0, 4) || !range(g->cpu_line, 0, 100) || !range(g->cpu_timer, 0, 45) || !range(g->cpu_event, 0, 7) || !range(g->cpu_gain, -4, 34) || !range(g->opponent_start, 0, 100)) return false;
    if (!range(g->kick_kind, KICK_PUNT, KICK_PAT) || !range(g->kick_meter, 0, 100) ||
        (g->kick_direction != -1 && g->kick_direction != 1) || !range(g->kick_ticks, 0, 150) ||
        !range(g->kick_distance, 0, 117) || !range(g->kick_return, 0, 15)) return false;
    if (g->phase == PHASE_SPECIAL && g->kick_kind == KICK_PAT) return false;
    if (g->pending_pat && (!g->new_drive || (g->phase != PHASE_RESULT && g->phase != PHASE_KICK))) return false;
    if (!range(g->shootout_round, 0, 1000) || (g->shootout_active && (!g->postseason || g->quarter != 4 || g->clock_ticks != 0 || g->shootout_round == 0 || (g->phase != PHASE_KICK && g->phase != PHASE_RESULT)))) return false;
    if (!valid_actor(g->support[0]) || !valid_actor(g->support[1])) return false;
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
    if (hash != checksum(b->data, end) || get(b) != 0x4e535052u) return false;
    uint32_t version = get(b);
    if (version < 1 || version > SAVE_VERSION) return false;
    app_init(a); a->save_sequence = get(b);
    uint32_t career = get(b), active = get(b);
    if (career > 1 || active > 1) return false;
    a->has_career = career != 0; a->match_active = active != 0;
    a->settings.difficulty = (int32_t)get(b); a->settings.quarter_seconds = (int32_t)get(b); a->settings.animations = (int32_t)get(b);
    Season *s = &a->season; Game *g = &a->game;
#define READ_S(f) s->f = (int32_t)get(b);
    SEASON_FIELDS(READ_S)
#undef READ_S
    for (int i = 0; i < 6; ++i) {
        s->roster[i].name = (int32_t)get(b); s->roster[i].rating = (int32_t)get(b);
        s->roster[i].xp = (int32_t)get(b); s->roster[i].condition = (int32_t)get(b);
    }
    for (int i = 0; i < (version < 3 ? 8 : TEAM_COUNT); ++i) {
        Standing *t = &s->table[i];
        t->wins = (int32_t)get(b); t->losses = (int32_t)get(b); t->ties = (int32_t)get(b);
        t->points_for = (int32_t)get(b); t->points_against = (int32_t)get(b);
    }
    for (int i = 0; i < (version < 3 ? 7 : SEASON_WEEKS); ++i) for (int j = 0; j < 2; ++j) s->results[i][j] = (int32_t)get(b);
#define READ_G(f) g->f = (int32_t)get(b);
    GAME_FIELDS(READ_G)
#undef READ_G
    g->carrier = actor_get(b); g->ball = actor_get(b); g->throw_start = actor_get(b); g->throw_target = actor_get(b);
    for (int i = 0; i < (version < 3 ? 2 : RECEIVER_COUNT); ++i) g->receivers[i] = actor_get(b);
    for (int i = 0; i < (version < 3 ? 4 : DEFENDER_COUNT); ++i) { g->defenders[i] = actor_get(b); g->blocked[i] = (int32_t)get(b); }
    for (int i = 0; i < (version < 3 ? 2 : BLOCKER_COUNT); ++i) g->blockers[i] = actor_get(b);
    for (int i = 0; i < 6; ++i) g->ratings[i] = (int32_t)get(b);
    if (version == 1) {
        if (g->phase > PHASE_FINAL || g->result > RESULT_MISSED_KICK) return false;
        season_migrate_v1(s); g->ratings[ROLE_K] = 60;
    } else {
        s->salary_cap = (int32_t)get(b);
        Player *k = &s->roster[ROLE_K];
        k->name = (int32_t)get(b); k->rating = (int32_t)get(b); k->xp = (int32_t)get(b); k->condition = (int32_t)get(b);
        for (int i = 0; i < (version < 3 ? 7 : ROSTER_COUNT); ++i) { s->roster[i].salary = (int32_t)get(b); s->roster[i].years = (int32_t)get(b); }
        g->ratings[ROLE_K] = (int32_t)get(b);
        g->kick_kind = (int32_t)get(b); g->kick_meter = (int32_t)get(b); g->kick_direction = (int32_t)get(b);
        g->kick_ticks = (int32_t)get(b); g->kick_distance = (int32_t)get(b); g->kick_return = (int32_t)get(b);
        uint32_t pending = get(b), touchback = get(b);
        if (pending > 1 || touchback > 1) return false;
        g->pending_pat = pending != 0; g->kick_touchback = touchback != 0;
    }
    if (version < 3) {
        if (!range(s->team, 0, 7) || !range(s->week, 0, 7) || (version == 2 && s->salary_cap != 100)) return false;
        s->league_size = 8; s->stage = s->week < 7 ? STAGE_REGULAR : STAGE_COMPLETE; s->salary_cap = SALARY_CAP;
        /* Validate before ranking untrusted legacy standings (which does arithmetic). */
        for (int t = 0; t < 8; ++t) {
            const Standing *v = &s->table[t];
            if (!range(v->wins, 0, 7) || !range(v->losses, 0, 7) || !range(v->ties, 0, 7) ||
                !range(v->points_for, 0, 7000) || !range(v->points_against, 0, 7000)) return false;
        }
        if (s->stage == STAGE_COMPLETE) for (int t = 0; t < 8; ++t) if (season_rank(s, t) == 1) s->champion = t;
        for (int i = 7; i < ROSTER_COUNT; ++i) { s->roster[i] = (Player){i,40,0,100,0,0}; g->ratings[i] = 40; }
        g->legacy_units = true;
        for (int w = 0; w < s->week; ++w) {
            int other = season_schedule_opponent(s, s->team, w);
            s->week_scores[w][s->team] = s->results[w][0]; s->week_scores[w][other] = s->results[w][1];
        }
    } else {
        s->league_size = (int32_t)get(b); s->stage = (int32_t)get(b); s->champion = (int32_t)get(b);
        for (int i = 7; i < ROSTER_COUNT; ++i) {
            Player *p = &s->roster[i]; p->name = (int32_t)get(b); p->rating = (int32_t)get(b); p->xp = (int32_t)get(b); p->condition = (int32_t)get(b); g->ratings[i] = (int32_t)get(b);
        }
        for (int i = 0; i < 14; ++i) s->playoff_teams[i] = (int32_t)get(b);
        for (int i = 0; i < 13; ++i) { s->playoff_winners[i] = (int32_t)get(b); s->playoff_scores[i][0] = (int32_t)get(b); s->playoff_scores[i][1] = (int32_t)get(b); }
        for (int w = 0; w < SEASON_WEEKS; ++w) for (int t = 0; t < TEAM_COUNT; ++t) s->week_scores[w][t] = (int32_t)get(b);
        g->support[0] = actor_get(b); g->support[1] = actor_get(b);
        uint32_t postseason = get(b), bowl = get(b), shootout = get(b), legacy = get(b), pass = get(b);
        if (postseason > 1 || bowl > 1 || shootout > 1 || legacy > 1 || pass > 1) return false;
        g->postseason = postseason; g->bowl_game = bowl; g->shootout_active = shootout; g->legacy_units = legacy; g->cpu_pass = pass;
        g->shootout_round = (int32_t)get(b);
    }
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
