/* File: board.c

   DESCRIPTION: manages the evolution of the board: collapses, normalization, couting grains, etc.

   */
#ifndef MAKEDEPEND_IGNORE
#include <print.h> 		/* fail(); from libphs */

#include <errno.h>
#include <stdlib.h> 		/* alloc, malloc & free */
#include <alloca.h> 		/* alloca */
#include <sys/utsname.h>	/* uname() */
#include <unistd.h>             /* usleep() */
#include <curses.h>
#include <stdio.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "trace.h"
#include "sand.h"

/* THE PHYSICAL BOARD IS A MEMORY BLOCK THAT HOLDS A PORTION OF THE INFINITE 2DIM SPACE */
/* local functions: */
extern void realloc_board (int newxdim, int newydim, int newxdepl, int newydepl);
extern void incr_square (int x, int y, int delta);
extern void enlarge_envelop_around (int x, int y);
extern void handle_terminated_board ( void );
extern void dump_waitinglist (FILE * stream, bool statstoo);
extern void dump_binary_board (FILE * stream, int * tab, int xd, int yd);

/* local vars: */
static bool board_inited = false;
/* We store a physical array lboard[lsize = lxdim x lydim] of ints's.*/
static int * lboard = NULL;	/* not yet inited */
static int   lxdim = -1;	/* not yet inited */
static int   lydim = -1;	/* not yet inited */
static int   lsize = -1;	/* not yet inited */

/* LBOARD stores a part [x0..x1) x [y0..y1) of B[ℤxℤ], the logical space.
   <xdepl,ydepl> are the coords in lboard of the logical (0,0)
   so that x0 == -xdepl && y0 == -ydepl
   entailing x1 = lxdim - xdepl && y1 == lydim - ydepl */
static int xdepl = -1;		/* not yet inited */
static int ydepl = -1;		/* not yet inited */
static int x0, y0, x1, y1;
/* A pointer CBOARD points to virtual origin, &lboard[xdepl,ydepl] */
static int * cboard = NULL;	/* not yet inited */

void realloc_board (int newxdim, int newydim, int newxdepl, int newydepl)
{
//  int newxdepl = newdim/2;
//  int newydepl = newdim/2;
//  int newxdim = newdim;
//  int newydim = newdim;
  int newsize = newxdim * newydim;
  int i, j, *p, *q;
  TRACEINW("(xdim=%d,ydim=%d,xdepl=%d,ydepl=%d)",
	   newxdim, newydim, newxdepl, newydepl);
  if ((xdim > newxdim) || (ydim > newydim)) {
    fprintf (stderr, "ERROR: Can't realloc to smaller board. Asking for %dx%d, was %dx%d\n", newxdim, newydim, xdim, ydim);
    fail();
  }
  int * new_lboard = calloc (newsize, sizeof(int));
  if (errno == ENOMEM) cantcontinue("Out of memory. Can't realloc board.\n");
  assert (new_lboard);

  for (p = new_lboard+newsize; p > new_lboard;)
    *(--p) = UNUSEDSQ;/* init with never used square */
#ifdef DEBUG_BINARY_BOARD
  dump_binary_board (stdout, new_lboard, newxdim, newydim);
#endif /* DEBUG_BINARY_BOARD */
  if (lboard != NULL) {/* there is a previous board to be replaced */
    /* copy the [xmin..xmax] x [ymin..ymax] subarray */
    p = lboard + (xmin+xdepl)*ydim + ydepl;
    q = new_lboard + (xmin+newxdepl)*newydim + newydepl;
    for (i = xmin; i <= xmax; i++) {
      for (j = ymin; j <= ymax; j++) {
	*(q+j) = *(p+j);
      }
      p += ydim; q += newydim;
    }
#ifdef DEBUG_BINARY_BOARD
    dump_binary_board (stdout, new_lboard, newxdim, newydim);
#endif /* DEBUG_BINARY_BOARD */
    /* free old mem */
    free(lboard);
  }
  /* now we set proper board vals */
  xdim = newxdim; ydim = newydim;
  xdepl = newxdepl; ydepl = newydepl;
  lboard = new_lboard; cboard = lboard + xdepl*ydim + ydepl;
  TRACEOUT;
}

