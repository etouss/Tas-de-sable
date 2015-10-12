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
#include "version.h"

/* local vars: */

bool terminal_mode = true;	/* default mode */
bool cursing_mode = false;
bool underground_mode = false;
bool cheat_opt = false;
bool with_data_file = true;
bool from_snapshot_mode = false;

enum job_t selected_job = DEFAULT_JOB;	      /* user input */
enum report_t selected_report = DEFAULT_REPORT;	/* user input */
const char * snapshot_source_file_arg = NULL; /* user input */
int anim_level = 0;		/* user input */
int max_height = 0;		/* user input */
int max_dim = 0;		/* user input */
long int nb_toppling_grains = 0; /* for SQUARE, DIAMOND, .. */
long int sp_job_number = 0;	 /* for SPECIAL */
bool asymmetrical_job = false;	 /* by default */

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
  sync();/* finished writing */;
  if (fclose (f) != 0)
    cantcontinue("ERROR: %s: fclose failed.\n", __func__);
  TRACEOUT;
}

void deal_with_normal_form ( void )
{
  TRACEINW(": mass=%d", mass);
#ifdef DEBUG_BINARY_BOARD
  dump_current_memmatrix (stdout);
#endif /* DEBUG_BINARY_BOARD */
  assert (asymmetrical_job || xmax - xmin == diam - 1);
  assert (asymmetrical_job || ymax - ymin == diam - 1);
  assert (asymmetrical_job || xmin + xmax == 0);
  assert (asymmetrical_job || ymin + ymax == 0);
  if (cheat_opt == false) assert (stabilized_p());
  assert ( (stabilized_p() == false) || (mass == count1 + 2 * count2 + 3 * count3) );
  assert ( (stabilized_p() == false) || (area == count0 + count1 + count2 + count3) );
  static int prev_area = -1;
  switch (selected_report) {
  case NEW_MASS:
    record_normal_form (report_file);	       /* on -o file */
    if (anim_level > 0) display_this_board (); /* and on screen */
    break;
  case NEW_AREA:
    if (area > prev_area) {
      record_normal_form (report_file);		 /* on -o file */
      if (anim_level > 0) display_this_board (); /* and on screen */
    }
    break;
  default:
    cantcontinue("ERROR: %s: Unknown selected_report = %d.\n", __func__, selected_report);
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

void config_for_underground ( void )
{
  if (report_file == NULL) {
    cantcontinue("Can't run underground: no report file?!\n");
  }
  stdout = stderr = report_file; /* we dont want to lose any output */
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
    open_report_file ();

  if (underground_mode)  /* underground after open_report_file */
    config_for_underground ();

  init_board (DEFAULT_INIT_DIM);

  int i=0;
  int sqmass, j, k;
  switch(selected_job) {
  case PILE_JOB:
    while (mass < max_height) {
      add_grains_on_origin(1);
      goto_normal_form();
    }
    break;
  case SQUARE_JOB:
    sqmass = 0;
    i = -1;
    for (k = 0; k < nb_toppling_grains; k++)
      add_grains_on_origin(1);
    sqmass = nb_toppling_grains;
    while (sqmass < max_height) {
      if (i++ == -1) {		/* base case */
	for (k = 0; k < 3; k++) add_grains_on_origin(1);
	assert(mass == 3+nb_toppling_grains);
	//REMOVE	display_the_board(stdout, true);
      } else {		/* inductive step */
	for (j = -i; j < i; j++) {
	  for (k = 0; k < 3; k++) add_grains_on_square(i, j, 1);
	  for (k = 0; k < 3; k++) add_grains_on_square(j, -i, 1);
	  for (k = 0; k < 3; k++) add_grains_on_square(-i, -j, 1);
	  for (k = 0; k < 3; k++) add_grains_on_square(-j, i, 1);
	}
	//REMOVE	display_the_board(stdout, true);
	assert(mass == nb_toppling_grains+3*(2*i+1)*(2*i+1));
	sqmass=mass;
      }
      goto_normal_form();
    }
    break;
  case DIAMOND_JOB:
    sqmass = 0;
    i = -1;
    for (k = 0; k < nb_toppling_grains; k++)
      add_grains_on_origin(1);
    sqmass = nb_toppling_grains;
    while (sqmass < max_height) {
      if (i++ == -1) {		/* base case */
	for (k = 0; k < 3; k++) add_grains_on_origin(1);
	assert(mass == 3+nb_toppling_grains);
	//REMOVE: display_the_board(stdout, true);
      } else {		/* inductive step */
	for (j = 0; j < i; j++) {
	  for (k = 0; k < 3; k++) add_grains_on_square(i-j, j, 1);
	  for (k = 0; k < 3; k++) add_grains_on_square(-j, i-j, 1);
	  for (k = 0; k < 3; k++) add_grains_on_square(j-i, -j, 1);
	  for (k = 0; k < 3; k++) add_grains_on_square(j, j-i, 1);
	}
	//REMOVE: display_the_board(stdout, true);
	assert(mass == nb_toppling_grains+3*(2*i*i+2*i+1));
	sqmass=mass;
      }
      goto_normal_form();
    }
    break;
  case SPECIAL_JOB:
    /* later we'll discriminate w/r sp_job_number.
       For the moment we just play with possibilities  */
    asymmetrical_job = true;	/* defuse asserts written for piles etc. */
    while (mass < max_height) {
      for (i = -1; i <= 1; i++)
	for (j = i; j <= 1; j++)
	  add_grains_on_square(i,j,1);
      add_grains_on_square(3,0,1);
      //REMOVE: display_the_board(stdout, true);
      goto_normal_form();
    }
    break;
  default:
    cantcontinue("Sorry. Job %d not yet supported.\n", selected_job);
  }

  close_board ();

  if (with_data_file)
    close_report_file (true);
  if (cursing_mode) {
    wait_in_cursing();
    terminate_cursing();
  }
  exit (EXIT_SUCCESS);		/* end of program */
}
