#ifndef MAKEDEPEND_IGNORE
#include <print.h>	        /* from libphs */

#include <stdlib.h> 	        /* exit, ..*/
#include <assert.h>
#include <getopt.h>		/* parsing args & options */
#include <string.h>		/* strcmp */
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"
#include "utils.h"
#include "version.h"

extern void parse_options (int argc, char *argv[]);
extern void display_help (FILE *stream);
extern void display_version (FILE *stream);

extern void display_help 	(FILE *stream);
extern void parse_ofile_arg     (char * arg);

static bool   display_help_opt       = false; /* -h Causes exit */
//static bool   display_flags_opt      = false; /* -l Causes exit */
static bool   display_version_opt    = false; /* -v Causes exit */
static bool   underground_opt	     = false; /* -u Detach from terminal */
static bool   graphical_opt	     = false; /* -g Show with curses */
static bool   interactive_opt	     = false; /* -i Keys control showing flow */
bool          fileaway_opt           = false; /* -o <file> NOT YET IMPLEMENTED */

static char * opt_ofile_path = NULL;

void parse_ofile_arg (char * arg)
{
  if (fileaway_opt != false) {
    fprintf (stderr, "ERROR: %s: can't accept clashing options -o %s .. -o %s\n",
	     __func__, opt_ofile_path, arg);
    fail();
  }
  opt_ofile_path = arg;
  fileaway_opt = true;
}

