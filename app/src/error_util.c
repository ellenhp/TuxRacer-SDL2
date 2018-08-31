/*
 * Tux Racer
 * Copyright (C) 1999-2001 Jasmin F. Patry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "error_util.h"
#include "string_util.h"
#include "tuxracer.h"

/* A note on warning levels:
   A warning level of 0 prints nothing
   A warning level of 100 prints everything

   1 is reserved for critical warnings -- things that the user really
   shouldn't do but that we can continue from anyway. */

void print_warning(int warning_level, char *fmt, ...) {
  va_list args;
  char strbuf[1000]; // should be enough

  /* We enforce the fact that warning level 0 prints no warnings */
  check_assertion(warning_level > 0, "warning levels must be > 0");
  check_assertion(warning_level <= 100, "warning levels must be <= 100");

  if (warning_level > getparam_warning_level()) {
    return;
  }

  va_start(args, fmt);

  print_debug(DEBUG_OTHER, "%%%%%% " PROG_NAME " warning: ");

  vsprintf(strbuf, fmt, args);
  print_debug(DEBUG_OTHER, strbuf);
  print_debug(DEBUG_OTHER, "\n");

  va_end(args);
}

void handle_error(int exit_code, char *fmt, ...) {
  va_list args;
  char strbuf[1000]; // should be enough

  va_start(args, fmt);

  print_debug(DEBUG_OTHER, "*** " PROG_NAME " error: ");
  vsprintf(strbuf, fmt, args);
  print_debug(DEBUG_OTHER, strbuf);
  print_debug(DEBUG_OTHER, "\n");

  va_end(args);

  winsys_exit(exit_code);
}

void handle_system_error(int exit_code, char *fmt, ...) {
  va_list args;
  char strbuf[1000]; // should be enough

  va_start(args, fmt);

  print_debug(DEBUG_OTHER, "*** " PROG_NAME " error: ");
  vsprintf(strbuf, fmt, args);
  print_debug(DEBUG_OTHER, strbuf);
  print_debug(DEBUG_OTHER, " (%s)\n", strerror(errno));

  va_end(args);

  winsys_exit(exit_code);
}
