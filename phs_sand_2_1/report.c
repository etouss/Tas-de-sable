#ifndef MAKEDEPEND_IGNORE
#include <print.h> 		/* fail(); from libphs */

#include <stdlib.h>		/* free() */
#include <string.h>		/* strlen(), strcpy() */
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"
#include "version.h"

/* We try to use "RECORDING" and "REPORTING" consistently.
   - RECORDING means writing on the RECORD_FILE,
   - REPORTING is more general, it includes deciding to RECORD or not, and whether to involve the user.

   The reporting functions are called after specific events (at the moment REPORT_NORMAL_FORM or REPORT_COLLAPSE). They decide when/what to report and record. This involves story history data like "last area reported" etc.
*/

void report_collapse ( void )
{
  if (anim_level >= 2) { ;		/* do nothing */
    /* not yet written */
  }
}

void report_normal_form (bool initial_board)
{
  /* history variables: local count between two normal forms */
  static int prev_area = -1;
  // not yet used static int prev_diam = -1;
  // not yet used static unsigned long long int prev_nbsteps = 0;
  TRACEINW(": mass=%d", mass);
#ifdef DEBUG_BINARY_BOARD
  dump_current_memmatrix (stdout);
#endif /* DEBUG_BINARY_BOARD */
  assert (asymmetrical_job || xmax - xmin == diam - 1);
  assert (asymmetrical_job || ymax - ymin == diam - 1);
  assert (asymmetrical_job || xmin + xmax == 0);
  assert (asymmetrical_job || ymin + ymax == 0);
  assert (cheat_opt == true || stabilized_p());
  assert (stabilized_p() == false || mass == count1 + 2 * count2 + 3 * count3);
  assert (stabilized_p() == false || area == count0 + count1 + count2 + count3);
  /* 1. RECORD */
  switch (selected_report) {
  case NEW_MASS:
    record_normal_form (record_file);	       /* on -o file */
    if (anim_level > 0) display_this_board (); /* and on screen */
    break;
  case NEW_AREA:
    if (area > prev_area) {
      record_normal_form (record_file);		 /* on -o file */
      if (anim_level > 0) display_this_board (); /* and on screen */
    }
    break;
  default:
    cantcontinue("ERROR: %s: Unknown selected_report = %d.\n", __func__, selected_report);
  }
  /* 2. SNAPSHOTS */
#ifdef FORCE_SNAPSHOTS
  take_snapshot();
#else /* normal process */
  if (snapshot_delay) {
    static time_t now;
    assert(snapshot_delay>0);
    if (time(&now) == (time_t)-1) cantcontinue("ERROR: %s: time() failed.\n", __func__);
    if (now > timer_start+snapshot_delay) {
      timer_start += snapshot_delay;
      take_snapshot();
    }
  }
#endif /* FORCE_SNAPSHOTS */
  /* 3. DISPLAY */
  if (display_mode == TERMINAL_MODE) {
    switch (selected_report) {
    case NEW_MASS:
      printf ("! Normal form for M=%d: T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", mass, nbsteps, diam, count0, count1, count2, count3 );
      break;
    case NEW_AREA:
      printf ("! Normal form for A=%d: M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", area, mass, nbsteps, diam, count0, count1, count2, count3 );
      break;
    default:
      cantcontinue("ERROR: %s: Unknown selected_report = %d.\n", __func__, selected_report);
    }
  } else if (display_mode == CURSING_MODE) {			/* cursing mode */
    if (initial_board)
      display_cursing_dims();	/* dims displayed when changed */
    display_cursing_mass_data();
  } else {
    assert(display_mode == UNDERGROUND_MODE);
  }
  /* MAINTAIN HISTORY */
  //  prev_nbsteps = nbsteps; prev_diam = diam;
  prev_area = area;		/* update */

  /* DELAY AFTER DISPLAY */
  if (display_mode == CURSING_MODE) {
    //FIXME: no display delay in interactive mode
    if (anim_level < 2) {
      if (mass < 200) {
	usleep(DELAYAFTERDISPLAY);
      } else {					/* delays add up in the long run */
	usleep(DELAYAFTERDISPLAY * 200 / mass); /* decreasing delay */
      }
    } else {
      wait_in_cursing (false);
    }
  }
  TRACEOUT;
}

/* The RECORD_FILE is the main record we compute and archive.
   Not every normal form is recorded,
   the record for one normal form depends on the job at hand.
*/
FILE * record_file = NULL;	/* the file for reports */