/* THE LOGICAL BOARD is an infinite board but only points "in range", i.e. between xmin..xmax and ymin..ymax can be touched.

   NOTE: Collapsing at extremal positions may enlarge the range.

   NOTATION CONVENTION: We use (i,j) for physical & (x,y)∈ℤxℤ for logical/absolute coords

   The physical and the logical boards are connected via
   B[x,y] ~ lboard[(x+xdepl)*ydim + y+ydepl]
*/
/* outside modules can only use CURR_VAL to read the board. */
extern int curr_val (int x, int y)
{
  assert (x >= xmin); assert (y >= ymin);
  assert (x <= xmax); assert (y <= ymax);
  assert (cboard);		/* inited */
  return cboard[x*ydim + y];
}

/* while collapsing we have a waiting list of squares to process */
static bool waitlist_inited = false;
static int * waitinglist = NULL;
static int * wait_curr = NULL;
static int * wait_max = NULL;
#define STACK_SIZE ((size_t)5*K10)
void init_waitlist ( void )
{
  TRACEIN;
  //  dump_waitinglist (stdout, true);
  if (waitlist_inited) {
    /* do nothing */
  } else {
    assert (STACK_SIZE <= SIZE_MAX / 2);
    size_t maxnbwaitingelems = STACK_SIZE * 2;
    wait_curr = waitinglist = calloc (maxnbwaitingelems, sizeof(int));
    if (errno == ENOMEM) {
      cantcontinue("%s can't alloc for stack\n", __func__);
    }
    assert (waitinglist != NULL);
    wait_max = waitinglist + maxnbwaitingelems;
    waitlist_inited = true;
  }
  //  dump_waitinglist (stdout, true);
  TRACEOUT;
}

void close_waitinglist ( void )
{
  if (waitlist_inited) {
    free(waitinglist);
    waitlist_inited = false;
    waitinglist = wait_curr = wait_max = NULL;
  }
}

void stackin_waiting (int x, int y)
{
  if (wait_curr == wait_max) {
    cantcontinue("Waiting list is full. Can't add to. FIXME: with resize?\n");
  }
  *(wait_curr++) = x;		 /* push x 1st, see popping */
  assert (wait_curr < wait_max);/* need no test since we alloced even number */
  *(wait_curr++) = y;
}

/* INCR_SQUARE modifies the board by adding DELTA (can be 0) on X,Y.
   Maintains counts. No collapse.
   New val ≥ 4 are stacked in waiting list (no checking if new val ≠ prev val)
   Prev val ≥ 4 are assumed to have been unstacked by caller */
void incr_square (int x, int y, int delta)
{
  TRACEINW("(x=%d, y=%d, delta=%d)", x, y, delta);
  //  dump_waitinglist(stdout, true);
  int * p = cboard + x*ydim + y;
  int val = *p;
  assert ((val == UNUSEDSQ) || (delta >= - val));
  switch (val) {//update count
  case 0: assert(count0 > 0); count0--; break;
  case 1: assert(count1 > 0); count1--; break;
  case 2: assert(count2 > 0); count2--; break;
  case 3: assert(count3 > 0); count3--; break;
  case UNUSEDSQ: val=0; area++; break; /* needs real val in later computation */
  default: assert(val>3); /* do nothing */
  }
  val += delta;
  *p = val;
  switch (val) {
  case 0: assert(count0 < INT_MAX); count0++; break;
  case 1: assert(count1 < INT_MAX); count1++; break;
  case 2: assert(count2 < INT_MAX); count2++; break;
  case 3: assert(count3 < INT_MAX); count3++; break;
  default:
    assert(val>3);
    /* only stackin when val goes 3->4, 7->8, ... Other options are 5->6, 8->4 etc. */
    if ((val%4 == 0) && (delta == 1)) stackin_waiting(x,y);
    break;
  }
  if ((terminal_mode != false) && (anim_level >= 2))
    dump_waitinglist(stdout, false);
  TRACEOUT;
}

void collapse (int x, int y)
{
  assert (cboard[x*ydim+y] > 3);
  TRACEINW("(x=%d, y=%d)", x, y);

  if ((x <= xmin) || (x >= xmax) || (y <= ymin) || (y >= ymax)) {
    TRACEMESS ("have to enlarge"o);
    enlarge_envelop_around (x,y);
  }
  incr_square(x,y,-4);
  incr_square(x+1,y,1);
  incr_square(x-1,y,1);
  incr_square(x,y+1,1);
  incr_square(x,y-1,1);
  if (nbsteps == ULLONG_MAX) {
    cantcontinue("ERROR: Max number of steps UINTMAX = %llu reached. I must stop", ULLONG_MAX);
  }
  nbsteps++;
  TRACEOUT;
}

