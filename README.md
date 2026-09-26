# Nspire Retro v1.1.0

An arcade football game for the **TI-Nspire CX II with Ndless**. Choose a team,
manage a roster, play offensive snaps, watch simulated opponent possessions, and
build a career across seasons. Rendering is native RGB565 at 320x240; there is
no SDL, desktop graphics dependency, downloaded art, or floating-point game loop.

## Teams and career

The eight selectable clubs are the **New York Giants, Green Bay Packers, Seattle
Seahawks, Kansas City Chiefs, Buffalo Bills, Baltimore Ravens, Philadelphia
Eagles, and Detroit Lions**. Names are from the [NFL team directory](https://www.nfl.com/teams/).
Uniforms are original pixel sprites using team-inspired colors. Player names and
ratings are fictional game data, not a live NFL roster or current power ranking.

Each season is a custom eight-team, seven-week round robin: every club faces every
other club once. The remaining three games each week are simulated. Standings
track wins, losses, ties, points scored and allowed. Ranking uses two points per
win and one per tie, then point differential, points scored, and stable team ID.
The top team earns a trophy and 25 credits after week seven. Start another season
to retain your roster, money, trophies, and career record; all players recover
fitness, and opponents gradually become stronger. This is a compact custom league,
not the NFL's 32-team schedule or postseason format.

The clubhouse provides roster management, schedule/results, standings, settings,
and the next match. Seven roster slots affect gameplay:

| Role | Effect |
|---|---|
| QB | Scramble speed, passing range, accuracy under pressure |
| RB | Running speed |
| WR1 / WR2 | Route speed, speed after a catch, catch radius |
| OL | How long the two blockers hold defenders |
| DEF | Opponent gains, turnovers, and kick success during simulated possessions |
| K | Player punt distance, field-goal range, and kick accuracy |

Wins earn 14 credits and five XP per player; losses/ties earn nine credits and
three XP. Ten XP increases a rating by one. Training costs six credits for two
rating points and five fitness. Team recovery costs five credits and restores
25 fitness. Recruiting replaces the selected player with the displayed weekly
prospect, resetting that slot's XP and restoring fitness. Low fitness reduces
effective ratings; ratings cap at 95. Rosters are locked while a match is active.

### Salaries and contracts

The seven-player roster has a **100M salary cap**, separate from coach credits.
New careers use 80M. The roster shows each player's annual cap charge, remaining
contract years, total payroll, and available room. Enter opens that player's
management screen with training, recruiting, renewal, release, and team recovery.

Signings and renewals last **two seasons**. Their quoted salary replaces the
outgoing player's cap charge; an over-cap transaction changes neither the roster
nor credits. Recruit signing costs remain coach credits; renewing costs three
credits. Salaries stay fixed during a contract, even when training or XP improves
the player. Renewals use the player's current rating and role to set a new salary.

Starting a new season reduces remaining contract years by one. Expired players
become free rating-40 reserves, so the team always remains playable. Release also
replaces a player with a reserve and clears the salary, after an in-app confirmation.
There is no dead-money penalty or release credit refund. Reserves cannot train,
earn XP, or renew; recruit a player into the slot to develop it. This is a compact
arcade cap model, not an implementation of NFL collective-bargaining rules.

## Match rules

- Four quarters, configurable to 60, 90, or 120 simulation seconds each.
- The player receives the opening kickoff at the 25; the opponent receives after
  halftime. Possession and field position carry through the first/third breaks.
- Four downs to gain ten yards; near the goal line, the line to gain is the goal.
- Player touchdowns score **six**, followed by a manual extra-point kick worth
  one. The attempt still occurs when the quarter or game clock expires on the
  touchdown. Opponent conversions remain automatic. Field goals score three and
  safeties two to the defense. Regular-season ties are allowed.
- Offense always attacks right. The blue line marks the snap spot; gold marks
  the first-down target. The HUD shows both scores, quarter, clock, down, field
  position, and sprint energy.
- Split and Sweep are runs with different starting lanes. Slant and Cross have
  two moving receivers. Choose a target and throw a bullet or lob behind the
  snap line; control transfers to the receiver on a catch.
- Two blockers engage the rush. Two other defenders cover receivers, close run
  lanes, then pursue the carrier or pass destination. Defender movement remains
  slower than player maximum speed; difficulty changes positioning and speed.
- Passes lead routes. Range is limited by QB rating, and nearby pressure reduces
  accuracy. Bullets can be intercepted throughout flight; lobs clear the rush
  but give coverage more time, becoming interceptable during their final quarter.
- Shift provides a short sprint that drains energy; release it to recover energy.
  Energy resets between snaps. Diagonal movement is normalized.
- Tackles and sidelines end the play; sacks lose yardage. Incompletions preserve
  the snap spot. Interceptions and failed fourth downs flip field position.
- Ctrl on the play-call screen opens **Special Teams**. Select Punt or Field Goal
  with Up/Down, then Enter to start the kick. Ctrl cancels before committing.
- Press Enter again to stop the meter near its center. Kicker rating and timing
  determine punt distance or field-goal range/accuracy. The meter commits after
  five simulation seconds if you do not press Enter. Esc pauses it normally.
- Field-goal distance includes 17 yards for the snap and end zone. Long attempts
  can fall short even with perfect timing. Misses give the opponent the kick spot,
  or its 20, whichever is farther from its goal line. Punt touchbacks start at the
  20; short punts can be returned, with worse timing allowing longer returns.
- Kickoffs are still automatic at the 25. Punt returns are simulated; no manual
  return or onside-kick mode is included.
- Opponent possessions resolve one snap at a time using downs, field position,
  defense rating, difficulty, and deterministic saved randomness. Fourth down
  produces a punt or field-goal attempt. Watch them or press Enter to advance.
- The clock runs during live snaps, with five seconds of compact huddle/runoff
  after in-bounds non-scoring plays. CPU snaps use four to eight seconds; kicks
  use five. Menus, pause, and result screens do not consume match time. A play
  already in progress can finish after the clock reaches zero. A 20-second
  simulation play limit prevents endless scrambling.
- Final scores produce victory, defeat, or tie screens with passing/rushing
  statistics. Enter records the result exactly once and returns to the clubhouse.

## Controls

| Context | Keys | Action |
|---|---|---|
| Menus | Up / Down, Enter | Select / confirm |
| Team selection | Up / Down, Enter | Choose one of eight clubs |
| Play calling | Up / Down, Enter | Pick play / snap |
| Play calling | Ctrl | Open Special Teams |
| Special Teams | Up / Down, Enter | Choose Punt / Field Goal and start meter |
| Special Teams | Ctrl | Cancel before starting meter |
| Kick meter | Enter | Stop near center and kick |
| Live play | Arrows / touchpad arrows | Move carrier |
| Live play | Shift + movement | Sprint while energy lasts |
| Pass play | Ctrl | Select receiver 1 or 2 |
| Pass play | Enter | Bullet pass |
| Pass play | Shift + Enter | Lob pass |
| Match | Esc | Pause; freezes the complete match state |
| Result / quarter break | Enter | Continue |
| Opponent possession | Enter | Advance next snap / continue |
| Roster | Enter | Open selected player management |
| Player management | Up / Down, Enter | Train, sign, renew, release, or recover team |
| Schedule / standings | Left / Right or Enter | Switch pages |
| Settings | Left / Right or Enter | Change selected setting |
| Other menus | Esc | Return |

Enter, Ctrl, Esc, and menu navigation are edge-triggered; release between actions.
Difficulty and quarter-length changes apply to the next match. Animation settings
apply immediately. Pause offers resume, save/exit, settings, help, and a return
to the clubhouse with the match suspended. Continue/Resume restores that match.
The title and pause menus also offer an explicit **Exit without saving** option
if storage is unavailable.

## Saves

Save format 2 automatically imports format-1 (`v1.0.0`) careers, including live
passes and match clocks. Existing six players retain their progress and receive
10M, two-season contracts; a rating-60 kicker receives an 8M contract. Migrated
payroll is 68M regardless of previous player ratings, so no player is removed to
meet the cap. Future renewals use normal salary quotes. Previously awarded
`v1.0.0` automatic extra points are not awarded again. New saves include all
contracts and the exact kick-meter state. `v1.0.0` cannot read format-2 saves;
back up both original slots before returning to that older version.

The game uses two small, versioned, checksummed files **beside the executable**:
`nspire-retro-save0.tns` and `nspire-retro-save1.tns`. These are data files; launch
`nspire-retro.tns`, not either save file. Keep both when transferring a career.

Saves contain settings, the full season and roster, both scores, clock, all actor
positions, pass flight, and the random generator state. Live matches can resume
exactly where saved. The game autosaves at match phase changes and after career
or settings changes; Save and Exit also captures a paused mid-play state.

Writes alternate slots, leaving the previous slot untouched. Loading selects the
newest valid slot and can fall back after truncation or checksum corruption.
File contents use explicit little-endian fields, not compiler-dependent structure
images, and are validated before use. A failed Save and Exit shows an error and
keeps the app open for retry or deliberate exit without saving. An abrupt shutdown
loses changes since the last successful save. Both invalid/missing files start
at the title with no loaded career. New Career asks before replacing progress.

## Build and sideload

Install a current [official Ndless SDK and ARM toolchain](https://github.com/ndless-nspire/Ndless/wiki/Ndless-SDK:-C-and-assembly-development-introduction)
with the modern LCD-blit API and CX II support, then add its tools to PATH:

```sh
export PATH="/absolute/path/to/Ndless/ndless-sdk/bin:$PATH"
command -v nspire-gcc nspire-ld genzehn make-prg
make
```

Output: **`build/nspire-retro.tns`**. The Makefile follows the SDK conventions:
`nspire-gcc` -> `nspire-ld` -> `genzehn` -> `make-prg`. It targets ARM926EJ-S,
ARM mode, and `-O2`, with unused-section removal and LCD-blit metadata. Debug:
`make clean && make DEBUG=TRUE`. Cleaning only removes the generated `build/`
directory; calculator saves live beside the installed program.

1. Check the [official Ndless project](https://github.com/ndless-nspire/Ndless) for
   support for your exact calculator OS and install the matching Ndless release.
   Do not assume an OS update is Ndless-compatible.
2. Connect the CX II by USB and use TI file-transfer software that supports your
   device to copy `build/nspire-retro.tns` into a Documents subfolder.
3. Open that program while Ndless is active. Its directory must allow save files.
4. Use Save and Exit from the title, clubhouse, or pause screen. LCD mode is
   restored before control returns to the OS.

## Source layout

```text
src/main.c       Ndless startup, filesystem setup, loop, save I/O, clean exit
src/app.h/.c     Menus, pause, career/match transitions, exactly-once results
src/game.h/.c    On-field play, opponent snaps, match clock, scoring, football AI
src/season.h/.c  NFL team identifiers, schedule, standings, roster, progression
src/save.h/.c    Versioned serialization, validation, alternating save slots
src/input.h/.c   Held movement and key-press edges
src/render.h/.c  Clipped RGB565 graphics, menus, HUD, sprite and ball animations
```

The game and career logic do not depend on Ndless. The application allocates no
heap objects; the framebuffer is one static **153,600-byte** RGB565 array, plus
small fixed-size game/career structures. The SDK and file library may allocate
internally. Pass-range scaling uses two 64-bit integer divisions only when a
throw exceeds range; all normal movement is small integer arithmetic. There is
no unmeasured assembly optimization.

The main loop sleeps 33 ms and targets approximately 30 fps; rendering/blitting
and save I/O add overhead. **This is not hardware-measured or locked 30 fps.**
Simulation slows with a slower frame rate. Ndless newlib's `clock()` is not a
usable frame timer and its `gettimeofday()` has one-second resolution, so the
code does not use either for false precision. Graphics use `lcd_init`, a local
clipped rasterizer, and `lcd_blit`, never a direct LCD pointer or old screen API.

## Verification and current limits

```sh
make check
```

Requires a host C99 compiler with AddressSanitizer and UndefinedBehaviorSanitizer.
The suite covers football rules; run/pass/catch/interception/pressure/range;
blocking, coverage, stamina and difficulty effects; 100,000 simulation ticks;
all eight round-robin schedules; roster economics and progression; pause/input
edges; quarters/halftime/final scores; kick timing, touchbacks, returns, missed
kicks, buzzer extra points, cap-safe transactions, renewal/expiry, and migration
of a fixture produced by the actual `v1.0.0` encoder; save round trips and damaged-slot recovery;
and **two complete seven-game seasons through the public app state machine**.
It also renders every menu with clipping checks and writes 320x240 PPM previews
in `build/`. Host stubs are never included in calculator builds.

Host checks pass, and the generated screens have been visually inspected.
Straight-run bot trials were used to remove a guaranteed-touchdown exploit and
separate difficulty levels. These checks are not human playtesting: on-device
controls, game feel, Ndless linking, save persistence, and frame rate still need
CX II testing. The development host does not have `nspire-gcc`, so no verified
calculator `.tns` binary is included. Defense is intentionally simulated when
the opponent has possession; there are no real NFL player rosters, online play,
manual defensive controls, or audio.

SDK references:
[API](https://github.com/ndless-nspire/Ndless/blob/master/ndless-sdk/include/libndls.h),
[build template](https://github.com/ndless-nspire/Ndless/blob/master/ndless-sdk/samples/newlib/Makefile),
[LCD](https://github.com/ndless-nspire/Ndless/blob/master/ndless-sdk/libndls/lcd_blit.cpp),
[relative paths](https://github.com/ndless-nspire/Ndless/blob/master/ndless-sdk/libndls/enable_relative_paths.c),
[time implementation](https://github.com/ndless-nspire/Ndless/blob/master/ndless-sdk/libsyscalls/stdlib.cpp).
