# Changelog
All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2026-05-13
### Fixed
- Fixed function name collision in `minimap.h` and `menu.h` for `init_background`.
- Fixed segmentation faults and heap corruption caused by incorrect memory allocation for `perso` struct (`barre` array).
- Fixed floating point exception (division by zero) in calculating `dt` for frame delay.
- Reduced audio noise and clipping by optimizing `Mix_OpenAudio` and `Mix_Volume` parameters.
- Reordered core SDL initialization (`SDL_Init` moved to the very top) to prevent start-up unresponsiveness.
### Added
- Added `IMG_Init` for explicit SDL_image PNG loading capability.
- Added defensive `NULL` pointer checks during surface rendering to prevent crashes.
- Implemented temporary dummy SDL surfaces to bypass corrupted PNG assets during `IMG_Load`, allowing the game to boot.

## [0.1.0] - 2023-05
### Added
- Integrated initial complete game source code.
- Combined main menu, settings menu, and gameplay structure into a single repository.
- Baseline source files (`main_menu.c`, `menu.c`, `minimap.c`, `perso.c`) containing original logic structure.
