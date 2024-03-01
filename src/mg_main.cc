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
#include <u_opt.h>
#include "mg_out.h"
#include "mg_in.h"
#include "mg_pp.h"
#include <patchlev.h>
#include <c_comand.h>
#include <e_cardlist.h>
#include "mg_.h" // TODO
#include "config.h"
/*global*/ int errorcount = 0;
std::basic_ostream<char>* diag_out; // mg_error.cc
/*--------------------------------------------------------------------------*/
#if 0
static void make_module_file(const File& in, std::string dump_name)
{ untested();
  { // chop prefix path
    std::string::size_type loc = dump_name.find_last_of(ENDDIR);
    if (loc != std::string::npos) { untested();
      dump_name.erase(0, loc+1);
    }else{ untested();
      untested();
    }
  }

  // open file
  std::ofstream out(dump_name);
  if (!out) { untested();
    os_error(dump_name);
  }else{ untested();
  }

  make_cc(out, in);
}
#endif
/*--------------------------------------------------------------------------*/
class OUTPUT {
  std::basic_ostream<char>& _default;
  std::basic_ostream<char>* _o{NULL};
public:
  explicit OUTPUT(std::basic_ostream<char>& d) : _default(d) {};
  ~OUTPUT(){
    delete _o;
    _o = NULL;
  }
  void set(std::string const& name){
    delete _o;
    _o = NULL;
    _o = new std::ofstream(name);
  }

  operator std::basic_ostream<char>&(){
    if(_o){
      return *_o;
    }else{
      return _default;
    }
  }

  template<class T>
  OUTPUT& operator<<(T t){
    (std::basic_ostream<char>&) *this << t;
    return *this;
  }
};
OUTPUT output(std::cout);
OUTPUT diag(std::cerr);
/*--------------------------------------------------------------------------*/
static void prepare_env()
{
  assert(strlen(GNUCAP_PLUGPATH));
  static const char* plugpath="PLUGPATH=" GNUCAP_PLUGPATH
                              "\0         (reserved space)                 ";
  assert(strlen("PLUGPATH=") == 9);
  OS::setenv("GNUCAP_PLUGPATH", plugpath+9, false);
}
/*--------------------------------------------------------------------------*/
Base& modelgen_opts();
int main(int argc, char** argv)
{
  prepare_env();
  File f;
  Preprocessor p;
  --argc;
  ++argv;
  trace1("main", argc);
  try {
    CMD::command(std::string("load " DEFAULT_PLUGINS), &CARD_LIST::card_list);
  }catch (Exception& e) {untested();
    error(bDANGER, e.message() + '\n');
    exit(1);
  }

  for(; argc>1; --argc, ++argv) try{
    trace2("main", argc, argv[0]);
    if (strcmp(argv[0],"-o")==0) { untested();
      output.set(argv[1]);
      --argc;
      ++argv;
    }else if (strcasecmp(argv[0], "-a") == 0) { untested();
      --argc;
      ++argv;
      if (argc) { untested();
	CMD::command(std::string("attach ") + argv[0], &CARD_LIST::card_list);
      }else{untested();
      }
    }else if (strcmp(argv[0],"-d")==0
           || strcmp(argv[0],"--diag")==0) {
      diag.set(argv[1]);
      --argc;
      ++argv;
    }else if (strcmp(argv[0],"--cc")==0) {
      p.set_diag(diag);
      diag_out = &(std::basic_ostream<char>&)diag; // mg_error.cc
      p.read(argv[1]);
      f.parse(p);
      make_cc(output, f);
      --argc;
      ++argv;
    }else if (strcmp(argv[0],"--pp")==0
          ||  strcmp(argv[0],"-E")==0) {
      p.set_diag(diag);
      diag_out = &(std::basic_ostream<char>&)diag; // mg_error.cc
      p.read(argv[1]);
      p.dump(output);
      --argc;
      ++argv;
    }else if (argc > 1 && strncmp(argv[0],"-D", 2)==0) {
      if(argv[0][2]) {
	p.define(argv[0]+2);
      }else{ untested();
	p.define(argv[1]);
	--argc;
	++argv;
      }
    }else if (argc > 1 && strncmp(argv[0], "-I", 2)==0) {
      if(argv[0][2]) {
	p.add_include_path(argv[0]+2);
      }else{
	p.add_include_path(argv[1]);
	--argc;
	++argv;
      }
    }else if (argc > 1 && strcmp(argv[0],"--dump")==0) {
      trace1("dump", argv[1]);
      diag_out = &(std::basic_ostream<char>&)diag; // mg_error.cc
      p.read(argv[1]);
      File ff;
      ff.parse(p);
      ff.dump(output);
      --argc;
      ++argv;
    }else if (argc > 1 && ( strcmp(argv[0],"-v")==0
	                 || strcmp(argv[0],"--version")==0 )) {untested();
      std::cout <<
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
    }else if (argc && strncmp(argv[0], "--", 2) == 0) {itested();
      CS cmd(CS::_STRING, argv[0]+2); // command line
      modelgen_opts().parse(cmd);
    }else{untested();
      throw Exception("no input files");
    }
  }catch (Exception const& e) {
    diag << e.message() << '\n';
    exit(1);
  }

  return errorcount;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
