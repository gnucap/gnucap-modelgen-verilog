/*                        -*- C++ -*-
 * Copyright (C) 2026 Arnout Engelen
 * Copyright (C) 2023 Felix Salfelder
 * Author: Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 */

#ifndef GNUCAP_L_VA_H
#define GNUCAP_L_VA_H
/*--------------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdio.h>
/*--------------------------------------------------------------------------*/
namespace va {
/*--------------------------------------------------------------------------*/
/* Verilog has conversion specifiers that C (POSIX?) does not have.
 * This would be the place to implement them. Until then just call fprintf.
 */
int fprintf(FILE *stream, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int ret = ::vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
/*--------------------------------------------------------------------------*/
int dprintf(int fd, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int ret = ::vdprintf(fd, format, args);
  va_end(args);
  return ret;
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet:
