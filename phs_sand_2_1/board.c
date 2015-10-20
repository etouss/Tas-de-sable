/* File: board.c

   DESCRIPTION: manages the evolution of the board: collapses, normalization, counting grains, etc.

   */
#ifndef MAKEDEPEND_IGNORE
#include <print.h> 		/* fail(); from libphs */

#include <errno.h>
#include <regex.h>		/* regular expressions: regcomp() */
#include <stdlib.h> 		/* alloc, malloc & free */
#include <alloca.h> 		/* alloca */
#include <curses.h>
#include <stdio.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

//#define TRACE
//#define DEBUG_BINARY_BOARD

#include "sand.h"
#include "utils.h"

/* THE LOGICAL BOARD stores a part [xmin..xmax] x [ymin..ymax] of B[ℤxℤ], the logical space.
   <xdepl,ydepl> are the coords in memmatrix of the logical (0,0)
The variables describing it at all times are: */
static bool board_inited = false;
int xmin = -1;			/* coord of extremal virtual points */
int xmax = -1;
int ymin = -1;
int ymax = -1;
int diam = -1;			/* longest distance */

/* The link with memmatrix is given by the following local variables */
static int xdim, ydim; /* dimension of memmatrix */
static int xdepl, ydepl;/* coord in memmatrix of virtual (0,0) origin */
/* A pointer CBOARD points to virtual origin, memmatrix(xdepl,ydepl) */
static int * cboard = NULL;	/* not yet inited */

/* The board also has vital statistics, maintained by program */
int mass = 0;			/* total number of grains */
int count0 = 0;			/* number of squares that have been emptied */
int count1 = 0;			/* number of squares that contain 1 */
int count2 = 0;			/* number of squares that contain 2 */
int count3 = 0;			/* number of squares that contain 3 */
int area = 0;			/* number of used squares */
unsigned long long int nbsteps = 0;

void display_board_vars (FILE * f)
{
  fprintf(f, "Underlying memmat:\n");
  fprintf(f, "\txdim = %d, ydim = %d\n", xdim, ydim);
  fprintf(f, "\txdepl = %d, ydepl = %d\n", xdepl, ydepl);
  fprintf(f, "Envelop:\n");
  fprintf(f, "\txmin = %d, xmax = %d\n", xmin, xmax);
  fprintf(f, "\tymin = %d, ymax = %d\n", ymin, ymax);
  fprintf(f, "\tdiam = %d\n", diam);
  fprintf(f, "Contents:\n");
  fprintf(f, "\tmass = %d, area = %d\n", mass, area);
  fprintf(f, "\tc0=%d, c1=%d, c2=%d, c3=%d\n", count0, count1, count2, count3);
}

/* local functions: */
extern void add_grains_on_square (int x, int y, int delta);
extern void enlarge_envelop_for (int x, int y);
extern void report_normal_form ( void );
extern void init_board_contents_from_snapshot (const char * path, bool justheader);
extern void get_res_filename_from_snapshot (const char * path);
extern void init_waitlist ( void );
extern void dump_waitinglist (FILE * stream, bool statstoo);

void init_board (int some_dim)
{
  TRACEIN;
  assert (cboard == NULL);
  assert (some_dim <= MAX_ALLOWED_DIM);
  if (board_inited != false) {
    cantcontinue ("ERROR: %s: board was already inited?!\n", __func__);
  }
  init_waitlist ();
  realloc_memmatrix (some_dim, some_dim, 0, 0, UNUSEDSQ); /* set board vars properly. No copy made since board was not inited */
  xdim = ydim = some_dim;
  xdepl = ydepl = some_dim / 2;
  xmin = xmax = 0; ymin = ymax = 0; diam = 1;
  cboard = memmatrix(xdepl,ydepl);
  mass = 0; nbsteps = 0; count1 = count2 = count3 = 0;
  area = count0 = 0;		/* init count0 */
  add_grains_on_square(0,0,0);	/* set orig as "used" */
  if (from_snapshot_mode) {
    init_board_contents_from_snapshot (snapshot_source_file_arg, false);
  }
  if (anim_level > 0) display_initial_board ();
  TRACESIGNEDMESS ("xdim=%d ydim=%d", xdim, ydim);
  TRACEOUT;
}

