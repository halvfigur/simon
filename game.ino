#include "game.h"


void game_new(game_t *g, unsigned int round) {
  randomSeed(millis());

  g->round = round;
  g->seq_idx = 0;
  g->seq_len = MIN_SEQ + round;

  int byte_offset = 0;
  int bit_offset = 0;

  for (int i = 0; i < g->seq_len / 4; i++) {
    char v = ((char)random(0,4) << 6) |
      ((char)random(0,4) << 4) |
      ((char)random(0,4) << 2) |
      ((char)random(0,4));
    g->seq[i] = v;
  }
}

void game_reset(game_t *g) {
  g->seq_idx = 0;
}

static int game_get_idx(game_t *g, int idx) {
  int v = g->seq[idx / 4];
  int shift = 2 * (idx % 4);
  return (int)((v & (0x3 << shift)) >> shift);
}

int game_current_color(game_t *g) {
  if (g->seq_idx == g->seq_len) {
    return -1;
  }

  return game_get_idx(g, g->seq_idx);
}

int game_has_more_colors(game_t *g) {
  return g->seq_idx < g->seq_len;
}

void game_next(game_t *g) {
  if (game_has_more_colors(g)) {
    g->seq_idx++;
  }
}

int game_next_color(game_t *g) {
  if (g->seq_idx < g->seq_len) {
    g->seq_idx++;
    return game_get_idx(g, g->seq_idx);
  }

  return -1;
}
