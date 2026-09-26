#ifndef NSPIRE_RETRO_SEASON_H
#define NSPIRE_RETRO_SEASON_H
#include <stdbool.h>
#include <stdint.h>
#define TEAM_COUNT 8
#define SEASON_WEEKS 7
#define ROSTER_COUNT 7
#define SALARY_CAP 100
enum { ROLE_QB, ROLE_RB, ROLE_WR1, ROLE_WR2, ROLE_OL, ROLE_DEF, ROLE_K };
typedef struct { int name, rating, xp, condition, salary, years; } Player;
typedef struct { int wins, losses, ties, points_for, points_against; } Standing;
typedef struct {
    int salary_cap;
    int team, year, week, credits, trophies, career_wins, career_losses;
    Player roster[ROSTER_COUNT];
    Standing table[TEAM_COUNT];
    int results[SEASON_WEEKS][2]; /* -1 means not played. */
    uint32_t rng;
} Season;
void season_init(Season *s, int team);
int season_opponent(int team, int week);
int season_team_rating(int team, int year);
int season_effective_rating(const Season *s, int role);
int season_rank(const Season *s, int team);
void season_record(Season *s, int scored, int allowed);
void season_next(Season *s);
bool season_train(Season *s, int role);
bool season_recover(Season *s);
int season_recruit_rating(const Season *s, int role);
int season_recruit_cost(const Season *s, int role);
int season_payroll(const Season *s);
int season_salary(int rating, int role);
bool season_can_sign(const Season *s, int role, int salary);
bool season_renew(Season *s, int role);
bool season_release(Season *s, int role);
void season_migrate_v1(Season *s);
bool season_recruit(Season *s, int role);
const char *season_team_name(int team);
const char *season_team_abbr(int team);
uint16_t season_team_color(int team);
const char *season_player_name(int name);
const char *season_role_name(int role);
#endif
