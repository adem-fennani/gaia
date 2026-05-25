# Planning for v0.4.0

## Core Goal
- Implement one complete, end-to-end playable mission (from the start menu, into the level, completing objectives, and returning to the menu/score screen).

## Feature Suggestions & Technical Debt
- **Asset Fixes**: Re-export or replace corrupted assets (like `Niv1.png`, `B_play1.png`) so dummy surfaces can be entirely removed.
- **SDL2 Migration Plan**: Begin evaluating a port from SDL 1.2 to SDL 2.0 to resolve modern linux hardware compatibility quirks (especially with audio heap overflows).
- **Collision & Physics Polish**: Refine `perso` jump and bounds checking. Ensure the character correctly interacts with level geometry and boundaries.
- **Audio Feedback**: Add small sound effect events for clicking menus, jumping, and receiving damage.
- **HUD & UI Revamp**: Clean up the score overlay and health bars for a more modernized look and feel.
- **Level Design**: Polish enemy placement, background scrolling, and objective layout for the first level.
