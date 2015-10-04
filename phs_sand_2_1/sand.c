#ifndef MAKEDEPEND_IGNORE
#include <print.h> 		/* fail(); from libphs */

#include <errno.h> 		
#include <stdlib.h> 		/* daemon, alloc & free */
#include <alloca.h> 		/* alloca */
#include <sys/utsname.h>	/* uname() */
#include <unistd.h>             /* usleep() */
#include <curses.h>
#include <stdio.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */


#include "trace.h"
#include "sand.h"

/* local functions: */
extern void set_value_for_anim_level (int x);
extern void set_value_for_height (int x);
extern void set_value_for_max_dim (int x);

static FILE * output_file = NULL;

bool terminal_mode = true;	/* default mode */
bool cursing_mode = false;
bool underground_mode = false;
bool with_data_file = true;

int anim_level = 0;		/* user input */
int max_height = 0;		/* user input */
int max_dim = 0;		/* user input */
int xdim, ydim = 0;             /* dim of alloc'ed array */
int xmin, xmax, ymin, ymax = 0;	/* coord of extremal points */
int xdepl, ydepl = 0;	        /* coord of logical (0,0) in array */
//int * * the_board = NULL;
/* output data maintained by program */
int mass = 0;
int current_dim = 0;
int area = 0;
int used_radius = 0;
unsigned long long int nbsteps = 0;
int count0 = 0;
int count1 = 0;
int count2 = 0;
int count3 = 0;

bool okij (int i, int j) /* useful in debugging */
{
  if ((i >= xmin) && (i <= xmax) && (j >= ymin) && (j <= ymax)) {
    return true;
  } else {
    fprintf (stderr, "okij(%d,%d) fails\n", i, j);
    return false;
  }
}

void set_value_for_anim_level (int x)
{
  TRACEINW("(%d)", x);
  assert (x <= MAX_ALLOWED_ANIM_LEVEL);
  anim_level = x;
  TRACEOUT;
}

void set_value_for_height (int x)
{
  TRACEINW("(%d)", x);
  assert (x <= MAX_ALLOWED_HEIGHT);
  max_height = x;
  TRACEOUT;
}

void set_value_for_max_dim (int x)
{
  TRACEINW("(%d)", x);
  assert (x <= MAX_ALLOWED_DIM);
  max_dim = x;
  TRACEOUT;
}

void record_normal_form ( void )
{
  static int prev_nbsteps = 0;
  static int prev_diam = 0;
  int diam = 1+2*used_radius;	/* FIXME: should be maintained in board.c */
  if (with_data_file) {
    assert (output_file);
    fprintf (output_file, "M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", mass, nbsteps, diam, count0, count1, count2, count3);
    if ( (nbsteps - prev_nbsteps > K100) || (diam > prev_diam) )
      fflush (output_file);
  }
  if (cursing_mode == false) {
    printf ("! Normal form for M=%d: T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", mass, nbsteps, diam, count0, count1, count2, count3 );
  } else {
    display_cursing_mass_data();
  }
  prev_nbsteps = nbsteps; prev_diam = diam; /* record for next time */
}

void record_initial_board ( void )
{
  TRACEIN;
  if (cursing_mode) {
    if (anim_level > 0) {
      display_cursing_dims ();
    }
  }
  record_this_board ();
  TRACEOUT;
}

void record_this_board ( void )
{
  TRACEIN;
  if (anim_level > 0) {
    if (cursing_mode) {
      display_cursing_board ();
    } else {
      display_the_board(stdout);
    }
    if (mass < 200) {
      usleep(DELAYAFTERDISPLAY);
    } else {			 /* delays add up in the long run */
      usleep(DELAYAFTERDISPLAY * 200 / mass); /* decreasing delay */
    }
  }
  TRACEOUT;
}

void open_res_file ( void )
{
  TRACEIN;
  char * buff = alloca(150);
  if (errno == ENOMEM) {
      cantcontinue("%s can't alloc for string buff\n", __func__);
  }
  assert (buff);
  if (max_height != DEFAULT_HEIGHT) {
    sprintf (buff, "stats_tas_sable_n%d.txt", max_height);
  } else {
    sprintf (buff, "stats_tas_sable.txt");
  }
  output_file = fopen(buff, "w");
  assert (output_file);
  TRACEOUTMESS("%s", buff);
}

void close_res_file (bool with_stats)
{
  TRACEINW("(%s)", boolstr(with_stats));
  assert (output_file);

  if (with_stats) {
    struct utsname unameData;
    if (uname(&unameData) != 0) {
      fprintf (stderr, "ERROR: %s: system call to uname() failed\n", __func__);
      fail();
    };
    fprintf (output_file, "# date = %s\n", now_str(true));
    fprintf (output_file, "# machine = %s\n", unameData.nodename);
    fprintf (output_file, "# code version = %s\n", SANDPILE_VERSION);
    display_rusage (output_file);
  }
  fclose(output_file);
  output_file = NULL;
  TRACEOUT;
}

void config_for_underground ( void )
{
  if (output_file == NULL) {
    cantcontinue("Can't run underground: output file == NULL\n");
  }
  stderr = output_file;
  if (daemon(1,0) != 0) {
    cantcontinue("daemon(1,0) failed. Can't run underground\n");
  }
}

int main (int argc, char *argv[])
{
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
  if (cursing_mode)
    init_cursing();

  if (with_data_file)
    open_res_file ();

  if (underground_mode)  /* underground after open_res_file */
    config_for_underground ();

  init_board (DEFAULT_INIT_DIM);

  int i;
  for (i = 1; i <= max_height; i++) {
    increase_orig_mass(1);
    goto_normal_form();
  }

  close_board ();
  
  if (with_data_file)
    close_res_file (true);
  if (cursing_mode) {
    wait_in_cursing();
    terminate_cursing();
  }
  exit (EXIT_SUCCESS);
}
