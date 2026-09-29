# Changelog

## 1.2.0 (unreleased)

- Replace receiver-lock throws with drop-back, hold-Enter aiming, a free landing
  crosshair, and release-to-throw controls. Slow the simulation while aiming;
  receivers catch by proximity rather than a selected target ID.
- Start new careers with exactly three randomized stars. Show half-star ratings,
  a persistent free-agent board, and a three-round offseason draft before week one.
- Save format 4 preserves draft picks, boards, signings, and aimed passes, imports
  previous saves without stripping their roster, and protects throws on resume.

- Expand to all 32 NFL clubs, 17 custom regular-season weeks, conference standings,
  weekly league scores, and a 14-team postseason with top-seed byes and reseeding.
- Add conference finals and the TI Bowl, championship rewards, elimination/bye
  advancement, and paired kicking shootouts to resolve postseason ties.
- Expand to 12 fixed star slots and a 200M cap: QB, RB, two WRs, TE, two OL stars,
  two DL stars, LB, DB, and K. Use role-specific blocking and defensive ratings.
- Field full 11-player offensive and defensive units with reserve positions.
- Add Counter, Draw, Go, Out, Post, and Screen for a ten-play book with three
  receiving targets; add striped turf, hashes, crowd details, colored end zones,
  goalposts, and TI Bowl field markings.
- Add paginated club/roster/league screens and a responsive static landing page.
- Import support retains v1.0.0/v1.1.0 careers and exact suspended matches. Existing
  eight-team seasons finish before expanding; new roster roles begin as reserves.
- Validate every club's championship path, playoff saves/shootouts, old-save
  migration, all passing concepts, and complete seasons with host sanitizers.

No verified calculator binary is included: Ndless linking, hardware controls,
performance, and game feel still need CX II testing.

## 1.1.0

- Add a Special Teams menu with explicit punt and field-goal choices, a timed kick
  meter, kicker-dependent range/accuracy, punt returns, and touchbacks.
- Replace automatic player extra points with manual attempts, including after a
  touchdown at the final horn. Opponent conversions remain automatic.
- Add a seventh roster slot for the kicker and a 100M salary cap separate from
  coach credits, with two-season contracts, renewals, confirmed releases, and
  free reserves for expired/released players.
- Show salary, cap room, years remaining, and transaction quotes in roster menus.
- Migrate v1.0.0 saves without losing career or live-match progress. Format-2 saves
  persist contracts and kick-meter state and cannot be loaded by v1.0.0.
- Expand host tests for special teams, contracts, cap enforcement, and migration.

The calculator build and device performance remain unverified; this project still
requires an installed Ndless toolchain to produce a `.tns`.

## 1.0.0

Initial eight-team football and career source release.
