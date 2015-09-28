#ifndef MAKEDEPEND_IGNORE
#include <stdlib.h> 		/* alloc & free */
#include <alloca.h> 		/* alloca */
#include <limits.h> 		/* UINT_MAX */
#include <sys/utsname.h>	/* uname() */
#include <unistd.h>             /* usleep() */
#include <stdio.h>
#include <assert.h>

#include <print.h> 		/* fail(); from libphs */
#endif /* MAKEDEPEND_IGNORE */


#include "trace.h"
#include "sand.h"

bool cursing_mode = false;
bool data_file_mode = true;

bool board_inited = false;
unsigned int current_dim = 0;	/* > 2*radius when alloc'ed */
unsigned int mass = 0;
unsigned long long int nbsteps = 0;
unsigned int anim_level = 0;	/* user input */
unsigned int max_height = 0;	/* user input */
unsigned int max_dim = 0;	/* user input */
int xdim, ydim;                 /* dim of alloc'ed array */
int xmin, xmax, ymin, ymax;	/* coord of extremal points */
int xdepl, ydepl;	        /* coord of logical (0,0) in array */
unsigned int * * the_board = NULL;
unsigned int used_radius;
unsigned int count1;
unsigned int count2;
unsigned int count3;

bool okij (int i, int j) /* useful in debugging */
{
  if ((i >= xmin) && (i <= xmax) && (j >= ymin) && (j <= ymax)) {
    return true;
  } else {
    fprintf (stderr, "okij(%d,%d) fails\n", i, j);
    return false;
  }
}

void set_value_for_anim_level (unsigned int x)
{
  TRACEINW("(%u)", x);
  assert (x <= MAX_ALLOWED_ANIM_LEVEL);
  anim_level = x;
  TRACEOUT;
}

void set_value_for_height (unsigned int x)
{
  TRACEINW("(%u)", x);
  assert (x <= MAX_ALLOWED_HEIGHT);
  max_height = x;
  TRACEOUT;
}

void set_value_for_max_dim (unsigned int x)
{
  TRACEINW("(%u)", x);
  assert (x <= MAX_ALLOWED_DIM);
  max_dim = x;
  TRACEOUT;
}

static FILE * output_file = NULL;

void dump_binary_board(FILE * f, unsigned int * * tab, unsigned int xd, unsigned int yd)
{
  unsigned int i, j, * tabi;
  fprintf (f, "== START DUMP_BINARY_BOARD(xd=%u,yd=%u) ==\n", xd, yd);
  if (tab == NULL) {
    fprintf (f, "! tab = NULL !!\n");
  } else {
    for (j = yd; j-- > 0;) { /* downward loop on unsigned :-( */
      fprintf (f, "L%u:\t", j);
      for (i = 0; i < xd; i++) {
	tabi = tab[i];
	if (tabi == NULL) {
	  fprintf (f, "\n! tab[i=%u] = NULL !!\n", i);
	  goto quit;
	}
	putchar(char_for_val(tabi[j]));
      }
      fterpri(f);
    }
  }
 quit:
  fprintf (f, "== FINISH DUMP_BINARY_BOARD(xd=%u,yd=%u) ==\n", xd, yd);
}

void realloc_board (int newdim)
{
  TRACEINW("(%u)", newdim);
  int newxdepl = newdim / 2;
  int newydepl = newdim / 2;
  if ((xdim > newdim) || (ydim > newdim)) {
    fprintf (stderr, "ERROR: Can't realloc to smaller board. Asking for %dx%d, was %dx%d\n", newdim, newdim, xdim, ydim);
    fail();
  }
  //  int shiftx = newxdepl - xdepl;  not used
  //  int shifty = newydepl - ydepl;
  unsigned int * * new_board = calloc(newdim, sizeof(unsigned int *));
  if (new_board == NULL) {
    fprintf (stderr, "out of memory\n");
    fail();
  }
  int i, j;
  for (i = 0; i < newdim; i++) {
    new_board[i] = calloc(newdim, sizeof(unsigned int)); /* fills with 0 */
    if (new_board[i] == NULL) {
      fprintf (stderr, "out of memory\n");
      fail();
    }
  }
  // dump_binary_board(stdout, new_board, newdim, newdim);
  if (the_board != NULL) {/* there was a previous board */
    /* copy */
    for (i = xmin; i <= xmax; i++) {
      for (j = ymin; j <= ymax; j++) {
	new_board[i+newxdepl][j+newydepl] = the_board[i+xdepl][j+ydepl];
      }
    }
    // dump_binary_board(stdout, new_board, newdim, newdim);
    /* free old mem */
    for (i = 0; i < xdim; i++) {
      free (the_board[i]);
    }
    free(the_board);
  }
  /* now we set proper board vals */
  xdim = ydim = newdim;
  xdepl = newxdepl; ydepl = newydepl;
  the_board = new_board;
  TRACEOUT;
}