/* blindly double all dimensions */
void enlarge_underlying_mat ( void )
{
  TRACEIN;
  assert (xdim == memmatrix_xdim());
  assert (ydim == memmatrix_ydim());
#ifdef TRACE
  int prev_xdim = xdim;
  int prev_ydim = ydim;
#endif /* TRACE */
  realloc_memmatrix (xdim+1, ydim+1, 1+xdim/2, 1+ydim/2, UNUSEDSQ);
  xdepl += 1+xdim/2;
  ydepl += 1+ydim/2;
  xdim  += xdim+1;
  ydim  += ydim+1;
  cboard = memmatrix(xdepl,ydepl);
  assert (xdim == memmatrix_xdim()); assert (ydim == memmatrix_ydim());
  TRACEOUTW ("was %dx%d, now is %dx%d", prev_xdim, prev_ydim, memmatrix_xdim(), memmatrix_ydim());
}

/* THE LOGICAL BOARD is an infinite board but only points "in range", i.e. between xmin..xmax and ymin..ymax can be touched.

   NOTE: Collapsing at extremal positions may enlarge the range.

   NOTATION CONVENTION: We use (i,j) for physical & (x,y)∈ℤxℤ for logical/absolute coords

   The physical and the logical boards are connected via
   B[x,y] ~ ph_board[(x+xdepl)*ydim + y+ydepl]
*/
/* outside modules can only use CURR_VAL to read the board. */
int curr_val (int x, int y)
{
  assert (x >= xmin); assert (y >= ymin);
  assert (x <= xmax); assert (y <= ymax);
  assert (cboard);		/* inited */
  return cboard[x*ydim + y];
}

/* while collapsing we have a waiting list of squares to process */
static bool waitlist_inited = false;
static size_t waiting_nbslots = 0;
static int * waitinglist = NULL;
static int * wait_curr = NULL;
static int * wait_max = NULL;
#define STARTING_STACK_SIZE ((size_t)1000) /* suitable for non-challenging runs */
/* #define MAX_STACK_SIZE ((size_t)5*K10) : Crashes for M=6257180 */
#define MAX_STACK_SIZE ((size_t)M1) /* 10^6 */
void alloc_waitlist (size_t nbslots)
{
  TRACEINW("(%zu)", nbslots);
  // dump_waitinglist (stdout, true);
  if (nbslots <= waiting_nbslots) {
    cantcontinue("ERROR: %s(%zu): arg is below current=%zu.\n", __func__, nbslots, waiting_nbslots);
  } else if (nbslots > MAX_STACK_SIZE) {
    cantcontinue("ERROR: %s(%zu): arg is largest than max allowed=%zu.\n", __func__, nbslots, MAX_STACK_SIZE);
  }
  int delta = (waitlist_inited ? wait_curr - waitinglist : 0);
  waitinglist = realloc(waitinglist, nbslots * 2 * sizeof(int));
  if (errno == ENOMEM) {
    cantcontinue("ERROR: %s(%zu): alloc failed.\n", __func__, nbslots);
    /* NB: if realloc fails, old waitinglist has not been free'd */
  }
  assert (waitinglist != NULL);
  waiting_nbslots = nbslots;
  wait_max = waitinglist + nbslots*2;
  wait_curr = waitinglist + delta;
  //  dump_waitinglist (stdout, true);
  TRACEOUT;
}

bool stabilized_p ( void )
{
  return (wait_curr == waitinglist ? true : false);
}

void init_waitlist ( void )
{
  TRACEIN;
  if (waitlist_inited) {
    /* do nothing */
  } else {
    alloc_waitlist (STARTING_STACK_SIZE);
    waitlist_inited = true;
  }
  TRACEOUT;
}

void close_waitinglist ( void )
{
  if (waitlist_inited) {
    free(waitinglist);
    waitlist_inited = false;
    waitinglist = wait_curr = wait_max = NULL;
  }
}

void stackin_waiting (int x, int y)
{
  if (wait_curr == wait_max) {
    alloc_waitlist(waiting_nbslots + waiting_nbslots/2); /* increase by 50% */
  }
  *(wait_curr++) = x;		 /* push x 1st, see popping */
  assert (wait_curr < wait_max);/* need no test since we alloced even number */
  *(wait_curr++) = y;
}

/* ADD_GRAINS_ON_SQUARE modifies the board by adding DELTA (can be 0) on X,Y.
   Maintains counts. No collapse.
   New val ≥ 4 are stacked in waiting list (no checking if new val ≠ prev val)
   Prev val ≥ 4 are assumed to have been unstacked by caller */
