/* File: utils.h
 *
 * specific declarations for utils.c
 */

extern void read_longint_val_in_range (char * str, long int * var, long int range_min, long int range_max, const char * varname);
/* Read the number in STR and return the value by writing in VAR.
   Check that the value is between RANGE_MIN and RANGE_MAX.
   VARNAME is used for error messages.
   */
