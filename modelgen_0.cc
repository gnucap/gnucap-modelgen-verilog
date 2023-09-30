/*                        -*- C++ -*-
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
 * mockup component generator for analog block testing
 */
#include <globals.h>
#include "mg_expression.h"
#include <c_comand.h>
#include <e_cardlist.h>
#include <stack>
#include <set>
#include <fstream>
#include "mg_.h"
#include "mg_pp.h"
#include "mg_out.h"
#include "m_tokens.h"
/*--------------------------------------------------------------------------*/
class Variable;
class Probe;
class AnalogBlock;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
Preprocessor& pp()
{
  static Preprocessor p;
  return p;
}
/*--------------------------------------------------------------------------*/
class CMD_ : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST*)override {
    if(OPT::case_insensitive == 0){
    }else{ untested();
      error(bWARNING, "running modelgen in insensitive mode\n");
    }

    std::string name;

    std::string module_content;
    for (;;) {
      cmd.get_line("verilog-module>");
      trace1("content", cmd.fullstring());

      module_content += cmd.fullstring();
      if (cmd >> "endmodule ") {
	break;
      }else{
      }
    }


    CS file(CS::_STRING, "");
    file = module_content;
    file >> "module ";
//    size_t here = file.cursor();
    file >> name;
    file.reset();

    File F;

    {
      Preprocessor& p = pp();
      p.read("../disciplines.vams");
      F.parse(p);
    }

    file >> F;

    std::ofstream o;
    o.open(name + ".cc");
    make_cc(o, F);
    o.close();
  }
} p0;
DISPATCHER<CMD>::INSTALL d0(&command_dispatcher, "`modelgen", &p0);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
