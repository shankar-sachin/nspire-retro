#include "season.h"
#include <string.h>
static uint32_t roll(Season *s) { s->rng = s->rng * 1664525u + 1013904223u; return s->rng; }
static int cap(int n, int max) { return n > max ? max : n; }
const char *season_team_name(int team) {
    static const char *const names[TEAM_COUNT] = {"NEW YORK GIANTS", "GREEN BAY PACKERS", "SEATTLE SEAHAWKS", "KANSAS CITY CHIEFS", "BUFFALO BILLS", "BALTIMORE RAVENS", "PHILADELPHIA EAGLES", "DETROIT LIONS", "LOS ANGELES RAMS", "LOS ANGELES CHARGERS", "MIAMI DOLPHINS", "CLEVELAND BROWNS", "CINCINNATI BENGALS", "NEW YORK JETS", "SAN FRANCISCO 49ERS", "DALLAS COWBOYS", "PITTSBURGH STEELERS", "DENVER BRONCOS", "NEW ENGLAND PATRIOTS", "HOUSTON TEXANS", "TAMPA BAY BUCCANEERS", "MINNESOTA VIKINGS", "ATLANTA FALCONS", "WASHINGTON COMMANDERS", "TENNESSEE TITANS", "INDIANAPOLIS COLTS", "JACKSONVILLE JAGUARS", "LAS VEGAS RAIDERS", "NEW ORLEANS SAINTS", "CAROLINA PANTHERS", "ARIZONA CARDINALS", "CHICAGO BEARS"};
    return names[team];
}
const char *season_team_abbr(int team) {
    static const char *const names[TEAM_COUNT] = {"NYG", "GB", "SEA", "KC", "BUF", "BAL", "PHI", "DET", "LAR", "LAC", "MIA", "CLE", "CIN", "NYJ", "SF", "DAL", "PIT", "DEN", "NE", "HOU", "TB", "MIN", "ATL", "WAS", "TEN", "IND", "JAX", "LV", "NO", "CAR", "ARI", "CHI"};
    return names[team];
}
uint16_t season_team_color(int team) {
    static const uint16_t colors[TEAM_COUNT] = {0x0235, 0x1328, 0x0949, 0xe104, 0x023b, 0x3010, 0x036e, 0x049b, 0x023c, 0x05bf, 0x0557, 0x8220, 0xf340, 0x03a9, 0xb002, 0x0230, 0xfda0, 0xfb20, 0x020c, 0xb804, 0xe002, 0x5017, 0xb802, 0x8005, 0x0454, 0x023b, 0x03cf, 0x4208, 0xcdb3, 0x0435, 0xb803, 0xf380};
    return colors[team];
}
const char *season_player_name(int name) {
    static const char *const names[] = {"A REED", "J VALE", "K MOSS", "R FINCH", "T STONE", "S LAKE",
        "D NORTH", "M ASH", "P LANE", "C WEST", "B FROST", "N DALE"};
    return names[name % 12];
}
const char *season_role_name(int role) {
    static const char *const names[] = {"QB", "RB", "WR1", "WR2", "OL1", "DL1", "K", "TE", "OL2", "DL2", "LB", "DB"};
    return names[role];
}
int season_team_rating(int team, int year) {
    /* Tunable arcade balance, not live NFL rankings or a licensed player database. */
    static const int base[TEAM_COUNT] = {60,72,73,80,79,78,80,77,75,73,74,66,76,66,79,72,72,71,65,76,73,73,68,72,64,68,67,65,66,62,69,70};
    return cap(base[team] + (year - 1) * 2, 88);
}
int season_effective_rating(const Season *s, int role) {
    return s->roster[role].rating - (100 - s->roster[role].condition) / 5;
}
int season_conference(int team) {
    static const int conference[TEAM_COUNT] = {0,0,0,1,1,1,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0};
    return conference[team];
}
int season_regular_weeks(const Season *s) { return s->league_size == 8 ? 7 : SEASON_WEEKS; }
static int circle_opponent(int *ring, int count, int team, int week) {
    for (int w = 0; w < week; ++w) {
        int last = ring[count - 1];
        for (int i = count - 1; i > 1; --i) ring[i] = ring[i - 1];
        ring[1] = last;
    }
    for (int i = 0; i < count / 2; ++i) {
        if (ring[i] == team) return ring[count - 1 - i];
        if (ring[count - 1 - i] == team) return ring[i];
    }
    return -1;
}
int season_opponent(int team, int week) {
    int own[16], other[16], n = 0, m = 0;
    for (int i = 0; i < TEAM_COUNT; ++i) {
        if (season_conference(i) == season_conference(team)) own[n++] = i;
        else other[m++] = i;
    }
    if (week >= 15) {
        int offset = week - 15;
        for (int i = 0; i < 16; ++i) if (own[i] == team) return other[(i + (season_conference(team) == CONF_NFC ? offset : -offset) + 16) % 16];
    }
    return circle_opponent(own, 16, team, week);
}
int season_schedule_opponent(const Season *s, int team, int week) {
    if (s->league_size == 8) {
        int ring[8] = {0,1,2,3,4,5,6,7};
        return circle_opponent(ring, 8, team, week);
    }
    return season_opponent(team, week);
}
static void reset_results(Season *s) {
    s->champion = -1; s->stage = STAGE_REGULAR;
    for (int i = 0; i < SEASON_WEEKS; ++i) {
        s->results[i][0] = s->results[i][1] = -1;
        for (int t = 0; t < TEAM_COUNT; ++t) s->week_scores[i][t] = -1;
    }
    for (int i = 0; i < 14; ++i) s->playoff_teams[i] = -1;
    for (int i = 0; i < 13; ++i) { s->playoff_winners[i] = -1; s->playoff_scores[i][0] = s->playoff_scores[i][1] = -1; }
}
void season_init(Season *s, int team) {
    memset(s, 0, sizeof(*s));
    s->league_size = TEAM_COUNT; s->salary_cap = SALARY_CAP;
    s->team = team; s->year = 1; s->credits = 20; s->rng = 0x4e535052u + (uint32_t)team;
    for (int i = 0; i < ROSTER_COUNT; ++i) s->roster[i] = (Player){i, 60 + i % 3 * 3, 0, 100, season_salary(60 + i % 3 * 3, i), 2};
    reset_results(s);
}
static void record_pair(Season *s, int a, int b, int pa, int pb) {
    Standing *x = &s->table[a], *y = &s->table[b];
    x->points_for += pa; x->points_against += pb;
    y->points_for += pb; y->points_against += pa;
    if (pa > pb) { ++x->wins; ++y->losses; }
    else if (pa < pb) { ++y->wins; ++x->losses; }
    else { ++x->ties; ++y->ties; }
    s->week_scores[s->week][a] = pa; s->week_scores[s->week][b] = pb;
}
static bool outranks(const Season *s, int a, int b) {
    const Standing *x = &s->table[a], *y = &s->table[b];
    int p = x->wins * 2 + x->ties, q = y->wins * 2 + y->ties;
    int d = x->points_for - x->points_against, e = y->points_for - y->points_against;
    return p > q || (p == q && (d > e || (d == e && (x->points_for > y->points_for || (x->points_for == y->points_for && a < b)))));
}
int season_rank(const Season *s, int team) {
    int rank = 1;
    for (int i = 0; i < s->league_size; ++i) if (i != team && outranks(s, i, team)) ++rank;
    return rank;
}
int season_conference_rank(const Season *s, int team) {
    int rank = 1;
    for (int i = 0; i < s->league_size; ++i)
        if (i != team && season_conference(i) == season_conference(team) && outranks(s, i, team)) ++rank;
    return rank;
}
void season_bracket_pair(const Season *s, int game, int *a, int *b) {
    if (game < 6) {
        int c = game / 3, match = game % 3;
        *a = s->playoff_teams[c * 7 + match + 1]; *b = s->playoff_teams[c * 7 + 6 - match];
    } else if (game < 10) {
        int c = (game - 6) / 2, winners[3];
        for (int i = 0; i < 3; ++i) winners[i] = s->playoff_winners[c * 3 + i];
        for (int i = 0; i < 3; ++i) if (winners[i] < 0) { *a = *b = -1; return; }
        for (int i = 0; i < 3; ++i) for (int j = i + 1; j < 3; ++j)
            if (season_conference_rank(s, winners[j]) < season_conference_rank(s, winners[i])) { int t = winners[i]; winners[i] = winners[j]; winners[j] = t; }
        if ((game - 6) % 2 == 0) { *a = s->playoff_teams[c * 7]; *b = winners[2]; }
        else { *a = winners[0]; *b = winners[1]; }
    } else if (game < 12) { *a = s->playoff_winners[6 + (game - 10) * 2]; *b = s->playoff_winners[7 + (game - 10) * 2]; }
    else { *a = s->playoff_winners[10]; *b = s->playoff_winners[11]; }
}
static int round_start(const Season *s) { return s->stage == STAGE_WILDCARD ? 0 : (s->stage == STAGE_DIVISIONAL ? 6 : (s->stage == STAGE_CONFERENCE ? 10 : 12)); }
static int round_end(const Season *s) { return s->stage == STAGE_WILDCARD ? 6 : (s->stage == STAGE_DIVISIONAL ? 10 : (s->stage == STAGE_CONFERENCE ? 12 : 13)); }
int season_current_opponent(const Season *s) {
    if (s->stage == STAGE_COMPLETE) return -1;
    if (s->stage == STAGE_REGULAR) return season_schedule_opponent(s, s->team, s->week);
    for (int i = round_start(s); i < round_end(s); ++i) {
        int a, b; season_bracket_pair(s, i, &a, &b);
        if (a == s->team) return b;
        if (b == s->team) return a;
    }
    return -1;
}
const char *season_stage_name(const Season *s) {
    static const char *const names[] = {"REGULAR SEASON", "WILD CARD ROUND", "DIVISIONAL ROUND", "CONFERENCE FINALS", "TI BOWL", "SEASON COMPLETE"};
    return names[s->stage];
}
static void seed_playoffs(Season *s) {
    for (int c = 0; c < 2; ++c) for (int seed = 1; seed <= 7; ++seed)
        for (int t = 0; t < TEAM_COUNT; ++t) if (season_conference(t) == c && season_conference_rank(s, t) == seed)
            s->playoff_teams[c * 7 + seed - 1] = t;
    s->stage = STAGE_WILDCARD;
}
static int simulate_score(Season *s) { return (int)(roll(s) % 5) * 7 + (int)(roll(s) % 2) * 3; }
static void reward(Season *s, int scored, int allowed) {
    if (scored > allowed) ++s->career_wins;
    if (scored < allowed) ++s->career_losses;
    s->credits = cap(s->credits + (scored > allowed ? 14 : 9), 9999);
    for (int i = 0; i < ROSTER_COUNT; ++i) {
        Player *p = &s->roster[i]; if (!p->years) continue;
        p->xp += scored > allowed ? 5 : 3;
        if (p->xp >= 10) { p->xp -= 10; p->rating = cap(p->rating + 1, 95); }
        p->condition = p->condition > 48 ? p->condition - 8 : 40;
    }
}
static void playoff_round(Season *s, bool played, int scored, int allowed) {
    for (int i = round_start(s); i < round_end(s); ++i) {
        int a, b; season_bracket_pair(s, i, &a, &b);
        int pa, pb;
        if (played && (a == s->team || b == s->team)) { pa = a == s->team ? scored : allowed; pb = b == s->team ? scored : allowed; }
        else { pa = simulate_score(s); pb = simulate_score(s); if (pa == pb) { if (roll(s) & 1u) pa += 3; else pb += 3; } }
        s->playoff_scores[i][0] = pa; s->playoff_scores[i][1] = pb; s->playoff_winners[i] = pa > pb ? a : b;
    }
    if (s->stage == STAGE_TI_BOWL) {
        s->champion = s->playoff_winners[12]; s->stage = STAGE_COMPLETE;
        if (s->champion == s->team) { ++s->trophies; s->credits = cap(s->credits + 50, 9999); }
    } else ++s->stage;
}
void season_simulate_round(Season *s) {
    if (s->stage > STAGE_REGULAR && s->stage < STAGE_COMPLETE && season_current_opponent(s) < 0) playoff_round(s, false, 0, 0);
}
void season_record(Season *s, int scored, int allowed) {
    int opponent = season_current_opponent(s);
    if (opponent < 0 || (s->stage != STAGE_REGULAR && scored == allowed)) return;
    reward(s, scored, allowed);
    if (s->stage != STAGE_REGULAR) { playoff_round(s, true, scored, allowed); return; }
    record_pair(s, s->team, opponent, scored, allowed);
    s->results[s->week][0] = scored; s->results[s->week][1] = allowed;
    for (int a = 0; a < s->league_size; ++a) {
        int b = season_schedule_opponent(s, a, s->week);
        if (a >= b || a == s->team || b == s->team) continue;
        int pa = simulate_score(s), pb = simulate_score(s); record_pair(s, a, b, pa, pb);
    }
    ++s->week;
    if (s->week == season_regular_weeks(s)) {
        if (s->league_size == 8) {
            s->stage = STAGE_COMPLETE;
            for (int t = 0; t < 8; ++t) if (season_rank(s, t) == 1) s->champion = t;
            if (s->champion == s->team) { ++s->trophies; s->credits = cap(s->credits + 25, 9999); }
        } else seed_playoffs(s);
    }
}
void season_next(Season *s) {
    if (s->stage != STAGE_COMPLETE) return;
    s->year = cap(s->year + 1, 999); s->week = 0; s->league_size = TEAM_COUNT;
    memset(s->table, 0, sizeof(s->table)); reset_results(s);
    for (int i = 0; i < ROSTER_COUNT; ++i) {
        s->roster[i].condition = 100;
        if (s->roster[i].years && --s->roster[i].years == 0) s->roster[i] = (Player){i, 40, 0, 100, 0, 0};
    }
}
bool season_train(Season *s, int role) {
    Player *p = &s->roster[role];
    if (!p->years || s->credits < 6 || p->rating >= 95 || p->condition < 50) return false;
    s->credits -= 6; p->rating = cap(p->rating + 2, 95); p->condition -= 5;
    return true;
}
bool season_recover(Season *s) {
    bool needed = false;
    for (int i = 0; i < ROSTER_COUNT; ++i) if (s->roster[i].condition < 100) needed = true;
    if (!needed || s->credits < 5) return false;
    s->credits -= 5;
    for (int i = 0; i < ROSTER_COUNT; ++i) s->roster[i].condition = cap(s->roster[i].condition + 25, 100);
    return true;
}
int season_recruit_rating(const Season *s, int role) {
    return cap(66 + (s->year - 1) * 2 + (s->week * 3 + role * 7) % 13, 95);
}
int season_recruit_cost(const Season *s, int role) { return 15 + (season_recruit_rating(s, role) - 60) / 2; }
bool season_recruit(Season *s, int role) {
    int cost = season_recruit_cost(s, role);
    int salary = season_salary(season_recruit_rating(s, role), role);
    if (s->credits < cost || !season_can_sign(s, role, salary)) return false;
    s->credits -= cost;
    s->roster[role] = (Player){(role + 6 + s->week) % 12, season_recruit_rating(s, role), 0, 100, salary, 2};
    return true;
}

int season_salary(int rating, int role) {
    int salary = (rating - 40) / 2 + (role == ROLE_QB ? 4 : (role == ROLE_K ? -2 : 0));
    return salary < 4 ? 4 : salary;
}
int season_payroll(const Season *s) {
    int total = 0;
    for (int i = 0; i < ROSTER_COUNT; ++i) total += s->roster[i].salary;
    return total;
}
bool season_can_sign(const Season *s, int role, int salary) {
    return role >= 0 && role < ROSTER_COUNT && salary >= 4 &&
        season_payroll(s) - s->roster[role].salary + salary <= s->salary_cap;
}
bool season_renew(Season *s, int role) {
    Player *p = &s->roster[role];
    int salary = season_salary(p->rating, role);
    if (!p->years || s->credits < 3 || !season_can_sign(s, role, salary)) return false;
    s->credits -= 3; p->salary = salary; p->years = 2; return true;
}
bool season_release(Season *s, int role) {
    if (!s->roster[role].years) return false;
    /* A free reserve keeps every role playable; released contracts have no dead money. */
    s->roster[role] = (Player){role, 40, 0, 100, 0, 0}; return true;
}
void season_migrate_v1(Season *s) {
    s->salary_cap = SALARY_CAP;
    for (int i = 0; i < 6; ++i) { s->roster[i].salary = 10; s->roster[i].years = 2; }
    s->roster[ROLE_K] = (Player){6, 60, 0, 100, 8, 2};
}

int season_roster_role(int row) {
    static const int roles[ROSTER_COUNT] = {ROLE_QB, ROLE_RB, ROLE_WR1, ROLE_WR2, ROLE_TE, ROLE_OL, ROLE_OL2, ROLE_DEF, ROLE_DL2, ROLE_LB, ROLE_DB, ROLE_K};
    return roles[row];
}
int season_roster_row(int role) {
    for (int i = 0; i < ROSTER_COUNT; ++i) if (season_roster_role(i) == role) return i;
    return 0;
}
