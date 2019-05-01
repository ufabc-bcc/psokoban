#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARD_SIZE 1024
#define BUFFER_SIZE 32

typedef struct {
  int x;
  int y;
} player_t;

void read_level(char dest_board[], char curr_board[], int y_loc[],
                int *y_height, player_t *player) {
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

      if (c == 0) {
        y_loc[r] = board_idx;
        *y_height = r + 1;
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

int main(void) {
  char dest_board[BOARD_SIZE], curr_board[BOARD_SIZE];
  int y_loc[BOARD_SIZE], y_height;
  player_t *player;

  player = malloc(sizeof(player));

  read_level(dest_board, curr_board, y_loc, &y_height, player);

  printf("dest_board: %s\n", dest_board);
  printf("curr_board: %s\n", curr_board);
  printf("player->x: %d, player->y: %d\n", player->x, player->y);
  for (int i = 0; i < y_height; i++)
    printf("y_loc[%d] = %d\n", i, y_loc[i]);

  return 0;
}
