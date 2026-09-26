#include "season.h"
#include <string.h>
static uint32_t roll(Season *s) { s->rng = s->rng * 1664525u + 1013904223u; return s->rng; }
static int cap(int n, int max) { return n > max ? max : n; }
const char *season_team_name(int team) {
    static const char *const names[TEAM_COUNT] = {"NEW YORK GIANTS", "GREEN BAY PACKERS", "SEATTLE SEAHAWKS", "KANSAS CITY CHIEFS", "BUFFALO BILLS", "BALTIMORE RAVENS", "PHILADELPHIA EAGLES", "DETROIT LIONS"};
    return names[team];
}
const char *season_team_abbr(int team) {
    static const char *const names[TEAM_COUNT] = {"NYG", "GB", "SEA", "KC", "BUF", "BAL", "PHI", "DET"};
    return names[team];
}
uint16_t season_team_color(int team) {
    static const uint16_t colors[TEAM_COUNT] = {0x0235, 0x1328, 0x0949, 0xe104, 0x023b, 0x3010, 0x036e, 0x049b};
    return colors[team];
}
const char *season_player_name(int name) {
    static const char *const names[] = {"A REED", "J VALE", "K MOSS", "R FINCH", "T STONE", "S LAKE",
        "D NORTH", "M ASH", "P LANE", "C WEST", "B FROST", "N DALE"};
    return names[name % 12];
}
const char *season_role_name(int role) {
    static const char *const names[] = {"QB", "RB", "WR1", "WR2", "OL", "DEF", "K"};
    return names[role];
}
int season_team_rating(int team, int year) {
    /* Tunable arcade balance, not live NFL rankings or a licensed player database. */
    static const int base[TEAM_COUNT] = {60, 72, 73, 80, 79, 78, 80, 77};
    return cap(base[team] + (year - 1) * 2, 88);
}
int season_effective_rating(const Season *s, int role) {
    return s->roster[role].rating - (100 - s->roster[role].condition) / 5;
}
int season_opponent(int team, int week) {
    int ring[TEAM_COUNT];
    for (int i = 0; i < TEAM_COUNT; ++i) ring[i] = i;
    for (int w = 0; w < week; ++w) {
        int last = ring[TEAM_COUNT - 1];
        for (int i = TEAM_COUNT - 1; i > 1; --i) ring[i] = ring[i - 1];
        ring[1] = last;
    }
    for (int i = 0; i < TEAM_COUNT / 2; ++i) {
        if (ring[i] == team) return ring[TEAM_COUNT - 1 - i];
        if (ring[TEAM_COUNT - 1 - i] == team) return ring[i];
    }
    return 0;
}
void season_init(Season *s, int team) {
    memset(s, 0, sizeof(*s));
    s->salary_cap = SALARY_CAP;
    s->team = team; s->year = 1; s->credits = 20; s->rng = 0x4e535052u + (uint32_t)team;
    for (int i = 0; i < ROSTER_COUNT; ++i) s->roster[i] = (Player){i, 60 + i % 3 * 3, 0, 100, season_salary(60 + i % 3 * 3, i), 2};
    for (int i = 0; i < SEASON_WEEKS; ++i) s->results[i][0] = s->results[i][1] = -1;
}
static void record_pair(Season *s, int a, int b, int pa, int pb) {
    Standing *x = &s->table[a], *y = &s->table[b];
    x->points_for += pa; x->points_against += pb;
    y->points_for += pb; y->points_against += pa;
    if (pa > pb) { ++x->wins; ++y->losses; }
    else if (pa < pb) { ++y->wins; ++x->losses; }
    else { ++x->ties; ++y->ties; }
}
int season_rank(const Season *s, int team) {
    int rank = 1;
    const Standing *t = &s->table[team];
    for (int i = 0; i < TEAM_COUNT; ++i) {
        const Standing *o = &s->table[i];
        int p = t->wins * 2 + t->ties, q = o->wins * 2 + o->ties;
        int d = t->points_for - t->points_against, e = o->points_for - o->points_against;
        if (q > p || (q == p && (e > d || (e == d && (o->points_for > t->points_for ||
            (o->points_for == t->points_for && i < team)))))) ++rank;
    }
    return rank;
}
void season_record(Season *s, int scored, int allowed) {
    if (s->week >= SEASON_WEEKS) return;
    int opponent = season_opponent(s->team, s->week);
    record_pair(s, s->team, opponent, scored, allowed);
    s->results[s->week][0] = scored; s->results[s->week][1] = allowed;
    for (int a = 0; a < TEAM_COUNT; ++a) {
        int b = season_opponent(a, s->week);
        if (a >= b || a == s->team || b == s->team) continue;
        int pa = (int)(roll(s) % 5) * 7 + (int)(roll(s) % 2) * 3;
        int pb = (int)(roll(s) % 5) * 7 + (int)(roll(s) % 2) * 3;
        record_pair(s, a, b, pa, pb);
    }
    if (scored > allowed) ++s->career_wins;
    if (scored < allowed) ++s->career_losses;
    s->credits = cap(s->credits + (scored > allowed ? 14 : 9), 9999);
    for (int i = 0; i < ROSTER_COUNT; ++i) {
        Player *p = &s->roster[i];
        if (!p->years) continue;
        p->xp += scored > allowed ? 5 : 3;
        if (p->xp >= 10) { p->xp -= 10; p->rating = cap(p->rating + 1, 95); }
        p->condition = p->condition > 48 ? p->condition - 8 : 40;
    }
    ++s->week;
    if (s->week == SEASON_WEEKS && season_rank(s, s->team) == 1) {
        ++s->trophies; s->credits = cap(s->credits + 25, 9999);
    }
}
void season_next(Season *s) {
    if (s->week != SEASON_WEEKS) return;
    s->year = cap(s->year + 1, 999); s->week = 0;
    memset(s->table, 0, sizeof(s->table));
    for (int i = 0; i < SEASON_WEEKS; ++i) s->results[i][0] = s->results[i][1] = -1;
    for (int i = 0; i < ROSTER_COUNT; ++i) {
        s->roster[i].condition = 100;
        if (s->roster[i].years && --s->roster[i].years == 0)
            s->roster[i] = (Player){i, 40, 0, 100, 0, 0};
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
