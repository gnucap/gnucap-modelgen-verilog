/*                                   -*- C++ -*-
 * Copyright (C) 2025 Felix Salfelder <felix@salfelder.org>
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
#include "e_cardlist.h"
#include "u_lang.h"
#include "c_comand.h"
#include "globals.h"
#include "e_node.h"
#include "e_subckt.h"
#include "u_nodemap.h"
#include "u_sim_data.h"
#include "e_usernode.h"
#include <iostream>
#include <string>
/*--------------------------------------------------------------------------*/
class CMD_DISCIPLINES : public CMD {
  void do_it(CS& cmd, CARD_LIST*)override {
    OMSTREAM o = IO::mstdout;
    outset(cmd, &o);

    for(std::pair<std::string, CKT_BASE*> dd : node_dispatcher) {
      if(dd.second){
	auto n = prechecked_cast<NODE const*>(dd.second);
	assert(n);
	o << "discipline " << dd.first << ";\n";
	for(int i=0; i < n->param_count(); ++i){
	  if(n->param_is_printable(i)) {
	    o << "  " << n->param_name(i) << " "
	              << n->param_value(i) << ";\n";
	  }else{
	  }
	}
	o << "enddiscipline\n";
      }else{
      }
    }
  }
} p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "`disciplines", &p1);
/*--------------------------------------------------------------------------*/
class CMD_NODES : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    OMSTREAM o = IO::mstdout;
    outset(cmd, &o);

    for(auto dd : *Scope->nodes()) {
      if(dd.second){
	if(auto u = prechecked_cast<USER_NODE const*>(dd.second)){
	  o << u->short_label() << " " << u->type_number() << "\n";
	}else{ untested();
	  auto n = prechecked_cast<NODE const*>(dd.second);
	  assert(n);
	  o << n->short_label() << "\n";
	}
      }else{
      }
    }
  }
} p2;
DISPATCHER<CMD>::INSTALL d2(&command_dispatcher, "`debug_nodes", &p2);
// vim:ts=8:sw=2:noet:
