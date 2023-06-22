/*$Id: mg_error.cc 2016/05/15 al $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
 * Error handler.
 */
//testing=script,sparse 2006.10.31
#include <ap.h>
extern int errorcount;
extern std::basic_ostream<char>* diag_out;
/*--------------------------------------------------------------------------*/
#if 0
void error(int, const std::string& message)
{untested();
  std::cout << message << '\n';
  exit(2);
}
void error(int,const char* message,...)
{untested();
  std::cout << message << '\n';
  exit(2);
}
/*--------------------------------------------------------------------------*/
/* syntax_check: handle syntax errors
 * called on parsing an input string when nothing else matches.
 * if the rest of the line is nothing, just return
 * if comment, increment *cnt, so what is left is a valid comment string
 * otherwise, it is an error (the arrow pointing at the offending word)
 */
CS & CS::check(int badness, const std::string& message)
{untested();
  skipbl();
  switch (peek()) {
    case '\'':	_ok = true;  skip();	   break;
    case '\0':	_ok = true;		   break;
    default:	_ok = false; warn(badness, message); break;
  }
  return *this;
}
#endif
/*--------------------------------------------------------------------------*/
static std::basic_ostream<char>& diag()
{
  if(diag_out){
    return *diag_out;
  }else{ untested();
    return std::cerr;
  }
}
/*--------------------------------------------------------------------------*/
static void tab(size_t n)
{
  for (unsigned i=0; i<n; ++i) {
    diag() << ' ';
  }
}
/*--------------------------------------------------------------------------*/
/* syntax_msg: print syntax error message
 * echo a portion of the input with an arrow pointing to the current place
 */
//BUG// override CS::warn locally, works with posix, duplicate symbol with windows
// this missing means to use the library function, which is incomplete,
// losing the error messages
#if !defined(__WIN32__)
CS & CS::warn(int badness, size_t spot, const std::string& message)
{
  if (badness >= 0) {

    size_t linestart = spot;
    for (;;) {
      if (linestart == 0) {
	break;
      }else if (_cmd[linestart] == '\n') { untested();
	++linestart;
	break;
      }else{
	--linestart;
      }
    }

    int lineno = 1;
    for (size_t i=0; i<linestart; ++i) { untested();
      if (_cmd[i] == '\n') { untested();
	++lineno;
      }else{ untested();
      }
    }

    diag() << _name << ':' << lineno << ":\n";
    if (spot-linestart < 20) {
      for (size_t i=linestart; _cmd[i] && _cmd[i]!='\n'; ++i) {
	diag() << _cmd[i];
      }
      diag() << '\n';
      tab(spot-linestart);
    }else{ untested();
      diag() << "..";
      for (size_t i=spot-15; _cmd[i] && _cmd[i]!='\n'; ++i) { untested();
	diag() << _cmd[i];
      }
      diag() << '\n';
      tab(17);
    }
    diag() << "^ ? " << message << '\n';
  }else{untested();
  }
  if(badness >= bDANGER){ untested();
    exit(1);
  }else{
  }
  return *this;
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
