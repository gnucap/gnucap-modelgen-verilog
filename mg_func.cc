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
#include "m_tokens.h"
#include "mg_out.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Token* MGVAMS_FUNCTION::new_token(Module& m, size_t /*na*/, Deps&) const
{
  m.install(this);
  if(code_name() != ""){
    return new Token_CALL(label(), this);
  }else if(label() != ""){
    return new Token_CALL(label(), this);
  }else{
    incomplete();
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FUNCTION::make_cc_dev(std::ostream& o) const
{
  o__ "// " << label() << "\n";
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FUNCTION::make_cc_common(std::ostream& o) const
{
  o__ "// " << label() << "\n";
}
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
  }else{
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
double FUNCTION_::evalf(double const*)const
{
  throw Exception("not implemented");
}
/*--------------------------------------------------------------------------*/
FUNCTION_::~FUNCTION_()
{
  if(has_refs()){
    incomplete();
    std::cerr << "stale ref " << _label << "\n";
  }else{
  }
//  assert(has_refs());
}
/*--------------------------------------------------------------------------*/
Token* VAMS_ACCESS::new_token(Module& m, size_t na, Deps& d)const
{
  // use na?
  Branch_Ref br = m.new_branch(_arg0, _arg1);
  //  br->set_owner(this);
  assert(br);
  assert(const_cast<Branch const*>(br.operator->())->owner());
  // Probe const* p = m.new_probe(_name, _arg0, _arg1);
  //
  // install clone?
  FUNCTION_ const* p = m.new_probe(_name, br);

  return p->new_token(m, na, d);
}
/*--------------------------------------------------------------------------*/
Token* Probe::new_token(Module& m, size_t na, Deps& d)const
{
  std::string name;
  if(discipline()){
    if(_type==t_pot){
      assert(discipline()->potential());
      name = discipline()->potential()->access().to_string();
    }else if(_type==t_flow){
      assert(discipline()->flow());
      name = discipline()->flow()->access().to_string();
    }else{
      name = "UNKNOWN";
    }
  }else if(_type==t_pot){ untested();
    name = "potential";
  }else if(_type==t_flow){ untested();
    name = "flow";
  }else{ untested();
    unreachable();
    name = "UNKNOWN";
  }

  trace4("got a probe", name, na, pname(), nname());
  name += "(";
  if(_br.has_name()){
    name += "alias";
  }else if(nname() != ""){
    assert(na==2);
    name += pname() + ", " + nname();
  }else{
    name += pname();
    assert(na==1);
  }
  name += ")";

  Token_ACCESS* nt = new Token_ACCESS(name, this);
  assert(d.empty());
  d.insert(Dep(nt->prb(), Dep::_LINEAR));
  return nt;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
