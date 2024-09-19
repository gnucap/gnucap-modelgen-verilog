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
 * modelgen function plugins base
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_token.h"
#include "mg_out.h"
#include "mg_analog.h" // Probe
#include "mg_discipline.h" // Probe
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void FUNCTION_::stack_op(Expression const& arg, Expression* E) const
{
  Expression const* ee = &arg;
  bool all_float = false;
  double argv[5];
  argv[0] = NOT_VALID;
  argv[1] = NOT_VALID;
  argv[2] = NOT_VALID;
  argv[3] = NOT_VALID;
  argv[4] = NOT_VALID;
  double* seek = argv;
  for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
    trace1("float?", (**i).name());
    auto F = dynamic_cast<Float const*>((**i).data());
    all_float = F;
    if(!all_float){
      break;
    }else{
      *seek = F->value();
      ++seek;
      assert(seek<argv+5);
    }
  }

  if(!all_float){
    throw Exception("invalid");
    // restore argument.
  }else if(arg.size()==1){
    double value = evalf(argv);
    const Float* v = new Float(value);
    E->push_back(new Token_CONSTANT(to_string(value), v, ""));
  }else if(arg.size()==2){
    double value = evalf(argv);
    const Float* v = new Float(value);
    E->push_back(new Token_CONSTANT(to_string(value), v, ""));
  }else if(arg.size()==3){
    double value = evalf(argv);
    const Float* v = new Float(value);
    E->push_back(new Token_CONSTANT(to_string(value), v, ""));
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Node_Ref MGVAMS_FILTER::p() const
{ untested();
  return Node_Ref(); // (&Node_Map::mg_ground_node);
}
/*--------------------------------------------------------------------------*/
Node_Ref MGVAMS_FILTER::n() const
{ untested();
  return Node_Ref(); // (&Node_Map::mg_ground_node);
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FILTER::setup(Module* m)
{
  if(has_refs()){
  }else{
  }
  if(branch()){
    int c_cnt = 0;
    bool assigned = false;
    bool always = false;
    bool output_var = false;
    bool rdeps = false;
    Contribution const* cont = NULL;
    trace1("filter used_in?", branch()->used_in().size());
    for(auto b : branch()->used_in()) {
      trace0("filter use ..");
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
      }else if(auto aa=dynamic_cast<Assignment const*>(b)){
	trace1("use in assignment", aa->is_output_var());
	assigned = true;
	output_var = aa->is_output_var();
      }else if(auto bb=dynamic_cast<Branch const*>(b)){
	trace1("use in branch", bb->code_name());
	rdeps = true;
	// covered by rdeps?
      }else if(dynamic_cast<Variable_List_Collection const*>(b)){ untested();
      }else if(b == &tr_accept_tag) {
      }else{
	trace1("xdt unknown?", c_cnt);
	unreachable();
      }
    }
    // for(auto b : branch()->deps().rdeps()) { untested();
    //   (void)b;
    //   rdeps = true;
    // }

    trace4("filter use?", c_cnt, rdeps, assigned, branch()->code_name());
    _output = NULL;
    if(output_var){
      // can't tet optimise out if there is both
      // single contribution and output var...
    }else{
    }
    if(!has_refs()){
      set_p_to_gnd(m);
   // }else if(cont && cont->has_sensitivities()) { untested();
      // BUG.
    }else if(c_cnt == 1 && always && !output_var){
      assert(cont);
      for(auto d : cont->ddeps()){
	if(d->branch() != branch()) {
	}else if(d.is_linear()){
	  incomplete(); // propagate loss?
	  _output = cont->branch(); // polarity?
	  set_p_to_gnd(m);
	}
	if(cont->reversed()){
	}else{
	}
      }
    }else if(rdeps){
    }else if(c_cnt==0){
      incomplete(); // analysis?
      set_p_to_gnd(m);
      // func->_output = cont->branch(); // polarity?
    }else if(assigned){ untested();
    }else if(c_cnt!=1){ untested();
    }else{
      incomplete(); // later
      // func->set_p_to_gnd();
    }
  }else{

  }
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FILTER::set_n_to_gnd(Module* m) const
{
  assert(m);
  assert(branch());
  m->set_to_ground(branch()->n());
  assert(n()->is_ground());
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FILTER::set_p_to_gnd(Module* m) const
{
  assert(m);
  assert(branch());
  m->set_to_ground(branch()->p());
  assert(p()->is_ground());
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
