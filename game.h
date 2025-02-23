#ifndef GAME_H_
#define GAME_H_

#define MIN_SEQ 4
#define MAX_SEQ (MIN_SEQ * 8)

typedef struct {
  int round;

  char seq[MAX_SEQ];
  int seq_len;
  int seq_idx;

} game_t;

void game_new(game_t *g, unsigned int round);
void game_reset(game_t *g);
int game_current_color(game_t *g);
int game_has_more_colors(game_t *g);
void game_next(game_t *g);


#endif //GAME_H_
