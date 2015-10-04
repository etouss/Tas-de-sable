#ifndef MAKEDEPEND_IGNORE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>		/* for formats PRIu64 etc */
#include <time.h>		/* clock() */
#include <sys/resource.h>	/* getrusage() */

#include <print.h>	        /* from libphs */
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"

#define LENG4PFCSTR 28 /* max length required for printfcomma of ULLONG_MAX */


/* wallclock */
#define STBUFSIZE (size_t)100
static char stimebuffer[STBUFSIZE];

static const char * now_dh_format = "%F %H:%M:%S";
static const char * now_h_format = "%H:%M:%S";

char * now_str (bool datetoo)
{
  time_t rawtime;
  struct tm *info;
  time (&rawtime);		/* get current time*/
  info = localtime(&rawtime);	/* break down rawtime */
  if (datetoo) {
    strftime(stimebuffer, STBUFSIZE, now_dh_format, info); /* also with date */
  } else {
    strftime(stimebuffer, STBUFSIZE, now_h_format, info); /* HHMMSS only */
  }
  return stimebuffer;
}

/* memory & other resources usage */
static struct rusage use;

void display_rusage (FILE *stream)
{
  if (getrusage(RUSAGE_SELF, &use) != 0) {
    fprintf(stderr, "ERROR: %s: getrusage(..) failed\n", __func__);
    fail();
  }
  double usr_time = ((double)1.0) * use.ru_utime.tv_sec + use.ru_utime.tv_usec / (double)1000000.0;
  double sys_time = ((double)1.0) * use.ru_stime.tv_sec + use.ru_stime.tv_usec / (double)1000000.0;
  fprintf(stream, "# %0.3f secs elapsed (user time)\n",	  usr_time);
  fprintf(stream, "# %0.3f secs elapsed (system time)\n", sys_time);
  fprintf(stream, "# %ld pages reclaims\n",		  use.ru_minflt);
  fprintf(stream, "# %ld pages faults\n",		  use.ru_majflt);
  fprintf(stream, "# %ld swaps\n",			  use.ru_nswap);
  fprintf(stream, "# %ld voluntary context switches\n",	  use.ru_nvcsw);
  fprintf(stream, "# %ld involuntary context switches\n", use.ru_nivcsw);
}

char pfc_s[LENG4PFCSTR];/* buffers shared for all modules */
char pfc_s2[LENG4PFCSTR];
char pfc_s3[LENG4PFCSTR];

char* printfcomma (const uint64_t n, char * s) /* to be moved to libphs eventually */
{
  char * p = s;
  char * lim = s + LENG4PFCSTR;	/* cant write there */
  uint64_t n1 = n;
  uint64_t n2 = 0;
  uint64_t scale = 1;
  while (n1 >= 1000) {
    n2 = n2 + scale * (n1 % 1000);
    n1 /= 1000;
    scale *= 1000;
  }
  p += sprintf (p, "%"PRIu64, n1);
  if (p >= lim) {
  crash:
    fprintf(stderr, "ERROR: %s: LENG4PFCSTR=%d not long enough for n=%"PRIu64"\n",
	    __func__, LENG4PFCSTR, n);
    fail();
  }
  while (scale != 1) {
    scale /= 1000;
    n1 = n2 / scale;
    n2 = n2  % scale;
    p += sprintf (p, ",%03"PRIu64, n1);
    if (p >= lim) goto crash;
  }
  assert (*p == '\0');
  return s;
}