void add_grains_on_square (int x, int y, int delta)
{
  TRACEINW("(x=%d, y=%d, delta=%d)", x, y, delta);
  //  dump_waitinglist(stdout, true);
  if ((x < xmin) || (x > xmax) || (y < ymin) || (y > ymax)) {
    TRACEMESS ("have to enlarge");
    enlarge_envelop_for (x,y);
  }
  int * p = cboard + x*ydim + y;
  int val = *p;
  assert ((val + delta >= 0) || (val == UNUSEDSQ));
  switch (val) {//update count
  case 0: assert(count0 > 0); count0--; break;
  case 1: assert(count1 > 0); count1--; break;
  case 2: assert(count2 > 0); count2--; break;
  case 3: assert(count3 > 0); count3--; break;
  case UNUSEDSQ: val=0; area++; break; /* needs real val in later computation */
  default: assert(val>3); /* do nothing */
  }
  mass += delta;
  val += delta;
  *p = val;
  switch (val) {
  case 0: assert(count0 < INT_MAX); count0++; break;
  case 1: assert(count1 < INT_MAX); count1++; break;
  case 2: assert(count2 < INT_MAX); count2++; break;
  case 3: assert(count3 < INT_MAX); count3++; break;
  default:
    assert(val>3);
    /* only stackin when val goes 3->4, 7->8, ... Other options are 5->6, 8->4 etc. */
    if ((val%4 == 0) && (delta == 1)) stackin_waiting(x,y);
    break;
  }
#ifdef TRACE
  if ((display_mode != TERMINAL_MODE) && (anim_level >= 2))
    dump_waitinglist(stdout, false);
#endif /* TRACE */
  TRACEOUT;
}

void collapse_and_report (int x, int y)
{
  assert (cboard[x*ydim+y] > 3);
  TRACEINW("(x=%d, y=%d)", x, y);
  add_grains_on_square (x,y,-4);
  add_grains_on_square (x+1,y,1);
  add_grains_on_square (x-1,y,1);
  add_grains_on_square (x,y+1,1);
  add_grains_on_square (x,y-1,1);
  if (nbsteps == ULLONG_MAX) {
    cantcontinue("ERROR: Max number of steps UINTMAX = %llu reached. I must stop.\n", ULLONG_MAX);
  }
  nbsteps++;
  report_collapse (); 
  TRACEOUT;
}

void normalize_and_report ( void )
{
  TRACEIN;
  if (cheat_opt) goto skip_all;

  int x, y, val;
  while (wait_curr > waitinglist) { /* MAIN LOOP: collapse until stable */
    assert (wait_curr >= waitinglist+2);
    y = *(--wait_curr);		/* pop y first, see pushing */
    x = *(--wait_curr);
    val = cboard[x*ydim+y];
#ifdef TRACE /* FIXME: used for debugging waitinglist. Will be removed later */
    if ((display_mode != TERMINAL_MODE) && (anim_level >= 2))
      printf ("We popped B[%d,%d]=%d\n", x, y, val);
#endif /* TRACE */
    if (val > 3) {
      collapse_and_report(x,y);
    } else if (display_mode == TERMINAL_MODE) {
      printf ("! False positive in waitinglist: x=%d y=%d val=%d\n",
	      x, y, cboard[x*ydim+y]);
    }
  }
#ifndef NDEBUG
  // FIXME: REMOVE LATER
  // SENSIBLE SAFETY CHECK FOR CATCHING BUGS, BUT COSTLY ON LARGE BOARDS
  bool finished = false;
  int count = 0;
  int startx = 0;
  int starty = 0;
  if (asymmetrical_job) { startx = xmin; starty = ymin; }
  do {
    finished = true; /* tentatively */
    for (x = startx; x <= xmax; x++) {
      for (y = starty; y <= ymax; y++) {
	if (cboard[x*ydim+y] > 3) {
	  finished = false;
	  count++;
	  fprintf (stdout, "! Safety check has to collapse (%d,%d) val=%d\n", x, y, cboard[x*ydim+y]);
	  collapse_and_report(x,y);
	  if (! asymmetrical_job)
	    cantcontinue("! FIXME: %s: SAFETY CHECK IS NOT COMPLETE ON SYMMETRICAL BOARDS", __func__);
	}
      }
    }
  } while (finished == false);
  if (count > 0) {
    fprintf (stdout, "ERROR: %s: Safety check had to collapse %d times\n", __func__, count);
    fprintf (stderr, "ERROR: %s: Safety check had to collapse %d times\n", __func__, count);
    fail();
  }
#endif /* NDEBUG */

 skip_all:
  report_normal_form ();
  TRACEOUT;
}

