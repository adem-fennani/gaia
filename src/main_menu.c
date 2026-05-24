#include "../include/menu.h"
#include "../include/minimap.h"
#include "../include/perso.h"

static SDL_Surface *screen = NULL;
static image background,
  B_play,
  B_play1,
  B_settings,
  B_settings1,
  B_quit,
  B_quit1,
  settings,
  slayed,
  exits,
  exits1;

static image backgame;

static SDL_Rect pos_plus,
  pos_moin;

static minimap m;
static temps t;
static perso p, p1;
static Mix_Music *music = NULL;
static Mix_Chunk *son = NULL;
static SDL_Event event;
static int done = 1, n = 0;
static int k = 0, etat = 0, s = 0, v = 64, counterr = 0;
static int count_p = 0, dep = 0, acc = 0, posy = 0, count_p1 = 0, dep1 = 0,
           acc1 = 0, posy1 = 0;
static char score[20];
static char score1[20];
static Uint32 dt = 0, t_prev = 0;

static void play_click_sound() {
  if (son != NULL) {
    Mix_PlayChannel(-1, son, 0);
  }
}

bool init_engine() {
  // Zero-initialize all image structures
  memset(&background, 0, sizeof(image));
  memset(&B_play, 0, sizeof(image));
  memset(&B_play1, 0, sizeof(image));
  memset(&B_settings, 0, sizeof(image));
  memset(&B_settings1, 0, sizeof(image));
  memset(&B_quit, 0, sizeof(image));
  memset(&B_quit1, 0, sizeof(image));
  memset(&settings, 0, sizeof(image));
  memset(&slayed, 0, sizeof(image));
  memset(&exits, 0, sizeof(image));
  memset(&exits1, 0, sizeof(image));
  // Zero-initialize the perso structs to avoid garbage values
  memset(&p, 0, sizeof(perso));
  memset(&p1, 0, sizeof(perso));
  memset(&backgame, 0, sizeof(image));
  memset(&m, 0, sizeof(minimap));
  memset(&t, 0, sizeof(temps));

  // Initialize SDL first - MUST happen before using any SDL functions
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("FAIL SDL INIT %s\n", SDL_GetError());
    return false;
  }

  // initial video
  putenv("SDL_VIDEO_CENTRED=1=");
  screen = SDL_SetVideoMode(
      1150, 650, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
  SDL_WM_SetCaption("GAIA REPRESENT ",
                    NULL);

  // Initialize SDL_image for PNG loading
  printf("DEBUG: Initializing SDL_image...\n");
  fflush(stdout);
  IMG_Init(IMG_INIT_PNG);
  printf("DEBUG: SDL_image initialized\n");
  fflush(stdout);

  if (screen == NULL) {
    printf("Can't set video mode: %s \n", SDL_GetError());
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
    printf("FAIL AUDIO %s\n", Mix_GetError());
  }

  printf("DEBUG: Initializing TTF...\n");
  fflush(stdout);
  TTF_Init();

  return true;
}

