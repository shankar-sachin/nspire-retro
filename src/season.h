#ifndef NSPIRE_RETRO_SEASON_H
#define NSPIRE_RETRO_SEASON_H
#include <stdbool.h>
#include <stdint.h>
#define TEAM_COUNT 32
#define SEASON_WEEKS 17
#define ROSTER_COUNT 12
#define SALARY_CAP 200
enum { ROLE_QB, ROLE_RB, ROLE_WR1, ROLE_WR2, ROLE_OL, ROLE_DEF, ROLE_K, ROLE_TE, ROLE_OL2, ROLE_DL2, ROLE_LB, ROLE_DB };
enum { STAGE_REGULAR, STAGE_WILDCARD, STAGE_DIVISIONAL, STAGE_CONFERENCE, STAGE_TI_BOWL, STAGE_COMPLETE };
enum { CONF_NFC, CONF_AFC };
typedef struct { int name, rating, xp, condition, salary, years; } Player;
typedef struct { int wins, losses, ties, points_for, points_against; } Standing;
typedef struct {
    int league_size, stage, champion;
    int playoff_teams[14], playoff_winners[13], playoff_scores[13][2];
    int week_scores[SEASON_WEEKS][TEAM_COUNT];
    int salary_cap;
    int team, year, week, credits, trophies, career_wins, career_losses;
    Player roster[ROSTER_COUNT];
    Standing table[TEAM_COUNT];
    int results[SEASON_WEEKS][2]; /* -1 means not played. */
    Player free_agents[ROSTER_COUNT], prospects[ROSTER_COUNT];
    uint32_t free_signed, draft_taken;
    int draft_picks;
    bool draft_active;
    uint32_t rng;
} Season;
void season_init(Season *s, int team);
void season_init_seed(Season *s, int team, uint32_t seed);
int season_star_count(const Season *s);
int season_stars(const Player *p); /* Half-star units, 0 for reserves. */
void season_refresh_agents(Season *s);
bool season_draft(Season *s, int role);
void season_skip_pick(Season *s);
int season_roster_role(int row);
int season_roster_row(int role);
int season_regular_weeks(const Season *s);
int season_schedule_opponent(const Season *s, int team, int week);
int season_current_opponent(const Season *s);
int season_conference(int team);
int season_conference_rank(const Season *s, int team);
void season_bracket_pair(const Season *s, int game, int *a, int *b);
void season_simulate_round(Season *s);
const char *season_stage_name(const Season *s);
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
