#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARD_SIZE 1024
#define BUFFER_SIZE 32

typedef struct player_s {
  int x;
  int y;
} player_t;

typedef struct board_s {
  char *cur;
  char *sol;
  int x;
  int y;
} board_t;

typedef struct node_s {
  board_t *value;
  struct node_s *next;
} node_t;

typedef struct queue_s {
  node_t *head;
  node_t *tail;
} queue_t;

board_t *mkboard(const char *cur, const char *sol, int x, int y) {
  board_t *board = malloc(sizeof(board_t));
  board->cur = malloc(strlen(cur) + 2);
  board->sol = malloc(strlen(sol) + 2);
  if (board->cur)
    strcpy(board->cur, cur);
  if (board->sol)
    strcpy(board->sol, sol);
  board->x = x;
  board->y = y;
  return board;
}

node_t *mknode(board_t *board) {
  node_t *node = malloc(sizeof(node_t));
  node->value = board;
  node->next = NULL;
  return node;
}

queue_t *mkqueue() {
  queue_t *queue = malloc(sizeof(queue_t));
  queue->head = queue->tail = NULL;
  return queue;
}

void enqueue(queue_t *queue, board_t *board) {
  node_t *node = mknode(board);

  if (NULL == queue->tail)
    queue->head = queue->tail = node;
  else {
    queue->tail->next = node;
    queue->tail = node;
  }
}

node_t *dequeue(queue_t *queue) {
  if (NULL == queue->head)
    return NULL;

  node_t *tmp = queue->head;
  queue->head = queue->head->next;

  if (NULL == queue->head)
    queue->tail = NULL;

  return tmp;
}

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

int move(char **trial_board, int y_loc[], int x, int y, int dx, int dy) {
  int new_player_idx = y_loc[y + dy] + x + dx;

  if ((*trial_board)[new_player_idx] != ' ') {
    (*trial_board)[0] = '\0';
    return 0;
  } else {
    (*trial_board)[y_loc[y] + x] = ' ';
    (*trial_board)[new_player_idx] = '@';
    return 1;
  }
}

int push(char **trial_board, int y_loc[], int x, int y, int dx, int dy) {
  int new_box_idx = y_loc[y + 2 * dy] + x + 2 * dx;

  if ((*trial_board)[new_box_idx] != ' ') {
    (*trial_board)[0] = '\0';
    return 0;
  } else {
    (*trial_board)[y_loc[y] + x] = ' ';
    (*trial_board)[y_loc[y + dy] + x + dx] = '@';
    (*trial_board)[new_box_idx] = '$';
    return 1;
  }
}

int is_solved(char trial_board[], char dest_board[]) {
  for (int board_idx = 0; dest_board[board_idx] != '\0'; board_idx++)
    if ((dest_board[board_idx] == '.') != (trial_board[board_idx] == '$'))
      return 0;
  return 1;
}

#ifdef DEBUG
void test(char *dest_board, char *curr_board, int *y_loc, int y_height,
          player_t *player) {
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

  char *moved_board = malloc(strlen(curr_board) + 1);

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player->x, player->y, 0, -1);
  assert(0 ==
         strcmp(moved_board,
                "########     ##     ##  #  ##  $$ ## $$ @## #   ########"));

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player->x, player->y, 1, 0);
  assert(0 == strcmp(moved_board, ""));

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player->x, player->y, 0, 1);
  assert(0 == strcmp(moved_board, ""));

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player->x, player->y, -1, 0);
  assert(0 ==
         strcmp(moved_board,
                "########     ##     ##  #  ##  $$ ## $$  ## # @ ########"));

  free(moved_board);

  char *pushed_board = malloc(strlen(curr_board) + 1);

  strcpy(pushed_board, curr_board);
  push(&pushed_board, y_loc, player->x, player->y, 0, -1);
  assert(0 ==
         strcmp(pushed_board,
                "########     ##     ##  #  ##  $$$## $$ @## #   ########"));

  strcpy(pushed_board, curr_board);
  push(&pushed_board, y_loc, player->x, player->y, 1, 0);
  assert(0 == strcmp(pushed_board, ""));

  strcpy(pushed_board, curr_board);
  push(&pushed_board, y_loc, player->x, player->y, -1, 0);
  assert(0 ==
         strcmp(pushed_board,
                "########     ##     ##  #  ##  $$ ## $$  ## #$@ ########"));

  free(pushed_board);

  assert(1 ==
         is_solved("########     ##@    ##$ #  ##$    ##$    ##$#   ########",
                   dest_board));
  assert(0 ==
         is_solved("########     ##     ##  #$ ##  $@ ## $$  ## #   ########",
                   dest_board));

  queue_t *queue = mkqueue();
  enqueue(queue, mkboard("######@$.######", "", 1, 1));
  enqueue(queue, mkboard("###### @$######", "R", 2, 1));
  node_t *first, *second, *third;
  first = dequeue(queue);
  second = dequeue(queue);
  third = dequeue(queue);

  assert(first->next == second);
  assert(0 == strcmp(first->value->cur, "######@$.######"));
  assert(0 == strcmp(first->value->sol, ""));
  assert(first->value->x == 1);
  assert(first->value->y == 1);

  assert(second->next == NULL);
  assert(0 == strcmp(second->value->cur, "###### @$######"));
  assert(0 == strcmp(second->value->sol, "R"));
  assert(second->value->x == 2);
  assert(second->value->y == 1);
  
  assert(third == NULL);
}
#endif

int main(void) {
  char dest_board[BOARD_SIZE], curr_board[BOARD_SIZE];
  int y_loc[BOARD_SIZE], y_height;
  player_t *player;

  player = malloc(sizeof(player_t));

  read_level(&dest_board, &curr_board, &y_loc, &y_height, &player);

#ifdef DEBUG
  test(dest_board, curr_board, y_loc, y_height, player);
#endif

  free(player);

  return 0;
}
