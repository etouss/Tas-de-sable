/* File: sand.h
 *
 * declarations for sandpile simulator
 */

#ifndef MAKEDEPEND_IGNORE
#include <limits.h> 		/* (U)INT_MAX */
#include <time.h> 		/* time_t */
#include <curses.h>		/* used in cantcontinue */
#endif /* MAKEDEPEND_IGNORE */

#include "trace.h"

/* useful constants: */
#define K1   1000
#define K10  10000
#define K100 100000
#define M1   1000000
#define M10  10000000
#define M100 100000000

#define SERVER_UNAME           "sume"
#define REQUIRED_DIR_ON_SERVER "/local/phs"

#define MAX_ALLOWED_HEIGHT         INT_MAX
#define MAX_ALLOWED_DIM	           1000000
#define MAX_ALLOWED_TOPPLING       INT_MAX / 10
#define MAX_ALLOWED_ANIM_LEVEL     2
#define MAX_ALLOWED_SNAPSHOT_DELAY 3600*24*7 /* 1 week !!! */

#define DEFAULT_HEIGHT   100
#define DEFAULT_INIT_DIM 3 /* 31 / * odd value prefered */
#define DEFAULT_MAX_DIM  20000

#define DELAYAFTERDISPLAY 100000 /* 10^5 microsec = 0.1 second */

#define UNUSEDSQ INT_MIN

/* board.c */
extern int xmin, xmax, ymin, ymax; /* dimensions for virtual board */
extern int mass, diam, area;	   /* current vals */
extern unsigned long long int nbsteps;
extern int count0, count1, count2, count3;
extern bool stabilized_p ( void );
extern void get_mass_from_snapshot (const char * path, long int * mass_var);

/* collapse.c */
extern int  curr_val (int x, int y);
extern void init_board (int some_dim);
extern void close_board ( void );
extern void display_the_board (FILE * stream, bool withvars);
extern void collapse (int x, int y);
extern void goto_normal_form ( void );
extern void add_grains_on_origin (int delta);
extern void add_grains_on_square (int x, int y, int delta);

/* display.c */
extern char char_for_val (int val);
extern void init_cursing ( void );
extern void terminate_cursing ( void );
extern void display_cursing_board ( void );
extern void display_cursing_dims ( void );
extern void display_cursing_mass_data ( void );
extern void wait_in_cursing ( void );
extern void prepare_cursing_message ( void );

/* memmat.c */
extern bool memmatrix_inited ();
extern int  memmatrix_xdim ();
extern int  memmatrix_ydim ();
extern int* memmatrix (int i, int j);
extern void realloc_memmatrix (int xincr, int yincr, int xshift, int yshift, int valnews);
extern void dump_current_memmatrix (FILE * stream);


/* options.c */
extern void process_calling_arguments (int argc, char *argv[]);
extern void output_calling_options (FILE * f, const char * beg, const char * sep, const char * end);

/* report.c */
extern FILE * report_file;
extern void open_report_file ( void );
extern void record_normal_form (FILE * stream);
extern void close_report_file (bool with_stats);

/* system.c */
extern const char * machine_uname ( void );
extern const char * cwd ( void ); /* current working dir */

/* time.c */
extern char * now_str (bool datetoo);
extern void display_rusage (FILE *stream);
extern int snapshot_delay; /* user input */
extern time_t timer_start; /* starting point for snapshot_delay */

/* MAIN */

extern bool terminal_mode;
extern bool cursing_mode;
extern bool underground_mode;
extern bool cheat_opt;
extern bool with_data_file;
extern bool from_snapshot_mode;

enum job_t { PILE_JOB, SQUARE_JOB, DIAMOND_JOB, SPECIAL_JOB };
#define DEFAULT_JOB PILE_JOB
extern enum job_t selected_job;	/* user input */
extern bool asymmetrical_job;

enum report_t { NEW_MASS, NEW_AREA, NEW_STEP };
#define DEFAULT_REPORT NEW_MASS
extern enum report_t selected_report; /* user input */

extern const char * snapshot_source_file_arg; /* user input */
extern int anim_level;		/* user input */
extern int max_height;		/* user input */
extern int max_dim;		/* user input */
extern long int nb_toppling_grains;     /* for SQUARE, DIAMOND, .. */
#define MAX_SPECIAL_JOB 0
extern long int sp_job_number;

extern void set_value_for_height (int n);
extern void set_value_for_max_dim (int d);
extern void set_value_for_anim_level (int n);
extern void set_value_for_snapshot_delay (int x);
extern void take_snapshot ( void );
extern void display_initial_board ( void );
extern void display_this_board ( void );

/* cursing-smart fail message */
#define cantcontinue(...) do{if(cursing_mode){prepare_cursing_message();printw(__VA_ARGS__);wait_in_cursing();terminate_cursing();}else{fprintf(stderr,__VA_ARGS__);}fail();}while(0)