void init_board (unsigned int some_dim)
{
  TRACEIN;
  assert (board_inited == false); assert (the_board == NULL);
  assert (some_dim <= MAX_ALLOWED_DIM);
  current_dim = 0; /* dummy values for "previous" board */
  mass = 0; nbsteps = 0; used_radius = 0; count1 = count2 = count3 = 0;
  xdim = xmin = xmax = xdepl = 0; ydim = ymin = ymax = ydepl = 0;
  realloc_board (some_dim);	/* will set all board vars properly */
  TRACESIGNEDMESS ("xdim=%d ydim=%d", xdim, ydim);
  TRACEOUT;
}

char char_for_val (unsigned int val)
{
  DEEPTRACEINW("(%u)", val);
  char res;
  if (val == 0) {
    res = '.';
  } else if (val <= 9) {
    res = '0' + val;
  } else if (val <= 35) {
    res = 'A' + val - 10;
  } else {
    res = '?';
  }
  DEEPTRACEOUT;
  return res;
}

void display_the_board ( void )
{
  TRACEIN;
  assert (xmin >= - (int)used_radius);
  assert (xmax <= (int)used_radius);
  assert (ymin >= - (int)used_radius);
  assert (ymax <= (int)used_radius);
  // dump_binary_board(stdout,the_board,xdim,ydim);
  int i, j;
  printf ("! board has xdim=%d, ydim=%d: xdepl=%d, ydepl=%d\n",
	  xdim, ydim, xdepl, ydepl);
  printf ("! M=%u, x=%d..%d, y=%d..%d\n", mass, xmin, xmax, ymin, ymax);
  print_n_char(xmax - xmin + 1, '-'); terpri();
  for (j = ymax+1; j-- > ymin;) { /* downward loop :-( */
    for (i = xmin; i <= xmax; i++) {
      putchar (char_for_val(the_board[i+xdepl][j+ydepl]));
    }
    terpri();
  }
  print_n_char(xmax - xmin + 1, '-'); terpri();
  TRACEOUT;
}

void record_normal_form ( void )
{
  if (data_file_mode) {
    assert (output_file);
    fprintf (output_file, "mass=%u steps=%llu diam=%u c1=%u c2=%u c3=%u\n", mass, nbsteps, 1+2*used_radius, count1, count2, count3);
  }
  if (cursing_mode == false) {
    printf ("! Normal form for mass=%u steps=%llu diam=%u c1=%u c2=%u c3=%u\n", mass, nbsteps, 1+2*used_radius, count1, count2, count3 );
  } else {
    display_cursing_mass_data();
  }
}

void handle_terminated_board ( void )
{
  TRACEIN;
  assert (mass = count1 + 2 * count2 + 3 * count3);
  assert (xmax == (int)used_radius);
  assert (ymax == (int)used_radius);
  assert (xmin == - xmax);
  assert (ymin == - ymax);
  record_normal_form (); /* on screen and on -o file */
  if (anim_level > 0) {
    if (cursing_mode) {
      display_cursing_board ();
    } else {
      display_the_board();
    }
    if (mass < 200) {		
      usleep(DELAYAFTERDISPLAY);
    } else {			 /* delays add up in the long run */
      usleep(DELAYAFTERDISPLAY * 200 / mass); /* decreasing delay */
    }
  }
  TRACEOUT;
}

void enlarge_envelop (int x, int y)
{
  TRACEINW("(x=%d y=%d)", x, y);
  // dump_binary_board(stdout, the_board, xdim, ydim);
  if (   (x + xdepl == 0) || (x + xdepl == (int)xdim-1)
      || (y + ydepl == 0) || (y + ydepl == (int)ydim-1)) {
    realloc_board(xdim * 2 + 1); /* assuming xdim was odd */
  }
  if (x == xmin) {
    xmin--;
    if (xmin < - (int)used_radius) used_radius = (unsigned int)(- xmin);
  }
  if (x == xmax) {
    xmax++;
    if (xmax > (int)used_radius) used_radius = xmax;
  }
  if (y == ymin) {
    ymin--;
    if (ymin < - (int)used_radius) used_radius = (unsigned int)(- ymin);
  }
  if (y == ymax) {
    ymax++;
    if (ymax > (int)used_radius) used_radius = ymax;
  }
  // dump_binary_board(stdout, the_board, xdim, ydim);
  if (cursing_mode) {
    display_cursing_dims ();
  }
  TRACEOUT;
}