void goto_normal_form ( void )
{
  TRACEIN;
  assert(lboard != NULL);

  bool finished = false;
  int x, y, val;
  while (wait_curr > waitinglist) {
    assert (wait_curr >= waitinglist+2);
    y = *(--wait_curr);		/* pop y first, see pushing */
    x = *(--wait_curr);
    val = cboard[x*ydim+y];
    if ((terminal_mode != false) && (anim_level >= 2)) {
      printf ("We popped B[%d,%d]=%d\n", x, y, val);
    }
    if (val > 3) {
      collapse(x,y);
    } else if (terminal_mode) {
      printf ("! False positive in waitinglist: x=%d y=%d val=%d\n",
	      x, y, cboard[x*ydim+y]);
    }
  }
#ifndef NDEBUG
  // FIXME: REMOVE LATER
  // CHECK FOR SAFETY:
  int count = 0;
  do {
    finished = true; /* tentatively */
    for (x = xmin; x <= xmax; x++) {
      for (y = xmin; y <= ymax; y++) {
	if (cboard[x*ydim+y] > 3) {
	  finished = false;
	  count++;
	  fprintf (stdout, "! Safety check has to collapse (%d,%d) val=%d\n", x, y, cboard[x*ydim+y]);
	  collapse(x,y);
	}
      }
    }
  } while (finished == false);
  if (count > 0) {
    fprintf (stdout, "ERROR: %s: Safety check had to collapse %d times\n", __func__, count);
    fprintf (stderr, "ERROR: %s: Safety check had to collapse %d times\n", __func__, count);
    fail();
  }
#endif /* NDEBUG */
  
  handle_terminated_board ();

  TRACEOUT;
}

void increase_orig_mass (int delta)
{
  assert ((delta < 0) || (mass <= INT_MAX - delta));
  mass += delta;
  incr_square(0, 0, delta);
}

void enlarge_envelop_around (int x, int y)
{
  TRACEINW("(x=%d y=%d)", x, y);
#ifdef DEBUG_BINARY_BOARD
  dump_binary_board (stdout, lboard, xdim, ydim);
#endif /* DEBUG_BINARY_BOARD */
  if (   (x + xdepl == 0) || (x + xdepl == xdim - 1)
	 || (y + ydepl == 0) || (y + ydepl == ydim - 1)) {
    realloc_board(xdim * 2 + 1, ydim * 2 + 1, xdim, ydim); /* assuming xdim was odd */
  }
  if (x == xmin) {
    xmin--;
    if (xmin < - used_radius) used_radius = - xmin;
  }
  if (x == xmax) {
    xmax++;
    if (xmax > used_radius) used_radius = xmax;
  }
  if (y == ymin) {
    ymin--;
    if (ymin < - used_radius) used_radius = - ymin;
  }
  if (y == ymax) {
    ymax++;
    if (ymax > used_radius) used_radius = ymax;
  }
  if ((xmax - xmin + 1 > max_dim) || (xmax - xmin + 1 > max_dim)) {
    cantcontinue("Max_dim = %d reached. Am not allowed to enlarge board...\n", max_dim);
  }
#ifdef DEBUG_BINARY_BOARD
  dump_binary_board (stdout, lboard, xdim, ydim);
#endif /* DEBUG_BINARY_BOARD */
  if (cursing_mode) {
    display_cursing_dims ();
  }
  TRACEOUT;
}

void handle_terminated_board ( void )
{
  TRACEIN;
#ifdef DEBUG_BINARY_BOARD
  dump_binary_board (stdout, lboard, xdim, ydim);
#endif /* DEBUG_BINARY_BOARD */
  assert (mass == count1 + 2 * count2 + 3 * count3); /* since terminated */
  assert (area == count0 + count1 + count2 + count3);/* since terminated */
  assert (xmax == used_radius);
  assert (ymax == used_radius);
  assert (xmin + xmax == 0);
  assert (ymin + ymax == 0);
  record_normal_form (); /* on screen and on -o file */
  record_this_board ();
  TRACEOUT;
}

