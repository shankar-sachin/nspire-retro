# Nspire Retro v1.2.0

An arcade football game for the **TI-Nspire CX II with Ndless**. Choose a team,
manage a roster, play offensive snaps, watch simulated opponent possessions, and
build a career across seasons. Rendering is native RGB565 at 320x240; there is
no SDL, desktop graphics dependency, downloaded art, or floating-point game loop.

## Teams and career

All **32 NFL clubs** are selectable, including NYG, GB, SEA, both Los Angeles
teams, Miami, Cleveland, Cincinnati, and the Jets. Names follow the
[NFL team directory](https://www.nfl.com/teams/). Uniforms are original pixel
sprites using team-inspired colors. Player names and ratings are fictional game
data, not a live NFL roster or current power ranking.

A new career has **17 regular-season weeks**: 15 distinct conference opponents
and two distinct opponents from the other conference. All 16 weekly games have
results; the other 15 games are simulated. This is a custom arcade schedule with
no regular-season byes or divisions, not the NFL's actual schedule. Standings use
two points per win and one per tie, then point differential, points scored, and
stable team ID.

The **top seven clubs in each conference** qualify. Each number-one seed receives
a Wild Card bye; seeds 2–7, 3–6, and 4–5 play. The Divisional round reseeds so the
highest seed faces the lowest surviving seed. Conference winners meet in the
**TI Bowl**. Only the TI Bowl champion earns the new season's trophy and 50 bonus
credits. If you have a bye or are eliminated, Advance Playoff Round simulates the
remaining fixtures. Start Next Season becomes available after the championship.
Your roster, credits, trophies, and career record carry forward.

The clubhouse has four league tabs: your schedule, conference standings, playoff
bracket, and every week's scores. Left/Right switches tabs; Up/Down changes pages
or the selected week.

New careers start with **exactly three star players**, randomly assigned to three
different positions. The other positions use free, rating-40 reserves. You can
build up to **twelve stars** through free agency and the offseason draft. Every
named player has a visible **0.5–5.0 star rating** derived from their ability;
reserves display 0.0 stars. New-career seeds use launch time and menu timing, while
saved careers keep their exact generated boards and roster.

**Twelve star slots** sit within complete on-field units. Every new match has 11
offensive players (QB, RB, two WRs, TE, five offensive linemen, and a fullback)
and 11 defenders (four defensive linemen, three linebackers, three cornerbacks,
and a safety). Reserve players fill positions beyond your star slots. The twelve
managed positions are fixed, rather than an unrestricted position mix:

| Star role | Effect |
|---|---|
| QB | Scramble speed, passing range, accuracy under pressure |
| RB | Running speed |
| WR1 / WR2 / TE | Route speed, speed after a catch, catch radius |
| OL1 / OL2 | Blocking duration for the left/center and right line groups |
| DL1 / DL2 | Defensive front strength during simulated opponent possessions |
| LB | Run and pass defense during simulated opponent possessions |
| DB | Pass defense during simulated opponent possessions |
| K | Punt distance, field-goal range, and kick accuracy |

The opponent's defenders play live pursuit, coverage, and blocking interactions
while you control offense. Your own defense resolves snap by snap through the
simulation, with DL/LB/DB ratings weighted differently for runs and passes.

Wins earn 14 credits and five XP per player; losses/ties earn nine credits and
three XP. Ten XP increases a rating by one. Training costs six credits for two
rating points and five fitness. Team recovery costs five credits and restores
25 fitness. Free agency offers a generated candidate at each position. Inspect their stars,
salary, and credit fee, then confirm the signing. Signing fills an empty star
slot or replaces your star at that position; each candidate can be signed only
once. The board refreshes after a completed match. Reopening menus or reloading
a save does not reroll it. Low fitness reduces
effective ratings; ratings cap at 95. Rosters are locked while a match is active.

### Salaries and contracts

The twelve-player roster has a **200M salary cap**, separate from coach credits.
Starting payroll depends on your three generated stars. The roster shows each player's annual cap charge, remaining
contract years, total payroll, and available room. Enter opens that player's
management screen with training, recruiting, renewal, release, and team recovery.

Signings and renewals last **two seasons**. Their quoted salary replaces the
outgoing player's cap charge; an over-cap transaction changes neither the roster
nor credits. Recruit signing costs remain coach credits; renewing costs three
credits. Salaries stay fixed during a contract, even when training or XP improves
the player. Renewals use the player's current rating and role to set a new salary.

After the championship, **Start Offseason Draft** advances the year, reduces
remaining contracts by one, and opens a saved draft board before week one. There
are **three rounds with one pick each**. Select a prospect, inspect their stars
and two-season rookie salary, and confirm. Draft picks cost no coach credits but
must fit the cap. You can replace a current star or fill a reserve position.
A prospect is selectable once per draft. Passing a pick requires confirmation;
leaving the screen preserves it. All three picks must be used or passed before
playing the new season. Returning to the draft cannot age contracts again.

Starting the offseason reduces remaining contract years by one. Expired players
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
- A tied playoff game uses an arcade kicking shootout: each side attempts one
  field goal per round, and each make adds three points. Equal rounds repeat; an
  unequal round decides the winner after both attempts. This is a custom
  tiebreaker, not NFL overtime.
- Offense always attacks right. The blue line marks the snap spot; gold marks
  the first-down target. The HUD shows both scores, quarter, clock, down, field
  position, and sprint energy.
- Split, Sweep, Counter, and Draw are runs with different lanes or blocking
  duration. Slant, Cross, Go, Out, Post, and Screen offer three moving receivers.
  After snapping, move the QB with the arrows to drop back or scramble. Hold
  **Enter**, move a free crosshair with the arrows, then **release Enter** to
  throw. Range and direction come from the point you choose, with a dotted guide.
  Hold Shift when releasing for a lob. Ctrl cancels aiming. The QB stops moving
  while aiming and the simulation slows to one-quarter speed; defenders still
  close in. Release the snap key before beginning a fresh aiming hold.
- Five offensive linemen and a fullback engage the front seven. Cornerbacks
  cover routes while linebackers and a deep safety pursue the carrier or pass. Defender movement remains
  slower than player maximum speed; difficulty changes positioning and speed.
- Throws go to your chosen landing point, with no receiver lock or automatic
  leading. Any receiver within catch range can catch it; control then transfers
  to that receiver. You must anticipate the route and lead the pass yourself.
  Range is limited by QB rating, and nearby pressure reduces
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
| Team selection | Up / Down, Enter | Choose one of 32 clubs |
| Play calling | Up / Down, Enter | Pick play / snap |
| Play calling | Ctrl | Open Special Teams |
| Special Teams | Up / Down, Enter | Choose Punt / Field Goal and start meter |
| Special Teams | Ctrl | Cancel before starting meter |
| Kick meter | Enter | Stop near center and kick |
| Live play | Arrows / touchpad arrows | Move carrier |
| Live play | Shift + movement | Sprint while energy lasts |
| Pass play | Hold Enter + arrows | Aim the landing point |
| Aiming | Release Enter | Throw a bullet |
| Aiming | Shift + release Enter | Throw a lob |
| Aiming | Ctrl | Cancel and resume QB movement |
| Match | Esc | Pause; freezes the complete match state |
| Result / quarter break | Enter | Continue |
| Opponent possession | Enter | Advance next snap / continue |
| Roster | Enter / Ctrl | Manage player / browse free agents |
| Free agents / draft | Up / Down, Enter | Inspect player and confirm signing / pick |
| Player management | Up / Down, Enter | Train, sign, renew, release, or recover team |
| League screens | Left / Right or Enter | Switch schedule, standings, bracket, scores |
| League screens | Up / Down | Change page or week |
| Settings | Left / Right or Enter | Change selected setting |
| Other menus | Esc | Return |

Enter, Ctrl, Esc, and menu navigation are edge-triggered; release between actions.
Difficulty and quarter-length changes apply to the next match. Animation settings
apply immediately. Pause offers resume, save/exit, settings, help, and a return
to the clubhouse with the match suspended. Continue/Resume restores that match.
The title and pause menus also offer an explicit **Exit without saving** option
if storage is unavailable.

## Saves

Save format 4 imports **v1.0.0, v1.1.0, and earlier v1.2.0 preview careers**. Existing progress, contracts,
clocks, passes, and kick meters are retained. An existing eight-team season
finishes its original seven-week schedule and old championship rules; the next
season expands to all 32 teams and the new playoffs. The currently suspended
legacy match retains its smaller units; subsequent matches use full units.

The cap expands to 200M. Five added roles begin as free rating-40 reserves,
without removing or charging for existing stars. Format-1 careers receive the
same six 10M contracts and 8M kicker used by the previous migration; previously
awarded automatic extra points are not repeated. Existing careers keep their roster rather than being reset to three stars.
New saves preserve all twelve roles, free-agent availability, draft board/picks,
weekly scores, playoff seeds/results, shootouts, and an aimed pass. A pass already
in flight in an older save finishes using its original target rules. After
resuming an aimed pass, hold Enter again to arm the throw; the menu confirmation
cannot accidentally launch it. Older versions
cannot read format-4 saves; back up both original slots before upgrading if you
might return to an older version. Historical scores for other teams were not
stored in older saves and remain unavailable for already completed legacy weeks.

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
src/season.h/.c  32 teams, schedules, standings, playoffs, roster, progression
src/save.h/.c    Versioned serialization, validation, alternating save slots
src/input.h/.c   Held movement and key-press edges
website/dist/   Static landing page, styles, team filters, actual renderer image
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
The suite covers three-star generation across 300 seeds, free-agent availability,
draft picks/contracts, aim pause/resume, and old-save imports; football rules; all six passes with three targets;
blocking, coverage, stamina and difficulty; 100,000 simulation ticks;
32 symmetric 17-week schedules; cap-safe roster transactions and contracts;
quarters, halftime, kicks, and buzzer extra points; and migration of fixtures
produced by the actual v1.0.0 and v1.1.0 encoders. Each of the 32 clubs is tested
through qualification, a first-seed bye, reseeding, playoff shootouts, and a TI
Bowl win, with exact save round trips at each stage. It also covers elimination,
damaged-slot recovery, and **two complete seasons through the public app state
machine**.
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

## Landing page

The landing page lives in `website/dist/` with no build dependencies. Preview it:

```sh
python3 -m http.server 4173 --directory website/dist
```

Open `http://127.0.0.1:4173/`. It includes all 32 teams with conference filters,
playbook and career details, and installation instructions. Its screenshot comes
from the game's actual RGB565 renderer. The page labels v1.2.0 as a preview while
the version is awaiting merge/release; the latest-release link remains separate.
