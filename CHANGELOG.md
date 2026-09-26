# Changelog

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