void init_board (int some_dim)
{
  TRACEIN;
  assert (cboard == NULL);
  assert (some_dim <= MAX_ALLOWED_DIM);
  if (board_inited != false) {
    cantcontinue ("ERROR: %s: board was already inited?!\n", __func__);
  }
  init_waitlist ();
  current_dim = 0;		/* dummy values for "previous" board */
  xdim = xmin = xmax = xdepl = 0; ydim = ymin = ymax = ydepl = 0;
  realloc_board (some_dim, some_dim, some_dim / 2, some_dim / 2); /* set board vars properly. No copy made since board was not inited */
  mass = 0; nbsteps = 0; used_radius = 0; count1 = count2 = count3 = 0;
  area = count0 = 0;		/* init count0 */
  incr_square(0,0,0);		/* set orig as "used" */
  record_initial_board ();
  TRACESIGNEDMESS ("xdim=%d ydim=%d", xdim, ydim);
  TRACEOUT;
}

void close_board ( void )
{
  if (board_inited) {
    board_inited = false;
    free(cboard);
    close_waitinglist ();
    current_dim = xdim = xmin = xmax = xdepl = ydim = ymin = ymax = ydepl = 0;
    mass = 0; nbsteps = 0; used_radius = 0; count1 = count2 = count3 = 0;
    area = count0 = 0;
  }
}

void display_the_board (FILE * f)
{
  TRACEIN;
  assert (xmin + used_radius >= 0);
  assert (xmax <= used_radius);
  assert (ymin + used_radius >= 0);
  assert (ymax <= used_radius);
  TRACESIGNEDMESS ("c0=%d c1=%d c2=%d c3=%d", count0, count1, count2, count3);
#ifdef DEBUG_BINARY_BOARD
  dump_binary_board (stdout, lboard, xdim, ydim);
#endif /* DEBUG_BINARY_BOARD */
  int x, y;
  int wid = xmax - xmin + 1;
  fputc(' ', f); fprint_n_char(wid, '-', f); fputc(' ', f); fterpri(f);
  for (y = ymax+1; y-- > ymin;) { /* downward loop :-( */
    fputc('|', f);
    for (x = xmin; x <= xmax; x++) {
//fixme: remove      assert (cboard[x*ydim+y] == GROUNDBOARD(x+xdepl,y+ydepl));//
      fputc (char_for_val(cboard[x*ydim+y]), f);
    }
    fputc('|', f); fterpri(f);
  }
  fputc(' ', f); fprint_n_char(wid, '-', f); fputc(' ', f); fterpri(f);
  TRACEOUT;
}

/* FOLLOWING FUNCTIONS USED FOR TRACES IN TESTING AND DEBUGGING */
void dump_waitinglist (FILE * stream, bool statstoo)
{
  int nb, x, y, *p;
  fprintf (stream, "== START DUMP_WAITINGLIST ==\n");
  if (statstoo) {
    fprintf (stream, "waitlist_inited = %s\n", boolstr(waitlist_inited));
    fprintf (stream, "waitinglist = %p\n", waitinglist);
    fprintf (stream, "wait_curr = %p\n", wait_curr);
    fprintf (stream, "wait_max = %p\n", wait_max);
  }
  if (waitlist_inited) {
    if (0 != (wait_curr - waitinglist) % 2) {
      fprintf (stderr, "ERROR: %s: nb elems in waitinglist should be even.\n", __func__);
      fail();
    }
    if (statstoo) {
      fprintf (stream, "Total capacity:\t%ld ints\n", wait_max - waitinglist);
      fprintf (stream, "Used capacity:\t%ld ints\n", wait_curr - waitinglist);
      fprintf (stream, "Remaining cap.:\t%ld ints\n", wait_max - wait_curr);
    }
  }
  nb = 0; p = waitinglist;
  while (p < wait_curr) {
    x = *(p++); y = *(p++);  nb++;
    fprintf (stream, "WL[%d] = (%d,%d)\n", nb, x, y);
  }
}

void dump_binary_board (FILE * stream, int * tab, int xd, int yd)
{
  int i, j, * tabi;
  TRACEIN;
  assert(xd>=0); assert(yd>=0);
  fprintf (stream, "== START DUMP_BINARY_BOARD(xd=%d,yd=%d) ==\n", xd, yd);
  if (tab == NULL) {
    fprintf (stream, "! tab = NULL !!\n");
  } else {
    for (j = yd; j-- > 0;) { /* downward loop :-( */
      fprintf (stream, "L%d:\t", j);
      for (i = 0; i < xd; i++) {
	tabi = tab+i*yd;
	if (tabi == NULL) {
	  fprintf (stream, "\n! tab[i=%d] = NULL !!\n", i);
	  goto quit;
	}
	putchar(char_for_val(tabi[j]));
      }
      fterpri(stream);
    }
  }
 quit:
  TRACEOUT;
  return;
}
