#ifndef MAKEDEPEND_IGNORE
#include <print.h> /* before stdbool.h can be included */

#include <curses.h>
#include <locale.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "trace.h"
#include "sand.h"

static int terminal_max_x = 0;
static int terminal_max_y = 0;

#define COLPAIR4TEXT   1	/* color for text */
#define COLPAIR1VAL    2	/* color for 1 and below */
#define COLPAIR2VAL    3  	/* color for 2 */
#define COLPAIR3VAL    4  	/* color for 3 */
#define COLPAIR4VAL    5  	/* color for 4 */
#define COLPAIR5VAL    6  	/* color for 5 and above */

/* new names for the new colors */
#define BLUE1 COLOR_CYAN /* will be redefined hence renamed */
#define BLUE2 COLOR_MAGENTA /* will be redefined hence renamed */
#define BLUE3 COLOR_BLUE /* will be redefined hence renamed */
#define BLUE4 COLOR_YELLOW /* will be redefined hence renamed */
#define BLUE5 COLOR_GREEN /* will be redefined hence renamed */

void init_colored_cursing ()
{
  TRACEIN;
  if (has_colors() == false) {
    terminate_cursing();
    fprintf(stderr, "Your terminal does not support colors.\n");
    fail();
  }
  if (can_change_color() == false) {
    terminate_cursing();
    fprintf(stderr, "Your terminal does not support change_color.\n");
    fail();
  }
  start_color ();
  assert (COLOR_PAIRS > 10);

  /* RGB vals found in http://cloford.com/resources/colours/500col.htm */
  init_color(BLUE1, 202, 225, 255);
  init_color(BLUE2, 72, 118, 255);
  init_color(BLUE3, 65, 105, 225);
  init_color(BLUE4, 61, 89, 171);
  init_color(BLUE5, 25, 25, 112);

  init_pair(COLPAIR4TEXT, COLOR_BLACK, -1);
  init_pair(COLPAIR1VAL,  BLUE1,       -1);
  init_pair(COLPAIR2VAL,  BLUE2,       -1);
  init_pair(COLPAIR3VAL,  BLUE3,       -1);
  init_pair(COLPAIR4VAL,  BLUE4,       -1);
  init_pair(COLPAIR5VAL,  BLUE5,       -1);

  attron(COLOR_PAIR(COLPAIR4TEXT));
  TRACEOUT;
}

void get_stdscr_size ( void )
{
    terminal_max_x = getmaxx (stdscr); /* initialize term dims */
    terminal_max_y = getmaxy (stdscr); /* initialize term dims */
    if (terminal_max_x < 40) {
      terminate_cursing();
      fprintf(stderr, "Your terminal is too small (< 40 cols).\n");
      fail();
    }
    if (terminal_max_y < 10) {
      terminate_cursing();
      fprintf(stderr, "Your terminal is too small (< 10 lines).\n");
      fail();
    }
}

void init_cursing ( void )
{
  TRACEIN;
  TRACEWAITFORCHAR;
  setlocale(LC_ALL, "");
  initscr();
  get_stdscr_size ();
  use_default_colors();
  cbreak(); noecho();		/* disable line buffering and echoing */
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);		/* interpret arrow keys etc. */
  if (curs_set(0) == ERR) {	/* make cursor invisible */
    terminate_cursing ();
    fprintf(stderr, "ERROR: %s: Can't make cursor invisible\n", __func__);
    fail();
  }
  init_colored_cursing ();
  refresh();
  TRACEOUT;
}

void terminate_cursing ( void )
{
  TRACEIN;
  endwin ();
  TRACEOUT;
}

void wait_in_cursing ( void )
{
  int prev_curs = curs_set(2);		/* very visible */
  if (prev_curs == ERR) {
    terminate_cursing ();
    fprintf(stderr, "ERROR: %s: Can't make cursor very visible\n", __func__);
    fail();
  }
  attron(COLOR_PAIR(COLPAIR4TEXT));
  mvprintw(2, 0, "hit a key");
  refresh();
  getch();
  curs_set(prev_curs);
  refresh();
}

void display_cursing_dims ( void )
{
  attron(COLOR_PAIR(COLPAIR4TEXT));
  mvprintw (0, 0, "Xmin: %d, Xmax: %d, Ymin=%d, Ymax=%d%s",
	    xmin, xmax, ymin, ymax, "\t\t" /*padding*/);
  refresh();
}

void display_cursing_mass_data ( void )
{
  attron(COLOR_PAIR(COLPAIR4TEXT));/* text */
  mvprintw (1, 0, "T=%llu, M=%u, nb3=%u, nb2=%u, nb1=%u%s",
	    nbsteps, mass, count3, count2, count1, "\t\t" /*padding*/);
  refresh();
}

#define BOARDXOFFSET 0
#define BOARDYOFFSET 3
void display_cursing_board ( void )
{
  int i, j;
  unsigned int val;
  char ch;
  char ch0 = char_for_val(0);

  if (terminal_max_x < (int)(xmax - xmin + 3 + BOARDXOFFSET)) {
    terminate_cursing();
    fprintf(stderr, "Now need more than %d cols on your terminal.\n", terminal_max_x);
    fail();
  }
  if (terminal_max_y < (int)(ymax - ymin + 3 + BOARDYOFFSET)) {
    terminate_cursing();
    fprintf(stderr, "Now need more than %d lines on your terminal.\n", terminal_max_y);
    fail();
  }

  move (BOARDYOFFSET, BOARDXOFFSET);
  attron(COLOR_PAIR(COLPAIR1VAL));
  for (i = xmin; i <= xmax+2; i ++) {
    addch(ch0);
  }
  for (j = ymax+1; j-- > ymin;) { /* downward loop :-( */
    move (BOARDYOFFSET + 1 + ymax - j, BOARDXOFFSET);
    addch(ch0);
    for (i = xmin; i <= xmax; i++) {
      val = the_board[i+xdepl][j+ydepl];
      ch = char_for_val(val);
      switch (val) {
      case 0: case 1: attron(COLOR_PAIR(COLPAIR1VAL));break;
      case 2: attron(COLOR_PAIR(COLPAIR2VAL));break;
      case 3: attron(COLOR_PAIR(COLPAIR3VAL));break;
      case 4: attron(COLOR_PAIR(COLPAIR4VAL));break;
      default: attron(COLOR_PAIR(COLPAIR5VAL));break;
      }
      addch(ch);
    }
    addch(ch0);
  }
  move (BOARDYOFFSET + 1 + ymax - j, BOARDXOFFSET);
  attron(COLOR_PAIR(COLPAIR1VAL));
  for (i = xmin; i <= xmax+2; i ++) {
    addch(ch0);
  }
  refresh ();
}
