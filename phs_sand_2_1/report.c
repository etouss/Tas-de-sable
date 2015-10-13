#ifndef MAKEDEPEND_IGNORE
#include <print.h> 		/* fail(); from libphs */

#include <stdlib.h>		/* free() */
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"
#include "version.h"

FILE * report_file = NULL;	/* the file for reports */

void record_normal_form (FILE * f)
{
  static time_t now;
  static int prev_nbsteps = 0;
  static int prev_diam = 0;
  if (with_data_file) {
    assert (f);
    switch (selected_report) {
    case NEW_MASS:
      fprintf (f, "M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", mass, nbsteps, diam, count0, count1, count2, count3);
      break;
    case NEW_AREA:
      fprintf (f, "A=%d M=%d T=%llu D=%d c0=%d c1=%d c2=%d c3=%d\n", area, mass, nbsteps, diam, count0, count1, count2, count3);
      display_the_board (f, false);
      break;
    default:
      cantcontinue("ERROR: %s: Unknown selected_report = %d.\n", __func__, selected_report);
    }
    //FIXME: we need a clearer policy of fflush for stats file.
    if ( (nbsteps - prev_nbsteps > K100) || (diam > prev_diam) )
      fflush(f);
  }
  if (snapshot_delay) {
    assert(snapshot_delay>0);
    if (time(&now) == (time_t)-1) cantcontinue("ERROR: %s: time() failed.\n", __func__);
    if (now > timer_start+snapshot_delay) {
      timer_start += snapshot_delay;
      take_snapshot();
    }
  }
  if (terminal_mode) {
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
  } else if (cursing_mode) {			/* cursing mode */
    display_cursing_mass_data();
  } else {
    assert(underground_mode);
  }
  prev_nbsteps = nbsteps; prev_diam = diam; /* record for next time */
}


/* The REPORT FILE is the file of results that gather all reports, usually one line per report */
void init_report_file ( void )
{
  if (report_file == NULL) {
    cantcontinue("ERROR: %s: output file not opened.\n", __func__);
  }
  fprintf (report_file, "# SAND v%s running on %s\n", SANDPILE_VERSION, machine_uname());
  output_calling_options (report_file, "# Called with: ", " ", "\n");
  fprintf (report_file, "# Started: %s\n", now_str(true));
}

void open_report_file ( void )
{
  TRACEIN;
  char * fname = NULL;
  char * jobname = NULL;
  char * frompart = NULL;

  switch(selected_job) {
  case PILE_JOB: asprintf(&jobname, "pile");break;
  case SQUARE_JOB: asprintf(&jobname, "sqr%ld", nb_toppling_grains);break;
  case DIAMOND_JOB: asprintf(&jobname, "diam%ld", nb_toppling_grains);break;
  case SPECIAL_JOB: asprintf(&jobname, "spec%ld", sp_job_number);break;
  default:
    cantcontinue("ERROR: %s: Doesn't know about job %d.\n", __func__, selected_job);
  }
  if (jobname == NULL)
    cantcontinue("ERROR: %s: asprintf could not alloc jobname.\n", __func__);
  
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

  report_file = fopen(fname, "w");
  if (report_file == NULL)
    cantcontinue("ERROR: %s: Could not open file %s.\n", __func__, fname);

  init_report_file ();
  TRACEOUTMESS("%s", fname);
  free(jobname); free(frompart); free(fname);
}

void close_report_file (bool with_stats)
{
  TRACEINW("(%s)", boolstr(with_stats));
  assert (report_file);

  if (with_stats) {
    fprintf (report_file, "# Finished: %s\n", now_str(true));
    display_rusage (report_file);
  }
  if (fclose(report_file) != 0) cantcontinue("ERROR: %s: fclose failed.\n", __func__);
  report_file = NULL;
  TRACEOUT;
}
