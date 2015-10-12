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

#define FULL_PLANE_MODE 0
#define HALF_PLANE_MODE 1
#define QUARTER_PLANE_MODE 2
static int plane_mode = FULL_PLANE_MODE;//QUARTER_PLANE_MODE;

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

#define BOARDXOFFSET 0 /* OFFSET FOR BOARD IN CURSING */
#define BOARDYOFFSET 3 /* OFFSET FOR BOARD IN CURSING */

static int endx;/* coordinates of partial plan to display in cursing */
static int endy;
static int begx;
static int begy;

void set_display_ends ( void )
{
  endx = xmax; endy = ymax;
  if (plane_mode != FULL_PLANE_MODE)    begy = 0; else begy = ymin;
  if (plane_mode == QUARTER_PLANE_MODE) begx = 0; else begx = xmin;
}

void display_cursing_board ( void )
{
  int x, y, val;
  chtype ch;

  terminal_max_x = getmaxx (stdscr);    /* (re)initialize term dims */
  terminal_max_y = getmaxy (stdscr);    /* (re)initialize term dims */
  set_display_ends ();
  if (terminal_max_x < endx - begx + 3 + BOARDXOFFSET) {
    cantcontinue("Now need more than %d cols on your terminal.\n", terminal_max_x);
  }
  if (terminal_max_y < endy - begy + 3 + BOARDYOFFSET) {
    cantcontinue("Now need more than %d lines on your terminal.\n", terminal_max_y);
  }
  move (BOARDYOFFSET, BOARDXOFFSET);
  chtype corner = '+' | COLOR_PAIR(COLPAIR4TEXT) | A_BOLD; /*FIXME: compute once */
  chtype top = '-' | COLOR_PAIR(COLPAIR4TEXT) | A_BOLD;
  chtype bot = top;
  if (plane_mode != FULL_PLANE_MODE) {
    bot = '-' | COLOR_PAIR(COLPAIR4TEXT); /* no bold */
  }
  chtype right = '|' | COLOR_PAIR(COLPAIR4TEXT) | A_BOLD;
  chtype left = right;
  if (plane_mode == QUARTER_PLANE_MODE) {
    left = ':' | COLOR_PAIR(COLPAIR4TEXT); /* no bold */
  }
  addch(corner); for (x = begx; x <= endx; x++) addch(top); addch(corner);
  for (y = endy+1; y-- > begy;) { /* downward loop :-( */
    move (BOARDYOFFSET + 1 + endy - y, BOARDXOFFSET);
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
    addch(right);
  }
  move (BOARDYOFFSET + 1 + endy - y, BOARDXOFFSET);
  addch(corner);
  for (x = begx; x <= endx; x++)
    if ( (plane_mode == FULL_PLANE_MODE) || ((x-begx)%2 == 0) ) { /* every other one */
      addch(bot);
    } else {
      addch (' ');
    }
  addch(corner);
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
  if (anim_level == 0) {
    line4mess = 4;
  } else {/* a board may be displayed*/
    line4mess = endy - begy + BOARDYOFFSET + 4;
  }
  terminal_max_y = getmaxy (stdscr);    /* (re)initialize term dims */
  if (terminal_max_y - 1 < line4mess) {
    line4mess = terminal_max_y - 1;
  }
  attron(COLOR_PAIR(COLPAIR4TEXT));
  mvprintw(line4mess, 0, "Hit a key...");
  addch('_' | A_BLINK);
  getyx(stdscr, y, x);
  assert (x > 0); assert (y == line4mess);
  move(y, x-1);/* we want cursor on blinking _ */
  refresh();
  getch();
  mvprintw(line4mess, 0, "\t\t");//clear previous message
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
  switch (selected_job) {
  case TIME_JOB:
  mvprintw (1, 0, "M=%d T=%llu W=%d c0=%d c1=%d c2=%d c3=%d%s",
	    mass, nbsteps, diam, count0, count1, count2, count3, "\t\t" /*padding*/); break;
  case AREA_JOB:
  mvprintw (1, 0, "A=%d M=%d T=%llu W=%d c0=%d c1=%d c2=%d c3=%d%s",
	    area, mass, nbsteps, diam, count0, count1, count2, count3, "\t\t" /*padding*/); break;
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
