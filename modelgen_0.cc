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
#include <gnucap/globals.h>
#include <gnucap/m_expression.h>
#include <gnucap/c_comand.h>
#include <gnucap/e_cardlist.h>
#include <stack>
#include <set>
#include <fstream>
#include "mg_.h"
#include "mg_out.h"
#include "m_tokens.h"
/*--------------------------------------------------------------------------*/
class Variable;
class Probe;
class AnalogBlock;
/*--------------------------------------------------------------------------*/
static int is_function(std::string const& n)
{
  // stub, need sth.
  if (n == "exp"
   || n == "log"
   || n == "cos"
   || n == "sin") {
    return 1;
  }else if (n == "pow"){
    return 2;
  }else{
    return 0;
  }
}
/*--------------------------------------------------------------------------*/
static bool is_node(std::string const& n)
{
  // stub, need module/node
  return n.size() && n[0] == 'n';
}
/*--------------------------------------------------------------------------*/
static bool is_pot_function(std::string const& n)
{ untested();
  // stub, need discipline.h
  return n == "V";
}
/*--------------------------------------------------------------------------*/
static bool is_flow_function(std::string const& n)
{
  // stub, need discipline.h
  return n == "I";
}
/*--------------------------------------------------------------------------*/
static bool is_xs_function(std::string const& n)
{
  // stub, need discipline.h
  return n == "V" || n == "I";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#if 0
class Branch : public Base {
  std::string _pname; // TODO: object...
  std::string _nname; // TODO: object...
public:
  explicit Branch(std::string const& p, std::string const& n)
    : _pname(p), _nname(n) {}
  std::string name()const {return _pname+_nname;}
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override {incomplete();}
};
/*--------------------------------------------------------------------------*/
class Probe : public Base {
  std::string _name;
//  Branch const* _p;
  std::string _pp;
  std::string _pn;
public:
  explicit Probe(std::string const& Name, std::string const&, std::string const& p, std::string const& n)
    : _name(Name), _pp(p), _pn(n) {}
  std::string const& name()const {return _name;}
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override {incomplete();}

  std::string const& pname() const{ return _pp; }
  std::string const& nname() const{ return _pn; }
};
/*--------------------------------------------------------------------------*/
class Block;
class Variable : public Base {
private:
  Block* _ctx;
  std::string _name;
protected:
  std::set<Probe const*> _deps;
  Block const* ctx() const{ return _ctx; }
public:
  std::set<Probe const*> const& deps()const { return _deps; }
  Variable(std::string const& name, Block* ctx)
   :Base()
   ,_ctx(ctx)
   ,_name(name)
  {}
  ~Variable() {
  }
public:
  std::string const& name()const {return _name;}
  virtual void parse(CS&) { untested();
    incomplete();
  }
  void dump(std::ostream& o)const override;
  Branch const* new_branch(std::string const& a, std::string const& b);

protected:
  void resolve_symbols(Expression const& e, Expression& E);
};
#endif
/*--------------------------------------------------------------------------*/
#if 0
class Assignment : public Variable {
protected:
  std::string _lhsname;
  Expression* _rhs;
public:
  Assignment(std::string const& lhsname, Block* ctx)
   :Variable(lhsname, ctx)
   ,_lhsname(lhsname)
   ,_rhs(NULL) {}
  ~Assignment() {
    delete _rhs;
  }
public:
  virtual void parse(CS& cmd) {
    trace1("Assignment::parse", cmd.tail());
    Expression rhs(cmd);
    assert(!_rhs);
#if 1
    Expression tmp;
    resolve_symbols(rhs, tmp);
    trace1("Assignment::parse resolved", rhs.size());
    _rhs = new Expression(tmp, &CARD_LIST::card_list);
#else
    Expression tmp(rhs, &CARD_LIST::card_list);
    _rhs = new Expression();
    resolve_symbols(tmp, *_rhs);
#endif
  }
  virtual void dump(std::ostream& o)const override;

};
/*--------------------------------------------------------------------------*/
class Block : public List_Base<Base> {
  typedef std::map<std::string, Base*> map;
  typedef map::const_iterator const_iterator;
protected:
  map _items;
public:
  virtual Probe const* new_probe(std::string const& xs, std::string const& p, std::string const& n) = 0;
  virtual Branch const* new_branch(std::string const& p, std::string const& n) = 0;
  Base const* find(std::string const& k) const{
    const_iterator f = _items.find(k);
    if(f!=_items.end()) {
      return f->second;
    }else{
      return NULL;
    }
  }
  Base* item(std::string const&name) { untested();
    const_iterator f = _items.find(name);
    if(f != _items.end()) { untested();
      return f->second;
    }else{ untested();
      return NULL;
    }
  }
};
/*--------------------------------------------------------------------------*/
class Contribution : public Assignment {
protected: // BUG
  std::string _name;
protected:
  Branch const* _branch;
public:
  Contribution(std::string const& lhsname, Block* ctx)
    : Assignment(lhsname, ctx), _branch(NULL) {}

  std::string const& name() const{return _name;}
};
/*--------------------------------------------------------------------------*/
class PotContribution : public Contribution {
public:
  PotContribution(std::string const& lhsname, Block* ctx) : Contribution(lhsname, ctx) {}
public:
  void parse(CS& cmd)override {}
  void dump(std::ostream& o)const override{}
};
/*--------------------------------------------------------------------------*/
class FlowContribution : public Contribution {
public:
  FlowContribution(std::string const& lhsname, Block* ctx) : Contribution(lhsname, ctx) {}
public:
  virtual void parse(CS& cmd)override;
  virtual void dump(std::ostream& o)const override;
};
/*--------------------------------------------------------------------------*/
class AnalogBlock : public Block {
public:
  Block* _ctx{nullptr};
public:
  void set_ctx(Block* ctx) {_ctx = ctx;}

  // this is a stub
  void parse_item(CS& cmd) {
    std::string what;
    cmd >> what;
    if(is_flow_function(what)) {
      assert(_ctx);
      FlowContribution* a = new FlowContribution(what, this);
      a->parse(cmd);
      push_back(a);
    }else if(what == "int") { untested();
    }else if(what == "real") {
      trace1("AnalogBlock::parse real", cmd.tail());

      std::string name;
      cmd >> name >> ';';
      Variable* a = new Variable(name, this);
      push_back(a);
      trace1("AnalogBlock::parse real", name);
      _items[name] = a;

    }else if(cmd >> "*=") { untested();
      incomplete();
    }else if(cmd >> "+=") { untested();
      incomplete();
    }else if(cmd >> "=") {
      Assignment* a = new Assignment(what, this);
      a->parse(cmd);
      push_back(a);
      _items[what] = a;
    }

  }
  void parse(CS& cmd) override{
    cmd >> "begin";
    for (;;) {
      cmd.get_line("analogblock>");

      if (cmd >> "end ") {
	break;
      }else{
	parse_item(cmd);
      }
    }
  }
  void dump(std::ostream& o)const override;
  Probe const* new_probe(std::string const& xs, std::string const& p, std::string const& n) override {
    assert(_ctx);
    return _ctx->new_probe(xs, p, n);
  }
  Branch const* new_branch(std::string const& p, std::string const& n) override {
    assert(_ctx);
    return _ctx->new_branch(p, n);
  }
};
/*--------------------------------------------------------------------------*/
#if 0
class Module : public Block {
  std::map<std::string, Probe*> _probes;
  std::map<std::string, Branch*> _branches;
  std::map<std::string, Node*> _nodes;
  std::vector<Node const*> _n;
  Base* _parent;
public:
  explicit Module() {}
  Module(const Module&) = delete;
  ~Module() {
    for(auto& i : _probes) {
      delete i.second;
      i.second = NULL;
    }
  }
  void parse(CS&cmd)override { untested();
    parse_ports(cmd);
    incomplete();
  }
  void parse_ports(CS&cmd);
  void dump(std::ostream& o)const override;
private:
  void declare_ddouble(std::ostream& o)const;
  Branch const* new_branch(std::string const& p, std::string const& n) override;
  Probe const* new_probe(std::string const& xs, std::string const& p,
      std::string const& n) override;
  Node const* new_node(std::string const& p);
  Node const* node(std::string const& p) const;
  void set_port_by_index(int num, std::string& ext_name);
  size_t num_nodes() const{
    return _n.size();
  }
}; // Module
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Module::set_port_by_index(int num, std::string& ext_name)
{
  _n.push_back(new_node(ext_name));
}
/*--------------------------------------------------------------------------*/
void Module::parse_ports(CS& cmd)
{
  if (!(cmd >> '(')) {
    throw Exception("'(' required (parse ports)");
  }else if (cmd.is_alnum()) {
    // by order
    int index = 0;
    while (cmd.is_alnum()) {
      size_t here = cmd.cursor();
      try{
	std::string value;
	cmd >> value;
	set_port_by_index(index, value);
	if (value == "0"){ untested();
	  throw Exception("port 0 not allowed");
	}else if (num_nodes() != index+1) { untested();
	  throw Exception("duplicate port");
	}else{
	  ++index;
	}
      }catch (Exception_Too_Many& e) { untested();
	cmd.warn(bDANGER, here, e.message());
      }
    }
    cmd >> ')';
  }else if(cmd >> ')'){ untested();
    // no port?
  }else{ untested();
    incomplete();
    // error. probably
  }
}
/*--------------------------------------------------------------------------*/
// u_nodemap?
Node const* Module::new_node(std::string const& p)
{
  Node*& cc = _nodes[p];
  if(cc) { untested();
  }else{
    cc = new Node(p);
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Node const* Module::node(std::string const& p) const
{
  auto i = _nodes.find(p);
  if(i != _nodes.end()) {
    return i->second;
  }else{ untested();
    throw Exception("no such node " + p );
  }
}
/*--------------------------------------------------------------------------*/
Branch const* Module::new_branch(std::string const& p, std::string const& n)
{
  std::string k = p + " " + n;
  Branch*& cc = _branches[k];
  if(cc) { untested();
  }else{
    size_t s = _branches.size() - 1;
    // TODO: resolve k
    cc = new Branch(p, n);
    //      cc->deps().insert(cc)??
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Probe const* Module::new_probe(std::string const& xs, std::string const& p, std::string const& n)
{
  std::string k = xs + "_" + p + "_" + n;
  Node const* pp = node(p);
  Node const* pn = node(n);
  Probe*& prb = _probes[k];
  if(prb) {
  }else{
    size_t s = _probes.size() - 1;
    // TODO: resolve k
    prb = new Probe("prb_" + std::to_string(s), xs, p, n);
    //      prb->deps().insert(prb)??
  }
  return prb;
}
#endif
/*--------------------------------------------------------------------------*/
#if 0
Branch const* Variable::new_branch(std::string const& a, std::string const& b)
{
  return _ctx->new_branch(a, b);
}
/*--------------------------------------------------------------------------*/
void Variable::resolve_symbols(Expression const& e, Expression& E)
{
  assert(ctx());
  // resolve symbols
  for(auto i : e) {
    trace1("resolve top:", i->name());

    auto s = dynamic_cast<Token_SYMBOL*>(i);
    std::string const& n = i->name();
    Base const* r = ctx()->find(n);
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
/*--------------------------------------------------------------------------*/
void FlowContribution::parse(CS& cmd)
{
  trace1("FlowContribution::parse", cmd.tail());

  cmd >> "(";
  std::string pp = cmd.ctos(",)");
  std::string pn = cmd.ctos(",)");
  _name = _lhsname + pp + pn;
  _branch = new_branch(pp, pn);
  cmd >> ")";
  cmd >> "<+";
  trace1("Contribution::parse", cmd.tail());
  Assignment::parse(cmd);
}
#endif
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class CMD_ : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST*)override {

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
    size_t here = file.cursor();
    file >> name;
    file.reset(here);

    Module mock(file);

    std::ofstream o;
    o.open(name + ".cc");
    o << "#include <gnucap/globals.h>\n"
         "#include <gnucap/e_compon.h>\n"
         "#include <gnucap/e_node.h>\n"
         "#include \"../va.h\"\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "namespace {\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    make_cc_module(o, mock);
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "} // namespace\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o.close();
  }
} p0;
DISPATCHER<CMD>::INSTALL d0(&command_dispatcher, "`modelgen|demo_module", &p0);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
