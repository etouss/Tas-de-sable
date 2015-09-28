/* File: sand.h
 *
 * declarations for sandpile simulator
 */

#define MAX_ALLOWED_HEIGHT         UINT_MAX
#define MAX_ALLOWED_DIM	           1000000
#define MAX_ALLOWED_ANIM_LEVEL     2

#define DEFAULT_HEIGHT   100
#define DEFAULT_INIT_DIM 3 /* 31 / * odd value prefered */
#define DEFAULT_MAX_DIM  300

#define DELAYAFTERDISPLAY 100000 /* 10^5 microsec = 0.1 second */

/* display.c */
extern void init_cursing ( void );
extern void terminate_cursing ( void );
extern void display_cursing_board ( void );
extern void display_cursing_dims ( void );
extern void display_cursing_mass_data ( void );
extern void wait_in_cursing ( void );
extern void try_curses ( void );/* FIXME: to be erased lated */

/* options.c */
extern void process_options (int argc, char *argv[]);

/* time.c */
extern char * now_str (bool datetoo);
extern void display_rusage (FILE *f);

/* MAIN */
extern unsigned int mass;
extern unsigned long long int nbsteps;
extern int xdim, ydim;
extern int xmin, xmax, ymin, ymax;
extern int xdepl, ydepl;
extern unsigned int * * the_board;
extern unsigned int used_radius, count1, count2, count3;

extern void set_value_for_anim_level (unsigned int n);
extern void set_value_for_height (unsigned int n);
extern void set_value_for_max_dim (unsigned int d);
extern char char_for_val (unsigned int val);
extern bool cursing_mode;
