// SPDX-FileCopyrightText: 1995 1999, 2003-2004, 2007-2012 Free Software Foundation, Inc.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/* This file is exactly like compiler.c.  I could just use compiler.c if
   I could be sure that every C++ compiler accepted extensions of ".c".  */

/* Note the semicolon at the end of this line.  Older versions of
   hp c++ have a bug in string preprocessing: if the last token on a
   line is a string, then the preprocessor concatenates the next line
   onto the current line and eats the newline!  That messes up TCL of
   course.  That happens with HP aC++ A.03.13, but it no longer happens
   with HP aC++ A.03.45. */

set compiler_info "unknown" ;

#if defined (__GNUC__)
#if defined (__GNUC_PATCHLEVEL__)
/* Only GCC versions >= 3.0 define the __GNUC_PATCHLEVEL__ macro.  */
set compiler_info [join {gcc __GNUC__ __GNUC_MINOR__ __GNUC_PATCHLEVEL__} -]
#else
set compiler_info [join {gcc __GNUC__ __GNUC_MINOR__ "unknown"} -]
#endif
#endif

#if defined (__HP_CXD_SPP)
/* older hp ansi c, such as A.11.01.25171.gp, defines this */
set compiler_info [join {hpcc __HP_CXD_SPP} -]
#endif

#if defined (__HP_cc)
/* newer hp ansi c, such as B.11.11.28706.gp, defines this */
set compiler_info [join {hpcc __HP_cc} -]
#endif

#if defined (__HP_aCC)
set compiler_info [join {hpacc __HP_aCC} -]
#endif

#if defined (__xlc__)
/* IBM'x xlc compiler. NOTE:  __xlc__ expands to a double quoted string of four
   numbers seperated by '.'s: currently "7.0.0.0" */
set need_a_set [regsub -all {\.} [join {xlc __xlc__} -] - compiler_info]
#endif

#if defined (__ARMCC_VERSION)
set compiler_info [join {armcc __ARMCC_VERSION} -]
#endif
