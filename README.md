# Gaia
A 2D platformer and menu-based game built in C using SDL 1.2. 
## Current State & Known Issues
**⚠️ Work in Progress:** The game currently has several significant issues that need to be fixed before it is fully playable:
- **Asset Corruption:** Some original PNG assets (e.g., `Niv1.png`, `B_play1.png`) are corrupted and cause `libpng` to abort. Temporary dummy surfaces are currently bypassing these crashes. These assets need to be fixed or re-exported.
- **Memory Management:** Immediate heap corruptions and null-pointer crashes have been addressed locally. Ongoing scrutiny is required to ensure that new entities remain cleanly allocated and freed.
- **Legacy Framework:** This is built on SDL 1.2, which is outdated. There may be audio/video compatibility quirks on modern Linux systems until it is ported to SDL 2.0.
## Tech Stack
- **Language:** C
- **Graphics & Audio:** 
  - [SDL 1.2](https://www.libsdl.org/)
  - SDL_image (PNG, JPG support)
  - SDL_mixer (WAV, MP3 support)
  - SDL_ttf (Font rendering)
- **Build System:** Make / GCC
## Requirements
To compile and run this project, you need to install the SDL 1.2 development libraries.
**For Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install build-essential libsdl1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev
```
**For Fedora:**
```bash
sudo dnf install gcc make SDL-devel SDL_image-devel SDL_mixer-devel SDL_ttf-devel
```
## How to Build and Run
1. Compile the project using `make`:
   ```bash
   make pro
   ```
2. Run the generated executable:
   ```bash
   ./pro
   ```
