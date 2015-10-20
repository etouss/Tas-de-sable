/* File: display.c

   DESCRIPTION: handle "cursing_mode", i.e. the low-level animation and graphics built with ncurses.

   */

#ifndef MAKEDEPEND_IGNORE
#include <print.h> /* before stdbool.h can be included */

#include <curses.h>
#include <locale.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"

bool curse_only_top_half = false;
bool curse_only_right_half = false;
bool curse_only_fitting = true;

/* terminal coords are in [0,max_y) X [0,max_x) */
static int terminal_max_x = 0;
static int terminal_max_y = 0;

/* local functions: */
extern void paint_blank_screen ( void );

/* Get/update terminal X/Y dims for cursing */
void get_stdscr_size ( void )
{
  static int prev_term_max_x = -1;
  static int prev_term_max_y = -1;
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
  if (terminal_max_x != prev_term_max_x || terminal_max_y != prev_term_max_y) {
    if (prev_term_max_x >= 0 || prev_term_max_y >= 0) /* dims changed after init */
      paint_blank_screen ();
    prev_term_max_x = terminal_max_x; prev_term_max_y = terminal_max_y;
  }
}

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

#define BOARDXOFFSET 0 /* OFFSET FOR BOARD IN CURSING */
#define BOARDYOFFSET 3 /* OFFSET FOR BOARD IN CURSING */

static int begx, endx, begy, endy;
/* coords (inclusive) of full/half/quarter plan ⊆ ℤxℤ to be displayed */

void set_display_ends ( void )
{
  begx = curse_only_right_half ? 0 : xmin;
  begy = curse_only_top_half ? 0 : ymin;
  endx = xmax;
  endy = ymax;
}

/* CURSES: DISPLAY BOARD IN TERMINAL
 *
 * We use BOARDYOFFSET lines for messages on top of terminal. Zero at bottom.
 */

void display_cursing_board ( void )
{
  int x, y, val;
  chtype ch;
  /* init chars */
  chtype corner = '+' | COLOR_PAIR(COLPAIR4TEXT) | A_BOLD; /*FIXME: compute once */
  chtype top = '-' | COLOR_PAIR(COLPAIR4TEXT) | A_BOLD;
  chtype bot = curse_only_top_half
    ? (bot = '-' | COLOR_PAIR(COLPAIR4TEXT))  /* no bold */
    : top;
  chtype right = '|' | COLOR_PAIR(COLPAIR4TEXT) | A_BOLD;
  chtype left = curse_only_right_half
    ? (':' | COLOR_PAIR(COLPAIR4TEXT)) /* no bold */
    : right;
  /* init dims */
  get_stdscr_size();    /* (re)initialize term dims */
  set_display_ends ();
  /* draw */
  bool skipright = false, skiptop = false;
  if (endx > terminal_max_x + begx - 3 - BOARDXOFFSET) {
    if (curse_only_fitting) {
      cantcontinue("Now need more than %d cols on your terminal.\n", terminal_max_x);
    } else {
      skipright = true;
      endx = terminal_max_x + begx - 2 - BOARDXOFFSET; /* new end col */
    }
  }
  if (endy > terminal_max_y + begy - 3 - BOARDYOFFSET) {
    if (curse_only_fitting) {
      cantcontinue("Now need more than %d lines on your terminal.\n", terminal_max_y);
    } else {
      skiptop = true;
      endy = terminal_max_y + begy - 2  - BOARDYOFFSET; /* new end line */
    }
  }
  /* draw bottom frame line: */
  move (terminal_max_y - 1, BOARDXOFFSET);
  addch(corner);
  for (x = begx; x <= endx; x++)
    if ( ! curse_only_top_half || ((x-begx)%2 == 0) ) { /* every other one */
      addch(bot);
    } else {
      addch (' ');
    }
  if (! skipright) addch(corner);
  /* draw lines */
  for (y = begy; y <= endy; y++) {
    move (terminal_max_y - 2 + begy - y, BOARDXOFFSET);
    addch(left);
    for (x = begx; x <= endx; x++) {
      val = curr_val(x,y);
      ch = char_for_val(val);
      switch (val) {
      case 0: case 1: ch |= COLOR_PAIR(COLPAIR1VAL);break;
      case 2:         ch |= COLOR_PAIR(COLPAIR2VAL);break;
      case 3:         ch |= COLOR_PAIR(COLPAIR3VAL) | A_BOLD;break;
      case 4:         ch |= COLOR_PAIR(COLPAIR4VAL);break;
      default:        ch |= COLOR_PAIR(COLPAIR5VAL);break;
      }
      addch(ch);
    }
    if (! skipright) addch(right);
  }

  /* draw top frame line */
  if (! skiptop) {
    move (terminal_max_y - 2 + begy - y, BOARDXOFFSET);
    addch(corner);
    for (x = begx; x <= endx; x++) addch(top);
    if (! skipright) addch(corner);
  }

  refresh ();
}

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

