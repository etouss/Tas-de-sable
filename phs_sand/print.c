/* File: print.c -- Time-stamp: <Mon 19 Jan 2015 18:01 - phs>
 *
 */

#ifndef MAKEDEPEND_IGNORE
#include <assert.h>
#include <stdlib.h>
#endif /* MAKEDEPEND_IGNORE */

#include "print.h"

char* emptystring = "";

bool crash_smoothly = false;
bool fuse_stderr = false;

void FFLUSH ( void )
{ fflush (stdout); fflush (stderr); }

void BUGIN (const char* functionname)
{
  fprintf (stderr, "BUG IN %s. ", functionname);
  crash (-1);
}

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

void print_substring (const char *s, size_t len) /* no '\0' allowed */
{
  fprint_substring (s, len, stdout);
}

void fprint_substring (const char *s, size_t len, FILE *stream)
{
  char c;
  while (len-- > 0) {
    if ((c = *(s++)) != '\0') {
      putc (c, stream);
    } else {
      fprintf (stderr,
	       "ERROR: libphs: No '\\0' allowed in (f)print_substring\n");
      crash (-1);
    }
  }
}

void print_string (const char* s)
{
  fprint_string (s, stdout);
}

void fprint_string (const char* s, FILE *stream)
{
  assert (s);
  fputs (s, stream);
  FFLUSH ();
}

void print_int (int n)
{
  fprint_int (n, stdout);
}

void fprint_int (int n, FILE *stream)
{
  fprintf (stream, "%d", n);
  FFLUSH ();
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

void print_char (char c)
{
  fprint_char (c, stdout);
}

void fprint_char (char c, FILE *stream)
{
  fprint_n_char (1, c, stream);
}


/*
Local Variables:
fill-column: 123
End:
*/
