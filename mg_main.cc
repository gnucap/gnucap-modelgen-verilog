/*                  -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023 Felix Salfelder
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
#include <gnucap/u_opt.h>
#include "mg_out.h"
#include <gnucap/patchlev.h>
/*global*/ int errorcount = 0;
std::string ind = "  ";
/*--------------------------------------------------------------------------*/
static void make_module_file(const File& in, std::string dump_name)
{
  { // chop prefix path
    std::string::size_type loc = dump_name.find_last_of(ENDDIR);
    if (loc != std::string::npos) {
      dump_name.erase(0, loc+1);
    }else{
      itested();
    }
  }

  // open file
  std::ofstream out(dump_name);
  if (!out) { untested();
    os_error(dump_name);
  }else{
  }

  make_cc(out, in);
}
/*--------------------------------------------------------------------------*/
static void preprocess(const File& in, std::ostream& of)
{
  of << in.fullstring();
}
/*--------------------------------------------------------------------------*/
static void dump(const File& in, std::ostream& out)
{
  out << in.define_list() << '\n'
      << in.nature_list() << '\n'
      << in.discipline_list() << '\n'
      << in.module_list() << '\n'
      << in.macromodule_list() << '\n'
      << in.connectmodule_list() << '\n';
}
/*--------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
  File f;
  --argc;
  ++argv;
  trace1("main", argc);
  for(; argc>1; --argc, ++argv) try{
    trace2("main", argc, argv[0]);
    // if (argc > 2 && strcmp(argv[1],"-h")==0) {
    //   for (int i=2; i<argc; ++i) {
    //     File f(argv[i]);
    //     make_h_file(f);
    //   }
    // }else
    if (strcmp(argv[0],"--cc")==0) { untested();
      f.read(argv[1]);
      make_cc(std::cout, f);
      --argc;
      ++argv;
    }else if (strcmp(argv[0],"--pp")==0
          ||  strcmp(argv[0],"-E")==0) {
      f.read(argv[1]);
      preprocess(f, std::cout);
      --argc;
      ++argv;
    }else if (argc > 1 && strcmp(argv[0],"-D")==0) {
      if(argv[0][2]) { untested();
	f.define(argv[0]+2);
      }else{
	f.define(argv[1]);
	--argc;
	++argv;
      }
    }else if (argc > 1 && strcmp(argv[0],"-I")==0) {
      if(argv[0][2]) { untested();
	f.add_include_path(argv[0]+2);
      }else{
	f.add_include_path(argv[1]);
	--argc;
	++argv;
      }
    }else if (argc > 1 && strcmp(argv[0],"--dump")==0) {
      trace1("dump", argv[1]);
      f.read(argv[1]);
      dump(f, std::cout);
      --argc;
      ++argv;
    }else if (argc > 1 && ( strcmp(argv[0],"-v")==0
	                 || strcmp(argv[0],"--version")==0 )) {untested();
      std::cerr <<
	"Gnucap verilog compiler "  PATCHLEVEL  "\n"
	"Part of the Gnu Circuit Analysis Package\n"
	"Never trust any version less than 1.0\n"
	" or any version with a number that looks like a date.\n"
	"Copyright 2001-2003, 2023 Albert Davis\n"
	"          2023, Felix Salfelder\n"
	"Gnucap comes with ABSOLUTELY NO WARRANTY\n"
	"This is free software, and you are welcome\n"
	"to redistribute it under certain conditions\n"
	"according to the GNU General Public License.\n"
	"See the file \"COPYING\" for details.\n";
//    }else if (argc > 1) {itested();
//      for (int i=1; i<argc; ++i) {itested();
//	File f;
//	f.read(argv[i]);
//	make_dump_file(f);
//	make_module_file(f, f.name()+".cc");
//      }
    }else{untested();
      throw Exception("no input files");
    }
  }catch (Exception const& e) {
    std::cout << e.message() << '\n';
  }
  return errorcount;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
