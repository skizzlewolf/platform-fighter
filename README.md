# 4-Corners — 2D Platform Fighter

The original idea was a terreria-like game but it quickly turned into a fully playable 2-player platform fighter built from scratch in **C++** and **SFML**. No game engine — all physics, combat, and rendering are custom-built.

> **Pre-alpha** — core gameplay loop is complete and playable. Characters and stages are represented 
> by colored rectangles — no art assets yet. The focus so far has been entirely on the underlying 
> game systems (physics, combat, frame data). Visual polish is a future milestone.
---

## Gameplay Features

- 2-player local versus and training mode
- Full game loop: start screen → menus → character select → stage select → match → results
- 2 handcrafted stages with layered parallax visuals and one-way platforms
- Stock and timer-based match rules
- Blast zones and respawn system with spawn invulnerability

## Combat System

- **Frame data** — every move has startup, active, and recovery frames
- **3-hit autocombo chain** (Light 1 → Light 2 → Light 3) with hit-confirm gating
- **Heavy cancel** — cancel a light attack into heavy within a defined window, on hit only
- **Input buffer** — inputs registered up to ~180ms early so actions don't get dropped
- **Knockback scaling** — knockback and hitstun grow with accumulated damage percent
- **Hitstop** — brief freeze on hit for impact feel
- **Run light** — unique sliding attack only available out of a run
- **Defense** — block (reduces knockback and damage), dodge (invulnerability frames + cooldown), shield HP with break state

## Movement

- Walk, run (double-tap or hold), and separate air/ground physics
- Variable jump height (hold to rise higher, release to fall faster)
- Double jump
- Fast fall
- Drop through one-way platforms

## Training Mode & Debug Tools

- Toggleable hitbox and hurtbox overlays
- State tint overlays (hitstun, knockdown, block, dodge invuln)
- Live combo counter, hitstun timer, dodge cooldown, and frame timers
- Pause menu with in-match training settings

---

## Controls

| Action | Player 1 | Player 2 |
|---|---|---|
| Move | A / D | ← / → |
| Jump | Space | Right Shift |
| Down / Fast Fall | S | ↓ |
| Light Attack | J | Num 1 |
| Heavy Attack | K | Num 2 |
| Block / Dodge | L | Num 3 |

**Run:** Double-tap a direction  
**Drop through platform:** Hold Down while grounded  
**Dodge:** Press Block while holding a direction  

<img width="1920" height="1080" alt="Screenshot (39)" src="https://github.com/user-attachments/assets/14e63c3f-10bc-4d41-b957-4a63d10c9cb9" />
![gameplay screenshot — placeholder art](screenshot.png)
*Pre-alpha build — rectangle placeholders for characters and platforms*

---

## Building & Running

### Requirements
- [SFML 2.6+](https://www.sfml-dev.org/download.php)
- Visual Studio 2022 (Windows)
- A font file named `COLONNA.ttf` in the project root (or swap it out in `main.cpp`)

### Setup
1. Clone the repo
2. Open `Terreria Clone.slnx` in Visual Studio
3. Make sure SFML is linked in the project properties (include + lib paths)
4. Build and run

---

## Project Structure

| File | Purpose |
|---|---|
| `main.cpp` | Game loop, state machine, rendering, hit resolution |
| `Player.cpp / .h` | All player logic — physics, input, combat, movement |
| `Stage.cpp / .h` | Stage building, platform data, visual layers |
| `UI.cpp / .h` | Menus, buttons, pause screen, training settings |
| `GameTypes.h` | Shared enums and structs (moves, match rules, session data) |

---

## What I Learned

This was my first solo game project built without an engine. The biggest technical challenges were:

- Designing a frame-data system that cleanly separates startup/active/recovery and drives both hitbox activation and cancel windows
- Building an input buffer so attacks queue correctly during animation recovery
- Getting one-way platform collision right (pass through going up, land going down, drop-through on input)
- Knockback and hitstun scaling that feels fair at low percent and dangerous at high percent

---

*Built with C++ and SFML — no engine, no art assets, no shortcuts. All gameplay systems from scratch.*