void incr_square (int x, int y, int delta)
{
  TRACEINW("(x=%d, y=%d, delta=%d)", x, y, delta);
  unsigned int val = the_board[x+xdepl][y+ydepl];
  switch (val) {
  case 1: assert(count1 > 0); count1--; break;
  case 2: assert(count2 > 0); count2--; break;
  case 3: assert(count3 > 0); count3--; break;
  default: ; /* do nothing */
  }
  val += delta;
  the_board[x+xdepl][y+ydepl] = val;
  switch (val) {
  case 1: assert(count1 < UINT_MAX); count1++; break;
  case 2: assert(count2 < UINT_MAX); count2++; break;
  case 3: assert(count3 < UINT_MAX); count3++; break;
  default: ; /* do nothing */
  }
  TRACEOUT;
}

void increase_orig_mass (int delta)
{
  assert ((delta < 0) || (mass <= UINT_MAX - delta));
  mass += delta;
  incr_square(0, 0, delta);
}

void collapse (int x, int y)
{
  assert (the_board[x+xdepl][y+ydepl] > 3);
  TRACEINW("(x=%d, y=%d)", x, y);

  if ((x <= xmin) || (x >= xmax) || (y <= ymin) || (y >= ymax)) {
    TRACEMESS ("have to enlarge");
    enlarge_envelop (x,y);
  }
  incr_square(x,y,-4);
  incr_square(x+1,y,1);
  incr_square(x-1,y,1);
  incr_square(x,y+1,1);
  incr_square(x,y-1,1);
  if (nbsteps == ULLONG_MAX) {
    fprintf (stderr, "ERROR: Max number of steps UINTMAX = %llu reached. I must stop", ULLONG_MAX);
    fail();
  }
  nbsteps++;
  TRACEOUT;
}

void goto_normal_form ( void )
{
  TRACEIN;
  assert(the_board != NULL);

  bool finished = false;
  int i, j;
  do {
    finished = true; /* tentatively */
    for (i = xmin; i <= xmax; i++) {
      for (j = xmin; j <= ymax; j++) {
	if (the_board[i+xdepl][j+xdepl] > 3) {
	  finished = false;
	  collapse(i,j);
	}
      }
    }
  } while (finished == false);

  handle_terminated_board ();

  TRACEOUT;
}

void open_res_file ( void )
{
  TRACEIN;
  char * buff = alloca(150);
  if (max_height != DEFAULT_HEIGHT) {
    sprintf (buff, "stats_tas_sable_n=%u.txt", max_height);
  } else {
    sprintf (buff, "stats_tas_sable.txt");
  }
  output_file = fopen(buff, "w");
  assert (output_file);
  TRACEOUT;
}

void close_res_file (bool with_stats)
{
  TRACEINW("(%d)",(int)with_stats);
  assert (output_file);

  if (with_stats) {
    struct utsname unameData;
    if (uname(&unameData) != 0) {
      fprintf (stderr, "ERROR: %s: system call to uname() failed\n", __func__);
      fail();
    };
    fprintf (output_file, "# date = %s\n", now_str(true));
    fprintf (output_file, "# machine = %s\n", unameData.nodename);
    display_rusage (output_file);
  }
  fclose(output_file);
  output_file = NULL;
  TRACEOUT;
}

int main (int argc, char *argv[])
{
  unsigned int i;

  set_value_for_height(DEFAULT_HEIGHT); /* default */
  set_value_for_max_dim(DEFAULT_MAX_DIM);    /* default */
  process_options(argc,argv);

#ifdef TRACE
  if (cursing_mode) {
    fprintf (stderr, "Cannot use cursing_mode when compiled with -DTRACE.\n");
    fprintf (stderr, "I disable it.\n");
    cursing_mode = false;
    TRACEWAITFORCHAR;
  }
#endif /* TRACE */
  if (cursing_mode) {
    init_cursing();
  }
  if (data_file_mode)
    open_res_file ();

  init_board(DEFAULT_INIT_DIM);

  for (i = 1; i <= max_height; i++) {
    increase_orig_mass(1);
    goto_normal_form();
  }

  if (data_file_mode)
    close_res_file (true);
  if (cursing_mode) {
    wait_in_cursing();
    terminate_cursing();
  }
  exit (EXIT_SUCCESS);
}
