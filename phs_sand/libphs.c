/* File: libphs.c -- Time-stamp: <Mon 28 Sep 2015 19:14 - phs>
 *
 */

#ifndef MAKEDEPEND_IGNORE
#include <assert.h>
#include <stdlib.h>
#endif /* MAKEDEPEND_IGNORE */

#include "print.h"

bool crash_smoothly = false;

void FFLUSH ( void )
{ fflush (stdout); fflush (stderr); }

void fail ( void )
{
  crash (EXIT_FAILURE);
}

void crash (const int status)
{
  fflush (stdout);
  fprintf (stderr, "Sorry I must crash...\n");
  fflush (stderr);
  if (crash_smoothly) {
    exit (0);
  } else {
    exit (status);
  }
}

void terpri ()
{
  fterpri (stdout);
}

void fterpri (FILE *stream)
{
  fprint_char ('\n', stream);
}

void do_indent (int n)
{
  fdo_indent (n, stdout);
}

void fdo_indent (int n, FILE *stream)
{
  assert (n >= 0);
  fprint_n_char (n, ' ', stream);
}

void print_n_char (int n, char c)
{
  fprint_n_char (n, c, stdout);
}

void fprint_n_char (int n, char c, FILE *stream)
{
  int i;
  assert (n >= 0);
  assert (c != '\0');
  for (i = 0; i < n; i++) {
    putc (c, stream);
  }
  FFLUSH ();
}

void fprint_char (char c, FILE *stream)
{
  fprint_n_char (1, c, stream);
}
