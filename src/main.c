#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"
#include <omp.h>

typedef struct board_s {
  char *state;
  char *solution;
  int player_x;
  int player_y;
} board_t;

typedef struct node_s {
  board_t *board;
  struct node_s *next;
} node_t;

typedef struct queue_s {
  node_t *head;
  node_t *tail;
  int size;
} queue_t;

typedef struct history_s {
  char *key;
  UT_hash_handle hh;
} history_t;

history_t *global_history = NULL;

board_t *mkboard(const char *state, const char *solution, int player_x,
                 int player_y) {
  board_t *board = malloc(sizeof(board_t));
  board->state = malloc(strlen(state) + 1);
  board->solution = malloc(strlen(solution) + 1);

  if (board->state)
    strcpy(board->state, state);
  if (board->solution)
    strcpy(board->solution, solution);
  board->player_x = player_x;
  board->player_y = player_y;

  return board;
}

node_t *mknode(board_t *board) {
  node_t *node = malloc(sizeof(node_t));

  node->board = board;
  node->next = NULL;

  return node;
}

queue_t *mkqueue() {
  queue_t *queue = malloc(sizeof(queue_t));

  queue->head = queue->tail = NULL;
  queue->size = 0;

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
  queue->size++;
}

node_t *dequeue(queue_t *queue) {
  if (NULL == queue->head)
    return NULL;
  node_t *tmp = queue->head;
  queue->head = queue->head->next;
  if (NULL == queue->head)
    queue->tail = NULL;
  queue->size--;

  return tmp;
}

void add_history(const char *value) {
  history_t *entry = malloc(sizeof(history_t));
  entry->key = malloc(strlen(value) + 1);

  strcpy(entry->key, value);
  HASH_ADD_STR(global_history, key, entry);
}

int history_exists(const char *value) {
  history_t *entry = NULL;

  HASH_FIND_STR(global_history, value, entry);

  return NULL != entry;
}

void put_char_at_end(char *dest, char value) {
  int len = strlen(dest);
  dest[len] = value;
  dest[len + 1] = '\0';
}

void freeboard(board_t *board) {
  if (NULL != board->state)
    free(board->state);
  if (NULL != board->solution)
    free(board->solution);
  free(board);
}

void freenode(node_t *node) {
  if (NULL != node->board)
    freeboard(node->board);
  free(node);
}

void freequeue(queue_t *queue) {
  while (NULL != queue->head) {
    node_t *tmp = dequeue(queue);
    freenode(tmp);
  }
  free(queue);
}

void freehistory() {
  history_t *entry, *tmp;

  HASH_ITER(hh, global_history, entry, tmp) {
    HASH_DEL(global_history, entry);
    free(entry->key);
    free(entry);
  }
}

