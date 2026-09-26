#include <libndls.h>
#include <stdio.h>
#include "render.h"
#define WIDTH 320
#define HEIGHT 240
#define FIELD_TOP 44
#define RGB(r,g,b) ((uint16_t)(((r) << 11) | ((g) << 5) | (b)))
#define INK RGB(2,6,9)
#define PAPER RGB(29,60,28)
#define GOLD RGB(31,49,5)
#define TURF RGB(3,29,8)
#define STRIPE RGB(4,34,10)
#define BLUE RGB(4,42,31)
#define RED RGB(30,12,10)
/* One static 150 KiB buffer. No per-frame allocations or direct LCD register access. */
static uint16_t pixels[WIDTH * HEIGHT];
static bool initialized;
static bool animate = true;
static int animation_frame;
static bool fallen;
/* Original compact 5x7 alphabet, columns with least significant bit at the top. */
static const uint8_t letters[26][5] = {
 {126,9,9,9,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},
 {127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},
 {65,65,127,65,65},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},
 {127,2,12,2,127},{127,2,4,8,127},{62,65,65,65,62},{127,9,9,9,6},
 {62,65,81,33,94},{127,9,25,41,70},{38,73,73,73,50},{1,1,127,1,1},
 {63,64,64,64,63},{31,32,64,32,31},{127,32,24,32,127},{99,20,8,20,99},
 {3,4,120,4,3},{97,81,73,69,67}
};
static const uint8_t numbers[10][5] = {
 {62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},
 {24,20,18,127,16},{39,69,69,69,57},{62,73,73,73,50},{1,113,9,5,3},
 {54,73,73,73,54},{38,73,73,73,62}
};
static int clamp(int n, int lo, int hi) { return n < lo ? lo : (n > hi ? hi : n); }
static void rect(int x, int y, int w, int h, uint16_t color) {
    int right = clamp(x + w, 0, WIDTH), bottom = clamp(y + h, 0, HEIGHT);
    x = clamp(x, 0, WIDTH); y = clamp(y, 0, HEIGHT);
    for (int row = y; row < bottom; ++row)
        for (int col = x; col < right; ++col) pixels[row * WIDTH + col] = color;
}
static void text(int x, int y, const char *s, uint16_t color, int scale) {
    for (; *s; ++s, x += 6 * scale) {
        const uint8_t *glyph = NULL;
        uint8_t punctuation[5] = {0};
        if (*s >= 'A' && *s <= 'Z') glyph = letters[*s - 'A'];
        else if (*s >= '0' && *s <= '9') glyph = numbers[*s - '0'];
        else {
            if (*s == '-') punctuation[1] = punctuation[2] = punctuation[3] = 8;
            if (*s == '+') { punctuation[1] = punctuation[3] = 8; punctuation[2] = 28; }
            if (*s == ':') punctuation[2] = 36;
            if (*s == '>') { punctuation[1] = 34; punctuation[2] = 20; punctuation[3] = 8; }
            if (*s == '/') { punctuation[0] = 64; punctuation[1] = 48; punctuation[2] = 8; punctuation[3] = 6; punctuation[4] = 1; }
            glyph = punctuation;
        }
        for (int col = 0; col < 5; ++col)
            for (int row = 0; row < 7; ++row)
                if (glyph[col] & (1 << row)) rect(x + col * scale, y + row * scale, scale, scale, color);
    }
}
static void player(Actor a, int camera, uint16_t jersey, bool carrier) {
    int x = a.x / FP - camera, y = a.y / FP + FIELD_TOP;
    if (x < -10 || x > WIDTH + 10) return;
    if (carrier && fallen) {
        rect(x - 6, y, 12, 5, jersey); rect(x + 6, y, 4, 4, PAPER); return;
    }
    int stride = animate ? (animation_frame / 5 % 2) * 2 : 0;
    rect(x - 4, y + 4, 9, 3, INK);
    rect(x - 3, y - 4, 6, 8, jersey);
    rect(x - 5, y - 2, 2, 4, jersey);
    rect(x + 3, y - 2, 2, 4, jersey);
    rect(x - 2, y - 7, 5, 4, PAPER);
    rect(x + 2, y - 6, 2, 2, INK);
    rect(x - 3, y + 3 + stride, 2, 3, PAPER);
    rect(x + 1, y + 5 - stride, 2, 3, PAPER);
    if (carrier) rect(x + 4, y, 3, 2, GOLD);
}
bool render_init(void) {
    initialized = lcd_init(SCR_320x240_565);
    return initialized;
}
void render_shutdown(void) {
    if (initialized) { lcd_init(SCR_TYPE_INVALID); initialized = false; }
}
static void draw_game(const Game *g) {
    animation_frame = g->phase == PHASE_LIVE ? g->animation : 0;
    fallen = g->phase == PHASE_RESULT && (g->result == RESULT_TACKLE || g->result == RESULT_DOWNS);
    uint16_t home = season_team_color(g->home_team);
    uint16_t away = season_team_color(g->away_team);
    char line[64];
    int focus = g->in_flight ? g->ball.x : g->carrier.x;
    int camera = clamp(focus / FP - 100, -24, FIELD_LENGTH - WIDTH + 24);
    rect(0, 0, WIDTH, HEIGHT, INK);
    rect(0, FIELD_TOP, WIDTH, FIELD_WIDTH, TURF);
    for (int yard = 0; yard < 100; yard += 5) {
        int x = yard * PX_PER_YARD - camera;
        if ((yard / 5) % 2 == 0) rect(x, FIELD_TOP, 5 * PX_PER_YARD, FIELD_WIDTH, STRIPE);
    }
    rect(-camera - 60, FIELD_TOP, 60, FIELD_WIDTH, home);
    rect(FIELD_LENGTH - camera, FIELD_TOP, 60, FIELD_WIDTH, away);
    for (int yard = 0; yard <= 100; yard += 10) {
        int x = yard * PX_PER_YARD - camera;
        rect(x, FIELD_TOP + 4, 1, FIELD_WIDTH - 8, RGB(17,45,19));
        snprintf(line, sizeof(line), "%d", yard <= 50 ? yard : 100 - yard);
        text(x + 4, FIELD_TOP + 13, line, PAPER, 1);
        text(x + 4, FIELD_TOP + FIELD_WIDTH - 22, line, PAPER, 1);
        for (int y = 48; y <= 104; y += 56) rect(x - 2, FIELD_TOP + y, 5, 1, PAPER);
    }
    for (int yard = 1; yard < 100; ++yard) {
        int x = yard * PX_PER_YARD - camera;
        rect(x, FIELD_TOP + 52, 1, 3, PAPER); rect(x, FIELD_TOP + 98, 1, 3, PAPER);
    }
    int middle = FIELD_LENGTH / 2 - camera;
    rect(middle - 34, FIELD_TOP + 65, 68, 22, home);
    text(middle - (g->bowl_game ? 30 : 16), FIELD_TOP + 71, g->bowl_game ? "TI BOWL" : season_team_abbr(g->home_team), g->bowl_game ? GOLD : PAPER, 1);
    text(-camera - 22, FIELD_TOP + 70, season_team_abbr(g->home_team), PAPER, 1);
    text(FIELD_LENGTH - camera + 5, FIELD_TOP + 70, season_team_abbr(g->away_team), PAPER, 1);
    for (int x = 0; x < WIDTH; x += 6) {
        uint16_t crowd = ((x + camera) / 6) % 3 ? PAPER : home;
        rect(x, FIELD_TOP, 3, 3, crowd); rect(x + 2, FIELD_TOP + FIELD_WIDTH - 3, 3, 3, crowd);
    }
    for (int end = 0; end <= FIELD_LENGTH; end += FIELD_LENGTH) {
        rect(end - camera - 1, FIELD_TOP + 4, 3, 4, GOLD);
        rect(end - camera - 1, FIELD_TOP + FIELD_WIDTH - 8, 3, 4, GOLD);
    }
    int post = FIELD_LENGTH - camera + 22;
    rect(post, FIELD_TOP + 58, 2, 36, GOLD); rect(post - 6, FIELD_TOP + 58, 8, 2, GOLD);
    rect(post - 6, FIELD_TOP + 92, 8, 2, GOLD);
    rect(0, FIELD_TOP + 3, WIDTH, 1, PAPER);
    rect(0, FIELD_TOP + FIELD_WIDTH - 4, WIDTH, 1, PAPER);
    rect(g->spot / FP - camera, FIELD_TOP + 2, 2, FIELD_WIDTH - 4, BLUE);
    rect(g->line_to_gain / FP - camera, FIELD_TOP + 2, 2, FIELD_WIDTH - 4, GOLD);
    for (int i = 0; i < game_receiver_count(g); ++i) {
        if (i == g->caught_receiver) continue;
        player(g->receivers[i], camera, home, false);
        if (game_is_pass(g->selected_play) && i == g->target && !g->passed) {
            int x = clamp(g->receivers[i].x / FP - camera - 2, 2, WIDTH - 8);
            text(x, g->receivers[i].y / FP + FIELD_TOP - 18, i == 0 ? "1" : (i == 1 ? "2" : "3"), GOLD, 1);
        }
    }
    for (int i = 0; i < game_blocker_count(g); ++i) player(g->blockers[i], camera, home, false);
    for (int i = 0; i < game_defender_count(g); ++i) {
        player(g->defenders[i], camera, PAPER, false);
        int x = g->defenders[i].x / FP - camera, y = g->defenders[i].y / FP + FIELD_TOP;
        rect(x - 2, y - 7, 5, 3, away);
        if (g->blocked[i]) rect(x - 3, y - 10, 6, 2, GOLD);
    }
    if (!g->legacy_units) for (int i = 0; i < 2; ++i) player(g->support[i], camera, home, false);
    if (g->caught_receiver >= 0) player(g->throw_start, camera, home, false);
    player(g->carrier, camera, home, !g->in_flight);
    if (g->in_flight) {
        int x = g->ball.x / FP - camera, y = g->ball.y / FP + FIELD_TOP;
        rect(x - 2, y + 2, 5, 2, INK);
        if (animate && g->lob) y -= g->flight_ticks * (g->flight_duration - g->flight_ticks) / 7;
        rect(x - 2, y - 1, 5, 3, GOLD);
        rect(x, y - 1, 1, 3, PAPER);
    }
    /* HUD is repainted after actors so no sprite can overlap the text. */
    rect(0, 0, WIDTH, FIELD_TOP, INK);
    snprintf(line, sizeof(line), "%s %d  %s %d", season_team_abbr(g->home_team), g->score, season_team_abbr(g->away_team), g->opponent_score);
    text(8, 5, line, GOLD, 1);
    snprintf(line, sizeof(line), "Q%d %d:%02d  DRIVE %d", g->quarter, (g->clock_ticks + GAME_HZ - 1) / GAME_HZ / 60, (g->clock_ticks + GAME_HZ - 1) / GAME_HZ % 60, g->drive);
    if (g->shootout_active) snprintf(line, sizeof(line), "PLAYOFF SHOOTOUT ROUND %d", g->shootout_round);
    text(8, 17, line, PAPER, 1);
    snprintf(line, sizeof(line), "DOWN %d  TO GO %d  BALL %d", g->down, game_yards_to_go(g), g->spot / (PX_PER_YARD * FP));
    text(8, 30, line, PAPER, 1);
    rect(0, 199, WIDTH, 41, INK);
    text(8, 204, "ARROWS MOVE  SHIFT BOOST  ESC PAUSE", PAPER, 1);
    rect(270, 30, 42, 5, PAPER); rect(271, 31, g->energy * 40 / 100, 3, GOLD);
    if (game_is_pass(g->selected_play) && !g->passed)
        text(8, 217, g->carrier.x <= g->spot ? "CTRL TARGET  ENTER THROW" : "PAST LINE - RUN ONLY", GOLD, 1);
    else text(8, 217, "ATTACK RIGHT  GOLD IS FIRST DOWN", GOLD, 1);
    if (g->phase == PHASE_CALL) {
        rect(32, 53, 256, 138, PAPER); rect(34, 55, 252, 134, INK);
        text(48, 64, "CALL YOUR PLAY", GOLD, 2);
        int first = ((int)g->selected_play / 5) * 5;
        for (int i = first; i < first + 5; ++i) {
            int y = 87 + (i - first) * 15;
            if (i == (int)g->selected_play) rect(43, y - 3, 232, 13, BLUE);
            text(48, y, i == (int)g->selected_play ? ">" : " ", PAPER, 1);
            text(64, y, game_play_name((Play)i), PAPER, 1);
        }
        text(43, 162, game_play_description(g->selected_play), GOLD, 1);
        text(43, 178, "ENTER SNAP  CTRL KICKS  UP/DN MORE", PAPER, 1);
    } else if (g->phase == PHASE_RESULT) {
        rect(28, 80, 264, 93, PAPER); rect(30, 82, 260, 89, INK);
        text(44, 94, game_result_name(g->result), GOLD, 1);
        if (g->result == RESULT_PUNT) snprintf(line, sizeof(line), "%d YD PUNT  %s", g->kick_distance, g->kick_touchback ? "TOUCHBACK" : "RETURN");
        else if (g->result == RESULT_FIELD_GOAL || g->result == RESULT_MISSED_KICK || g->result == RESULT_EXTRA_POINT || g->result == RESULT_EXTRA_MISSED)
            snprintf(line, sizeof(line), "%d YARD ATTEMPT", g->kick_distance);
        else snprintf(line, sizeof(line), "GAIN %+d YARDS", g->last_gain);
        text(44, 114, line, PAPER, 1);
        text(44, 136, g->shootout_active ? "ENTER SHOOTOUT RESULT" : g->pending_pat ? "ENTER EXTRA POINT ATTEMPT" : (g->new_drive ? "ENTER OPPONENT POSSESSION" : "ENTER TO CALL NEXT PLAY"), PAPER, 1);
        text(44, 153, "ESC PAUSE", PAPER, 1);
    }
    if (g->phase == PHASE_SPECIAL || g->phase == PHASE_KICK) {
        rect(20, 57, 280, 136, PAPER); rect(22, 59, 276, 132, INK);
        text(34, 70, g->shootout_active ? "TIEBREAKER" : "SPECIAL TEAMS", GOLD, 2);
        if (g->phase == PHASE_SPECIAL) {
            text(34, 100, g->kick_kind == KICK_PUNT ? "> PUNT" : "  PUNT", PAPER, 1);
            snprintf(line, sizeof(line), "%s FIELD GOAL - %d YARDS", g->kick_kind == KICK_FIELD_GOAL ? ">" : " ", 117 - g->spot / (PX_PER_YARD * FP));
            text(34, 120, line, PAPER, 1);
            text(34, 150, "UP/DOWN PICK  ENTER BEGIN", GOLD, 1);
            text(34, 172, "CTRL CANCEL  ESC PAUSE", PAPER, 1);
        } else {
            const char *kind = g->kick_kind == KICK_PUNT ? "PUNT" : (g->kick_kind == KICK_PAT ? "EXTRA POINT" : "FIELD GOAL");
            snprintf(line, sizeof(line), "%s  K RATING %d", kind, g->ratings[ROLE_K]); text(34, 99, line, PAPER, 1);
            rect(34, 121, 250, 14, PAPER); rect(36, 123, 246, 10, RED);
            rect(132, 123, 50, 10, TURF); rect(156, 120, 2, 16, GOLD);
            rect(36 + g->kick_meter * 244 / 100, 119, 3, 18, GOLD);
            text(34, 149, "ENTER STOP AT CENTER", GOLD, 1);
            text(34, 171, "KICK COMMITS AFTER 5 SECONDS", PAPER, 1);
        }
    }
    if (g->phase == PHASE_OPPONENT || g->phase == PHASE_BREAK || g->phase == PHASE_FINAL) {
        rect(20, 57, 280, 136, PAPER); rect(22, 59, 276, 132, INK);
        if (g->phase == PHASE_OPPONENT) {
            static const char *const events[] = {"OPPONENT TAKES THE FIELD", "OPPONENT ADVANCES", "OPPONENT TOUCHDOWN", "OPPONENT FIELD GOAL", "OPPONENT MISSES KICK", "OPPONENT PUNTS", "YOUR DEFENSE FORCES TURNOVER", "YOUR DEFENSE SCORES SAFETY"};
            text(34, 70, "DEFENSE ON THE FIELD", GOLD, 1);
            text(34, 88, events[g->cpu_event], PAPER, 1);
            snprintf(line, sizeof(line), "DOWN %d  BALL %d  GAIN %+d", g->cpu_down, g->cpu_spot, g->cpu_gain);
            text(34, 105, line, PAPER, 1);
            rect(34, 128, 250, 8, TURF); rect(34, 128, g->cpu_spot * 250 / 100, 8, away);
            text(34, 151, g->cpu_done || !g->clock_ticks ? "ENTER CONTINUE" : "ENTER NEXT SNAP OR WATCH", GOLD, 1);
            text(34, 174, g->cpu_pass ? "PASS - DL PRESSURE AND DB COVERAGE" : "RUN - DL FRONT AND LB PURSUIT", PAPER, 1);
        } else if (g->phase == PHASE_BREAK) {
            text(42, 78, g->quarter == 2 ? "HALFTIME" : "QUARTER BREAK", GOLD, 2);
            text(42, 112, g->quarter == 2 ? "OPPONENT RECEIVES NEXT" : "POSSESSION CARRIES FORWARD", PAPER, 1);
            text(42, 156, "ENTER NEXT QUARTER", GOLD, 1);
        } else {
            text(42, 72, g->score > g->opponent_score ? "VICTORY" : (g->score < g->opponent_score ? "DEFEAT" : "TIE GAME"), GOLD, 2);
            snprintf(line, sizeof(line), "FINAL %d - %d", g->score, g->opponent_score); text(42, 97, line, PAPER, 1);
            snprintf(line, sizeof(line), "PASS %d/%d  YARDS %d", g->completions, g->pass_attempts, g->passing_yards); text(42, 116, line, PAPER, 1);
            snprintf(line, sizeof(line), "RUSH %d  TD %d  TURNOVERS %d", g->rushing_yards, g->touchdowns, g->turnovers); text(42, 133, line, PAPER, 1);
            text(42, 163, "ENTER RECORD SEASON RESULT", GOLD, 1);
        }
    }
}
void render_frame(const Game *g) {
    draw_game(g);
    if (initialized) lcd_blit(pixels, SCR_320x240_565);
}
static void menu_row(int y, const char *label, bool selected) {
    if (selected) rect(16, y - 3, 288, 14, BLUE);
    text(22, y, selected ? ">" : " ", PAPER, 1); text(36, y, label, PAPER, 1);
}
static void page(const char *title) {
    rect(0, 0, WIDTH, HEIGHT, INK);
    rect(0, 0, WIDTH, 3, GOLD);
    text(16, 14, title, GOLD, 2);
    text(16, 218, "ARROWS SELECT  ENTER OK  ESC BACK", PAPER, 1);
}
void render_app(const App *a) {
    char line[64]; const Season *s = &a->season;
    animate = a->settings.animations != 0;
    switch (a->screen) {
    case SCREEN_MATCH: draw_game(&a->game); break;
    case SCREEN_PAUSE: {
        static const char *const rows[] = {"RESUME", "SAVE AND EXIT", "SETTINGS", "CONTROLS", "SAVE AND CLUBHOUSE", "EXIT WITHOUT SAVING"};
        draw_game(&a->game); rect(20, 52, 280, 155, INK); text(36, 63, "PAUSED", GOLD, 2);
        for (int i = 0; i < 6; ++i) menu_row(92 + i * 19, rows[i], i == a->selection);
        break;
    }
    case SCREEN_TITLE: {
        static const char *const rows[] = {"CONTINUE CAREER", "NEW CAREER", "SETTINGS", "CONTROLS", "SAVE AND EXIT", "EXIT WITHOUT SAVING"};
        page("NSPIRE RETRO"); text(264, 17, "V1.2.0", PAPER, 1); text(18, 40, "32 CLUBS - 12 STARS - TI BOWL", PAPER, 1);
        rect(18, 56, 284, 32, TURF);
        for (int i = 0; i < 8; ++i) {
            int team = i + (animate ? a->frame / 30 * 8 : 0);
            rect(21 + i * 35, 61, 28, 12, season_team_color(team)); text(22 + i * 35, 77, season_team_abbr(team), PAPER, 1);
        }
        for (int i = 0; i < 6; ++i) menu_row(99 + i * 19, i == 0 && !a->has_career ? "NO CAREER YET" : rows[i], i == a->selection);
        break;
    }
    case SCREEN_CONFIRM:
        page("NEW CAREER"); text(22, 63, "REPLACE YOUR EXISTING CAREER?", PAPER, 1);
        text(22, 84, "CURRENT PROGRESS WILL BE LOST", PAPER, 1);
        menu_row(120, "KEEP CURRENT CAREER", a->selection == 0);
        menu_row(145, "REPLACE AND CHOOSE TEAM", a->selection == 1); break;
    case SCREEN_TEAM: {
        page("CHOOSE YOUR TEAM");
        int first = a->selection / 8 * 8;
        for (int i = first; i < first + 8; ++i) {
            menu_row(47 + (i - first) * 18, season_team_name(i), i == a->selection);
            rect(284, 45 + (i - first) * 18, 14, 10, season_team_color(i));
        }
        snprintf(line, sizeof(line), "PAGE %d/4 - UP/DOWN FOR MORE", first / 8 + 1); text(18, 199, line, GOLD, 1);
        break;
    }
    case SCREEN_HUB: {
        page("CLUBHOUSE"); text(18, 40, season_team_name(s->team), PAPER, 1);
        if (s->stage == STAGE_REGULAR) snprintf(line, sizeof(line), "YEAR %d WEEK %d/%d  CREDITS %d", s->year, s->week + 1, season_regular_weeks(s), s->credits);
        else snprintf(line, sizeof(line), "%s  CREDITS %d", season_stage_name(s), s->credits);
        text(18, 55, line, GOLD, 1);
        snprintf(line, sizeof(line), "W%d L%d T%d  SEED %d  TROPHIES %d", s->table[s->team].wins, s->table[s->team].losses, s->table[s->team].ties, season_conference_rank(s, s->team), s->trophies); text(18, 70, line, PAPER, 1);
        const char *next = s->stage == STAGE_COMPLETE ? "START NEXT SEASON" : (season_current_opponent(s) < 0 ? "ADVANCE PLAYOFF ROUND" : "PLAY NEXT MATCH");
        const char *rows[] = {a->match_active ? "RESUME MATCH" : next, "ROSTER AND CONTRACTS", "LEAGUE AND PLAYOFF BRACKET", "SETTINGS", "SAVE AND EXIT", "TITLE SCREEN"};
        for (int i = 0; i < 6; ++i) menu_row(92 + i * 19, rows[i], i == a->selection);
        if (s->stage == STAGE_COMPLETE) snprintf(line, sizeof(line), "%s CHAMPIONS - %s", s->league_size == 8 ? "LEAGUE" : "TI BOWL", season_team_abbr(s->champion));
        else if (season_current_opponent(s) >= 0) snprintf(line, sizeof(line), "NEXT: %s", season_team_name(season_current_opponent(s)));
        else snprintf(line, sizeof(line), "BYE OR ELIMINATED - WATCH BRACKET");
        text(18, 207, line, GOLD, 1);
        break;
    }
    case SCREEN_ROSTER: {
        page("12 STAR SLOTS");
        snprintf(line, sizeof(line), "PAYROLL %dM / %dM  ROOM %dM", season_payroll(s), s->salary_cap, s->salary_cap - season_payroll(s)); text(18, 40, line, GOLD, 1);
        text(36, 55, "ROLE PLAYER   RATE PAY YEARS", PAPER, 1);
        int first = a->selection / 6 * 6;
        for (int row = first; row < first + 6; ++row) {
            int i = season_roster_role(row); const Player *p = &s->roster[i];
            snprintf(line, sizeof(line), "%-3s %-8s %2d  %2dM  %d", season_role_name(i), p->years ? season_player_name(p->name) : "RESERVE", p->rating, p->salary, p->years);
            menu_row(76 + (row - first) * 18, line, row == a->selection);
        }
        snprintf(line, sizeof(line), "PAGE %d/2  CREDITS %d", first / 6 + 1, s->credits); text(18, 191, line, GOLD, 1);
        text(18, 204, "UP/DOWN MORE  ENTER MANAGE", PAPER, 1);
        break;
    }
    case SCREEN_PLAYER: {
        int role = a->roster_selection; const Player *p = &s->roster[role];
        page("PLAYER CONTRACT");
        snprintf(line, sizeof(line), "%s %s  RATING %d", season_role_name(role), p->years ? season_player_name(p->name) : "RESERVE", p->rating); text(18, 40, line, PAPER, 1);
        snprintf(line, sizeof(line), "PAY %dM  YEARS %d  FITNESS %d  XP %d", p->salary, p->years, p->condition, p->xp); text(18, 54, line, PAPER, 1);
        snprintf(line, sizeof(line), "CAP %d/%dM  CREDITS %d", season_payroll(s), s->salary_cap, s->credits); text(18, 69, line, GOLD, 1);
        snprintf(line, sizeof(line), "TRAIN +2 RATING - 6 CREDITS"); menu_row(92, line, a->selection == 0);
        snprintf(line, sizeof(line), "SIGN R%d - %dM / %d CREDITS", season_recruit_rating(s, role), season_salary(season_recruit_rating(s, role), role), season_recruit_cost(s, role)); menu_row(113, line, a->selection == 1);
        snprintf(line, sizeof(line), "RENEW 2 YEARS - %dM / 3 CREDITS", season_salary(p->rating, role)); menu_row(134, line, a->selection == 2);
        menu_row(155, "RELEASE TO FREE RESERVE", a->selection == 3);
        menu_row(176, "RECOVER TEAM - 5 CREDITS", a->selection == 4);
        text(18, 202, a->match_active ? "LOCKED UNTIL MATCH FINISHES" : "PAY REPLACES CURRENT CAP CHARGE", GOLD, 1);
        break;
    }
    case SCREEN_RELEASE:
        page("RELEASE PLAYER");
        text(18, 57, "REPLACE WITH A FREE RATING 40 RESERVE?", PAPER, 1);
        text(18, 80, "PLAYER PROGRESS WILL BE LOST", GOLD, 1);
        menu_row(120, "KEEP PLAYER", a->selection == 0);
        menu_row(146, "RELEASE AND CLEAR SALARY", a->selection == 1);
        break;
    case SCREEN_SCHEDULE: {
        static const char *const titles[] = {"SCHEDULE", "STANDINGS", "TI BOWL BRACKET", "LEAGUE SCORES"};
        page(titles[a->selection]);
        if (a->selection == 0) {
            int first = a->page * 8, count = season_regular_weeks(s);
            for (int i = first; i < first + 8 && i < count; ++i) {
                int opponent = season_schedule_opponent(s, s->team, i);
                if (i < s->week) snprintf(line, sizeof(line), "W%02d %-3s  %d - %d", i + 1, season_team_abbr(opponent), s->results[i][0], s->results[i][1]);
                else snprintf(line, sizeof(line), "W%02d %-3s  %s", i + 1, season_team_abbr(opponent), i == s->week ? "NEXT" : "UPCOMING");
                menu_row(52 + (i - first) * 18, line, i == s->week);
            }
        } else if (a->selection == 1) {
            int conf = a->page / 2, first = a->page % 2 * 8 + 1;
            snprintf(line, sizeof(line), "%s SEED  W  L  T  PF   PA", conf ? "AFC" : "NFC"); text(22, 40, line, GOLD, 1);
            if (s->league_size == 8) { conf = -1; first = 1; }
            for (int rank = first; rank < first + 8; ++rank) for (int t = 0; t < s->league_size; ++t)
                if ((conf < 0 ? season_rank(s, t) : season_conference_rank(s, t)) == rank && (conf < 0 || season_conference(t) == conf)) {
                    const Standing *v = &s->table[t];
                    snprintf(line, sizeof(line), "%2d %-3s %2d %2d %2d %3d %3d", rank, season_team_abbr(t), v->wins, v->losses, v->ties, v->points_for, v->points_against);
                    menu_row(57 + (rank - first) * 18, line, t == s->team);
                }
        } else if (a->selection == 2) {
            if (s->league_size == 8) text(18, 62, "PLAYOFFS BEGIN NEXT SEASON", GOLD, 1);
            else for (int i = a->page * 7; i < a->page * 7 + 7 && i < 13; ++i) {
                int ta, tb; season_bracket_pair(s, i, &ta, &tb);
                const char *round = i < 6 ? "WC" : (i < 10 ? "DIV" : (i < 12 ? "CONF" : "BOWL"));
                const char *aa = ta < 0 ? "TBD" : season_team_abbr(ta), *bb = tb < 0 ? "TBD" : season_team_abbr(tb);
                if (s->playoff_winners[i] < 0) snprintf(line, sizeof(line), "%-4s %s VS %s", round, aa, bb);
                else snprintf(line, sizeof(line), "%-4s %s %d-%d %s", round, aa, s->playoff_scores[i][0], s->playoff_scores[i][1], bb);
                menu_row(56 + (i % 7) * 19, line, ta == s->team || tb == s->team);
            }
        } else {
            int w = a->page, match = 0;
            snprintf(line, sizeof(line), "WEEK %d RESULTS", w + 1); text(18, 40, line, GOLD, 1);
            for (int t = 0; t < s->league_size; ++t) {
                int opponent = season_schedule_opponent(s, t, w);
                if (t > opponent) continue;
                int pa = s->week_scores[w][t], pb = s->week_scores[w][opponent];
                if (pa < 0 || pb < 0) snprintf(line, sizeof(line), "%s - %s", season_team_abbr(t), season_team_abbr(opponent));
                else snprintf(line, sizeof(line), "%s %d-%d %s", season_team_abbr(t), pa, pb, season_team_abbr(opponent));
                text(18 + match / 8 * 152, 58 + match % 8 * 18, line, t == s->team || opponent == s->team ? GOLD : PAPER, 1); ++match;
            }
        }
        text(18, 204, "LEFT/RIGHT TAB  UP/DOWN PAGE", GOLD, 1); break;
    }
    case SCREEN_SETTINGS: {
        static const char *const difficulties[] = {"ROOKIE", "PRO", "ALL STAR"};
        page("SETTINGS");
        snprintf(line, sizeof(line), "DIFFICULTY  %s", difficulties[a->settings.difficulty]); menu_row(65, line, a->selection == 0);
        snprintf(line, sizeof(line), "QUARTER LENGTH  %d SECONDS", a->settings.quarter_seconds); menu_row(92, line, a->selection == 1);
        snprintf(line, sizeof(line), "ANIMATIONS  %s", a->settings.animations ? "ON" : "OFF"); menu_row(119, line, a->selection == 2);
        text(18, 159, "LEFT/RIGHT OR ENTER TO CHANGE", GOLD, 1);
        text(18, 180, "MATCH RULES APPLY NEXT MATCH", PAPER, 1); break;
    }
    case SCREEN_HELP: {
        page("CONTROLS");
        static const char *const rows[] = {"ARROWS MOVE / SELECT PLAY", "ENTER SNAP / THROW / CONTINUE", "CTRL SWITCH RECEIVER", "SHIFT RUN = LIMITED SPRINT", "SHIFT + ENTER = LOB PASS", "CALL SCREEN CTRL = PUNT OR FG", "ESC PAUSES / RETURNS", "TD 6 - KICK PAT 1 - FG 3", "DEFENSE IS SIMULATED", "AUTOSAVE AT PLAY TRANSITIONS"};
        for (int i = 0; i < 10; ++i) text(18, 46 + i * 16, rows[i], PAPER, 1);
        break;
    }
    }
    if (a->notice_ticks > 0) {
        static const char *const messages[] = {"", "SAVED", "SAVE FAILED - RETRY SAVE", "FUNDS FITNESS OR RATING LIMIT", "CLUB UPDATED", "SAVE RECOVERED", "SALARY CAP - RELEASE A PLAYER"};
        rect(0, 229, WIDTH, 11, INK); text(8, 231, messages[a->notice], a->notice == 2 ? RED : GOLD, 1);
    }
    if (initialized) lcd_blit(pixels, SCR_320x240_565);
}
