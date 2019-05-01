#include <assert.h>
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

void move(char *moved_board[], int y_loc[], int x, int y, int dx, int dy,
          char trial_board[]) {
  int new_player_idx = y_loc[y + dy] + x + dx;

  if (trial_board[new_player_idx] != ' ')
    *moved_board = NULL;
  else {
    strcpy(*moved_board, trial_board);
    (*moved_board)[y_loc[y] + x] = ' ';
    (*moved_board)[new_player_idx] = '@';
  }
}

int main(void) {
  char dest_board[BOARD_SIZE], curr_board[BOARD_SIZE];
  int y_loc[BOARD_SIZE], y_height;
  player_t *player;

  player = malloc(sizeof(player));

  read_level(dest_board, curr_board, y_loc, &y_height, player);
#ifdef DEBUG
  char *expected_dest_board =
      "########     ##     ##. #  ##.    ##.    ##.#   ########";
  assert(strcmp(dest_board, expected_dest_board) == 0);

  char *expected_curr_board =
      "########     ##     ##  #  ##  $$ ## $$  ## #  @########";
  assert(strcmp(curr_board, expected_curr_board) == 0);

  assert(player->x == 5);
  assert(player->y == 6);

  for (int i = 0; i < y_height; i++)
    assert(y_loc[i] == i * 7);
#endif

#ifdef DEBUG
  char *moved_board;
  moved_board = malloc(BOARD_SIZE);

  move(&moved_board, y_loc, player->x, player->y, 0, -1, curr_board);
  assert(strcmp(moved_board,
                "########     ##     ##  #  ##  $$ ## $$ @## #   ########") ==
         0);

  move(&moved_board, y_loc, player->x, player->y, 1, 0, curr_board);
  assert(moved_board == NULL);
#endif

  return 0;
}
