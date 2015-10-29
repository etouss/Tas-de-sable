#ifndef MAKEDEPEND_IGNORE
#include <print.h>	        /* from libphs */

#include <stdlib.h> 	        /* exit, ..*/
#include <assert.h>
#include <getopt.h>		/* parsing args & options */
#include <string.h>		/* strcmp */
#include <regex.h>		/* regular expressions: regcomp() */
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"
#include "utils.h"
#include "version.h"

extern void parse_options (int argc, char *argv[]);
extern void display_help (FILE *stream);
extern void display_version (FILE *stream);
extern void parse_joption_arg (char * arg);
extern void parse_ofile_arg (char * arg);

static int    display_help_opt       = 0; /* -h Causes exit */
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
#define sand_optstring 		"hvj:ugGn:s:f:t:d:RT"
#define _std_help_message_ 	"\
Usage: %s [options]\n\
  -h\t\tPrint this message and exit. Longer help message with -h -h\n\
  -v\t\tPrint version number and exit\n\
  -n N\t\tSet value for max mass (default value = %d)\n\
  -g, -G\tUses curses graphic library for displaying boards and info\n\
  -j jobname\tChoose job (default job = pile). See longer help [-hh]\n\
  -u\t\tUnderground mode, only output to record file\n\
  -s N\t\tTake snapshot of board every N seconds (default value = no snapshots)\n\
  -f filename\tStart from given snapshot\n\
  -t N\t\tSet trace level (default value = 0 unless -g)\n\
"

#define _long_help_message_ 	"\
Usage: %s [options]\n\
  -hh\t\tPrint this message and exit\n\
  -h\t\tPrint shorter help message and exit\n\
  -n N\t\tSet value for max mass (default value = %d)\n\
  -g\t\tUses curses graphic library for displaying boards and info\n\
  -G\t\tLike -g but clip view of board when window is too small\n\
  -R, -T\tWith -gG: only display Right (Top) half, the two can be combined\n\
  -j jobname\tChoose job (default job = pile). Available jobs are\n\
\t\t  - pile\tgrains stacked on origin\n\
\t\t  - [B]square[N]\tB (default = 3) grains on each cell of (odd) square around origin\n\
\t\t\twith N (default = 1 when B = 3, 0 otherwise) extra grains added at origin\n\
\t\t  - [B]diamond[N]\tB (default = 3) grains on each cell of (even) lozenge around origin\n\
\t\t\twith N (default = 1 when B = 3, 0 otherwise) extra grains added at origin\n\
\t\t  - random[N]\tnew grains are added (pseudo-)randomly in odd square -N..+N\n\
\t\t\taround origin (default value = %d)\n\
\t\t  - conjN\tlike pile but test conjecture about N[i,j]>=N[i+1,j]\n\
\t\t  - special\tused to try out unusual configurations\n\
  -D\t\tCheat mode, don't collapse, useful for checking/viewing start configurations\n\
  -u\t\tUnderground mode, only output to record file\n\
  -s N\t\tTake snapshot of board every N seconds (default value = no snapshots)\n\
  -f filename\tStart from given snapshot\n\
  -t N\t\tSet trace level (default value = 0, or 1 with -gG)\n\
  -d N\t\tSet max board dim (default value = %d)\n\
"
// NOT YET IMPLEMENTED:
//\t-i : turn on interactive mode
//\t-o <filename> : choose name for report file


/* parsing arguments */
/*===================*/
static int argc = 0;
static char * * argv = NULL;

void fprintf_calling_opts (FILE * f, const char * beg, const char * sep, const char * end)
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
    case 'R': curse_only_right_half = true; break;
    case 'T': curse_only_top_half = true; break;
    case 'g':
      graphical_opt = true;
      curse_only_fitting = true; /* default, but needed to handle -G -g */
      if (anim_level == 0) set_value_for_anim_level(1); /* -g implies -t */
      break;
    case 'G':
      graphical_opt = true;
      curse_only_fitting = false;
      if (anim_level == 0) set_value_for_anim_level(1); /* -G implies -t */
      break;
    case 'h':
      display_help_opt++; break;
    case 'i':
      printf ("option -i not yet implemented\n");
      interactive_opt = true; break;
    case 'u':
      underground_opt = true; break;
    case 'v':
      display_version_opt = true; break;
    case 'j':
      assert(optarg); /* expects arg for job */
      parse_joption_arg(optarg);
      break;
    case 'o': /* expects path for record file */
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

#define NMATCHJOBS 4 /* number of subexps in rx4jobs */
static regex_t regex1;		/* FIXME: share with rx in board.c */
static regmatch_t matches[NMATCHJOBS];
/* BEWARE: when extending the following RX4SQDIJOBS. Check test for selected_job in parse_joption_arg*/
static const char *rx4sqdijobs = "^[ \t]*([0-9]*)(square|diamond)([0-9]*)[ \t]*$";

