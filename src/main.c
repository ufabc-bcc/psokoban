#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 1024
#define BUFFER_SIZE 32

struct player_s {
  int x;
  int y;
};

void read_level(char dest_board[], char curr_board[], struct player_s *player);

int main(void) {
  char dest_board[BOARD_SIZE], curr_board[BOARD_SIZE];
  struct player_s *player;

  player = malloc(sizeof(struct player_s));

  read_level(dest_board, curr_board, player);

  printf("dest_board: %s\n", dest_board);
  printf("curr_board: %s\n", curr_board);
  printf("player->x: %d, player->y: %d\n", player->x, player->y);

  return 0;
}

void read_level(char dest_board[], char curr_board[], struct player_s *player) {
  int board_idx = 0;
  int r = 0;
  int c = 0;

  char buffer[BUFFER_SIZE];
  while (fgets(buffer, sizeof(buffer), stdin)) {
    for (int buffer_idx = 0; buffer_idx < sizeof(buffer); buffer_idx++) {
      if (buffer[buffer_idx] == '\0')
        break;
      else if (buffer[buffer_idx] == '\n') {
        c = 0;
        r++;
        break;
      }

      char ch = buffer[buffer_idx];
      dest_board[board_idx] = ch != '$' && ch != '@' ? ch : ' ';
      curr_board[board_idx] = ch != '.' ? ch : ' ';

      if (ch == '@') {
        player->x = c;
        player->y = r;
      }

      c++;
      board_idx++;
    }
  }

  dest_board[board_idx] = '\0';
  curr_board[board_idx] = '\0';
}