void load_game_resources() {
  printf("DEBUG: Creating game background surface...\n");
  fflush(stdout);
  // TODO: Niv1.png appears to be corrupted (libpng CRC error)
  // For now, creating a dummy surface to test the rest of the code
  backgame.img = SDL_CreateRGBSurface(0, 1150, 650, 32, 0, 0, 0, 0);
  if (backgame.img == NULL) {
    printf("ERROR: Failed to create backgame surface\n");
    fflush(stdout);
  }
  backgame.pos.x = 0;
  backgame.pos.y = 0;
  backgame.pos.w = 1150;
  backgame.pos.h = 650;

  // Disabled music due to sdl12-compat heap buffer overflow bug during MP3 decoding
  // music = Mix_LoadMUS("assets/audio/music.mp3");
  // if (music == NULL) {
  //   printf("FAIL MUSIC LOAD %s\n", Mix_GetError());
  // }
  // Mix_PlayMusic(music, -1); // play music

  // sound bref - use -1 for all channels, not channel 1
  Mix_Volume(-1, MIX_MAX_VOLUME / 3); // Start with lower volume
  son = Mix_LoadWAV("assets/audio/mouseclick.wav");
  if (son == NULL) {
    printf("FAIL SOUND LOAD %s\n", Mix_GetError());
  } else {
    Mix_VolumeChunk(son, MIX_MAX_VOLUME / 3);
  }

  printf("DEBUG: Loading background image...\n");
  fflush(stdout);
  // background init
  init_background(&background, "assets/img/background.jpg");

  printf("DEBUG: Loading menu buttons...\n");
  fflush(stdout);
  // MENU BUTTONS
  init_bouton(&B_play, "assets/img/B_play.png", &B_settings, "assets/img/B_settings.png", &B_quit, "assets/img/B_quit.png");
  init_bouton(&B_play1, "assets/img/B_play1.png", &B_settings1, "assets/img/B_settings1.png", &B_quit1, "assets/img/B_quit1.png");

  printf("DEBUG: Loading settings images...\n");
  fflush(stdout);
  init_background(&settings, "assets/img/settings.png");
  init_retour_bouton(&exits, "assets/img/exits.png");
  init_retour_bouton(&exits1, "assets/img/exits1.png");
  init_volume_slayed(&slayed, "assets/img/slayed.png");

  pos_moin.x = 489;
  pos_moin.y = 227;
  pos_moin.w = 512 - 489;
  pos_moin.h = 235 - 227;

  pos_plus.x = 730;
  pos_plus.y = 214;
  pos_plus.w = 30;
  pos_plus.h = 34;

  printf("DEBUG: Initializing perso...\n");
  fflush(stdout);
  initPerso(&p);
  printf("DEBUG: Initializing perso1...\n");
  fflush(stdout);
  initPerso1(&p1);

  printf("DEBUG: Initializing minimap...\n");
  fflush(stdout);
  initmap(&m);
  printf("DEBUG: Initializing temps...\n");
  fflush(stdout);
  init_temps(&t);
  printf("DEBUG: Setup complete, entering main loop...\n");
  fflush(stdout);

  posy = p.pos_background.y;
  posy1 = p1.pos_background.y;

  done = 1;
  etat = 0;
  k = 0;
  s = 0;
  v = 64;
  counterr = 0;
}

