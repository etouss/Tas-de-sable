#ifndef MAKEDEPEND_IGNORE
#include <print.h> 		/* fail(); from libphs */

#include <errno.h>
#include <stdlib.h> 		/* daemon, alloc & free */
#include <alloca.h> 		/* alloca */
#include <unistd.h>             /* usleep() */
#include <time.h>		/* time() */
#include <stdio.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"

/* local functions: none */
static FILE * output_file = NULL;

bool terminal_mode = true;	/* default mode */
bool cursing_mode = false;
bool underground_mode = false;
bool with_data_file = true;
bool from_snapshot_mode = false;

int selected_job = DEFAULT_JOB; /* user input */
const char * snapshot_source_file_arg = NULL; /* user input */
int anim_level = 0;		/* user input */
int max_height = 0;		/* user input */
int max_dim = 0;		/* user input */

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

void set_value_for_snapshot_delay (int x)
{
  TRACEINW("(%d)", x);
  assert (x <= MAX_ALLOWED_SNAPSHOT_DELAY);
  snapshot_delay = x;
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

void take_snapshot ( void )
{
  TRACEIN;
  char * buff = alloca(150);
  if (errno == ENOMEM) cantcontinue("ERROR: %s can't alloc for string buff.\n", __func__);
  assert (buff);
  /* snapshot filename and contents do not depend on selected_job */
  sprintf (buff, "snapshot_sand_v%s_M%d.txt", SANDPILE_VERSION, mass);
  FILE * f = fopen(buff, "w");
  if (f == NULL)
    cantcontinue("ERROR: %s: Could not open file %s.\n", __func__, buff);
  record_normal_form(f);
  display_the_board(f, false);
  fflush (f);
  if (fclose (f) != 0) cantcontinue("ERROR: %s: fclose failed.\n", __func__);
  TRACEOUT;
}

void record_normal_form (FILE * f)
{
  static time_t now;
  static int prev_nbsteps = 0;
  static int prev_diam = 0;
  if (with_data_file) {
    assert (f);
    switch (selected_job) {
    case TIME_JOB:
      fprintf (f, "M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", mass, nbsteps, diam, count0, count1, count2, count3);
      break;
    case AREA_JOB:
      fprintf (f, "A=%d M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", area, mass, nbsteps, diam, count0, count1, count2, count3);
      display_the_board (f, false);
      break;
    }
    if ( (nbsteps - prev_nbsteps > K100) || (diam > prev_diam) )
      fflush (f);
  }
  if (snapshot_delay) {
    assert(snapshot_delay>0);
    if (time(&now) == (time_t)-1) cantcontinue("ERROR: %s: time() failed.\n", __func__);
    if (now > timer_start+snapshot_delay) {
      timer_start += snapshot_delay;
      take_snapshot();
    }
  }
  if (cursing_mode == false) {
    switch (selected_job) {
    case TIME_JOB:
      printf ("! Normal form for M=%d: T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", mass, nbsteps, diam, count0, count1, count2, count3 ); break;
    case AREA_JOB:
      printf ("! Normal form for A=%d: M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", area, mass, nbsteps, diam, count0, count1, count2, count3 );
      break;
    }
  } else {
    display_cursing_mass_data();
  }
  prev_nbsteps = nbsteps; prev_diam = diam; /* record for next time */
}

void handle_terminated_board ( void )
{
  TRACEINW(": mass=%d", mass);
#ifdef DEBUG_BINARY_BOARD
  dump_memmatrix (stdout, ph_board, xdim, ydim);
#endif /* DEBUG_BINARY_BOARD */
  assert (mass == count1 + 2 * count2 + 3 * count3); /* since terminated */
  assert (area == count0 + count1 + count2 + count3);/* since terminated */
  assert (xmax - xmin == diam - 1);
  assert (ymax - ymin == diam - 1);
  assert (xmin + xmax == 0);
  assert (ymin + ymax == 0);
  static int prev_area = -1;
  switch (selected_job) {
  case TIME_JOB:
    record_normal_form (output_file);	       /* on -o file */
    if (anim_level > 0) display_this_board (); /* and on screen */
    break;
  case AREA_JOB:
    if (area > prev_area) {
      record_normal_form (output_file);		 /* on -o file */
      if (anim_level > 0) display_this_board (); /* and on screen */
    }
    break;
  default:
    cantcontinue ("ERROR: %s: don't know about selected_job=%d.\n",
		  __func__, selected_job);
  }
  prev_area = area;
  TRACEOUT;
}

void display_initial_board ( void )
{
  TRACEIN;
  if (cursing_mode) {
    display_cursing_dims ();
  }
  display_this_board ();
  TRACEOUT;
}

void display_this_board ( void )
{
  TRACEIN;
  if (cursing_mode) {
    display_cursing_board ();
    //FIXME: no display delay in interactive mode
    if (mass < 200) {
      usleep(DELAYAFTERDISPLAY);
    } else {			 /* delays add up in the long run */
      usleep(DELAYAFTERDISPLAY * 200 / mass); /* decreasing delay */
    }
  } else {
    display_the_board(stdout, false);
  }
  TRACEOUT;
}

void init_res_file ( void )
{
  if (output_file == NULL) {
    cantcontinue("ERROR: %s: output file not opened.\n", __func__);
  }
  fprintf (output_file, "# SAND v%s running on %s\n", SANDPILE_VERSION, machine_uname());
  output_calling_options (output_file, "# Called with: ", " ", "\n");
  fprintf (output_file, "# Started: %s\n", now_str(true));
}

void open_res_file ( void )
{
  TRACEIN;
  char * fname = NULL;
  char * frompart = NULL;
  if (from_snapshot_mode) {
    long int mass_sn;
    get_mass_from_snapshot(snapshot_source_file_arg, &mass_sn);
    asprintf(&frompart, "from_M%ld", mass_sn);
  } else {
    asprintf(&frompart, "");
  }
  if (frompart == NULL)
    cantcontinue("ERROR: %s: asprintf could not alloc frompart.\n", __func__);

  switch (selected_job) {
  case TIME_JOB:
    asprintf (&fname, "stats_sand_v%s_T%sjqa_M%d.txt", SANDPILE_VERSION, frompart, max_height);
    break;
  case AREA_JOB:
    asprintf (&fname, "stats_sand_v%s_A%sjqa_M%d.txt", SANDPILE_VERSION, frompart, max_height);
    break;
  }
  if (fname == NULL)
    cantcontinue("ERROR: %s: asprintf could not alloc fname.\n", __func__);

  output_file = fopen(fname, "w");
  if (output_file == NULL)
    cantcontinue("ERROR: %s: Could not open file %s.\n", __func__, fname);

  init_res_file ();
  TRACEOUTMESS("%s", fname);
}

void close_res_file (bool with_stats)
{
  TRACEINW("(%s)", boolstr(with_stats));
  assert (output_file);

  if (with_stats) {
    fprintf (output_file, "# Finished: %s\n", now_str(true));
    display_rusage (output_file);
  }
  if (fclose(output_file) != 0) cantcontinue("ERROR: %s: fclose failed.\n", __func__);
  output_file = NULL;
  TRACEOUT;
}

void config_for_underground ( void )
{
  if (output_file == NULL) {
    cantcontinue("Can't run underground: output file == NULL.\n");
  }
  stderr = output_file;
  if (daemon(1,0) != 0) {
    cantcontinue("daemon(1,0) failed. Can't run underground.\n");
  }
}

int main (int argc, char *argv[])
{
  if (time(&timer_start) == (time_t)-1)
    cantcontinue("ERROR: %s: Can't init timer_start.\n", __func__);

  set_value_for_height(DEFAULT_HEIGHT); /* default */
  set_value_for_max_dim(DEFAULT_MAX_DIM);    /* default */
  process_calling_arguments(argc,argv);

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
  TRACEMESS ("%s: Starting loop i=1..%d: Origin MASS = %d.", __func__, max_height, mass);
  for (i = mass+1; i <= max_height; i++) {
    add_grains_on_origin(1);
    goto_normal_form();
  }

  close_board ();

  if (with_data_file)
    close_res_file (true);
  if (cursing_mode) {
    wait_in_cursing();
    terminate_cursing();
  }
  exit (EXIT_SUCCESS);		/* end of program */
}
