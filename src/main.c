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

void read_level(char (*dest_board)[], char (*curr_board)[], int (*y_loc)[],
                int *y_height, player_t **player) {
  int row = 0;
  int col = 0;
  int board_idx = 0;

  char buffer[BUFFER_SIZE];
  while (fgets(buffer, sizeof(buffer), stdin)) {
    for (int buffer_idx = 0; buffer_idx < sizeof(buffer); buffer_idx++) {
      if (buffer[buffer_idx] == '\0')
        break;
      else if (buffer[buffer_idx] == '\n') {
        col = 0;
        row++;
        break;
      }

      if (col == 0) {
        (*y_loc)[row] = board_idx;
        *y_height = row + 1;
      }

      char ch = buffer[buffer_idx];
      (*dest_board)[board_idx] = ch != '$' && ch != '@' ? ch : ' ';
      (*curr_board)[board_idx] = ch != '.' ? ch : ' ';

      if (ch == '@') {
        (*player)->x = col;
        (*player)->y = row;
      }

      col++;
      board_idx++;
    }
  }

  (*dest_board)[board_idx] = '\0';
  (*curr_board)[board_idx] = '\0';
}

void move(char (*moved_board)[], int y_loc[], int x, int y, int dx, int dy,
          char trial_board[]) {
  int new_player_idx = y_loc[y + dy] + x + dx;

  if (trial_board[new_player_idx] != ' ')
    (*moved_board)[0] = '\0';
  else {
    strcpy(*moved_board, trial_board);
    (*moved_board)[y_loc[y] + x] = ' ';
    (*moved_board)[new_player_idx] = '@';
  }
}

void push(char (*pushed_board)[], int y_loc[], int x, int y, int dx, int dy,
          char trial_board[]) {
  int new_box_idx = y_loc[y + 2 * dy] + x + 2 * dx;

  if (trial_board[new_box_idx] != ' ')
    (*pushed_board)[0] = '\0';
  else {
    strcpy(*pushed_board, trial_board);
    (*pushed_board)[y_loc[y] + x] = ' ';
    (*pushed_board)[y_loc[y + dy] + x + dx] = '@';
    (*pushed_board)[new_box_idx] = '$';
  }
}

int is_solved(char trial_board[], char dest_board[]) {
  for (int board_idx = 0; dest_board[board_idx] != '\0'; board_idx++)
    if ((dest_board[board_idx] == '.') != (trial_board[board_idx] == '$'))
      return 0;
  return 1;
}

int main(void) {
  char dest_board[BOARD_SIZE], curr_board[BOARD_SIZE];
  int y_loc[BOARD_SIZE], y_height;
  player_t *player;

  player = malloc(sizeof(player_t));

  read_level(&dest_board, &curr_board, &y_loc, &y_height, &player);

#ifdef DEBUG
  assert(0 ==
         strcmp(dest_board,
                "########     ##     ##. #  ##.    ##.    ##.#   ########"));
  assert(0 ==
         strcmp(curr_board,
                "########     ##     ##  #  ##  $$ ## $$  ## #  @########"));

  assert(player->x == 5);
  assert(player->y == 6);

  for (int row = 0; row < y_height; row++)
    assert(y_loc[row] == row * 7);
#endif

#ifdef DEBUG
  char moved_board[BOARD_SIZE];

  move(&moved_board, y_loc, player->x, player->y, 0, -1, curr_board);
  assert(0 ==
         strcmp(moved_board,
                "########     ##     ##  #  ##  $$ ## $$ @## #   ########"));

  move(&moved_board, y_loc, player->x, player->y, 1, 0, curr_board);
  assert(0 == strcmp(moved_board, ""));

  move(&moved_board, y_loc, player->x, player->y, 0, 1, curr_board);
  assert(0 == strcmp(moved_board, ""));

  move(&moved_board, y_loc, player->x, player->y, -1, 0, curr_board);
  assert(0 ==
         strcmp(moved_board,
                "########     ##     ##  #  ##  $$ ## $$  ## # @ ########"));
#endif

#ifdef DEBUG
  char pushed_board[BOARD_SIZE];

  push(&pushed_board, y_loc, player->x, player->y, 0, -1, curr_board);
  assert(0 ==
         strcmp(pushed_board,
                "########     ##     ##  #  ##  $$$## $$ @## #   ########"));

  push(&pushed_board, y_loc, player->x, player->y, 1, 0, curr_board);
  assert(0 == strcmp(pushed_board, ""));

  push(&pushed_board, y_loc, player->x, player->y, -1, 0, curr_board);
  assert(0 ==
         strcmp(pushed_board,
                "########     ##     ##  #  ##  $$ ## $$  ## #$@ ########"));
#endif

#ifdef DEBUG
  assert(is_solved("########     ##@    ##$ #  ##$    ##$    ##$#   ########",
                   dest_board));
  assert(!is_solved("########     ##     ##  #$ ##  $@ ## $$  ## #   ########",
                    dest_board));
#endif

  free(player);

  return 0;
}
