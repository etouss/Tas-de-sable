/* File: print.h -- Time-stamp: <Tue 30 Dec 2014 13:11 - phs>
 *
 *
 */

#ifndef _PRINT_H
#define _PRINT_H

#ifndef MAKEDEPEND_IGNORE
#include <stdio.h> /* for type FILE */
#endif /* MAKEDEPEND_IGNORE */

#include "bool.h"

extern void terpri ( void );
extern void fterpri (FILE *stream);
extern void do_indent (int n); /* writes n ' ' */
extern void fdo_indent (int n, FILE *stream);
extern void print_substring (const char *s, size_t len); /* no '\0' allowed */
extern void fprint_substring (const char *s, size_t len, FILE *stream); /* no '\0' allowed */
extern void print_string (const char *s);
extern void fprint_string (const char *s, FILE *stream);
extern void print_int (int n);
extern void fprint_int (int n, FILE *stream);
extern void print_char (char c);
extern void fprint_char (char c, FILE *stream);
extern void print_n_char (int n, char c);
extern void fprint_n_char (int n, char c, FILE *stream);

extern void crash (int n);
extern void fail ( void );
extern void FFLUSH ( void );

/* pretty printing of messages */
/*=============================*/
extern char* emptystring;
extern char headline[];
extern void show_headline (int vthick, int hthick, int vspace, int hspace);

#endif /* _PRINT_H */

/*
Local Variables:
fill-column: 123
End:
*/
