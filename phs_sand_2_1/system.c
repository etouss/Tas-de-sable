/* File: system.c

   DESCRIPTION: system-related stuff, potentially not very portable, and that we'd rather abstract for the rest of the program.
   */

#ifndef MAKEDEPEND_IGNORE
#include <print.h> /* before stdbool.h can be included */

#include <sys/utsname.h>	/* uname() */
#include <sys/param.h>	        /* MAXPATHLEN() */
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"

const char * machine_uname ( void )
{
  TRACEIN;
  static struct utsname unameData;
  if (uname(&unameData) != 0)
    cantcontinue ("ERROR: %s: system call to uname() failed.\n", __func__);
  TRACEOUT;
  return unameData.nodename;
}

static char cwd_buff[MAXPATHLEN];

const char * cwd ( void )
{
  TRACEINW(": MAXPATHLEN = %d", MAXPATHLEN);
  if (getcwd(cwd_buff, MAXPATHLEN) == NULL)
    cantcontinue ("ERROR: %s: system call to getcwd() failed.\n", __func__);
  TRACEOUT;
  return cwd_buff;
}
