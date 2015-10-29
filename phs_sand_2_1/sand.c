#ifndef MAKEDEPEND_IGNORE
#include <print.h> 		/* fail(); from libphs */

#include <errno.h>
#include <stdlib.h> 		/* daemon, alloc & free */
#include <alloca.h> 		/* alloca */
#include <time.h>		/* time() */
#include <stdio.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

//#define TRACE

#include "sand.h"
#include "version.h"

/* local vars: */

//bool terminal_mode = true;	/* default mode */
//bool cursing_mode = false;
//bool underground_mode = false;
enum display_t display_mode = UNDEF_DT;
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
long int base_thickness = 0;	/* for SQUARE, DIAMOND, .. */
long int sp_job_number = 0;	/* for SPECIAL */
bool asymmetrical_job = false;	/* by default */

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
  char * jobname = jobname_string();
  assert(jobname);
  char * buff = alloca(150);
  if (errno == ENOMEM) cantcontinue("ERROR: %s can't alloc for string buff.\n", __func__);
  assert (buff);
  /* snapshot filename and contents do not depend on selected_job */
  sprintf (buff, "snapshot_sand_v%s_%s_M%d.txt", SANDPILE_VERSION, jobname, mass);
  TRACEMESS("Will now open file \"%s\"", buff);
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

void display_initial_board ( void )
{
  TRACEIN;
  if (display_mode == CURSING_MODE) {
    display_cursing_dims ();
  }
  display_this_board ();
  TRACEOUT;
}

void display_this_board ( void )
{
  TRACEIN;
  if (display_mode == CURSING_MODE) {
    display_cursing_board ();
  } else {
    display_the_board(stdout, false);
  }
  TRACEOUT;
}

void config_for_underground ( void )
{
  if (record_file == NULL) {
    cantcontinue("Can't run underground: no record file?!\n");
  }
  stdout = stderr = record_file; /* we dont want to lose any output */
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
  display_mode = TERMINAL_MODE;		     /* default */
  process_calling_arguments(argc,argv);

#ifdef TRACE
  if (display_mode == CURSING_MODE) {
    fprintf (stderr, "Cannot use cursing_mode when compiled with -DTRACE.\n");
    fprintf (stderr, "I disable it.\n");
    display_mode = TERMINAL_MODE;
    if (isatty(fileno(stdout))) { /* message PRESS ENTER on stdout */
      TRACEWAITFORCHAR;
    }
  }
#endif /* TRACE */
  if (display_mode == CURSING_MODE)
    init_cursing();

  if (with_data_file)
    open_record_file ();

  if (display_mode == UNDERGROUND_MODE)  /* config_for_underground */
    config_for_underground ();		 /* AFTER open_record_file */

  init_and_report_board (DEFAULT_INIT_DIM);

  int i=0;
  int sqmass, j, k, radius;

  switch(selected_job) {
  case CONJ_JOB:			/* like PILE but extra record */
    if (from_snapshot_mode) {
      cantcontinue("Can't do -f when recording N[i,j]\n");
    }
    /* branch into PILE_JOB */
  case PILE_JOB:
    while (mass < max_height) {
      add_grains_on_origin(1);
      normalize_and_report();
    }
    break;
  case RANDOM_JOB:
    asymmetrical_job = true;	/* defuse asserts written for piles etc. */
    radius = (random_radius >= INT_MAX ? INT_MAX : (int)random_radius);
    while (mass < max_height) {
      TRACEMESS("Current M=%d", mass);
      add_grain_on_random_square(radius, mass); /* use mass as random seed */
      normalize_and_report();
    }
    break;
  case SQUARE_JOB:
    sqmass = 0;
    i = -1;
    assert (base_thickness >= 0 && nb_toppling_grains >= 0);
    for (k = 0; k < nb_toppling_grains; k++)
      add_grains_on_origin(1);
    sqmass = nb_toppling_grains;
    while (sqmass < max_height) {
      if (i++ == -1) {		/* base case */
	for (k = 0; k < base_thickness; k++) add_grains_on_origin(1);
	assert(mass == base_thickness+nb_toppling_grains);
      } else {		/* inductive step */
	for (j = -i; j < i; j++) {
	  for (k = 0; k < base_thickness; k++) {
	    add_grains_on_square(i, j, 1);
	    add_grains_on_square(j, -i, 1);
	    add_grains_on_square(-i, -j, 1);
	    add_grains_on_square(-j, i, 1);
	  }
	}
	assert(mass == nb_toppling_grains+base_thickness*(2*i+1)*(2*i+1));
	sqmass=mass;
      }
      normalize_and_report();
    }
    break;
  case DIAMOND_JOB:
    sqmass = 0;
    i = -1;
    assert (base_thickness >= 0 && nb_toppling_grains >= 0);
    for (k = 0; k < nb_toppling_grains; k++)
      add_grains_on_origin(1);
    sqmass = nb_toppling_grains;
    while (sqmass < max_height) {
      if (i++ == -1) {		/* base case */
	for (k = 0; k < base_thickness; k++) add_grains_on_origin(1);
	assert(mass == base_thickness+nb_toppling_grains);
      } else {		/* inductive step */
	for (j = 0; j < i; j++) {
	  for (k = 0; k < base_thickness; k++) {
	    add_grains_on_square(i-j, j, 1);
	    add_grains_on_square(-j, i-j, 1);
	    add_grains_on_square(j-i, -j, 1);
	    add_grains_on_square(j, j-i, 1);
	  }
	}
	assert(mass == nb_toppling_grains+base_thickness*(2*i*i+2*i+1));
	sqmass=mass;
      }
      normalize_and_report();
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
      normalize_and_report();
    }
    break;
  default:
    cantcontinue("Sorry. Job %d not yet supported.\n", selected_job);
  }

  close_board ();

  if (with_data_file)
    close_record_file(true);
  if (display_mode == CURSING_MODE) {
    wait_in_cursing(false);
    terminate_cursing();
  }
  exit (EXIT_SUCCESS);		/* end of program */
}