void record_normal_form (FILE * f)
{
  /* history variables: local count between two normal forms */
  static int prev_diam = -1;
  static unsigned long long int nbsteps_at_prev_flush = 0;
  if (with_data_file) {
    assert (f);
    switch (selected_report) {
    case NEW_MASS:
      fprintf (f, "M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", mass, nbsteps, diam, count0, count1, count2, count3);
      break;
    case NEW_AREA:
      fprintf (f, "A=%d M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", area, mass, nbsteps, diam, count0, count1, count2, count3);
      break;
    default:
      cantcontinue("ERROR: %s: Unknown selected_report = %d.\n", __func__, selected_report);
    }
    //FIXME: we need a clearer policy of fflush for stats file.
    //FIXME: also, this function is called for snapshot headers
    if ( diam > prev_diam || nbsteps > K100 + nbsteps_at_prev_flush ) {
      fflush(f);
      nbsteps_at_prev_flush = nbsteps; /* update history */
    }
  }
  prev_diam = diam;		/* update history */
}


/* The RECORD FILE is the file of results that gather all reports, usually one line per report */
void init_record_file ( void )
{
  if (record_file == NULL) {
    cantcontinue("ERROR: %s: record file not opened.\n", __func__);
  }
  fprintf (record_file, "# SAND v%s running on %s\n", SANDPILE_VERSION, machine_uname());
  fprintf_calling_opts (record_file, "# Called with: ", " ", "\n");
  fprintf (record_file, "# Started: %s\n", now_str(true));
}

void open_record_file ( void )
{
  TRACEIN;
  char * fname = NULL;
  char * frompart = NULL;
  char * jobname = jobname_string();

  if (from_snapshot_mode) {
    long int mass_sn;
    get_mass_from_snapshot(snapshot_source_file_arg, &mass_sn);
    asprintf(&frompart, "from_M%ld", mass_sn);
  } else {
    asprintf(&frompart, "");
  }
  if (frompart == NULL)
    cantcontinue("ERROR: %s: asprintf could not alloc frompart.\n", __func__);

  switch (selected_report) {
  case NEW_MASS:
    asprintf (&fname, "stats_sand_v%s_%s_T%sjqa_M%d.txt", SANDPILE_VERSION, jobname, frompart, max_height);
    break;
  case NEW_AREA:
    asprintf (&fname, "stats_sand_v%s_%s_A%sjqa_M%d.txt", SANDPILE_VERSION, jobname, frompart, max_height);
    break;
  default:
    cantcontinue("ERROR: %s: Unknown selected_report = %d.\n", __func__, selected_report);
  }
  if (fname == NULL)
    cantcontinue("ERROR: %s: asprintf could not alloc fname.\n", __func__);

  record_file = fopen(fname, "w");
  if (record_file == NULL)
    cantcontinue("ERROR: %s: Could not open file %s.\n", __func__, fname);

  init_record_file ();
  TRACEOUTMESS("%s", fname);
  free(frompart); free(fname);
}

void close_record_file (bool with_stats)
{
  TRACEINW("(%s)", boolstr(with_stats));
  assert (record_file);

  if (with_stats) {
    fprintf (record_file, "# Finished: %s\n", now_str(true));
    display_rusage (record_file);
  }
  if (fclose(record_file) != 0) cantcontinue("ERROR: %s: fclose failed.\n", __func__);
  record_file = NULL;
  TRACEOUT;
}

/* low-level util */
char * jobname_string ( void )
{
  static char * jn_str = NULL;	/* build it once */
  TRACEINW("jn_str now: %p", jn_str);
  if (jn_str == NULL) {
    switch(selected_job) {
    case CONJ_JOB:    asprintf(&jn_str, "conjN"); break;
    case PILE_JOB:    asprintf(&jn_str, "pile"); break;
    case SQUARE_JOB:  asprintf(&jn_str, "%ldsqr%ld", base_thickness, nb_toppling_grains); break;
    case DIAMOND_JOB: asprintf(&jn_str, "%lddiam%ld", base_thickness, nb_toppling_grains); break;
    case RANDOM_JOB:  asprintf(&jn_str, "rand%ld", random_radius); break;
    case SPECIAL_JOB: asprintf(&jn_str, "spec%ld", sp_job_number); break;
    default:
      cantcontinue("ERROR: %s: Doesn't know about job %d.\n", __func__, selected_job);
    }
    if (jn_str == NULL)
      cantcontinue("ERROR: %s: asprintf could not alloc jobname.\n", __func__);
  }
  TRACEOUTW("\"%s\"", jn_str);
  return jn_str;
}
