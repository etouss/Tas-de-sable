/* File: sand.h
 *
 * declarations for sandpile simulator
 */

#ifndef MAKEDEPEND_IGNORE
#include <limits.h> 		/* (U)INT_MAX */
#include <time.h> 		/* time_t */
#endif /* MAKEDEPEND_IGNORE */

#include "version.h"

/* useful constants: */
#define K1   1000
#define K10  10000
#define K100 100000
#define M1   1000000
#define M10  10000000
#define M100 100000000

#define MAX_ALLOWED_HEIGHT         INT_MAX
#define MAX_ALLOWED_DIM	           1000000
#define MAX_ALLOWED_ANIM_LEVEL     2
#define MAX_ALLOWED_SNAPSHOT_DELAY 3600*24*7 /* 1 week !!! */

#define DEFAULT_HEIGHT   100
#define DEFAULT_INIT_DIM 3 /* 31 / * odd value prefered */
#define DEFAULT_MAX_DIM  20000

#define DELAYAFTERDISPLAY 100000 /* 10^5 microsec = 0.1 second */

#define UNUSEDSQ INT_MIN

/* collapse.c */
extern int  curr_val (int x, int y);
extern void init_board (int some_dim);
extern void close_board ( void );
extern void display_the_board (FILE * stream);
extern void collapse (int x, int y);
extern void goto_normal_form ( void );
extern void increase_orig_mass (int delta);

/* display.c */
extern char char_for_val (int val);
extern void init_cursing ( void );
extern void terminate_cursing ( void );
extern void display_cursing_board ( void );
extern void display_cursing_dims ( void );
extern void display_cursing_mass_data ( void );
extern void wait_in_cursing ( void );
extern void try_curses ( void );/* FIXME: to be erased lated */
extern void prepare_cursing_message ( void );

/* options.c */
extern void process_options (int argc, char *argv[]);
extern void output_calling_options (FILE * f, const char * beg, const char * sep, const char * end);

/* time.c */
extern char * now_str (bool datetoo);
extern void display_rusage (FILE *stream);
extern int snapshot_delay; /* user input */
extern time_t timer_start; /* starting point for snapshot_delay */

/* MAIN */

extern bool terminal_mode;
extern bool cursing_mode;
extern bool underground_mode;
extern bool with_data_file;

#define AREA_JOB 1
#define TIME_JOB 2
#define DEFAULT_JOB TIME_JOB
extern int selected_job;	/* user input */
extern int anim_level;		/* user input */
extern int max_height;		/* user input */
extern int max_dim;		/* user input */
extern int xdim, ydim;
extern int xmin, xmax, ymin, ymax;

/* output data maintained by program */
extern int mass;
extern int current_dim;
extern int area;
extern int used_radius;
extern unsigned long long int nbsteps;
extern int count0;
extern int count1;
extern int count2;
extern int count3;

extern void set_value_for_height (int n);
extern void set_value_for_max_dim (int d);
extern void set_value_for_anim_level (int n);
extern void set_value_for_snapshot_delay (int x);
extern void record_normal_form (FILE * stream);
extern void display_initial_board ( void );
extern void display_this_board ( void );

/* cursing-smart fail message */
#define cantcontinue(...) do{if(cursing_mode){prepare_cursing_message();printw(__VA_ARGS__);wait_in_cursing();terminate_cursing();}else{fprintf(stderr,__VA_ARGS__);}fail();}while(0)