void add_grains_on_origin (int delta)
{
  assert ((delta < 0) || (mass <= INT_MAX - delta));
  add_grains_on_square (0, 0, delta);
}

/* This updates xmin &c. so that (x,y) is contained */
void enlarge_envelop_for (int x, int y)
{
  TRACEINW("(x=%d y=%d)", x, y);
  bool pushed = false;
#ifdef DEBUG_BINARY_BOARD
  printf ("! memmatrix before enlarging at (%d,%d):\n", x, y);
  dump_current_memmatrix (stdout);
  display_the_board (stdout, true);
#endif /* DEBUG_BINARY_BOARD */
  if (x < xmin) {
    xmin = x; pushed = true;
  } else if (x > xmax) {
    xmax = x; pushed = true;
  }
  if (y < ymin) {
    ymin = y; pushed = true;
  } else if (y > ymax) {
    ymax = y; pushed = true;
  }
  if (pushed != false) {
    if (xmax - xmin >= diam) diam = xmax - xmin + 1;
    if (ymax - ymin >= diam) diam = ymax - ymin + 1;
    if (diam > max_dim)
      cantcontinue("Max_dim = %d reached. Am not allowed to enlarge board...\n", max_dim);
    while ( (x+xdepl < 0) || (x+xdepl >= xdim) || (y+ydepl < 0) || (y+ydepl >= ydim) ) {
      enlarge_underlying_mat (); /* push xdim, ydim */
    }
  }
#ifdef DEBUG_BINARY_BOARD
  printf ("! memmatrix after enlarging at (%d,%d):\n", x, y);
  dump_current_memmatrix (stdout);
  display_the_board (stdout, true);
#endif /* DEBUG_BINARY_BOARD */
  if (display_mode == CURSING_MODE) {
    display_cursing_dims ();
  }
  TRACEOUT;
}

void close_board ( void )
{
  if (board_inited) {
    board_inited = false;
    free(cboard);
    close_waitinglist ();
    xdim = xmin = xmax = xdepl = ydim = ymin = ymax = ydepl = 0;
    mass = 0; nbsteps = 0; diam = 0; count1 = count2 = count3 = 0;
    area = count0 = 0;
  }
}

void display_the_board (FILE * f, bool withvars)
{
  TRACEINW("(f=%p)", f);
  TRACESIGNEDMESS ("c0=%d c1=%d c2=%d c3=%d", count0, count1, count2, count3);
#ifdef DEBUG_BINARY_BOARD
  dump_current_memmatrix (stdout);
#endif /* DEBUG_BINARY_BOARD */
  int x, y;
  int wid = xmax - xmin + 1;
  fputc('+', f); fprint_n_char(wid, '-', f); fputc('+', f); fterpri(f);
  for (y = ymax+1; y-- > ymin;) { /* downward loop :-( */
    fputc('|', f);
    for (x = xmin; x <= xmax; x++) {
      fputc (char_for_val(cboard[x*ydim+y]), f);
    }
    fputc('|', f); fterpri(f);
  }
  fputc('+', f); fprint_n_char(wid, '-', f); fputc('+', f); fterpri(f);
  if (withvars)
    display_board_vars(f);
  TRACEOUT;
}

/* FOLLOWING FUNCTIONS USED FOR TRACES IN TESTING AND DEBUGGING */
void dump_waitinglist (FILE * stream, bool statstoo)
{
  int nb, x, y, *p;
  fprintf (stream, "== START DUMP_WAITINGLIST ==\n");
  if (statstoo) {
    fprintf (stream, "waitlist_inited = %s\n", boolstr(waitlist_inited));
    fprintf (stream, "waitinglist = %p\n", (void *)waitinglist);
    fprintf (stream, "wait_curr = %p\n", (void *)wait_curr);
    fprintf (stream, "wait_max = %p\n", (void *)wait_max);
  }
  if (waitlist_inited) {
    if (0 != (wait_curr - waitinglist) % 2) {
      fprintf (stderr, "ERROR: %s: nb elems in waitinglist should be even.\n", __func__);
      fail();
    }
    if (statstoo) {
      fprintf (stream, "Total capacity:\t%ld ints\n", wait_max - waitinglist);
      fprintf (stream, "Used capacity:\t%ld ints\n", wait_curr - waitinglist);
      fprintf (stream, "Remaining cap.:\t%ld ints\n", wait_max - wait_curr);
    }
  }
  nb = 0; p = waitinglist;
  while (p < wait_curr) {
    x = *(p++); y = *(p++);  nb++;
    fprintf (stream, "WL[%d] = (%d,%d)\n", nb, x, y);
  }
}