void parse_joption_arg (char * arg)
{
  if (strcmp(arg, "conjN") == 0) {
    selected_job = CONJ_JOB;
  } else if (strcmp(arg, "pile") == 0) {
    selected_job = PILE_JOB;
  } else if (regcomp(&regex1, rx4sqdijobs, REG_EXTENDED)) {
    cantcontinue("ERROR: %s: regcomp(rx4sqdijobs) does not compile.\n", __func__);
  } else if (0 == regexec(&regex1, arg, NMATCHJOBS, matches, 0)) {
    if (optarg[matches[2].rm_so] == 's') {
      selected_job = SQUARE_JOB;
    } else {			/* at the moment the rx leaves only 2 options */
      selected_job = DIAMOND_JOB;
    }
    TRACEMESS("%s matches rx4sqdijobs", arg);
    int mbeg = matches[1].rm_so;
    int mend = matches[1].rm_eo;
    if (mbeg < mend) {
      arg[mend]='\0';
      TRACEMESS("\t-jsquare|diamond has prefix arg %s", arg+mbeg);
      read_longint_val_in_range(arg+mbeg, &base_thickness, 0, MAX_ALLOWED_BASETHICKNESS, "base_thickness");
    }  else {
      base_thickness = (selected_job == SQUARE_JOB
			? DEFAULT_SQUARE_THICKNESS
			: DEFAULT_DIAMOND_THICKNESS);
    }
    mbeg = matches[3].rm_so;
    mend = matches[3].rm_eo;
    if (mbeg < mend) {
      arg[mend]='\0';
      TRACEMESS("\t-jsquare|diamond has suffix arg %s", arg+mbeg);
      read_longint_val_in_range(arg+mbeg, &nb_toppling_grains, 0, MAX_ALLOWED_TOPPLING, "nb_toppling_grains");
    } else if (selected_job == SQUARE_JOB) {
      nb_toppling_grains = (base_thickness == DEFAULT_SQUARE_THICKNESS ? 1 : 0); /* default */
    } else {
      nb_toppling_grains = (base_thickness == DEFAULT_DIAMOND_THICKNESS ? 1 : 0); /* default */
    }
    TRACEMESS("-jsquare|diamond: thickness=%ld toppling=%ld", base_thickness, nb_toppling_grains);
  } else if (strncmp(arg, "random", 6) == 0) {
    selected_job = RANDOM_JOB;
    if (arg[6] == '\0') {
      random_radius = DEFAULT_RANDOM_RADIUS;
    } else {
      read_longint_val_in_range(arg+6, &random_radius, 0, MAX_ALLOWED_RANDOM_RADIUS, "random_radius");
    }
    TRACEMESS("arg \"%s\" gives random_radius = %ld.", arg, random_radius);
  } else if (strncmp(arg, "special", 7) == 0) {
    selected_job = SPECIAL_JOB;
    if (arg[7] != '\0') {
      read_longint_val_in_range(arg+7, &sp_job_number, 0, MAX_SPECIAL_JOB, "sp_job_number");
    }
    TRACEMESS("arg \"%s\" gives sp_job_number = %ld.", arg, sp_job_number);
  } else {
    cantcontinue ("Option \"-j %s\" not understood.\n", arg);
  }
}

void display_help (FILE *stream)
{
  if (display_help_opt <= 1) {	/* short help message */
    fprintf (stream, _std_help_message_, _std_sand_name_, DEFAULT_HEIGHT);
  } else { /* long help message */
    fprintf (stream, _long_help_message_, _std_sand_name_, DEFAULT_HEIGHT,
	     DEFAULT_RANDOM_RADIUS, DEFAULT_MAX_DIM);
  }
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

  if (display_version_opt || display_help_opt) display_version (stdout);
  if (display_help_opt) display_help (stdout);
  if (display_help_opt || display_version_opt) {
    TRACEOUT;
    exit (EXIT_SUCCESS);	/* exits on -h and -v */
  }
  if (graphical_opt) {
    if (underground_opt) {
      cantcontinue("Graphical -g, -G and underground -u modes incompatible.\n");
    } else {
      display_mode = CURSING_MODE;
    }
  }
  if (underground_opt) {
    if (! with_data_file) {
      cantcontinue("Underground -u mode requires data_file mode.\n");
    }
    if (strcmp(machine_uname(), SERVER_UNAME) == 0) {
      if (strncmp(cwd(), REQUIRED_DIR_ON_SERVER, sizeof(REQUIRED_DIR_ON_SERVER) - 1) != 0) {
	cantcontinue("Underground -u mode on machine %s required being in dir %s.\n", SERVER_UNAME, REQUIRED_DIR_ON_SERVER);
      }
    }
    display_mode = UNDERGROUND_MODE;
  } /* underground_mode */
  if (display_mode != CURSING_MODE && (curse_only_top_half || curse_only_right_half)) {
    cantcontinue("-R or -T require -g or -G.\n");
  }

  TRACEOUT;
}