void run_game_loop() {
  while (done) {
    dt = SDL_GetTicks() - t_prev;
    t_prev = SDL_GetTicks();
    switch (etat) {
    case 0: // Menu state
    {
      /*affichier_imag(imgBACK[counterr],screen);
          if(counterr>=20)
          {
                  counterr=0;
          }
          counterr++;*/
      // SDL_BlitSurface(text,NULL,screen,&POS_txt)

      affichier_imag(background, screen);
      switch (k) {
      case 0: // Initial menu state
      {
        affichier_imag(B_play, screen);
        affichier_imag(B_settings, screen);
        affichier_imag(B_quit, screen);
        break;
      }

      case 1: // k=1 bout play selectionée
      {
        affichier_imag(B_play1, screen);
        affichier_imag(B_settings, screen);
        affichier_imag(B_quit, screen);
        break;
      }

      case 2: // k=2 menu settings selectionée
      {
        affichier_imag(B_play, screen);
        affichier_imag(B_settings1, screen);
        affichier_imag(B_quit, screen);
        break;
      }
      case 3: // menu quit selectionée
      {
        affichier_imag(B_play, screen);
        affichier_imag(B_settings, screen);
        affichier_imag(B_quit1, screen);
        break;
      }
      } // fin switch de k

      while (SDL_PollEvent(&event)) 
      {
        switch (event.type) {
        case SDL_QUIT:
          done = 0;
          break;

        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
          case SDLK_UP: {
            
            if (k == 0) {
              k = 3;
            }

            else if (k == 1) {
              k = 3;
            }

            else 
            {
              k--;
            }

            break;

          } 

          case SDLK_DOWN: {
            if (k == 0)
              k = 1;

            else if (k == 3) {
              k = 1;
            }

            else {
              k++;
            }

            break;
          } 

          case SDLK_RETURN: {
            if (k == 1) {
              play_click_sound();
              etat = 1;
            }

            else if (k == 2) {
              play_click_sound();
              etat = 2;
            }

            else if (k == 3) {
              play_click_sound();
              done = 0; 
            }

            break;

          } 
          } 

          break;
        } 

        case SDL_MOUSEBUTTONDOWN: {
          if (event.button.button ==
              SDL_BUTTON_LEFT) 
          {
            if (event.button.x > B_play.pos.x &&
                event.button.x < B_play.pos.x + B_play.img->w &&
                event.button.y > B_play.pos.y &&
                event.button.y < B_play.pos.y + B_play.img->h) {
              play_click_sound();
              etat = 1;
            }

            if (event.button.x > B_settings.pos.x &&
                event.button.x < B_settings.pos.x + B_settings.img->w &&
                event.button.y > B_settings.pos.y &&
                event.button.y < B_settings.pos.y + B_settings.img->h) {
              play_click_sound();
              etat = 2;
            }

            else if (event.button.x > B_quit.pos.x &&
                     event.button.x < B_quit.pos.x + B_quit.img->w &&
                     event.button.y > B_quit.pos.y &&
                     event.button.y < B_quit.pos.y + B_quit.img->h) // exit
            {
              play_click_sound();
              done = 0; // exit
            }
          } 
        } 

        case SDL_MOUSEMOTION: 
        {
          if (event.button.x > B_play.pos.x &&
              event.button.x < B_play.pos.x + B_play.img->w &&
              event.button.y > B_play.pos.y &&
              event.button.y < B_play.pos.y + B_play.img->h) // play
          {
            play_click_sound();
            k = 1;
          }

          else if (event.button.x > B_settings.pos.x &&
                   event.button.x < B_settings.pos.x + B_settings.img->w &&
                   event.button.y > B_settings.pos.y &&
                   event.button.y <
                       B_settings.pos.y + B_settings.img->h) 
          {
            play_click_sound();
            k = 2;
          }

          else if (event.button.x > B_quit.pos.x &&
                   event.button.x < B_quit.pos.x + B_quit.img->w &&
                   event.button.y > B_quit.pos.y &&
                   event.button.y < B_quit.pos.y + B_quit.img->h) // quit
          {
            play_click_sound();
            k = 3;
          }

          else
            k = 0;

        } 
        } 
      } 

      break;

    } 

    case 2: // Settings state
    {
      affichier_imag(settings, screen);
      switch (s) {
      case 0: // Initial settings state
      {
        affichier_imag(slayed, screen);
        affichier_imag(exits, screen);
        break;
      }

      case 1: // Back button selected
      {
        affichier_imag(slayed, screen);
        affichier_imag(exits1, screen);
        break;
      }
      } 

      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: 
          done = 0;
          break;

        case SDL_KEYDOWN: 
        {
          switch (event.key.keysym.sym) {
          case SDLK_ESCAPE: 
          {
            etat = 0;
            break;
          }

          case SDLK_LEFT: 
          {
            v -= 9; 
            Mix_VolumeMusic(v);
            slayed.pos.x -= 15;      
            if (slayed.pos.x <= 515) 
              slayed.pos.x = 515;
            break;
          }

          case SDLK_RIGHT: 
          {
            v += 9; 
            Mix_VolumeMusic(v);
            slayed.pos.x += 15;      
            if (slayed.pos.x >= 720) 
              slayed.pos.x = 720;
            break;
          }
          } 
        }

        case SDL_MOUSEBUTTONDOWN: 
        {
          if (event.button.button ==
              SDL_BUTTON_LEFT) 
          {
            if (event.button.x > pos_plus.x &&
                event.button.x < pos_plus.x + pos_plus.w &&
                event.button.y > pos_plus.y &&
                event.button.y < pos_plus.y + pos_plus.h) {
              v += 10; 
              Mix_VolumeMusic(v);
              slayed.pos.x += 15;      
              if (slayed.pos.x >= 720) 
                slayed.pos.x = 720;
            }

            else if (event.button.x > pos_moin.x &&
                     event.button.x < pos_moin.x + pos_moin.w &&
                     event.button.y > pos_moin.y &&
                     event.button.y < pos_moin.y + pos_moin.h) {
              v -= 10; 
              Mix_VolumeMusic(v);
              slayed.pos.x -= 15;      
              if (slayed.pos.x <= 515) 
                slayed.pos.x = 515;
            }

            else if (event.button.x > exits.pos.x &&
                     event.button.x < exits.pos.x + exits.img->w &&
                     event.button.y > exits.pos.y &&
                     event.button.y < exits.pos.y + exits.img->h) {
              play_click_sound();
              etat = 0;
            }

            break;
          }
        }

        case SDL_MOUSEMOTION: {
          if (event.button.x > exits.pos.x &&
              event.button.x < exits.pos.x + exits.img->w &&
              event.button.y > exits.pos.y &&
              event.button.y < exits.pos.y + exits.img->h) {
            s = 1;
          }

          else
            s = 0;
          // TODO: Add achievement and credits buttons
        } 
        } 
      } 
      break;

    }

    case 1: // In-game state
    {
      SDL_BlitSurface(backgame.img, NULL, screen, &backgame.pos);
      afficherPerso(p, screen);
      afficherPerso(p1, screen);
      afficherminimap(m, screen);
      SDL_PollEvent(&event);
      switch (event.type) {
      case SDL_QUIT:
        done = 0;
        break;

      case SDL_KEYDOWN: {
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE: 
        {
          etat = 0;
          break;
        }

        case SDLK_UP:
          p.up = 1;
          break;

        case SDLK_SPACE:
          ypos_jump(p, &posy);
          p.jump = 1;
          p.up = 1;
          break;

        case SDLK_DOWN:
          p.pos_background.y += 20;
          if (p.pos_background.y >= 400)
            p.pos_background.y = 400;
          break;

        case SDLK_LEFT:
          p.direction = 1;
          dep = 1;
          break;

        case SDLK_RIGHT:
          p.direction = 0;
          dep = 1;
          break;

        case SDLK_a:
          acc = 1;
          break;

        case SDLK_d:
          acc = 2;
          break;

        case SDLK_o:
          p1.up = 1;
          break;

        case SDLK_j:
          ypos_jump(p1, &posy1);
          p1.jump = 1;
          p1.up = 1;
          break;

        case SDLK_l:
          p1.pos_background.y += 20;
          if (p1.pos_background.y >= 400)
            p1.pos_background.y = 400;
          break;

        case SDLK_k:
          p1.direction = 1;
          dep1 = 1;
          break;

        case SDLK_m:
          p1.direction = 0;
          dep1 = 1;
          break;
        }

        break;
      }

      case SDL_KEYUP: {
        switch (event.key.keysym.sym) {
        case SDLK_LEFT:
          dep = 0;
          p.imag = 0;
          acc = 0;
          break;

        case SDLK_RIGHT:
          dep = 0;
          p.imag = 0;
          acc = 0;
          break;

        case SDLK_a:
          acc = 0;
          break;

        case SDLK_d:
          acc = 0;
          break;

        case SDLK_k:
          dep1 = 0;
          p1.imag = 0;
          acc1 = 0;
          break;

        case SDLK_m:
          dep1 = 0;
          p1.imag = 0;
          acc1 = 0;
          break;

        case SDLK_u:
          acc1 = 0;
          break;

        case SDLK_i:
          acc1 = 0;
          break;
        }

        break;
      }
      }

      break;
    }
    } 

    if (dep == 1) {
      deplacerPerso(&p, dt);
      animerPerso(&p);
    }
    if (acc == 1) {
      p.acceleration += 0.01;
      if (p.acceleration >= 2)
        p.acceleration = 2;
    } else if (acc == 2) {
      p.acceleration -= 0.01;
      if (p.acceleration <= -2)
        p.acceleration = -2;
    } else {
      p.acceleration -= 0.05;
      if (p.acceleration <= 0)
        p.acceleration = 0;
    }
    saut(&p, posy);

    if (dep1 == 1) {
      deplacerPerso(&p1, dt);
      animerPerso(&p1);
    }

    if (acc1 == 1) {
      p1.acceleration += 0.01;
      if (p1.acceleration >= 2)
        p1.acceleration = 2;
    }

    else if (acc1 == 2) {
      p1.acceleration -= 0.01;
      if (p1.acceleration <= -2)
        p1.acceleration = -2;
    }

    else {
      p1.acceleration -= 0.05;
      if (p1.acceleration <= 0)
        p1.acceleration = 0;
    }

    saut(&p1, posy1);
    SDL_Flip(screen);
    // Prevent division by zero; cap frame rate at ~30fps minimum
    if (dt > 0)
      SDL_Delay(300 / dt);
    else
      SDL_Delay(16); // Default ~60fps if dt is 0
    p.iscore++;
    if (p.iscore / 20 > 30) {
      p.vie = 5;
    }

    if (p.iscore / 20 > 60) {
      p.vie = 4;
    }

    if (p.iscore / 20 > 90) {
      p.vie = 3;
    }

    if (p.iscore / 20 > 120) {
      p.vie = 2;
    }

    if (p.iscore / 20 > 130) {
      p.vie = 1;
    }

    p1.iscore++;
    if (p1.iscore / 20 > 30) {
      p1.vie = 5;
    }

    if (p1.iscore / 20 > 60) {
      p1.vie = 4;
    }

    if (p1.iscore / 20 > 90) {
      p1.vie = 3;
    }

    if (p1.iscore / 20 > 120) {
      p1.vie = 2;
    }

    if (p1.iscore / 20 > 130) {
      p1.vie = 1;
    }

  } 
}

