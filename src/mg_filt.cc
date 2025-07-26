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
 * Verilog-AMS filters
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include "mg_analog.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
void filter_setup(MGVAMS_FILTER* f, Module* m)
{
  trace1("filter setup", f->code_name());
  if(f->has_refs()){
  }else{
  }
  if(f->branch()){
    int c_cnt = 0;
    bool assigned = false;
    bool always = false;
    bool output_var = false;
    bool rdeps_ = false;
    Contribution const* cont = nullptr;
    trace1("filter used_in?", f->branch()->used_in().size());

    // use rdeps, not used_in....?
   // for(Base const* b : rdeps()) { untested();
   // }
    for(Base const* b : f->branch()->used_in()) {
      trace2("filter use ..", f->code_name(), typeid(*b).name());
      if(auto c = dynamic_cast<Contribution const*>(b)){
	if(c->is_flow_contrib()) {
	  trace1("filter used_in flow", c->name());
	  ++c_cnt;
	  cont = c;
	}else{
	  trace1("filter used else", c->name());
	  ++c_cnt;
	  ++c_cnt;
	  // cont = c;
	  incomplete();
	}
	if(c->is_always()){
	  always = true;
	}else{
	}
      }else if(auto aa=dynamic_cast<AnalogProceduralAssignment const*>(b)){ untested();
	if(aa->is_always()){ untested();
	  always = true;
	}else{ untested();
	}
      }else if(auto a2=dynamic_cast<Assignment const*>(b)){
	trace1("use in assignment", a2->is_output_var());
	assigned = true;
	output_var = a2->is_output_var();

      }else if(auto bb=dynamic_cast<Branch const*>(b)){
	trace1("use in branch", bb->code_name());
	if(f->branch() == bb){ untested();
	}else{
	  rdeps_ = true;
	}
	// covered by rdeps?
      }else if(dynamic_cast<Variable_List_Collection const*>(b)){ untested();
      }else if(b == &tr_accept_tag) {
      }else if(b == &tr_begin_tag) {
      }else if(b == &tr_restore_tag) {
      }else if(b == &tr_advance_tag) {
      }else{ untested();
	trace1("xdt unknown?", c_cnt);
	unreachable();
      }
    }
    // for(auto b : branch()->deps().rdeps()) { untested();
    //   (void)b;
    //   rdeps = true;
    // }

    trace4("filter use?", c_cnt, rdeps_, assigned, f->branch()->code_name());
    f->set_output(nullptr);
    if(output_var){
      // can't optimise out if there is both
      // single contribution and output var...
    }else{
    }
    if(!f->has_refs()){
      f->set_p_to_gnd(m);
   // }else if(cont && cont->has_sensitivities()) { untested();
      // BUG.
    }else if(c_cnt == 1 && always && !output_var){
      assert(cont);
      for(auto d : cont->ddeps()){
	if(d->branch() != f->branch()) {
	}else if(d.is_linear()){
	  if(cont->is_pot_contrib()){ untested();
	    incomplete(); // propagate loss?
	  }else{
	  }
	  f->set_output(cont->branch()); // polarity?
	}
	if(cont->reversed()){
	}else{
	}
      }
    }else if(rdeps_){
      // ?
    }else if(c_cnt==0){
      if(rdeps_){ untested();
	incomplete(); // analysis?
      }else{
      }
      trace5("filter use2", f->branch()->code_name(), always, output_var, assigned, c_cnt);
      f->set_p_to_gnd(m);
      // func->_output = cont->branch(); // polarity?
    }else if(assigned){ untested();
    }else if(c_cnt!=1){ untested();
    }else{ untested();
      unreachable();
      incomplete(); // later
      // func->set_p_to_gnd();
    }
  }else{
    assert(!f->has_output());
  }

  if(f->has_output()){
    trace1("filter output", f->branch()->code_name());
    f->set_p_to_gnd(m);
  }else{
  }

}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
