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
int y_height, player_x, player_y;
int *y_loc;
char *dest_board;

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

char *read_level() {
  int row = 0;
  int col = 0;
  int board_idx = 0;

  y_loc = malloc(1 * sizeof(int));
  dest_board = malloc(4);

  char *curr_board = malloc(4);
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
        int *tmp = realloc(y_loc, (row + 1) * sizeof(int));
        y_loc = tmp;
        y_loc[row] = board_idx;
      }

      char ch = buffer[buffer_idx];

      char *tmp1 = realloc(dest_board, board_idx + 1);
      dest_board = tmp1;
      dest_board[board_idx] = ch != '$' && ch != '@' ? ch : ' ';

      char *tmp2 = realloc(curr_board, board_idx + 1);
      curr_board = tmp2;
      curr_board[board_idx] = ch != '.' ? ch : ' ';

      if (ch == '@') {
        player_x = col;
        player_y = row;
      }

      col++;
      board_idx++;
    }
  }

  char *tmp1 = realloc(dest_board, board_idx + 1);
  dest_board = tmp1;
  dest_board[board_idx] = '\0';

  char *tmp2 = realloc(curr_board, board_idx + 1);
  curr_board = tmp2;
  curr_board[board_idx] = '\0';

  free(buffer);

  return curr_board;
}

int move(char **trial_board, int x, int y, int dx, int dy) {
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

int push(char **trial_board, int x, int y, int dx, int dy) {
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

int is_solved(char trial_board[]) {
  for (int board_idx = 0; dest_board[board_idx] != '\0'; board_idx++)
    if ((dest_board[board_idx] == '.') != (trial_board[board_idx] == '$'))
      return 0;
  return 1;
}

char *solve(char *curr_board, int num_threads) {
  char *path = NULL;
  int found = 0;

  char dir_labels[][2] = {{'u', 'U'}, {'r', 'R'}, {'d', 'D'}, {'l', 'L'}};
  int dirs[][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

  queue_t *queue = mkqueue();

  add_history(curr_board);
  enqueue(queue, mkboard(curr_board, "", player_x, player_y));

  // run sol
  node_t *head = dequeue(queue);
  while (head && queue->size < num_threads * 4 && !found) {
    board_t *board = head->board;

    for (int i = 0; i < 4; i++) {
      char *trial = malloc(strlen(board->state) + 1);
      char *new_sol = malloc(strlen(board->solution) + 2);

      strcpy(trial, board->state);
      strcpy(new_sol, board->solution);

      int dx = dirs[i][0];
      int dy = dirs[i][1];

      // are we standing next to a box ?
      if (trial[y_loc[board->player_y + dy] + board->player_x + dx] == '$') {
        // can we push it ?
        if (push(&trial, board->player_x, board->player_y, dx, dy)) {
          // or did we already try this one ?
          if (!history_exists(trial)) {
            put_char_at_end(new_sol, dir_labels[i][1]);

            if (is_solved(trial)) {
              path = malloc(strlen(new_sol) + 1);
              strcpy(path, new_sol);
              found = 1;
              // free(trial);
              // free(new_sol);
              // freenode(head);
              // freequeue(queue);
              // return path;
            } else {
              enqueue(queue, mkboard(trial, new_sol, board->player_x + dx,
                                     board->player_y + dy));
              add_history(trial);
            }
          }
        }
      } else if (move(&trial, board->player_x, board->player_y, dx, dy)) {
        if (!history_exists(trial)) {
          put_char_at_end(new_sol, dir_labels[i][0]);
          enqueue(queue, mkboard(trial, new_sol, board->player_x + dx,
                                 board->player_y + dy));
          add_history(trial);
        }
      }
      free(trial);
      free(new_sol);
    }
    freenode(head);
    head = dequeue(queue);
  }

  queue_t **local_queues = malloc(num_threads * sizeof(queue_t));
  for (int i = 0; i < num_threads; i++) {
    queue_t *tmp = mkqueue();
    local_queues[i] = tmp;
  }

  for (int i = 0; queue->size > 0; i++) {
    if (i == num_threads)
      i = 0;
    node_t *local_head = dequeue(queue);
    enqueue(local_queues[i], local_head->board);
  }

#pragma omp parallel num_threads(num_threads)
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
          if (push(&trial, item->player_x, item->player_y, dx, dy)) {
            // or did we already try this one ?
            if (!history_exists(trial)) {
              put_char_at_end(new_sol, dir_labels[i][1]);

#pragma omp critical
              if (is_solved(trial)) {
                path = malloc(strlen(new_sol) + 1);
                strcpy(path, new_sol);
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
        } else if (move(&trial, item->player_x, item->player_y, dx, dy)) {
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

  return path;
}

int main(int argc, char* argv[]) {
  int num_threads = strtol(argv[1], NULL, 10);

  char *curr_board = read_level();
  char *path = solve(curr_board, num_threads);

  if (path)
    printf("%s\n", path);
  else
    printf("solution not found");

  free(path);
  free(curr_board);
  free(dest_board);
  free(y_loc);
  freehistory();

  return 0;
}