void cleanup_game() {
  int i, j;
  /*lib images
  for (i=0;i<25;i++){
      librer(imgBACK[i]);
  }
  */
  librer(B_play);
  librer(B_play1);
  librer(B_settings);
  librer(B_settings1);
  librer(B_quit);
  librer(B_quit1);

  librer(settings);
  librer(slayed);
  librer(exits);
  librer(exits1);

  // lib sounds
  Mix_FreeMusic(music);
  if (son != NULL) {
    Mix_FreeChunk(son);
  }

  // perso cleanup
  TTF_CloseFont(p.police_score);
  TTF_CloseFont(p1.police_score);
  TTF_Quit();

  // Free player 1 images
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 7; j++) {
      if (p.image[i][j] != NULL) {
        SDL_FreeSurface(p.image[i][j]);
      }
    }
  }
  // Free player 1 health bar images
  if (p.barre != NULL) {
    for (i = 0; i < 6; i++) {
      if (p.barre[i] != NULL) {
        SDL_FreeSurface(p.barre[i]);
      }
    }
    free(p.barre);
  }

  // Free player 2 images
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 7; j++) {
      if (p1.image[i][j] != NULL) {
        SDL_FreeSurface(p1.image[i][j]);
      }
    }
  }
  // Free player 2 health bar images
  if (p1.barre != NULL) {
    for (i = 0; i < 6; i++) {
      if (p1.barre[i] != NULL) {
        SDL_FreeSurface(p1.barre[i]);
      }
    }
    free(p1.barre);
  }

  // Cleanup game background image
  if (backgame.img != NULL) {
    SDL_FreeSurface(backgame.img);
  }

  // Cleanup score surfaces if they exist
  if (p.score != NULL) {
    SDL_FreeSurface(p.score);
  }
  if (p1.score != NULL) {
    SDL_FreeSurface(p1.score);
  }

  SDL_Quit(); 
}

int main() {
  if (!init_engine()) {
    return EXIT_FAILURE;
  }

  load_game_resources();
  run_game_loop();
  cleanup_game();
  return 0;
}
