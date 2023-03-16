/*$Id: mg_out_dev.cc 2014/11/23 al$ -*- C++ -*-
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
 */
#include "mg_out.h"
#include "m_tokens.h"
/*--------------------------------------------------------------------------*/
static bool is_xs_function(std::string const& n)
{
  // stub, need discipline.h
  return n == "V" || n == "I";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_dev_dispatcher(std::ostream& out, const Device& d)
{
  out << 
      "namespace DEV_" << d.name() << "_DISPATCHER { \n"
    "  static DEV_" << d.name() << " p0;\n"
    "  static DISPATCHER<CARD>::INSTALL\n"
    "    d0(&device_dispatcher, \"";
  if (d.id_letter() != "") {
    out << d.id_letter() << '|';
  }else{
  }
  out << d.parse_name() << "\", &p0);\n"
    "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_dev_eval(std::ostream& out, const Eval& e,
		const String_Arg& dev_type, const String_Arg& model_type)
{
  make_tag();
  std::string class_name = "EVAL_" + dev_type.to_string() + '_'
    + e.name().to_string();
  out << "static " << class_name << " Eval_" << e.name().to_string() 
      << "(CC_STATIC);\n"
    "void " << class_name << "::tr_eval(ELEMENT* d)const\n"
    "{\n"
    "  assert(d);\n"
    "  DEV_" << dev_type << "* p = prechecked_cast<DEV_"
      << dev_type << "*>(d->owner());\n"
    "  assert(p);\n"
    "  const COMMON_" << dev_type << "* c = prechecked_cast<const COMMON_"
      << dev_type << "*>(p->common());\n"
    "  assert(c);\n"
    "  const SDP_" << model_type << "* s = prechecked_cast<const SDP_"
      << model_type << "*>(c->sdp());\n"
    "  assert(s);\n"
    "  const MODEL_" << model_type << "* m = prechecked_cast<const MODEL_"
      << model_type << "*>(c->model());\n"
    "  assert(m);\n"
      << e.code() <<
    "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_dev_evals(std::ostream& out, const Device& d)
{
  make_tag();
  for (Eval_List::const_iterator
       e = d.eval_list().begin();
       e != d.eval_list().end();
       ++e) {
    make_dev_eval(out, **e, d.name(), d.model_type());
  }
}
/*--------------------------------------------------------------------------*/
#if 0
void make_dev_copy_constructor(std::ostream& out, const Module& d)
{
  make_tag();
  out << "DEV_" << d.name() << "::DEV_" << d.name()
      << "(const DEV_" << d.name() << "& p)\n"
    "  :BASE_SUBCKT(p)";

  out << ",\n   // input parameters";
  make_copy_construct_parameter_list(out, d.parameters());

  out << ",\n   // calculated parameters";
  make_copy_construct_parameter_list(out, d.device().calculated());

  out << ",\n   // netlist";
  for (Element_1_List::const_iterator
       p = d.circuit().elements().begin();
       p != d.circuit().elements().end();
       ++p) {
    out << ",\n   _" << (**p).name() << "(0)";
  }

  out << "\n{\n"
    "  _n = _nodes;\n"
    "  for (int ii = 0; ii < max_nodes() + int_nodes(); ++ii) {\n"
    "    _n[ii] = p._n[ii];\n"
    "  }\n"
    "  ++_count;\n";

  out << "  // overrides\n";
  for (Parameter_1_List::const_iterator
       p = d.device().override().begin();
       p != d.device().override().end();
       ++p) {untested();
    out << ",\n   " << (**p).code_name() << "(p." << (**p).code_name() << ")";
  }
  out << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
#endif
/*--------------------------------------------------------------------------*/
static void make_set_parameters(std::ostream& out, const Element_1& e)
{
  make_tag();
  out << "      _" << e.name() << "->set_parameters(\"" << e.name() << "\", this, ";
  if (e.eval() != "") {
    out << "&Eval_" << e.eval();
  }else if (e.args() != "") {
    out << "c->_" << e.args();
  }else{
    out << "NULL";
  }
  out << ", " << ((e.value() != "") ? e.value() : "0.");
  if (e.state() != "") {
    out << ", " << e.num_nodes()/2+1 << ", &" << e.state();
  }else{
    out << ", 0, NULL";
  }
  out << ", " << e.num_nodes() << ", nodes);\n";
}
/*--------------------------------------------------------------------------*/
static void make_dev_expand_one_element(std::ostream& out, const Element_1& e)
{
  make_tag();
  if (!(e.omit().empty())) {
    out <<
      "    if (" << e.omit() << ") {\n"
      "      if (_" << e.name() << ") {\n"
      "        subckt()->erase(_" << e.name() << ");\n"
      "        _" << e.name() << " = NULL;\n"
      "      }else{\n"
      "      }\n"
      "    }else{\n";
  }else{
    out << "    {\n";
  }
  
  out <<
    "      if (!_" << e.name() << ") {\n"
    "        const CARD* p = device_dispatcher[\"" << e.dev_type() << "\"];\n"
    "        assert(p);\n"
    "        _" << e.name() << " = dynamic_cast<COMPONENT*>(p->clone());\n"
    "        assert(_" << e.name() << ");\n"
    "        subckt()->push_front(_" << e.name() << ");\n"
    "      }else{\n"
    "      }\n";
  
  if (!(e.reverse().empty())) {
    out << "      if (" << e.reverse() << ") {\n";
    out << "        node_t nodes[] = {";
    
    Port_1_List::const_iterator p = e.ports().begin();
    if (p != e.ports().end()) {
      Port_1_List::const_iterator even = p;
      ++p;
      assert(p != e.ports().end());
      out<< "_n[n_" << (**p).name() << "], _n[n_" << (**even).name() << "]";
      bool even_node = true;
      while (++p != e.ports().end()) {untested();
	if (even_node) {untested();
	  even_node = false;
	  even = p;
	}else{untested();
	  even_node = true;
	  out<< ", _n[n_"<< (**p).name()<< "], _n[n_"<<(**even).name()<< "]";
	}
      }
    }else{untested();
    }
    
    out << "  };\n";
    make_set_parameters(out, e);
    out << "      }else{\n";
  }else{
    out << "      {\n";
  }
  
  out << "        node_t nodes[] = {";
  
  Port_1_List::const_iterator p = e.ports().begin();
  if (p != e.ports().end()) {
    assert(*p);
    out << "_n[n_" << (**p).name() << "]";
    while (++p != e.ports().end()) {
      out << ", _n[n_" << (**p).name() << "]";
    }
  }else{untested();
  }
  
  out << "};\n";
  make_set_parameters(out, e);
  out << "      }\n";
  out << "    }\n";
}
/*--------------------------------------------------------------------------*/
static void make_dev_allocate_local_nodes(std::ostream& out, const Port_1& p)
{
  make_tag();
  if (p.short_if().empty()) {
    out <<
      "    if (!(_n[n_" << p.name() << "].n_())) {\n"
      "      _n[n_" << p.name() << "] = _n[n_" << p.short_to() << "];\n"
      "    }else{\n"
      "    }\n";
    //BUG// generates bad code if no short_to
  }else{
    out <<
      "    //assert(!(_n[n_" << p.name() << "].n_()));\n"
      "    //BUG// this assert fails on a repeat elaboration after a change.\n"
      "    //not sure of consequences when new_model_node called twice.\n"
      "    if (!(_n[n_" << p.name() << "].n_())) {\n"
      "      if (" << p.short_if() << ") {\n"
      "        _n[n_" << p.name() << "] = _n[n_" << p.short_to() << "];\n"
      "      }else{\n"
      "        _n[n_" << p.name() << "].new_model_node(\".\" + long_label() + \"." << p.name() 
			   << "\", this);\n"
      "      }\n"
      "    }else{\n"
      "      if (" << p.short_if() << ") {\n"
      "        assert(_n[n_" << p.name() << "] == _n[n_" << p.short_to() << "]);\n"
      "      }else{\n"
      "        //_n[n_" << p.name() << "].new_model_node(\"" << p.name() 
		 << ".\" + long_label(), this);\n"
      "      }\n"
      "    }\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_dev_expand(std::ostream& out, const Device& d)
{
  make_tag();
  out << "void DEV_" << d.name() << "::expand()\n"
    "{\n"
    "  BASE_SUBCKT::expand(); // calls common->expand, attached model\n"
    "  assert(_n);\n"
    "  assert(common());\n"
    "  const COMMON_" << d.name() << "* c = static_cast<const COMMON_"
      << d.name() << "*>(common());\n"
    "  assert(c);\n"
    "  assert(c->model());\n"
    "  const MODEL_" << d.model_type() << "* m = prechecked_cast<const MODEL_"
      << d.model_type() << "*>(c->model());\n"
    "  assert(m);\n"
    "  assert(c->sdp());\n"
    "  const SDP_" << d.model_type() << "* s = prechecked_cast<const SDP_"
      << d.model_type() << "*>(c->sdp());\n"
    "  assert(s);\n"
    "  if (!subckt()) {\n"
    "    new_subckt();\n"
    "  }else{\n"
    "  }\n"
    "\n"
    "  if (_sim->is_first_expand()) {\n"
    "    precalc_first();\n"
    "    precalc_last();\n"
    "    // optional nodes\n";
  for (Port_1_List::const_iterator
       p = d.circuit().opt_nodes().begin();
       p != d.circuit().opt_nodes().end();
       ++p) {
    make_dev_allocate_local_nodes(out, **p);
  }
  out << "    // local nodes\n";
  for (Port_1_List::const_iterator
       p = d.circuit().local_nodes().begin();
       p != d.circuit().local_nodes().end();
       ++p) {
    make_dev_allocate_local_nodes(out, **p);
  }
  out << "\n"
    "    // clone subckt elements\n";
  for (Element_1_List::const_iterator
       e = d.circuit().elements().begin();
       e != d.circuit().elements().end();
       ++e) {
    make_dev_expand_one_element(out, **e);
  }
  out <<
    "  }else{\n"
    "    //precalc();\n"
    "  }\n"
    "  //precalc();\n"
    "  subckt()->expand();\n"
    "  //subckt()->precalc();\n"
    "  assert(!is_constant());\n";
  if (d.circuit().sync()) {
    out << "  subckt()->set_slave();\n";
  }else{
  }
  out << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static std::string fix_expression(const std::string& in)
{
  std::string out;
  out[0] = '\0';
  
  CS x(CS::_STRING, in);
  for (;;) {
    if (x.peek() == '@') {
      x.skip1('@');
      std::string object(x.ctos("[,"));
      x.skip1('[');
      std::string attrib(x.ctos("]"));
      x.skip1(']');
      if (object[0] == 'n') {
	out += " _n[" + object + "]";
	if (attrib != "") {
	  out += ".v0()";
	}else{
	}
      }else{
	out += " CARD::probe(_" + object + ",\"" + attrib + "\")";
      }
    }else if (x.more()) {
      out += ' ' + x.ctos("@");
    }else{
      break;
    }
  }
  return out;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_probe_parameter(std::ostream& out, const Parameter_1& p)
{
  make_tag();
  out << "if (Umatch(x, \"";
  if (!(p.alt_name().empty())) {untested();
    out << to_lower(p.alt_name()) << " \" || \"";
  }
  out << to_lower(p.user_name()) << " \")) {\n"
    "    return " << p.code_name() << ";\n"
    "  }else ";
}
/*--------------------------------------------------------------------------*/
void make_probe_parameter_list(std::ostream& out,const Parameter_1_List& pl)
{
  make_tag();
  for (Parameter_1_List::const_iterator
       p = pl.begin();
       p != pl.end();
       ++p) {
    make_probe_parameter(out, **p);
  }
}
/*--------------------------------------------------------------------------*/
static void make_dev_probe(std::ostream& out, const Device& d)
{
  make_tag();
  out << "double DEV_" << d.name() << "::tr_probe_num(const std::string& x)const\n"
    "{\n"
    "  assert(_n);\n"
    "  const COMMON_" << d.name() << "* c = prechecked_cast<const COMMON_"
      << d.name() << "*>(common());\n"
    "  assert(c);\n"
    "  const MODEL_" << d.model_type() << "* m = prechecked_cast<const MODEL_"
      << d.model_type() << "*>(c->model());\n"
    "  assert(m);\n"
    "  const SDP_" << d.model_type() << "* s = prechecked_cast<const SDP_"
      << d.model_type() << "*>(c->sdp());\n"
    "  assert(s);\n"
    "\n"
    "  ";
//   for (Probe_List::const_iterator
//        p = d.probes().begin();
//        p != d.probes().end();
//        ++p) {
//     assert(*p);
//     out << "if (Umatch(x, \"" << (**p).name() << " \")) {\n"
//       "    return " << fix_expression((**p).expression()) << ";\n"
//       "  }else ";
//   }
  make_probe_parameter_list(out, d.device().calculated());
  out << "{\n"
    "    return BASE_SUBCKT::tr_probe_num(x);\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_dev_aux(std::ostream& out, const Device& d)
{
  make_tag();
  for (Function_List::const_iterator
       p = d.function_list().begin();
       p != d.function_list().end();
       ++p) {
    out << "void DEV_" << d.name() << "::" << (**p).name() << "\n"
      "{\n"
	<< (**p).code() <<
      "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  }
}
/*--------------------------------------------------------------------------*/
#if 0
void make_cc_dev(std::ostream& out, const Device& d)
{
  make_tag();
  make_dev_dispatcher(out, d);
  make_dev_evals(out, d);
  make_dev_default_constructor(out, d);
  make_dev_copy_constructor(out, d);
  make_dev_expand(out, d);
  make_dev_probe(out, d);
  make_dev_aux(out, d);
  out << "/*--------------------------------------"
    "------------------------------------*/\n";
}
#endif
/*--------------------------------------------------------------------------*/
#if 0
// not dump
inline void AnalogBlock::dump(std::ostream& o)const
{
  o << "bool Module::do_tr(){ // AnalogBlock\n";
  o << ind << "clear_branch_contributions();\n";
  o << ind << "read_voltages();\n";
  for(auto i : *this) {
    // need commmon baseclass...
    if(auto v=dynamic_cast<Variable const*>(i)) {
      v->dump(o);
    }else if(auto f=dynamic_cast<FlowContribution const*>(i)) { untested();
      f->dump(o);
    }else if(auto p=dynamic_cast<PotContribution const*>(i)) { untested();
      p->dump(o);
    }else{ untested();
      incomplete();
    }
  }
  o << ind << "return true;\n";
  o << "} // AnalogBlock\n";
}
/*--------------------------------------------------------------------------*/
inline void Variable::resolve_symbols(Expression const& e, Expression& E)
{
  assert(ctx());
  // resolve symbols
  for(auto i : e) {
    trace1("resolve top:", i->name());

    auto s = dynamic_cast<Token_SYMBOL*>(i);
    std::string const& n = i->name();
    Base const* r = ctx()->resolve(n);
    trace1("resolve top found:", r);

    if(dynamic_cast<Token_STOP*>(i)) {
      trace1("resolve STOP?", i->name());
      Token* cl = i->clone();
      assert(i->name() == cl->name());
      E.push_back(cl);
    }else if(auto c = dynamic_cast<Token_CONSTANT*>(i)) { untested();
      Token* cl = c->clone();
      assert(i->name() == cl->name());
      E.push_back(cl);
    }else if(!s) {
      trace2("huh", name(), i->name());
      E.push_back(i->clone());
    }else if(is_xs_function(i->name())) {
      if(E.is_empty()) { untested();
	throw Exception("syntax error");
      }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
	throw Exception("syntax error");
      }else{
	delete E.back();
	E.pop_back();
	assert(!E.is_empty());
	std::string arg1;
	if(dynamic_cast<Token_STOP*>(E.back())) { untested();
	  throw Exception("syntax error");
	}else{
	}
	std::string arg0 = E.back()->name();
	delete E.back();
	E.pop_back();
	assert(!E.is_empty());

	while(!dynamic_cast<Token_STOP*>(E.back())) {
	  arg1 = E.back()->name();
	  delete E.back();
	  E.pop_back();
	  assert(!E.is_empty());
	}

	delete E.back();
	E.pop_back();
	Probe const* p = _ctx->new_probe(n, arg1, arg0);
	_deps.insert(p);
	E.push_back(new Token_PROBE(p->name(), p));

	trace3("got a probe", n, arg1, this);
      }
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      trace2("resolve: variable", name(), v->name());
      E.push_back(new Token_VAR_REF(n, v));
      for(auto d : v->deps()) {
	_deps.insert(d);
      }
    }else if(auto p = dynamic_cast<Probe const*>(r)) { untested();
      trace1("resolve: probe dep", p->name());
      E.push_back(new Token_PROBE(n, p));
      _deps.insert(p);
    }else if(r) { untested();
      assert(0);
      incomplete(); // unresolved symbol?
    }else if (strchr("0123456789.", n[0])) {
      // a number
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(i->name(), f, ""));
    }else if(is_function(i->name())) {
      E.push_back(i->clone()); // try later?
    }else if(is_node(i->name())) {
      E.push_back(i->clone()); // try later?
    }else{ untested();
      throw Exception("unresolved symbol: " + n);
    }
  }
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
