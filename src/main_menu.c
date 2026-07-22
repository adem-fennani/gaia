#include "../include/menu.h"
#include "../include/minimap.h"
#include "../include/perso.h"
#include "../include/utils.h"
static SDL_Surface *screen = NULL;
static image background;
static image B_play, B_play1;
static image B_settings, B_settings1;
static image B_quit, B_quit1;
static image settings;
static image slayed;
static image exits, exits1;
static image backgame;
static SDL_Rect pos_plus, pos_moin;
static minimap m;
static temps t;
static perso p, p1;
static Mix_Chunk *son = NULL;
static SDL_Event event;
static int done = 1;
static int k = 0, etat = 0, s = 0, v = 64;
static int dep = 0, acc = 0, posy = 0, dep1 = 0, acc1 = 0, posy1 = 0;
static Uint32 dt = 0, t_prev = 0;
#define SCREEN_W 1150
#define SCREEN_H 650
#define LEVEL1_GROUND_Y 300
#define VICTORY_AUTO_RETURN_MS 4000
static SDL_Rect level1_obstacles[3];
static SDL_Rect level1_goal;
static Uint32 level1_start_ticks = 0;
static Uint32 victory_ticks = 0;
static int victory_score_p = 0;
static int victory_score_p1 = 0;
static int victory_time_sec = 0;
static int victory_winner = 0;
static SDL_Rect camera;
static void play_click_sound(void) {
  if (son != NULL) {
    Mix_PlayChannel(-1, son, 0);
  }
}
static void draw_text(SDL_Surface *target, TTF_Font *font, const char *msg,
                      int x, int y, SDL_Color color) {
  SDL_Surface *text_surface;
  SDL_Rect pos;
  if (target == NULL || font == NULL || msg == NULL) {
    return;
  }
  text_surface = TTF_RenderText_Solid(font, msg, color);
  if (text_surface == NULL) {
    return;
  }
  pos.x = x;
  pos.y = y;
  SDL_BlitSurface(text_surface, NULL, target, &pos);
  SDL_FreeSurface(text_surface);
}
static SDL_Rect player_hitbox(const perso *player) {
  SDL_Rect hitbox;
  hitbox.x = player->pos_background.x + 55;
  hitbox.y = player->pos_background.y + 225;
  hitbox.w = 100;
  hitbox.h = 55;
  return hitbox;
}
static int rect_overlap(SDL_Rect a, SDL_Rect b) {
  return !(a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y ||
           b.y + b.h <= a.y);
}
static void reset_level1(void) {
  p.pos_background.x = 60;
  p.pos_background.y = LEVEL1_GROUND_Y;
  p.direction = 0;
  p.imag = 0;
  p.up = 0;
  p.jump = 0;
  p.acceleration = 0;
  p.iscore = 0;
  p.vie = 5;
  p1.pos_background.x = 200;
  p1.pos_background.y = LEVEL1_GROUND_Y;
  p1.direction = 0;
  p1.imag = 0;
  p1.up = 0;
  p1.jump = 0;
  p1.acceleration = 0;
  p1.iscore = 0;
  p1.vie = 5;
  posy = LEVEL1_GROUND_Y;
  posy1 = LEVEL1_GROUND_Y;
  dep = 0;
  acc = 0;
  dep1 = 0;
  acc1 = 0;
  k = 0;
  s = 0;
  level1_start_ticks = SDL_GetTicks();
  victory_ticks = 0;
  victory_score_p = 0;
  victory_score_p1 = 0;
  victory_time_sec = 0;
  victory_winner = 0;
  t_prev = SDL_GetTicks();
}
static void clamp_and_resolve_obstacles(perso *player, int previous_x) {
  int i;
  SDL_Rect hitbox = player_hitbox(player);
  SDL_Rect previous_box = hitbox;
  previous_box.x = previous_x + 55;
  for (i = 0; i < 3; ++i) {
    if (!rect_overlap(hitbox, level1_obstacles[i])) {
      continue;
    }
    if (previous_box.x + previous_box.w <= level1_obstacles[i].x) {
      player->pos_background.x = level1_obstacles[i].x - 55 - hitbox.w;
    } else if (previous_box.x >= level1_obstacles[i].x + level1_obstacles[i].w) {
      player->pos_background.x = level1_obstacles[i].x + level1_obstacles[i].w - 55;
    } else {
      player->pos_background.x = previous_x;
    }
    hitbox = player_hitbox(player);
  }
  if (player->pos_background.x < 0) {
    player->pos_background.x = 0;
  }
  if (player->pos_background.x > LEVEL1_W - 219) {
    player->pos_background.x = LEVEL1_W - 219;
  }
}
static int level1_completed(void) {
  return rect_overlap(player_hitbox(&p), level1_goal) ||
         rect_overlap(player_hitbox(&p1), level1_goal);
}
static void capture_victory_stats(void) {
  victory_score_p = p.iscore / 20;
  victory_score_p1 = p1.iscore / 20;
  victory_time_sec = (int)((SDL_GetTicks() - level1_start_ticks) / 1000);
  victory_winner = 0;
  if (rect_overlap(player_hitbox(&p), level1_goal) &&
      rect_overlap(player_hitbox(&p1), level1_goal)) {
    victory_winner = 3;
  } else if (rect_overlap(player_hitbox(&p), level1_goal)) {
    victory_winner = 1;
  } else if (rect_overlap(player_hitbox(&p1), level1_goal)) {
    victory_winner = 2;
  }
  victory_ticks = SDL_GetTicks();
}
static void update_camera(void) {
  int mid_x = (p.pos_background.x + p1.pos_background.x) / 2;
  camera.x = mid_x - SCREEN_W / 2;
  if (camera.x < 0)
    camera.x = 0;
  if (camera.x > LEVEL1_W - SCREEN_W)
    camera.x = LEVEL1_W - SCREEN_W;
  camera.y = 0;
  camera.w = SCREEN_W;
  camera.h = SCREEN_H;
}
static void draw_level1_scene(SDL_Surface *target) {
  SDL_Rect src_bg, dst_bg;
  SDL_Rect ground;
  SDL_Rect goal_fill = level1_goal;
  SDL_Color white = {255, 255, 255, 0};
  SDL_Color yellow = {255, 255, 0, 0};
  int i;
  if (target == NULL) {
    return;
  }
  src_bg.x = camera.x / 2;
  src_bg.y = 0;
  src_bg.w = SCREEN_W;
  src_bg.h = SCREEN_H;
  if (backgame.img != NULL && src_bg.x > backgame.img->w - SCREEN_W) {
    src_bg.x = backgame.img->w - SCREEN_W;
  }
  if (src_bg.x < 0)
    src_bg.x = 0;
  dst_bg.x = 0;
  dst_bg.y = 0;
  dst_bg.w = SCREEN_W;
  dst_bg.h = SCREEN_H;
  SDL_BlitSurface(backgame.img, &src_bg, target, &dst_bg);
  ground.x = -camera.x;
  ground.y = 515;
  ground.w = LEVEL1_W;
  ground.h = SCREEN_H - ground.y;
  SDL_FillRect(target, &ground, SDL_MapRGB(target->format, 52, 36, 18));
  for (i = 0; i < 3; ++i) {
    SDL_Rect obs = level1_obstacles[i];
    obs.x -= camera.x;
    SDL_FillRect(target, &obs, SDL_MapRGB(target->format, 175, 105, 45));
  }
  goal_fill.x -= camera.x;
  SDL_FillRect(target, &goal_fill, SDL_MapRGB(target->format, 190, 170, 40));
  draw_text(target, p.police_score,
            "Level 1: reach the exit beacon to finish the mission", 20, 10,
            yellow);
  draw_text(target, p.police_score, "Player 1: arrows + space", 20, 34,
            white);
  draw_text(target, p.police_score, "Player 2: O/J/K/M", 20, 58, white);
}
static void draw_victory_scene(SDL_Surface *target) {
  SDL_Rect panel;
  SDL_Color title = {255, 240, 90, 0};
  SDL_Color white = {255, 255, 255, 0};
  char line[128];
  if (target == NULL) {
    return;
  }
  panel.x = 0;
  panel.y = 0;
  panel.w = SCREEN_W;
  panel.h = SCREEN_H;
  SDL_FillRect(target, &panel, SDL_MapRGB(target->format, 18, 20, 42));
  draw_text(target, p.police_score, "MISSION COMPLETE", 420, 120, title);
  if (victory_winner == 3) {
    draw_text(target, p.police_score,
              "Team victory: both players reached the exit beacon.", 280, 205,
              white);
  } else if (victory_winner == 1) {
    draw_text(target, p.police_score,
              "Player 1 reached the exit beacon.", 360, 205, white);
  } else if (victory_winner == 2) {
    draw_text(target, p.police_score,
              "Player 2 reached the exit beacon.", 360, 205, white);
  } else {
    draw_text(target, p.police_score, "Mission complete.", 450, 205, white);
  }
  snprintf(line, sizeof(line), "Player 1 score: %d", victory_score_p);
  draw_text(target, p.police_score, line, 395, 280, white);
  snprintf(line, sizeof(line), "Player 2 score: %d", victory_score_p1);
  draw_text(target, p.police_score, line, 395, 320, white);
  snprintf(line, sizeof(line), "Completion time: %d seconds", victory_time_sec);
  draw_text(target, p.police_score, line, 380, 360, white);
  draw_text(target, p.police_score,
            "Press ENTER, ESC, SPACE, or click to return to the menu", 220,
            440, title);
}
bool init_engine(void) {
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
  memset(&p, 0, sizeof(perso));
  memset(&p1, 0, sizeof(perso));
  memset(&backgame, 0, sizeof(image));
  memset(&m, 0, sizeof(minimap));
  memset(&t, 0, sizeof(temps));
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("FAIL SDL INIT %s\n", SDL_GetError());
    return false;
  }
  putenv("SDL_VIDEO_CENTERED=1");
  screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32,
                            SDL_HWSURFACE | SDL_DOUBLEBUF);
  SDL_WM_SetCaption("GAIA REPRESENT", NULL);
  IMG_Init(IMG_INIT_PNG);
  if (screen == NULL) {
    printf("Can't set video mode: %s\n", SDL_GetError());
    return false;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
    printf("FAIL AUDIO %s\n", Mix_GetError());
  }
  TTF_Init();
  return true;
}
void load_game_resources(void) {
  // Load the real Level 1 map art so the mission scene is visible.
  init_background(&backgame, "assets/img/Niv1.png");
  backgame.pos.x = 0;
  backgame.pos.y = 0;
  backgame.pos.w = SCREEN_W;
  backgame.pos.h = SCREEN_H;
  son = load_wav_safe("assets/audio/mouseclick.wav");
  if (son != NULL) {
    Mix_VolumeChunk(son, MIX_MAX_VOLUME / 3);
  }
  init_background(&background, "assets/img/background.jpg");
  init_bouton(&B_play, "assets/img/B_play.png", &B_settings,
              "assets/img/B_settings.png", &B_quit, "assets/img/B_quit.png");
  init_bouton(&B_play1, "assets/img/B_play1.png", &B_settings1,
              "assets/img/B_settings1.png", &B_quit1, "assets/img/B_quit1.png");
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
  initPerso(&p);
  initPerso1(&p1);
  initmap(&m);
  init_temps(&t);
  level1_obstacles[0].x = 350;
  level1_obstacles[0].y = 525;
  level1_obstacles[0].w = 80;
  level1_obstacles[0].h = 125;
  level1_obstacles[1].x = 850;
  level1_obstacles[1].y = 520;
  level1_obstacles[1].w = 90;
  level1_obstacles[1].h = 130;
  level1_obstacles[2].x = 1350;
  level1_obstacles[2].y = 525;
  level1_obstacles[2].w = 80;
  level1_obstacles[2].h = 125;
  level1_goal.x = 1900;
  level1_goal.y = 360;
  level1_goal.w = 125;
  level1_goal.h = 180;
  reset_level1();
  done = 1;
  etat = 0;
  k = 0;
  s = 0;
  v = 64;
}
void run_game_loop(void) {
  while (done) {
    int prev_x_p = p.pos_background.x;
    int prev_x_p1 = p1.pos_background.x;
    dt = SDL_GetTicks() - t_prev;
    t_prev = SDL_GetTicks();
    switch (etat) {
    case 0:
      affichier_imag(background, screen);
      switch (k) {
      case 1:
        affichier_imag(B_play1, screen);
        affichier_imag(B_settings, screen);
        affichier_imag(B_quit, screen);
        break;
      case 2:
        affichier_imag(B_play, screen);
        affichier_imag(B_settings1, screen);
        affichier_imag(B_quit, screen);
        break;
      case 3:
        affichier_imag(B_play, screen);
        affichier_imag(B_settings, screen);
        affichier_imag(B_quit1, screen);
        break;
      default:
        affichier_imag(B_play, screen);
        affichier_imag(B_settings, screen);
        affichier_imag(B_quit, screen);
        break;
      }
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          done = 0;
        } else if (event.type == SDL_KEYDOWN) {
          switch (event.key.keysym.sym) {
          case SDLK_UP:
            k = (k <= 1) ? 3 : k - 1;
            break;
          case SDLK_DOWN:
            k = (k >= 3) ? 1 : k + 1;
            break;
          case SDLK_RETURN:
            if (k == 1) {
              play_click_sound();
              reset_level1();
              etat = 1;
            } else if (k == 2) {
              play_click_sound();
              etat = 2;
            } else if (k == 3) {
              play_click_sound();
              done = 0;
            }
            break;
          default:
            break;
          }
        } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                   event.button.button == SDL_BUTTON_LEFT) {
          if (event.button.x > B_play.pos.x &&
              event.button.x < B_play.pos.x + B_play.img->w &&
              event.button.y > B_play.pos.y &&
              event.button.y < B_play.pos.y + B_play.img->h) {
            play_click_sound();
            reset_level1();
            etat = 1;
          } else if (event.button.x > B_settings.pos.x &&
                     event.button.x < B_settings.pos.x + B_settings.img->w &&
                     event.button.y > B_settings.pos.y &&
                     event.button.y < B_settings.pos.y + B_settings.img->h) {
            play_click_sound();
            etat = 2;
          } else if (event.button.x > B_quit.pos.x &&
                     event.button.x < B_quit.pos.x + B_quit.img->w &&
                     event.button.y > B_quit.pos.y &&
                     event.button.y < B_quit.pos.y + B_quit.img->h) {
            play_click_sound();
            done = 0;
          }
        } else if (event.type == SDL_MOUSEMOTION) {
          if (event.button.x > B_play.pos.x &&
              event.button.x < B_play.pos.x + B_play.img->w &&
              event.button.y > B_play.pos.y &&
              event.button.y < B_play.pos.y + B_play.img->h) {
            k = 1;
          } else if (event.button.x > B_settings.pos.x &&
                     event.button.x < B_settings.pos.x + B_settings.img->w &&
                     event.button.y > B_settings.pos.y &&
                     event.button.y < B_settings.pos.y + B_settings.img->h) {
            k = 2;
          } else if (event.button.x > B_quit.pos.x &&
                     event.button.x < B_quit.pos.x + B_quit.img->w &&
                     event.button.y > B_quit.pos.y &&
                     event.button.y < B_quit.pos.y + B_quit.img->h) {
            k = 3;
          } else {
            k = 0;
          }
        }
      }
      break;
    case 1:
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          done = 0;
        } else if (event.type == SDL_KEYDOWN) {
          switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            etat = 0;
            break;
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
            if (p.pos_background.y >= LEVEL1_GROUND_Y)
              p.pos_background.y = LEVEL1_GROUND_Y;
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
            if (p1.pos_background.y >= LEVEL1_GROUND_Y)
              p1.pos_background.y = LEVEL1_GROUND_Y;
            break;
          case SDLK_k:
            p1.direction = 1;
            dep1 = 1;
            break;
          case SDLK_m:
            p1.direction = 0;
            dep1 = 1;
            break;
          default:
            break;
          }
        } else if (event.type == SDL_KEYUP) {
          switch (event.key.keysym.sym) {
          case SDLK_LEFT:
          case SDLK_RIGHT:
            dep = 0;
            p.imag = 0;
            acc = 0;
            break;
          case SDLK_a:
          case SDLK_d:
            acc = 0;
            break;
          case SDLK_k:
          case SDLK_m:
            dep1 = 0;
            p1.imag = 0;
            acc1 = 0;
            break;
          default:
            break;
          }
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
      } else if (acc1 == 2) {
        p1.acceleration -= 0.01;
        if (p1.acceleration <= -2)
          p1.acceleration = -2;
      } else {
        p1.acceleration -= 0.05;
        if (p1.acceleration <= 0)
          p1.acceleration = 0;
      }
      saut(&p1, posy1);
      clamp_and_resolve_obstacles(&p, prev_x_p);
      clamp_and_resolve_obstacles(&p1, prev_x_p1);
      update_camera();
      draw_level1_scene(screen);
      afficherPerso(p, screen, camera.x);
      afficherPerso(p1, screen, camera.x);
      afficherminimap(m, screen);
      p.iscore++;
      p1.iscore++;
      if (level1_completed()) {
        capture_victory_stats();
        etat = 3;
      }
      break;
    case 2:
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          done = 0;
        } else if (event.type == SDL_KEYDOWN) {
          switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            etat = 0;
            break;
          case SDLK_LEFT:
            v -= 9;
            Mix_VolumeMusic(v);
            slayed.pos.x -= 15;
            if (slayed.pos.x <= 515)
              slayed.pos.x = 515;
            break;
          case SDLK_RIGHT:
            v += 9;
            Mix_VolumeMusic(v);
            slayed.pos.x += 15;
            if (slayed.pos.x >= 720)
              slayed.pos.x = 720;
            break;
          default:
            break;
          }
        } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                   event.button.button == SDL_BUTTON_LEFT) {
          if (event.button.x > pos_plus.x &&
              event.button.x < pos_plus.x + pos_plus.w &&
              event.button.y > pos_plus.y &&
              event.button.y < pos_plus.y + pos_plus.h) {
            v += 10;
            Mix_VolumeMusic(v);
            slayed.pos.x += 15;
            if (slayed.pos.x >= 720)
              slayed.pos.x = 720;
          } else if (event.button.x > pos_moin.x &&
                     event.button.x < pos_moin.x + pos_moin.w &&
                     event.button.y > pos_moin.y &&
                     event.button.y < pos_moin.y + pos_moin.h) {
            v -= 10;
            Mix_VolumeMusic(v);
            slayed.pos.x -= 15;
            if (slayed.pos.x <= 515)
              slayed.pos.x = 515;
          } else if (event.button.x > exits.pos.x &&
                     event.button.x < exits.pos.x + exits.img->w &&
                     event.button.y > exits.pos.y &&
                     event.button.y < exits.pos.y + exits.img->h) {
            play_click_sound();
            etat = 0;
          }
        } else if (event.type == SDL_MOUSEMOTION) {
          if (event.button.x > exits.pos.x &&
              event.button.x < exits.pos.x + exits.img->w &&
              event.button.y > exits.pos.y &&
              event.button.y < exits.pos.y + exits.img->h) {
            s = 1;
          } else {
            s = 0;
          }
        }
      }
      affichier_imag(settings, screen);
      if (s == 0) {
        affichier_imag(slayed, screen);
        affichier_imag(exits, screen);
      } else {
        affichier_imag(slayed, screen);
        affichier_imag(exits1, screen);
      }
      break;
    case 3:
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          done = 0;
        } else if (event.type == SDL_KEYDOWN) {
          if (event.key.keysym.sym == SDLK_RETURN ||
              event.key.keysym.sym == SDLK_ESCAPE ||
              event.key.keysym.sym == SDLK_SPACE) {
            reset_level1();
            etat = 0;
          }
        } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                   event.button.button == SDL_BUTTON_LEFT) {
          reset_level1();
          etat = 0;
        }
      }
      draw_victory_scene(screen);
      if ((Uint32)(SDL_GetTicks() - victory_ticks) >= VICTORY_AUTO_RETURN_MS) {
        reset_level1();
        etat = 0;
      }
      break;
    default:
      etat = 0;
      break;
    }
    SDL_Flip(screen);
    if (dt > 0) {
      SDL_Delay(300 / dt);
    } else {
      SDL_Delay(16);
    }
  }
}
void cleanup_game(void) {
  int i, j;
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
  if (son != NULL) {
    Mix_FreeChunk(son);
  }
  if (p.police_score != NULL) {
    TTF_CloseFont(p.police_score);
  }
  if (p1.police_score != NULL) {
    TTF_CloseFont(p1.police_score);
  }
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 7; j++) {
      if (p.image[i][j] != NULL) {
        SDL_FreeSurface(p.image[i][j]);
      }
      if (p1.image[i][j] != NULL) {
        SDL_FreeSurface(p1.image[i][j]);
      }
    }
  }
  if (p.barre != NULL) {
    for (i = 0; i < 6; i++) {
      if (p.barre[i] != NULL) {
        SDL_FreeSurface(p.barre[i]);
      }
    }
    free(p.barre);
  }
  if (p1.barre != NULL) {
    for (i = 0; i < 6; i++) {
      if (p1.barre[i] != NULL) {
        SDL_FreeSurface(p1.barre[i]);
      }
    }
    free(p1.barre);
  }
  if (backgame.img != NULL) {
    SDL_FreeSurface(backgame.img);
  }
  if (p.score != NULL) {
    SDL_FreeSurface(p.score);
  }
  if (p1.score != NULL) {
    SDL_FreeSurface(p1.score);
  }
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}
int main(void) {
  if (!init_engine()) {
    return EXIT_FAILURE;
  }
  load_game_resources();
  run_game_loop();
  cleanup_game();
  return EXIT_SUCCESS;
}
