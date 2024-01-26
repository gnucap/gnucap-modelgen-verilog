/*                             -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
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
 */
#ifndef GNUCAP_MG_ERROR_H
#define GNUCAP_MG_ERROR_H
#include <io_error.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class CS;
struct Exception_CS_ :public Exception {
  std::string _cmd;
  size_t _cursor;
  const std::string message() const override;
  Exception_CS_(const std::string& Message, const CS& cmd, size_t cursor);
  Exception_CS_(const std::string& Message, const CS& cmd);
private:
  void construct(const CS& cmd, size_t here);
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