/* READ A SNAPSHOT FILE */

/* HEADER variables: where we store the data copied from snapshot header */
static long int mass_snap, nbsteps_snap, diam_snap, c0_snap, c1_snap, c2_snap, c3_snap;

#define NMATCH 8		/* number of subexps in rx4snphead */
static regex_t regex1, regex2, regex3;
static regmatch_t matches[NMATCH];
static const char *rx4snphead = "^M=([-]?[[:digit:]]+)[[:blank:]]+T=([-]?[[:digit:]]+)[[:blank:]]+D=([-]?[[:digit:]]+)[[:blank:]]+c0=([-]?[[:digit:]]+)[[:blank:]]+c1=([-]?[[:digit:]]+)[[:blank:]]+c2=([-]?[[:digit:]]+)[[:blank:]]+c3=([-]?[[:digit:]]+)[[:blank:]]*\n";
static const char *rx4snptopl = "^[ +](-+)[ +]\n";
static const char *rx4snpdataline = "^[|]([0123. ]*)[|]\n";

/* INIT_BOARD_DATA_FROM_SNAPSHOT uses the match data MATCHES after matching rx1,
   (i.e. after reading header line) to set values for header variables.
   PATH and LINENUM are given for error messages but unused in this version. */
void init_board_data_from_snapshot (char * chars, const char * path __attribute__ ((unused)), int linenum __attribute__ ((unused)))
{
  int mbeg, mend;
  TRACEIN;
  /* M for mass */
  mbeg = matches[1].rm_so; mend = matches[1].rm_eo; chars[mend] = '\0';/* truncate */
  read_longint_val_in_range (chars+mbeg, &mass_snap, 0, MAX_ALLOWED_HEIGHT, "M");
  /* T for nbsteps */
  mbeg = matches[2].rm_so; mend = matches[2].rm_eo; chars[mend] = '\0';/* truncate */
  read_longint_val_in_range (chars+mbeg, &nbsteps_snap, 0, LONG_MAX, "T");
  /* D for diam */
  mbeg = matches[3].rm_so; mend = matches[3].rm_eo; chars[mend] = '\0';/* truncate */
  read_longint_val_in_range (chars+mbeg, &diam_snap, 0, max_dim, "D");
  /* c0 */
  mbeg = matches[4].rm_so; mend = matches[4].rm_eo; chars[mend] = '\0';/* truncate */
  read_longint_val_in_range (chars+mbeg, &c0_snap, 0, MAX_ALLOWED_HEIGHT, "c0");
  /* c1 */
  mbeg = matches[5].rm_so; mend = matches[5].rm_eo; chars[mend] = '\0';/* truncate */
  read_longint_val_in_range (chars+mbeg, &c1_snap, 0, MAX_ALLOWED_HEIGHT, "c1");
  /* c2 */
  mbeg = matches[6].rm_so; mend = matches[6].rm_eo; chars[mend] = '\0';/* truncate */
  read_longint_val_in_range (chars+mbeg, &c2_snap, 0, MAX_ALLOWED_HEIGHT, "c2");
  /* c3 */
  mbeg = matches[7].rm_so; mend = matches[7].rm_eo; chars[mend] = '\0';/* truncate */
  read_longint_val_in_range (chars+mbeg, &c3_snap, 0, MAX_ALLOWED_HEIGHT, "c3");

  TRACEOUT;
}

