#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"

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

typedef struct history_entry_s {
  char *key;
  UT_hash_handle hh;
} history_entry_t;

board_t *mkboard(const char *cur, const char *sol, int x, int y) {
  board_t *board = malloc(sizeof(board_t));
  board->cur = malloc(strlen(cur) + 1);
  board->sol = malloc(strlen(sol) + 1);
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

void freeboard(board_t *board) {
  if (NULL != board->cur)
    free(board->cur);
  if (NULL != board->sol)
    free(board->sol);
  free(board);
}

void freenode(node_t *node) {
  if (NULL != node->value)
    freeboard(node->value);
  free(node);
}

void freequeue(queue_t *queue) {
  while (NULL != queue->head) {
    node_t *tmp = dequeue(queue);
    freenode(tmp);
  }
  free(queue);
}

history_entry_t *history = NULL;

void histappend(const char *val) {
  history_entry_t *entry = malloc(sizeof(history_entry_t));
  entry->key = malloc(strlen(val) + 1);
  strcpy(entry->key, val);
  HASH_ADD_STR(history, key, entry);
}

int histexists(const char *val) {
  history_entry_t *entry = NULL;
  HASH_FIND_STR(history, val, entry);
  return NULL != entry;
}

void freehist() {
  history_entry_t *entry, *tmp;

  HASH_ITER(hh, history, entry, tmp) {
    HASH_DEL(history, entry);
    free(entry->key);
    free(entry);
  }
}

void charappend(char *dest, char value) {
  int len = strlen(dest);
  dest[len] = value;
  dest[len + 1] = '\0';
}

void read_level(char **dest_board, char **curr_board, int **y_loc,
                int *y_height, player_t **player) {
  int row = 0;
  int col = 0;
  int board_idx = 0;

  int *tmp_yloc = malloc(1 * sizeof(int));
  char *tmp_dest = malloc(4);
  char *tmp_curr = malloc(4);
  char *buffer = malloc(16);
  
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
        int *tmp = realloc(tmp_yloc, (row + 1) * sizeof(int));
        tmp_yloc = tmp;
        tmp_yloc[row] = board_idx;
      }

      char ch = buffer[buffer_idx];

      char *tmp1 = realloc(tmp_dest, board_idx + 1);
      tmp_dest = tmp1;
      tmp_dest[board_idx] = ch != '$' && ch != '@' ? ch : ' ';

      char *tmp2 = realloc(tmp_curr, board_idx + 1);
      tmp_curr = tmp2;
      tmp_curr[board_idx] = ch != '.' ? ch : ' ';

      if (ch == '@') {
        (*player)->x = col;
        (*player)->y = row;
      }

      col++;
      board_idx++;
    }
  }

  char *tmp1 = realloc(tmp_dest, board_idx + 1);
  tmp_dest = tmp1;
  tmp_dest[board_idx] = '\0';

  char *tmp2 = realloc(tmp_curr, board_idx + 1);
  tmp_curr = tmp2;
  tmp_curr[board_idx] = '\0';

  *y_height = row + 1;
  (*y_loc) = tmp_yloc;
  (*dest_board) = tmp_dest;
  (*curr_board) = tmp_curr;

  free(buffer);
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

int solve(char **path, char *dest_board, char *curr_board, int *y_loc,
          player_t *player) {
  char dir_labels[][2] = {{'u', 'U'}, {'r', 'R'}, {'d', 'D'}, {'l', 'L'}};
  int dirs[][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

  queue_t *queue = mkqueue();

  histappend(curr_board);
  enqueue(queue, mkboard(curr_board, "", player->x, player->y));

  node_t *head = dequeue(queue);

  while (head) {
    board_t *item = head->value;

    for (int i = 0; i < 4; i++) {
      char *trial = malloc(strlen(item->cur) + 1);
      char *new_sol = malloc(strlen(item->sol) + 2);

      if (trial)
        strcpy(trial, item->cur);
      if (new_sol)
        strcpy(new_sol, item->sol);

      int dx = dirs[i][0];
      int dy = dirs[i][1];

      // are we standing next to a box ?
      if (trial[y_loc[item->y + dy] + item->x + dx] == '$') {
        // can we push it ?
        if (push(&trial, y_loc, item->x, item->y, dx, dy)) {
          // or did we already try this one ?
          if (!histexists(trial)) {
            charappend(new_sol, dir_labels[i][1]);

            if (is_solved(trial, dest_board)) {
              strcpy(*path, new_sol);
              free(trial);
              free(new_sol);
              freenode(head);
              freequeue(queue);
              return 1;
            }

            enqueue(queue, mkboard(trial, new_sol, item->x + dx, item->y + dy));
            histappend(trial);
          }
        }
      } else if (move(&trial, y_loc, item->x, item->y, dx, dy)) {
        if (!histexists(trial)) {
          charappend(new_sol, dir_labels[i][0]);
          enqueue(queue, mkboard(trial, new_sol, item->x + dx, item->y + dy));
          histappend(trial);
        }
      }
      free(trial);
      free(new_sol);
    }
    freenode(head);
    head = dequeue(queue);
  }

  freequeue(queue);

  return 0;
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

  freenode(first);
  freenode(second);
  freequeue(queue);

  histappend("Hello");
  histappend("World");

  assert(histexists("Hello"));
  assert(histexists("World"));
  assert(!histexists("Hello World!"));
}
#endif

int main(void) {
  char *dest_board;
  char *curr_board;
  char *path = malloc(200);
  int *y_loc;
  player_t *player = malloc(sizeof(player_t));

  int y_height;

  read_level(&dest_board, &curr_board, &y_loc, &y_height, &player);
#ifdef DEBUG
  test(dest_board, curr_board, y_loc, y_height, player);
#endif

  int found = solve(&path, dest_board, curr_board, y_loc, player);

  if (found)
    printf("%s\n", path);
  else
    printf("Path not found");

  free(dest_board);
  free(curr_board);
  free(path);
  free(y_loc);
  free(player);
  freehist();

  return 0;
}
