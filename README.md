# 2D Platform Game in C

A university programming project focused on implementing game-state logic and working with abstract data types in **C**, using **raylib** for graphics and input.

## Overview

The project is a 2D platform game featuring a ball, platforms, and collectible stars. The game maintains information about objects, player movement, score, and whether the game is paused or running.

The coursework provided a starter framework, ADT interfaces, libraries, and an example game. This repository includes that framework alongside my implementations and tests.

## My contributions

- Implemented game-state management in `modules/state.c`, including object creation, updates, and cleanup.
- Developed an alternative game-state implementation in `modules/state_alt.c` using a **Set** rather than the **Vector** used in the main implementation.
- Implemented ordered-set helper functions in `modules/set_utils.c` to find the nearest value greater than or equal to, or less than or equal to, a target.
- Added tests for game-state operations and Set utilities in `tests/`.
- Added a playable game entry point in `programs/game/` that connects keyboard input, state updates, and rendering.

## Project structure

```text
include/                 Header files and ADT interfaces
lib/                     Libraries and library build files
modules/
  state.c                Main game-state implementation
  state_alt.c            Alternative Set-based implementation
  set_utils.c            Ordered-set utility functions
programs/
  game/                  Game application
  game_example/          Example application from the starter project
tests/
  state_test.c           Game-state tests
  set_utils_test.c       Set utility tests
Makefile                 Root build targets
```

## Build and run

From the repository root on a compatible Linux environment with GCC and Make installed:

```bash
make
make run-programs-game
```

To build and run the tests:

```bash
make tests
make run-tests
```

The project includes preconfigured Makefiles and libraries. Platform-specific library compatibility may vary.

### Alternative implementation

The default `programs/game/Makefile` and `tests/Makefile` link `modules/state.o`. To use `modules/state_alt.c`, adjust the corresponding `OBJS` entries to link `modules/state_alt.o` and include `modules/set_utils.o` instead. This follows the original project's build arrangement; the Makefiles have not been modified here.

## Academic context and attribution

Developed for a university programming assignment (2023). The original starter project supplied ADT headers, libraries, example-game code, and build infrastructure. The structure of game-state code (minimum-starter implementation), Set helpers, additional game entry point, and tests described above were developed as part of my coursework.

The original project structure is retained. Existing third-party or course-provided files remain under their original paths.
