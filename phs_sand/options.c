#ifndef MAKEDEPEND_IGNORE
#include <stdlib.h> 	        /* exit, ..*/
#include <assert.h>
#include <getopt.h>		/* parsing args & options */
#include <limits.h>		/* UINT_MAX */

#include <print.h>	        /* from libphs */
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"
#include "version.h"

extern void parse_options (int argc, char *argv[]);
extern void display_help (FILE *stream);
extern void display_version (FILE *stream);

extern void display_help 	(FILE *stream);
extern void list_calling_options(FILE *stream);
extern void parse_ofile_arg     (char * arg);

static bool   display_help_opt       = false; /* -h. Causes exit */
static bool   display_flags_opt      = false; /* -l. Causes exit */
static bool   display_version_opt    = false; /* -v. Causes exit */
bool   fileaway_opt	      = false; /* -o <file> */

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

const char * _std_sand_name_ = "sand";
const char * _std_version_message_ = "%s version %s\n";
const char * _std_help_message_ = "\
Usage: %s [-hv] [-n <num>] [-d <num>] [-o file]\n\
\t-h : display this help message\n\
\t-v : display version and exit\n\
\t-g : turn on graphical mode\n\
\t-t <num> : set trace level\n\
\t-n <num> : set value for max mass (default = %u)\n\
\t-d <num> : set value for max board dim (default = %u)\n\
\t-o <filename> : output summary in file\n\
";


/* parsing arguments */
/*===================*/
void parse_options (int argc, char *argv[])
{
  int op;
  static unsigned int n_from_args;

  while ((op = getopt(argc, argv, "d:n:t:o:hvg")) != -1) {
    switch (op) {
    case 'h':
      display_help_opt = true;
      break;
    case 'v':
      display_version_opt = true;
      break;
    case 'g':
      cursing_mode = true;
      break;
    case 'o': /* expects path for output file */
      printf ("option -o not yet implemented\n");
      //parse_ofile_arg (optarg);
      break;
    case 't': /* expects numerical value for n */
      if (sscanf (optarg, "%u", &n_from_args) != 1) {
	fprintf (stderr, "ERROR: %s: \"-t %s\" is not an integer argument.\n", __func__, optarg);
	fail ();
      } else if (n_from_args > MAX_ALLOWED_ANIM_LEVEL) {
	fprintf (stderr, "ERROR: %s: \"-t %u\" is out of valid range 0..%u.\n", __func__, n_from_args, MAX_ALLOWED_ANIM_LEVEL);
	fail ();
      }
      set_value_for_anim_level (n_from_args);
      break;
    case 'n': /* expects numerical value for n */
      if (sscanf (optarg, "%u", &n_from_args) != 1) {
	fprintf (stderr, "ERROR: %s: \"-n %s\" is not an integer argument.\n", __func__, optarg);
	fail ();
      } else if (n_from_args > MAX_ALLOWED_HEIGHT) {
	fprintf (stderr, "ERROR: %s: \"-n %u\" is out of valid range 0..%u.\n", __func__, n_from_args, MAX_ALLOWED_HEIGHT);
	fail ();
      }
      set_value_for_height (n_from_args);
      break;
    case 'd': /* expects numerical value for d */
      if (sscanf (optarg, "%u", &n_from_args) != 1) {
	fprintf (stderr, "ERROR: %s: \"-d %s\" is not an integer argument.\n", __func__, optarg);
	fail ();
      } else if ((n_from_args > MAX_ALLOWED_DIM)) {
	fprintf (stderr, "ERROR: %s: \"-d %u\" is out of valid range 0..%u.\n", __func__, n_from_args, MAX_ALLOWED_DIM);
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
#ifdef HASHONNUMS
" -DHASHONNUMS"
#endif
#ifdef TRACE
" -DTRACE"
#endif
#ifdef DEEPTRACE
" -DDEEPTRACE"
#endif
#ifdef STATS
" -DSTATS"
#endif
#ifdef CONJ1
" -DCONJ1"
#endif
#ifdef TESTCONJ1
" -DTESTCONJ1"
#endif
" ";    /* space-terminated */

const char * compile_defs ( void )
{
  return _compile_def_str_;
}







static char* progname;
void process_options (int argc, char *argv[])
{
  progname = argv[0];

  parse_options (argc, argv);	/* first parse all. Crashing on errors */

  if (display_help_opt)   display_help (stdout);

  if (display_help_opt || display_flags_opt || display_version_opt) {
    display_version (stdout);
    exit (EXIT_SUCCESS);	/* exits on -h and -l */
  }
}