void read_level(char **dest_board, char **curr_board, int **y_loc,
                int *y_height, int *player_x, int *player_y) {
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
        *player_x = col;
        *player_y = row;
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

  *y_height = row;
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
          int player_x, int player_y) {
  char dir_labels[][2] = {{'u', 'U'}, {'r', 'R'}, {'d', 'D'}, {'l', 'L'}};
  int dirs[][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

  queue_t *queue = mkqueue();

  add_history(curr_board);
  enqueue(queue, mkboard(curr_board, "", player_x, player_y));

  node_t *head = dequeue(queue);

#ifndef THREADS
#define THREADS 5
#endif

  // run sol
  while (head && queue->size < THREADS * 4) {
    board_t *item = head->board;

    for (int i = 0; i < 4; i++) {
      char *trial = malloc(strlen(item->state) + 1);
      char *new_sol = malloc(strlen(item->solution) + 2);

      if (trial)
        strcpy(trial, item->state);
      if (new_sol)
        strcpy(new_sol, item->solution);

      int dx = dirs[i][0];
      int dy = dirs[i][1];

      // are we standing next to a box ?
      if (trial[y_loc[item->player_y + dy] + item->player_x + dx] == '$') {
        // can we push it ?
        if (push(&trial, y_loc, item->player_x, item->player_y, dx, dy)) {
          // or did we already try this one ?
          if (!history_exists(trial)) {
            put_char_at_end(new_sol, dir_labels[i][1]);

            if (is_solved(trial, dest_board)) {
              strcpy(*path, new_sol);
              free(trial);
              free(new_sol);
              freenode(head);
              freequeue(queue);
              return 1;
            }

            enqueue(queue, mkboard(trial, new_sol, item->player_x + dx,
                                   item->player_y + dy));
            add_history(trial);
          }
        }
      } else if (move(&trial, y_loc, item->player_x, item->player_y, dx, dy)) {
        if (!history_exists(trial)) {
          put_char_at_end(new_sol, dir_labels[i][0]);
          enqueue(queue, mkboard(trial, new_sol, item->player_x + dx,
                                 item->player_y + dy));
          add_history(trial);
        }
      }
      free(trial);
      free(new_sol);
    }
    freenode(head);
    head = dequeue(queue);
  }

  queue_t *local_queues[THREADS];
  for (int i = 0; i < THREADS; i++) {
    queue_t *tmp = mkqueue();
    local_queues[i] = tmp;
  }

  for (int i = 0; queue->size > 0; i++) {
    if (i == THREADS)
      i = 0;
    node_t *local_head = dequeue(queue);
    enqueue(local_queues[i], local_head->board);
  }

  int found = 0;

#pragma omp parallel num_threads(THREADS)
  {
    int k = omp_get_thread_num();

    node_t *local_head = dequeue(local_queues[k]);

    // run sol
    while (local_head && !found) {
      board_t *item = local_head->board;

      for (int i = 0; i < 4; i++) {
        char *trial = malloc(strlen(item->state) + 1);
        char *new_sol = malloc(strlen(item->solution) + 2);

        if (trial)
          strcpy(trial, item->state);
        if (new_sol)
          strcpy(new_sol, item->solution);

        int dx = dirs[i][0];
        int dy = dirs[i][1];

        // are we standing next to a box ?
        if (trial[y_loc[item->player_y + dy] + item->player_x + dx] == '$') {
          // can we push it ?
          if (push(&trial, y_loc, item->player_x, item->player_y, dx, dy)) {
            // or did we already try this one ?
            if (!history_exists(trial)) {
              put_char_at_end(new_sol, dir_labels[i][1]);

#pragma omp critical
              if (is_solved(trial, dest_board)) {
                strcpy(*path, new_sol);
                found = 1;
                // free(trial);
                // free(new_sol);
                // freenode(local_head);
                // freequeue(local_queues[k]);

              } else {
                enqueue(local_queues[k],
                        mkboard(trial, new_sol, item->player_x + dx,
                                item->player_y + dy));
                add_history(trial);
              }
            }
          }
        } else if (move(&trial, y_loc, item->player_x, item->player_y, dx,
                        dy)) {
          if (!history_exists(trial)) {
            put_char_at_end(new_sol, dir_labels[i][0]);
            enqueue((local_queues[k]),
                    mkboard(trial, new_sol, item->player_x + dx,
                            item->player_y + dy));
#pragma omp critical
            add_history(trial);
          }
        }
        free(trial);
        free(new_sol);
      }
      freenode(local_head);
      local_head = dequeue((local_queues[k]));
    }

    freequeue(local_queues[k]);
  }

  freequeue(queue);

  return found;
}

#ifdef DEBUG
void test(char *dest_board, char *curr_board, int *y_loc, int y_height,
          int player_x, int player_y) {
  assert(0 ==
         strcmp(dest_board,
                "########     ##     ##. #  ##.    ##.    ##.#   ########"));
  assert(0 ==
         strcmp(curr_board,
                "########     ##     ##  #  ##  $$ ## $$  ## #  @########"));

  assert(player_x == 5);
  assert(player_y == 6);

  for (int row = 0; row < y_height; row++)
    assert(y_loc[row] == row * 7);

  char *moved_board = malloc(strlen(curr_board) + 1);

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player_x, player_y, 0, -1);
  assert(0 ==
         strcmp(moved_board,
                "########     ##     ##  #  ##  $$ ## $$ @## #   ########"));

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player_x, player_y, 1, 0);
  assert(0 == strcmp(moved_board, ""));

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player_x, player_y, 0, 1);
  assert(0 == strcmp(moved_board, ""));

  strcpy(moved_board, curr_board);
  move(&moved_board, y_loc, player_x, player_y, -1, 0);
  assert(0 ==
         strcmp(moved_board,
                "########     ##     ##  #  ##  $$ ## $$  ## # @ ########"));

  free(moved_board);

  char *pushed_board = malloc(strlen(curr_board) + 1);

  strcpy(pushed_board, curr_board);
  push(&pushed_board, y_loc, player_x, player_y, 0, -1);
  assert(0 ==
         strcmp(pushed_board,
                "########     ##     ##  #  ##  $$$## $$ @## #   ########"));

  strcpy(pushed_board, curr_board);
  push(&pushed_board, y_loc, player_x, player_y, 1, 0);
  assert(0 == strcmp(pushed_board, ""));

  strcpy(pushed_board, curr_board);
  push(&pushed_board, y_loc, player_x, player_y, -1, 0);
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
  assert(0 == strcmp(first->board->state, "######@$.######"));
  assert(0 == strcmp(first->board->solution, ""));
  assert(first->board->player_x == 1);
  assert(first->board->player_y == 1);

  assert(second->next == NULL);
  assert(0 == strcmp(second->board->state, "###### @$######"));
  assert(0 == strcmp(second->board->solution, "R"));
  assert(second->board->player_x == 2);
  assert(second->board->player_y == 1);

  assert(third == NULL);

  freenode(first);
  freenode(second);
  freequeue(queue);

  add_history("Hello");
  add_history("World");

  assert(history_exists("Hello"));
  assert(history_exists("World"));
  assert(!history_exists("Hello World!"));

  freehistory();
}
#endif

int main(void) {
  char *dest_board;
  char *curr_board;
  char *path = malloc(1024);
  int *y_loc;

  int y_height;
  int player_x;
  int player_y;

  read_level(&dest_board, &curr_board, &y_loc, &y_height, &player_x, &player_y);
#ifdef DEBUG
  test(dest_board, curr_board, y_loc, y_height, player_x, player_y);
#endif

  int found = solve(&path, dest_board, curr_board, y_loc, player_x, player_y);

  if (found)
    printf("%s\n", path);
  else
    printf("Path not found");

  free(dest_board);
  free(curr_board);
  free(path);
  free(y_loc);
  freehistory();

  return 0;
}
