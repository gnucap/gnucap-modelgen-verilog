/*$Id: c_param.cc,v 26.130 2009/11/15 21:51:59 al Exp $ -*- C++ -*-
 * Copyright (C) 2005 Albert Davis
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
 */
#include "globals.h"
#include "c_comand.h"
#include "u_parameter.h"
#include "d_dot.h"
#include "e_cardlist.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
void parse(CS& cmd, PARAM_LIST* pl, CARD* owner)
{
  assert(pl);
  CARD_LIST* Scope;
  if(!owner){
    Scope = &CARD_LIST::card_list;
  }else{
    Scope = owner->scope();
  }
  PARAM_INSTANCE par;
  int type = 0;
  if(cmd >> "real"){
    type = 1;
  }else if(cmd >> "integer"){
    type = 2;
  }else if(cmd >> "string"){ untested();
    type = 3;
  }else{
  }
  assert(Scope);
  if(Scope->is_verilog_math()){ untested();
    type += 10;
    if(Scope == &CARD_LIST::card_list) { untested();
      error(bLOG, "Top level parameter. Not within spec, use with care\n");
    }else{ untested();
    }
  }else if(type == 0){
    error(bNOERROR, "Untyped top level parameter. Assuming Float\n");
  }

  switch(type){
  case 1:
	  par = PARAMETER<Float>();
	  break;
  case 11:untested();
	  par = PARAMETER<vReal>();
	  break;
  case 2:
	  par = PARAMETER<Integer>();
	  break;
  case 12:untested();
	  par = PARAMETER<vInteger>();
	  break;
  case 3:untested();
	  par = PARAMETER<vString>(); // !
	  break;
  case 13:untested();
	  par = PARAMETER<vString>();
	  break;
  case 10:untested();
	  par = PARAMETER<vReal>();
	  // fallback. see lang_verilog.
	  break;
  default:
	  par = PARAMETER<Float>();
	  break;
  }

  size_t here = cmd.cursor();
  for (;;) {
    if (!(cmd.more() && (cmd.is_alpha() || cmd.match1('_')))) {
      break;
    }else{
    }
    Name_String Name_;
    std::string Name;

    cmd >> Name_ >> '=' >> par;
    Name = Name_;

    trace2("parsed", Name, par.string());
    if (cmd.stuck(&here)) {untested();
      break;
    }else{
    }
    if (OPT::case_insensitive) {
      // error(bPICKY, "parameter " + Name + " in insensitve mode\n");
      notstd::to_lower(&Name);
    }else{
    }
    pl->set(Name, par);
    if(owner){
      owner->set_param_by_name(Name, "");
    }else{
    }
  }
  cmd.check(bDANGER, "syntax error");
}
/*--------------------------------------------------------------------------*/
class CMD_PARAM : public CMD {
public:
  explicit CMD_PARAM() : CMD() {}
private:
  explicit CMD_PARAM(CMD_PARAM const& p) : CMD(p) {}
  CMD* clone()const override {return new CMD_PARAM(*this); }
public:
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    if(owner()){
    }else{
    }
    PARAM_LIST* pl = Scope->params();
    if (cmd.is_end()) { untested();
      pl->print(IO::mstdout, OPT::language);
      IO::mstdout << '\n';
    }else{
      parse(cmd, pl, owner());
    }
  }
} p;
DISPATCHER<CMD>::INSTALL d(&command_dispatcher, "param|parameters|parameter", &p);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
