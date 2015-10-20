/* File: random.c

   DESCRIPTION: pseudo-random dropping of grains

   */

#ifndef MAKEDEPEND_IGNORE
#include <print.h>

#include <stdlib.h> /* random() */
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

//#define TRACE

#include "sand.h"

long int random_radius = 0;

int random_number_in_range (int min, int max)
{
  TRACEINW("(min=%d max=%d)", min, max);
  int width = max - min + 1;
  int res = min + (random() % width);
  TRACEOUTW("%d", res);
  return res;
}

void add_grain_on_random_square (int radius, int seed)
{
  TRACEINW("(radius=%d seed=%d)", radius, seed);
  srandom(seed);
  int random_x = random_number_in_range(-radius, radius);
  int random_y = random_number_in_range(-radius, radius);
  TRACEMESS("Add 1 grain on (%d,%d)", random_x, random_y);
  add_grains_on_square(random_x, random_y, 1);
  TRACEOUT;
}
