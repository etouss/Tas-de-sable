/* File: utils.c

   DESCRIPTION: utilitary functions that are not specific to this program.

   */

#ifndef MAKEDEPEND_IGNORE
#include <print.h>

#include <stdlib.h> /* strtol() */
#include <errno.h>
#include <assert.h>
#endif /* MAKEDEPEND_IGNORE */

#include "sand.h"
#include "utils.h"

/* READ_LONGINT_VAL_IN_RANGE is a version of scanf, built on top of strtol */
void read_longint_val_in_range (char * s, long int * val, long int range_min, long int range_max, const char * varname)
{
  TRACEINW("(\"%s\",min=%ld,max=%ld)", s, range_min, range_max);
  char * endptr = NULL;
  long int res = strtol(s, &endptr, 10);
  if (*endptr != '\0') {
    cantcontinue("ERROR: value \"%s\" for %s is not a number.\n", s, varname);
  } else if (errno == ERANGE) {
    cantcontinue("ERROR: value \"%s\" for %s cannot be scanned as a long int.\n", s, varname);
  } else if ((res < range_min) || (res > range_max)) {
    cantcontinue("ERROR: value %ld for %s not in valid range %ld..%ld.\n", res, varname, range_min, range_max);
  }
  val[0] = res;
  TRACEOUTW("%s=%ld", varname, *val);
}
