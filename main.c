#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#define COLS 60
#define ROWS 30
#define SNAKE_MAX_LENGTH 1000

int main() {
  // Cacher le curseur
  printf("\e[?25l");

  // Configurer le terminal (mode non canonique, désactiver l'écho)
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  // Position du serpent
  int snake_pos_x[SNAKE_MAX_LENGTH], snake_pos_y[SNAKE_MAX_LENGTH];
  int snake_head_index = 0, snake_tail_index = 0;
  int quit = 0;

  // Boucle principale
  while (!quit) {
    // Affichage de la grille
    printf("┌");
    for (int i = 0; i < COLS; i++) printf("─");
    printf("┐\n");

    for (int j = 0; j < ROWS; j++) {
      printf("│");
      for (int i = 0; i < COLS; i++) printf(" ");
      printf("│\n");
    }

    printf("└");
    for (int i = 0; i < COLS; i++) printf("─");
    printf("┘\n");

    // Position initiale
    snake_pos_x[snake_head_index] = COLS / 2;
    snake_pos_y[snake_head_index] = ROWS / 2;
    int snake_dx = 1, snake_dy = 0;
    int apple_x = -1, apple_y;
    int game_over = 0;

    while (!quit && !game_over) {
      // Générer une pomme si besoin
      if (apple_x < 0) {
        apple_x = rand() % COLS;
        apple_y = rand() % ROWS;

        for (int i = snake_tail_index; i != snake_head_index; i = (i + 1) % SNAKE_MAX_LENGTH) {
          if (snake_pos_x[i] == apple_x && snake_pos_y[i] == apple_y) {
            apple_x = -1; // Collision avec le serpent
          }
        }

        if (apple_x >= 0) {
          printf("\e[%iB\e[%iC\e[31m❤\e[0m", apple_y + 1, apple_x + 1);
          printf("\e[%iF", apple_y + 1);
        }
      }

      // Effacer la queue du serpent
      printf("\e[%iB\e[%iC ", snake_pos_y[snake_tail_index] + 1, snake_pos_x[snake_tail_index] + 1);
      printf("\e[%iF", snake_pos_y[snake_tail_index] + 1);

      // Vérifier si le serpent mange la pomme
      if (snake_pos_x[snake_head_index] == apple_x && snake_pos_y[snake_head_index] == apple_y) {
        apple_x = -1; // Supprimer la pomme
        printf("\a"); // Bip sonore
      } else {
        snake_tail_index = (snake_tail_index + 1) % SNAKE_MAX_LENGTH;
      }

      // Avancer la tête
      int new_head_index = (snake_head_index + 1) % SNAKE_MAX_LENGTH;
      snake_pos_x[new_head_index] = (snake_pos_x[snake_head_index] + snake_dx + COLS) % COLS;
      snake_pos_y[new_head_index] = (snake_pos_y[snake_head_index] + snake_dy + ROWS) % ROWS;
      snake_head_index = new_head_index;

      // Vérifier les collisions
      for (int i = snake_tail_index; i != snake_head_index; i = (i + 1) % SNAKE_MAX_LENGTH) {
        if (snake_pos_x[i] == snake_pos_x[snake_head_index] && snake_pos_y[i] == snake_pos_y[snake_head_index]) {
          game_over = 1;
        }
      }

      // Afficher la tête du serpent
      printf("\e[%iB\e[%iC\e[32m▓\e[0m", snake_pos_y[snake_head_index] + 1, snake_pos_x[snake_head_index] + 1);
      printf("\e[%iF", snake_pos_y[snake_head_index] + 1);
      fflush(stdout);

      usleep(5 * 1000000 / 60);

      // Lire le clavier
      struct timeval tv;
      fd_set fds;
      tv.tv_sec = 0;
      tv.tv_usec = 0;

      FD_ZERO(&fds);
      FD_SET(STDIN_FILENO, &fds);
      select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
      if (FD_ISSET(STDIN_FILENO, &fds)) {
        int ch = getchar();
        if (ch == 27 || ch == 'q') quit = 1;
        else if (ch == 'h' && snake_dx != 1) { snake_dx = -1; snake_dy = 0; }
        else if (ch == 'l' && snake_dx != -1) { snake_dx = 1; snake_dy = 0; }
        else if (ch == 'j' && snake_dy != -1) { snake_dx = 0; snake_dy = 1; }
        else if (ch == 'k' && snake_dy != 1) { snake_dx = 0; snake_dy = -1; }
      }
    }

    if (!quit) {
      // Afficher le message de fin
      printf("\e[%iB\e[%iC\e[31mGame Over!\e[0m", ROWS / 2, COLS / 2 - 5);
      printf("\e[%iF", ROWS / 2);
      fflush(stdout);
      getchar();
    }
  }

  // Rétablir le curseur
  printf("\e[?25h");
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return 0;
}
