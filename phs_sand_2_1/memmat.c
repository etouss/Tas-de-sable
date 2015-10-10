/* File: memmat.c

   DESCRIPTION: manages an X x Y array of ints.
   */
#ifndef MAKEDEPEND_IGNORE
#include <print.h>		/* fail(); from libphs */

#include <errno.h>
#include <stdlib.h>		/* alloc, malloc & free */
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

//#define TRACE
//#define DEBUG_BINARY_BOARD

#include "sand.h"

/* local functions: */
extern void dump_memmatrix (FILE * stream, int * tab, int xd, int yd);

/* THE PHYSICAL BOARD is a memory block that holds a portion of the infinite 2-dim space.
The variables describing it at all time are: */
static int * ze_memmat = NULL;	/* NULL = not yet inited */
static int ze_xdim = 0;		/* dim of alloc'ed array */
static int ze_ydim = 0;		/* dim of alloc'ed array */

bool memmatrix_inited ()
{
  if (ze_memmat != NULL) return true; else return false;
}

int memmatrix_xdim ()
{
  assert (ze_memmat != NULL);
  return ze_xdim;
}

int memmatrix_ydim ()
{
  assert (ze_memmat != NULL);
  return ze_ydim;
}

int * memmatrix(int i, int j)
{
  assert (ze_memmat != NULL);
  return ze_memmat+i*ze_ydim + j;
}

void dump_current_memmatrix (FILE * f)
{
  dump_memmatrix(f, ze_memmat, ze_xdim, ze_ydim);
}

void realloc_memmatrix (int xincr, int yincr, int xshift, int yshift, int valnews)
{
  TRACEINW("(incr=%d,%d shift=%d,%d,val=%d)", xincr, yincr, xshift, yshift, valnews);
  assert( ( (ze_memmat == NULL) && (ze_xdim == 0) && (ze_ydim == 0) ) ||
	  ( (ze_memmat != NULL) && (ze_xdim > 0) && (ze_ydim > 0) ) );
  int newxdim = ze_xdim + xincr;
  int newydim = ze_ydim + yincr;
  int newsize = newxdim * newydim;
  int i, j, *p, *q;
  if ((xincr < 0) || (yincr < 0)) {
    cantcontinue("ERROR: %s: Can't take negative size increment %d,%d.\n", __func__, xincr, yincr);
  } else if ((xshift > xincr) || (yshift > yincr)) {
    cantcontinue ("ERROR: %s: Can't shift %d,%d beyond size increment %d,%d.\n", __func__, xshift, yshift, xincr, yincr);
  }
  int * new_memmatrix = calloc (newsize, sizeof(int));
  if (errno == ENOMEM) cantcontinue("Out of memory. %s can't realloc board.\n", __func__);
  assert (new_memmatrix);

  // FIXME: init can be skipped? if valnews = 0;
  for (p = new_memmatrix+newsize; p > new_memmatrix;) /* downward loop */
    *(--p) = valnews;				    /* init with given val */
#ifdef DEBUG_BINARY_BOARD
  printf ("New memmatrix before copying old memmatrix:\n");
  dump_memmatrix (stdout, new_memmatrix, newxdim, newydim);
#endif /* DEBUG_BINARY_BOARD */
  if (ze_memmat != NULL) {/* there is a previous board to be replaced */
#ifdef DEBUG_BINARY_BOARD
    printf ("I have this old memmatrix to copy:\n");
    dump_memmatrix (stdout, ze_memmat, ze_xdim, ze_ydim);
#endif /* DEBUG_BINARY_BOARD */
    /* copy the [xmin..xmax] x [ymin..ymax] subarray */
    p = ze_memmat;
    q = new_memmatrix + xshift*newydim + yshift;
    for (i = 0; i < ze_xdim; i++) {
      for (j = 0; j < ze_ydim; j++) {
	*(q+j) = *(p++);
      }
      q += newydim;
    }
#ifdef DEBUG_BINARY_BOARD
    printf ("New memmatrix after copying old memmatrix:\n");
    dump_memmatrix (stdout, new_memmatrix, newxdim, newydim);
#endif /* DEBUG_BINARY_BOARD */
    /* free old mem */
    free(ze_memmat);
  }
  /* now we set proper board vals */
  ze_xdim = newxdim; ze_ydim = newydim;
  ze_memmat = new_memmatrix;
  TRACEOUT;
}


/* Useful for debugging. MEMX is the actual pointer to an XDxYD chunk of mem */
void dump_memmatrix (FILE * f, int * memx, int xd, int yd)
{
  int i, j, val;
  char c;
  TRACEIN;
  assert(xd>=0); assert(yd>=0);
  fprintf (f, "== START DUMP_MEMMATRIX(xd=%d,yd=%d) ==\n", xd, yd);
  if (memx == NULL) {
    fprintf (f, "! memx = NULL !!\n");
    goto quit;
  }
  for (j = yd; j-- > 0;) { /* downward loop :-( */
    fprintf (f, "L%d:\t|", j);
    for (i = 0; i < xd; i++) {
      val = memx[j+i*yd];
      if (val == UNUSEDSQ) c = ' ';
      else if (val == 0) c = '.';
      else if ((val > 0) && (val <= 36)) c = '0' + val;
      else c = '?';
      putc(c, f);
    }
    putc('|', f); fterpri(f);
  }
 quit:
  TRACEOUT;
  return;
}