/* (X0,Y0) = coords of CHARS[0] on the board */
void init_board_line_from_snapshot (int x0, int y0, char * chars, int len, const char * path, int linenum)
{
  TRACEINW("(x0=%d,y0=%d,s=\"%s\")", x0, y0, chars);
  char * p = chars;
  char ch;
  int i, x;
  for (i = 0; i < len; i++) {
    ch = *p++; assert(ch);
    switch (ch) {
    case ' ': /* UNUSEDSQ: don't add, don't count */
      break;

    case '.':
      ch = '0';	/* replace and branch into next case */

    case '0': case '1': case '2': case '3':
      x = x0+i;
      TRACEMESS("Putting %d at (%d,%d) where was %d.\n", ch - '0', x, y0, 0);
      if ((x <= xmin) || (x >= xmax) || (y0 <= ymin) || (y0 >= ymax)) {
	TRACEMESS("%s requires enlarging board[%d..%d][%d..%d] at (%d,%d).\n", __func__, xmin, xmax, ymin, ymax, x, y0);
	enlarge_envelop_for(x,y0);
      }
      add_grains_on_square(x, y0, ch - '0');
      break;

    default:
      cantcontinue("ERROR: %s did not expect char '%c' in line %d of %s.\n",
		   __func__, ch, linenum, path);
    }
  }
  TRACEOUT;
}

/* read a snapshot file, use it as board config after sanity check.

   If JUSTHEADER: only read header variables mass_snap etc., dont sanity check, dont touch board. */
