#ifndef NSPIRE_RETRO_GAME_H
#define NSPIRE_RETRO_GAME_H
#include <stdbool.h>
#include <stdint.h>
#include "input.h"
#include "season.h"
#define FP 256
#define PX_PER_YARD 6
#define FIELD_LENGTH (100 * PX_PER_YARD)
#define FIELD_WIDTH 152
#define DEFENDER_COUNT 4
#define RECEIVER_COUNT 2
#define PLAY_COUNT 4
#define GAME_HZ 30
#define TACKLE_RADIUS 7
#define PLAYER_SPEED 576
#define DEFENDER_SPEED 384
#define RECEIVER_SPEED 448
#define PASS_TICKS 16
#define PLAY_LIMIT_TICKS (20 * GAME_HZ)
#define TOUCHDOWN_POINTS 6
#define BLOCKER_COUNT 2

typedef enum { PHASE_CALL, PHASE_LIVE, PHASE_RESULT, PHASE_OPPONENT, PHASE_BREAK, PHASE_FINAL, PHASE_SPECIAL, PHASE_KICK } Phase;
typedef enum { RUN_SPLIT, RUN_SWEEP, PASS_SLANT, PASS_CROSS } Play;
typedef enum {
    RESULT_NONE, RESULT_TACKLE, RESULT_BOUNDS, RESULT_INCOMPLETE,
    RESULT_FIRST_DOWN, RESULT_TOUCHDOWN, RESULT_DOWNS,
    RESULT_INTERCEPTION, RESULT_SAFETY, RESULT_TIMEOUT, RESULT_PUNT, RESULT_FIELD_GOAL, RESULT_MISSED_KICK, RESULT_EXTRA_POINT, RESULT_EXTRA_MISSED
} Result;
enum { KICK_PUNT, KICK_FIELD_GOAL, KICK_PAT };
typedef struct { int32_t x, y; } Actor; /* Q8 world pixels */
typedef struct {
    Phase phase;
    Play selected_play;
    Result result;
    Actor carrier, receivers[RECEIVER_COUNT], defenders[DEFENDER_COUNT];
    Actor ball, throw_start, throw_target;
    Actor blockers[BLOCKER_COUNT];
    int blocked[DEFENDER_COUNT];
    int down, spot, line_to_gain; /* Q8 world pixels, always attacking right */
    int score, drive, turnovers, last_gain; /* last_gain in signed yards */
    int target, caught_receiver, ticks, flight_ticks;
    bool in_flight, passed, new_drive;
    int opponent_score, quarter, clock_ticks, quarter_seconds, difficulty;
    int home_team, away_team, ratings[ROSTER_COUNT], opponent_rating;
    int energy, flight_duration, throw_cooldown, animation;
    int pass_attempts, completions, passing_yards, rushing_yards, touchdowns;
    int cpu_spot, cpu_down, cpu_line, cpu_timer, cpu_event, cpu_gain, opponent_start;
    bool cpu_done, lob;
    Phase resume_phase;
    int kick_kind, kick_meter, kick_direction, kick_ticks, kick_distance, kick_return;
    bool pending_pat, kick_touchback;
    uint32_t rng;
} Game;
void game_init(Game *g);
void game_start(Game *g, int quarter_seconds, int difficulty, const int ratings[ROSTER_COUNT], int opponent_rating, uint32_t seed);
void game_kick(Game *g, bool field_goal);
void game_update(Game *g, const Input *input);
int game_yards_to_go(const Game *g);
const char *game_play_name(Play play);
const char *game_result_name(Result result);
#endif