void test_cursing_background (bool wait)
{
  int i;
  for (i = 0; i < terminal_max_x; i=i+2) {
    mvprintw(0, i, "#");
    mvprintw(terminal_max_y - 1, i, "#");
  }
  mvprintw(0, terminal_max_x/2 - 4, "Top line ");
  mvprintw(terminal_max_y - 1, terminal_max_x/2 - 6, "Bottom line ");
  for (i = 0; i < terminal_max_y; i=i+2) {
    mvprintw(i, 0, "#");
    mvprintw(i, terminal_max_x - 1, "#");
  }
  if (wait) wait_in_cursing();
}

void paint_blank_screen ( void )
{
  int i,j;
  for (j = 0; j < terminal_max_y; j++) {
    mvprintw(j, 0, "");
    for (i = 0; i < terminal_max_x; i++) {
      addch(' ');
    }
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
  //   test_cursing_background (false); /* used while debugging */
  refresh();
  TRACEOUT;
}

void terminate_cursing ( void )
{
  TRACEIN;
  endwin ();
  TRACEOUT;
}

void prepare_cursing_message ( void )
{
  attron(COLOR_PAIR(COLPAIR4TEXT));
  move(2,0);
}

void wait_in_cursing ( void )
{
  int line4mess, prev_curs, x, y;
  set_display_ends ();
  prev_curs = curs_set(1);		/* make cursor visible (2 = very visible) */
  if (prev_curs == ERR) {
    terminate_cursing ();
    fprintf(stderr, "ERROR: %s: Can't make cursor very visible\n", __func__);
    fail();
  }
  terminal_max_y = getmaxy (stdscr);    /* (re)initialize term dims */
  line4mess = terminal_max_y - 1;

  attron(COLOR_PAIR(COLPAIR4TEXT));
  mvprintw(line4mess, 0, "Hit a key...");
  addch('_' | A_BLINK);
  getyx(stdscr, y, x);
  assert (x > 0); assert (y == line4mess);
  move(y, x-1);			/* we want cursor on blinking _ */
  refresh();
  flushinp();			/* clear input buffer */
  getch();			/* wait for char */
  mvprintw(line4mess, 0, "\t\t");/* clear previous message */
  curs_set(prev_curs);
  refresh();
}

void display_cursing_dims ( void )
{
  attron(COLOR_PAIR(COLPAIR4TEXT));
  mvprintw (0, 0, "xmin=%d xmax=%d ymin=%d ymax=%d%s",
	    xmin, xmax, ymin, ymax, "\t\t" /*padding*/);
  refresh();
}

void display_cursing_mass_data ( void )
{
  attron(COLOR_PAIR(COLPAIR4TEXT));/* text */
  switch (selected_report) {
  case NEW_MASS:
    mvprintw (1, 0, "M=%d T=%llu W=%d c0=%d c1=%d c2=%d c3=%d%s",
	      mass, nbsteps, diam, count0, count1, count2, count3, "\t\t" /*padding*/);
    break;
  case NEW_AREA:
    mvprintw (1, 0, "A=%d M=%d T=%llu W=%d c0=%d c1=%d c2=%d c3=%d%s",
	      area, mass, nbsteps, diam, count0, count1, count2, count3, "\t\t" /*padding*/);
    break;
  default:
    cantcontinue("ERROR: %s: Unknown selected_report = %d.\n", __func__, selected_report);
  }
  refresh();
}

char char_for_val (int val)
{/* FIXME: could be faster :-( */
  DEEPTRACEINW("(%d)", val);
  char res = '\0';
  if (val == 0) {
    res = '.';
  } else if (val == UNUSEDSQ) {
#ifndef DEBUG_BINARY_BOARD
    res = ' ';
#else
    res = '#';			/* more visible */
#endif /* DEBUG_BINARY_BOARD */
  } else if (val <= 9) {
    res = '0' + val;
  } else if (val <= 35) {
    res = 'A' + val - 10;
  } else if (val > 0) {
    res = '?';
  } else {
    cantcontinue("ERROR: %s: can't handle val=%d.\n", __func__, val);
  }
  DEEPTRACEOUTW("%c", res);
  return res;
}
