// SPDX-FileCopyrightText: 2008 - 2012 Free Software Foundation, Inc.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/* Force outputs to unbuffered mode.  */

#include <stdio.h>

static int __gdb_set_unbuffered_output (void) __attribute__ ((constructor));
static int
__gdb_set_unbuffered_output (void)
{
  setvbuf (stdout, NULL, _IONBF, BUFSIZ);
  setvbuf (stderr, NULL, _IONBF, BUFSIZ);
}
