#ifndef MAKEDEPEND_IGNORE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <print.h> /* from libphs */
#endif /* MAKEDEPEND_IGNORE */

#include "trace.h"

const char * tracing_slices_header = "== tracing slices";
const char * tracing_words_header = "== tracing words";

void putnstrs(const char * s, unsigned int n)
{
  while (n-- > 0) fputs(s, stdout);
}

static unsigned int tracelevel=0;

#define MAX_TRACE_LEVEL 100

void traceindentin()
{
  if (tracelevel++ >= MAX_TRACE_LEVEL) {
    fprintf (stderr, "ERROR: MAX_TRACE_LEVEL=%d reached\n", MAX_TRACE_LEVEL);
    if (tracelevel > MAX_TRACE_LEVEL)
      fail();			/* we'd rather fail with traceout but there is the possibility of infinite loops */
  }
  printf("%d>", tracelevel);
  if (tracelevel > 1)
    putnstrs(">>", tracelevel - 1);
}

void traceindentout()
{
  if (tracelevel >= MAX_TRACE_LEVEL) {
    fprintf (stderr, "ERROR: MAX_TRACE_LEVEL=%d reached\n", MAX_TRACE_LEVEL);
    fail();
  }
  if (tracelevel == 0) {
    fprintf (stderr, "ERROR: trace level = 0?? Some TRACEIN must be missing\n");
    fail();
  }
  printf("%d<", tracelevel);
  if (tracelevel > 1)
    putnstrs("<<", tracelevel - 1);
  tracelevel--;
}

void traceindentmess()
{
  putchar('|'); do_indent(2*tracelevel);
}