#define _std_sand_name_ 	"sand"
#define _std_version_message_ 	"%s version %s\n"
#define sand_optstring 		"hvj:ugn:s:f:t:d:"
#define _std_help_message_ 	"\
Usage: %s [-n <num>] ..\n\
\t-h : display this help message\n\
\t-v : display version and exit\n\
\t-j <jobname> : select job among { pile (= default), squareNNN, diamondNNN }\n\
\t\tNumeric option NNN is number of extra grains (default = 1)\n\
\t-u : underground mode, only output to logfile\n\
\t-g : turn on graphical mode\n\
\t-n <num> : set value for max mass (default = %d)\n\
\t-s <num> : take a snapshot every <num> seconds (default = don't)\n\
\t-f <filename> : start from given snapshot\n\
\t-t <num> : set trace level\n\
\t-d <num> : set value for max board dim (default = %d)\n\
"
// NOT YET IMPLEMENTED:
//\t-i : turn on interactive mode
//\t-o <filename> : output summary in file


/* parsing arguments */
/*===================*/
static int argc = 0;
static char * * argv = NULL;

void output_calling_options (FILE * f, const char * beg, const char * sep, const char * end)
{
  int i;
  fputs (beg, f);
  for (i = 0; i < argc; i++) {
    if (i != 0) fputs(sep, f);
    fputs(argv[i], f);
  }
  fputs(end, f);
}

#define sand_cheatopts "D"
#define sand_fulloptsstring sand_optstring""sand_cheatopts

void parse_options (int actual_argc, char *actual_argv[])
{
  int op;
  static int n_from_args;
  argc = actual_argc; argv = actual_argv;
  TRACEIN;
  while ((op = getopt(argc, argv, sand_fulloptsstring)) != -1) {
    switch (op) {
    case 'D': cheat_opt = true; break;
    case 'g':
      graphical_opt = true; break;
    case 'h':
      display_help_opt = true; break;
    case 'i':
      printf ("option -i not yet implemented\n");
      interactive_opt = true; break;
    case 'u':
      underground_opt = true; break;
    case 'v':
      display_version_opt = true; break;
    case 'j':
      assert(optarg);
      if (strcmp(optarg, "pile") == 0) {
	selected_job = PILE_JOB;
      } else if (strncmp(optarg, "square", 6) == 0) {
	selected_job = SQUARE_JOB;
	if (optarg[6] == '\0') {
	  nb_toppling_grains = 1;	/* default */
	} else {
	  read_longint_val_in_range(optarg+6, &nb_toppling_grains, 0, MAX_ALLOWED_TOPPLING, "nb_toppling_grains");
	}
	TRACEMESS("optarg \"%s\" gives nb_toppling = %ld.", optarg, nb_toppling_grains);
      } else if (strncmp(optarg, "diamond", 7) == 0) {
	selected_job = DIAMOND_JOB;
	if (optarg[7] == '\0') {
	  nb_toppling_grains = 1;	/* default */
	} else {
	  read_longint_val_in_range(optarg+7, &nb_toppling_grains, 0, MAX_ALLOWED_TOPPLING, "nb_toppling_grains");
	}
	TRACEMESS("optarg \"%s\" gives nb_toppling = %ld.", optarg, nb_toppling_grains);
      } else if (strncmp(optarg, "special", 7) == 0) {
	selected_job = SPECIAL_JOB;
	if (optarg[7] != '\0') {
	  read_longint_val_in_range(optarg+7, &sp_job_number, 0, MAX_SPECIAL_JOB, "sp_job_number");
	}
	TRACEMESS("optarg \"%s\" gives sp_job_number = %ld.", optarg, sp_job_number);
      } else {
	cantcontinue ("Option \"-j %s\" not understood.\n", optarg);
      }
      break;
    case 'o': /* expects path for output file */
      assert(optarg);
      printf ("option -o not yet implemented\n");
      //parse_ofile_arg (optarg);
      break;
    case 'f': /* expects path for snapshot file */
      assert(optarg);
      snapshot_source_file_arg = optarg;
      from_snapshot_mode = true;
      break;
    case 's': /* expects numerical value for n */
      assert(optarg);
      if (sscanf (optarg, "%d", &n_from_args) != 1) {/*FIXME: prefer strtol*/
	fprintf (stderr, "ERROR: %s: \"-s %s\" is not an integer argument.\n", __func__, optarg);
	fail ();
      } else if ((n_from_args < 0) || (n_from_args > MAX_ALLOWED_SNAPSHOT_DELAY)) {
	fprintf (stderr, "ERROR: %s: \"-s %d\" is out of valid range 0..%d.\n", __func__, n_from_args, MAX_ALLOWED_SNAPSHOT_DELAY);
	fail ();
      }
      set_value_for_snapshot_delay (n_from_args);
      break;
    case 't': /* expects numerical value for n */
      assert(optarg);
      if (sscanf (optarg, "%d", &n_from_args) != 1) {/*FIXME: prefer strtol*/
	fprintf (stderr, "ERROR: %s: \"-t %s\" is not an integer argument.\n", __func__, optarg);
	fail ();
      } else if ((n_from_args < 0) || (n_from_args > MAX_ALLOWED_ANIM_LEVEL)) {
	fprintf (stderr, "ERROR: %s: \"-t %d\" is out of valid range 0..%d.\n", __func__, n_from_args, MAX_ALLOWED_ANIM_LEVEL);
	fail ();
      }
      set_value_for_anim_level (n_from_args);
      break;
    case 'n': /* expects numerical value for n */
      assert(optarg);
      if (sscanf (optarg, "%d", &n_from_args) != 1) {/*FIXME: prefer strtol*/
	fprintf (stderr, "ERROR: %s: \"-n %s\" is not an integer argument.\n", __func__, optarg);
	fail ();
      } else if  ((n_from_args < 0) || (n_from_args > MAX_ALLOWED_HEIGHT)) {
	fprintf (stderr, "ERROR: %s: \"-n %d\" is out of valid range 0..%d.\n", __func__, n_from_args, MAX_ALLOWED_HEIGHT);
	fail ();
      }
      set_value_for_height (n_from_args);
      break;
    case 'd': /* expects numerical value for d */
      assert(optarg);
      if (sscanf (optarg, "%d", &n_from_args) != 1) {/*FIXME: prefer strtol*/
	fprintf (stderr, "ERROR: %s: \"-d %s\" is not an integer argument.\n", __func__, optarg);
	fail ();
      } else if  ((n_from_args < 0) || (n_from_args > MAX_ALLOWED_DIM)) {
	fprintf (stderr, "ERROR: %s: \"-d %d\" is out of valid range 0..%d.\n", __func__, n_from_args, MAX_ALLOWED_DIM);
	fail ();
      }
      set_value_for_max_dim (n_from_args);
      break;
    case '?':
    default:
      fprintf (stderr, "ERROR: %s: Cannot parse optional args\n", __func__);
      display_help (stderr);
      fail ();
    }
  }
  if (argc > optind) {
    fprintf (stderr, "ERROR: %s: Does not know about arg \"%s\"\n",
	     __func__, argv[optind]);
    fail();
  }
  TRACEOUT;
}

void display_help (FILE *stream)
{
  fprintf (stream, _std_help_message_, _std_sand_name_,
	   DEFAULT_HEIGHT, DEFAULT_MAX_DIM);
}

void display_version (FILE *stream)
{
  fprintf (stream, _std_version_message_, _std_sand_name_, SANDPILE_VERSION);
}

/* COMPILE_DEFS */
/*==============*/
/* We build at compile-time a string like " -DOPT -DNUM "
   with an extra space in front and at end */
static const char * _compile_def_str_ = ""
#ifdef TRACE
" -DTRACE"
#endif
#ifdef DEEPTRACE
" -DDEEPTRACE"
#endif
#ifdef STATS
" -DSTATS"
#endif
" ";    /* space-terminated */

const char * compile_defs ( void )
{
  return _compile_def_str_;
}

static char* progname;

void process_calling_arguments (int argc, char *argv[])
{
TRACEIN;
  progname = argv[0];

  parse_options (argc, argv);	/* first parse all. Crashing on errors */

  /* After we have parsed the optional arguments, we set the right flags accordingly. This is the place where
     we do sanity checks *before* launching the actual computations. */

  if (display_version_opt) display_version (stdout);
  if (display_help_opt) display_help (stdout);

  if (display_help_opt || display_version_opt) {
    TRACEOUT;
    exit (EXIT_SUCCESS);	/* exits on -h and -l */
  }
  if (graphical_opt) {
    if (underground_opt) {
      cantcontinue("Graphical -g and underground -u modes incompatible.\n");
    } else {
      cursing_mode = true;
      terminal_mode = false;
    }
  }
  if (underground_opt) {
    if (with_data_file) {
      underground_mode = true;
      terminal_mode = false;
    } else {
      cantcontinue("Underground -u mode requires data_file mode.\n");
    }
    if (strcmp(machine_uname(), SERVER_UNAME) == 0) {
      if (strncmp(cwd(), REQUIRED_DIR_ON_SERVER, sizeof(REQUIRED_DIR_ON_SERVER) - 1) != 0) {
	cantcontinue("Underground -u mode on machine %s required being in dir %s.\n", SERVER_UNAME, REQUIRED_DIR_ON_SERVER);
      }
    }
  }
  TRACEOUT;
}