void init_board_contents_from_snapshot (const char * path, bool justheader)
{
  TRACEINW("%s", path);
  char * linebuff = NULL;
  size_t linecapp = 0;
  ssize_t nbread = -1;
  int nbmismatches = 0;
  int file_linenum = 0;
  int matrix_linenum = 0;
  int linenum_of_matrixtop = -1;
  int mbeg, mend, mlen = -1;
  int radius;
  if (path == NULL) {
    cantcontinue("No snapshot path?\n");
  } else if (access(path, F_OK) != 0) {
    cantcontinue("Can't find snapshot file %s.\n", path);
  } else if (access(path, R_OK) != 0) {
    cantcontinue("Can't read snapshot file %s.\n", path);
  }
  TRACEMESS("Option \"-f %s\" accepted", path);
  FILE * snapshotfile = fopen(path, "r");
  if (snapshotfile == NULL)
    cantcontinue("ERROR: %s: Could not open file %s.\n", __func__, path);

  if (regcomp(&regex1, rx4snphead, REG_EXTENDED))
    cantcontinue("ERROR: %s: regcomp(rx4snphead) does not compile.\n", __func__);
  if (regcomp(&regex2, rx4snptopl, REG_EXTENDED))
    cantcontinue("ERROR: %s: regcomp(rx4snptopl) does not compile.\n", __func__);
  if (regcomp(&regex3, rx4snpdataline, REG_EXTENDED))
    cantcontinue("ERROR: %s: regcomp(rx4snpdataline) does not compile.\n", __func__);

  /* 1st loop: looking for M=nn and other recap data */
  TRACEMESS("Starting 1st loop: looking for header");
  while ((nbread = getline(&linebuff, &linecapp, snapshotfile)) != -1) {
    file_linenum++;
    if (regexec(&regex1, linebuff, NMATCH, matches, 0)) {
      TRACEMESS("mismatch with rx1: %s", linebuff);
      nbmismatches++;
    } else {
      TRACEMESS("match rx1: %s", linebuff);
      init_board_data_from_snapshot(linebuff, path, file_linenum);
      TRACEMESS("Breaking out of 1st loop");
      if (justheader) goto clean_quit;
      break;			/* out of 1st loop */
    }
  }
  /* some sanity checks */
  if (mass_snap != c1_snap + 2*c2_snap + 3*c3_snap) {
    cantcontinue("ERROR: M=%ld != c1+2*c2+3*c3 ?!\n", mass_snap);
  }
  if (diam_snap%2 != 1) {
    cantcontinue("ERROR: D=%ld is not odd ?!. -f snapshot only accepts PILE jobs, with symmetrical boards.\n", diam_snap);
  }
  radius = (diam_snap+1)/2;

  /* 2nd loop: looking for top line */
  TRACEMESS("Starting 2nd loop: looking for top of board");
  while ((nbread = getline(&linebuff, &linecapp, snapshotfile)) != -1) {
    file_linenum++;
    if (regexec(&regex2, linebuff, NMATCH, matches, 0)) {
      TRACEMESS("mismatch rx2: %s", linebuff);
      nbmismatches++;
    } else {
      TRACEMESS("match rx2: %s", linebuff);
      linenum_of_matrixtop = file_linenum;
      mbeg = matches[1].rm_so; mend = matches[1].rm_eo;
      mlen = mend - mbeg;
      /* some sanity checks */
      if (mlen != diam_snap) {
	cantcontinue("ERROR: D=%ld different from length %d of line %d in %s ?!\n", diam_snap, mlen, file_linenum, path);
      }
      linebuff[mend] = '\0';/* truncate linebuff */
      TRACEMESS("Breaking out of 2nd loop");
      break;			/* out of 2nd loop */
    }
  }

  /* 3rd loop: reading data lines */
  TRACEMESS("Starting 3rd loop: reading board contents");
  while ((nbread = getline(&linebuff, &linecapp, snapshotfile)) != -1) {
    file_linenum++;
    if (regexec(&regex2, linebuff, NMATCH, matches, 0) == 0) {
      TRACEMESS("match rx2: %s", linebuff);
      mbeg = matches[1].rm_so; mend = matches[1].rm_eo;
      if (mend != mbeg + mlen) { /* this is mlen from top_line  */
      mlen_error:
	cantcontinue("ERROR: snapshot format not respected in %s : lines %d and %d have different lengths: %d and %d.\n", path, linenum_of_matrixtop, file_linenum, mlen, mend - mbeg);
      } else if (matrix_linenum != diam_snap) {
	cantcontinue("ERROR: D=%ld different from numbers of matrix lines=%d in %s ?!\n", diam_snap, matrix_linenum, path);
      } else {
	TRACEMESS("Breaking out of 3rd loop");
	break;			/* out of 3rd loop */
      }
    } else if (regexec(&regex3, linebuff, NMATCH, matches, 0) == 0) {
      TRACEMESS("match rx3: %s",  linebuff);
      matrix_linenum++;
      mbeg = matches[1].rm_so; mend = matches[1].rm_eo;
      if (mend != mbeg + mlen) { /* this is mlen from top_line  */
	goto mlen_error;
      } else {
	linebuff[mend] = '\0';	/* truncate linebuff */
	init_board_line_from_snapshot(1 - radius, radius - matrix_linenum, linebuff+mbeg, mlen, path, file_linenum);
      }
    } else {
      nbmismatches++;
    }
  }
  /* 4th loop: reading remaining lines */
  TRACEMESS("Starting 4th loop: counting remaining lines");
  while ((nbread = getline(&linebuff, &linecapp, snapshotfile)) != -1) {
    file_linenum++;
    nbmismatches++;
  }
  TRACEMESS("Finishing 4th loop");
  if (nbmismatches) {
    fprintf(stderr, "! %s(%s): %d lines do not fit snapshot format :-(\n", __func__, path, nbmismatches);
  }

  /* We now check that the header values are consistent with the values computed from the board contents */
  if (mass_snap != mass)
    cantcontinue("Snapshot file %s claims M=%ld but I found %d\n", path, mass_snap, mass);
  if (diam_snap != diam)
    cantcontinue("Snapshot file %s claims D=%ld but I found %d\n", path, diam_snap, diam);
  if (c0_snap != count0)
    cantcontinue("Snapshot file %s claims c0=%ld but I found %d\n", path, c0_snap, count0);
  if (c1_snap != count1)
    cantcontinue("Snapshot file %s claims c1=%ld but I found %d\n", path, c1_snap, count1);
  if (c2_snap != count2)
    cantcontinue("Snapshot file %s claims c2=%ld but I found %d\n", path, c2_snap, count2);
  if (c3_snap != count3)
    cantcontinue("Snapshot file %s claims c3=%ld but I found %d\n", path, c3_snap, count3);
  /* nbsteps value cannot be inferred from snapshot, we copy it from header */
  nbsteps = nbsteps_snap;

 clean_quit:			 /* we branch there when JUSTHEADER */
  if (fclose(snapshotfile) != 0) /* reading is finished */
    cantcontinue("ERROR: %s: fclose failed.\n", __func__);
  free(linebuff);		/* NB: free NULL is ok */
  TRACEOUT;
}

void get_mass_from_snapshot (const char * path, long int * mass_var)
{
  TRACEINW("(%s)", path);
  init_board_contents_from_snapshot (path, true); /* just header */
  *mass_var = mass_snap;
  TRACEOUTW("%ld", *mass_var);
}
