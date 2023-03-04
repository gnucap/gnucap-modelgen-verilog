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
/*--------------------------------------------------------------------------*/
class Variable;
class Probe;
class AnalogBlock;
/*--------------------------------------------------------------------------*/
static std::string ind = "  ";
struct indent{
  explicit indent(std::string s){
    _old = ind;
    ind = ind + s;
  }
  ~indent(){
    ind = _old;
  }
  std::string _old;
};
/*--------------------------------------------------------------------------*/
class Token_PROBE : public Token {
  Probe const* _item;
public:
  explicit Token_PROBE(const std::string Name, Probe const* data)
    : Token(Name, NULL, ""), _item(data) {}
  explicit Token_PROBE(const Token_PROBE& P) : Token(P) {}
  Token* clone()const  override{return new Token_PROBE(*this);}
  void stack_op(Expression* e)const override{
    e->push_back(clone());
  }
};
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
/*--------------------------------------------------------------------------*/
class Token_VARIABLE : public Token {
private:
  Variable const* _item;
public:
  explicit Token_VARIABLE(const std::string Name, Variable const* item)
    : Token(Name, NULL, ""), _item(item) {}
  explicit Token_VARIABLE(const Token_VARIABLE& P) : Token(P), _item(P._item) {}
  Token* clone()const  override{return new Token_VARIABLE(*this);}
  void stack_op(Expression* e)const override{
    e->push_back(clone());
  }
  Variable const* item()const {assert(_item); return _item;}
  std::set<Probe const*> const& deps() const{
    assert(item());
    return item()->deps();
  }
};
/*--------------------------------------------------------------------------*/
void dump(Expression const& e, std::ostream& out)
{
  typedef Expression::const_iterator const_iterator;

  // The _list is the expression in RPN.
  // print a program that computes the function and the derivatives.
  std::stack<int> idxs;
  int idx = -1;
  int idx_alloc = 0;
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    trace1("dump", (*i)->name());
  }
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    if (auto var = dynamic_cast<const Token_VARIABLE*>(*i)) {
      idxs.push(++idx);
      if(idx<idx_alloc) {
	// re-use temporary variable
      }else{
	assert(idx==idx_alloc);
	++idx_alloc;
	out << ind << "ddouble t" << idx << ";\n";
      }
      out << ind << "t" << idx << " = " << (*i)->name() << ".value();\n";
      for(auto v : var->deps()) {
//        out << ind << "t" << idx << "._" << v->name() << " = " << (*i)->name() << "._" << v->name() << ";\n";
        out << ind << "t" << idx << "[p_" << v->name() << "] = " << (*i)->name() << "[p_" << v->name() << "];\n";
      }
    }else if (dynamic_cast<const Token_CONSTANT*>(*i)) {
      idxs.push(++idx);
      if(idx<idx_alloc) {
	// re-use temporary variable
      }else{
	assert(idx==idx_alloc);
	++idx_alloc;
	out << ind << "ddouble t" << idx << ";\n";
      }
      out << ind << "t" << idx << " = " << (*i)->name() << ";\n";
    }else if( dynamic_cast<const Token_PROBE*>(*i)) {
      idxs.push(++idx);
      if(idx<idx_alloc) { untested();
	// re-use temporary variable
      }else{
	assert(idx==idx_alloc);
	++idx_alloc;
	out << ind << "ddouble t" << idx << ";\n";
      }
      out << ind << "t" << idx << " = " << (*i)->name() << ";\n";
      //out << ind << "t" << idx << "._" << (*i)->name() << " = " << "1.;\n";
      out << ind << "t" << idx << "[p_" << (*i)->name() << "] = " << "1.;\n";
    }else if(dynamic_cast<const Token_SYMBOL*>(*i)) {
      int arity = is_function((*i)->name());
      assert(arity);
      out << ind << "t" << idx << " = va::" << (*i)->name();
      if(arity == 1){
	out << "(t" << idx << ");\n";
      }else if(arity == 2){
	int idy = idx;
	idxs.pop();
	idx = idxs.top();

	out << "(t" << idx << ", t" << idy << ");\n";
      }else{ untested();
	unreachable();
      }
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) {
    }else if (dynamic_cast<const Token_BINOP*>(*i)) {
      int idy = idxs.top();
      assert( idy == idx );
      idxs.pop();
      idx = idxs.top();
      assert((*i)->name().size());

      char op = (*i)->name()[0];
      switch(op) {
      case '-':
      case '+':
      case '*':
      case '/':
	out << ind << "t" << idx << " "<<op<<"= t" << idy << ";\n";
      default:
	;
      }
      // ++idx;
    }
  }
  assert(!idx);
}
/*--------------------------------------------------------------------------*/
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
};
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
/*--------------------------------------------------------------------------*/
Branch const* Variable::new_branch(std::string const& a, std::string const& b)
{
  return _ctx->new_branch(a, b);
}
/*--------------------------------------------------------------------------*/
void Module::declare_ddouble(std::ostream& o) const
{
  size_t np = _probes.size();
  o << ind << "typedef ddouble_<"<<np<<"> ddouble;\n";
}
/*--------------------------------------------------------------------------*/
void Module::dump(std::ostream& o)const
{
  o << "class Module : public COMPONENT {\n";
  o << "public:\n";
  declare_ddouble(o);
  o << "private: // data\n";
  size_t total_nodes = _nodes.size() + 10;
  // circuit().req_nodes().size() + circuit().opt_nodes().size() + circuit().local_nodes().size();
  o << ind << "node_t _nodes[" << total_nodes << "];\n";
  o << "private: // construct\n";
  o << ind << "explicit Module(Module const&);\n";
  o << "public:\n";
  o << ind << "explicit Module() : COMPONENT() { _n = _nodes; }\n";
  o << ind << "CARD* clone()const override { return new Module(*this); }\n";
  o << "private: // overrides\n";
  o << ind << "double tr_probe_num(std::string const&) const;\n";
  o << ind << "std::string dev_type()const override {return \"demo\";}\n";
  o << ind << "int max_nodes()const override {return 4;}\n";
  o << ind << "int min_nodes()const override {return 2;}\n";
  o << ind << "std::string value_name()const override {untested(); return \"\";}\n";
  o << ind << "bool print_type_in_spice()const override {untested(); return false;}\n";
  o << ind << "std::string port_name(int i)const override {\n";
  o << ind << ind << "assert(i >= 0);\n";
  o << ind << ind << "assert(i < 4);\n";
  o << ind << ind << "static std::string names[] = {\"a\", \"b\", \"c\", \"d\", \"\"};\n";
  o << ind << ind << "return names[i];\n";
  o << ind << "}\n";
  o << "private: // impl\n";
  // o << ind << "void clear_branch_contributions();\n";
  o << "/* ========== */\n";

  o << "private: // data\n";
  o << ind << "ddouble _branches;\n";
  for(auto x : _branches){
    assert(x.second);
    o << ind << "ddouble _branch" << x.second->name() << ";\n";
  }

  std::string comma="";
  o << "private: // node list\n";
  o << ind << "enum {";
  for (auto nn : _nodes){ // BUG: array?
    o << comma << "n_" << nn.second->name();
    comma = ", ";
  }
  o << ind << "};\n";

  comma="";
  o << "private: // probe list\n";
  o << ind << "enum {";
  for (auto nn : _probes){
    o << comma << "p_" << nn.second->name();
    comma = ", ";
  }
  o << ind << "};\n";

  for(auto x : _probes){
    assert(x.second);
    o << ind << "double " << x.second->name() << ";\n";
  }

  o << ind << "bool tr_needs_eval() const override{ return true; }\n";
  o << ind << "bool do_tr() override; // AnalogBlock\n";
  o << ind << "void read_voltages();\n";
  o << ind << "void clear_branch_contributions(){\n";
  for(auto x : _branches){
    assert(x.second);
    o << ind << ind << "_branch" << x.second->name() << ".clear();\n";
  }
  o << ind << "}\n";

  o << "}m; /* Module */\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
//  o << "template<>\n"
//       "Module::ddouble chain_rule(Module::ddouble& x, double d)\n{\n";
//  o << ind << "return x.chain_rule(d);\n"
//       "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o << "Module::Module(Module const&p) : COMPONENT(p)\n{\n"
    << ind << "_n = _nodes;\n"
    << ind << "for (int ii = 0; ii < max_nodes() + int_nodes(); ++ii) {\n"
    << ind << ind << "_n[ii] = p._n[ii];\n"
    << ind << "}\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o << "double Module::tr_probe_num(std::string const& n) const\n{\n";
  // no range check whatsoever. debugging/testing, remove later.
  for(int i=0; i<4; ++i){
    o << ind << "if(n == \"v" << i << "\") return _n[" << i << "].v0();\n";
    o << ind << "if(n == \"i" << i << "\") return (&_branches+" << i << "+ 1)->value();\n";
    for(int j=0; j<4; ++j){
      o << ind << "if(n == \"i" << i << "_d"<<j<<"\") return (&_branches+" << i << "+ 1)->d("<<j<<");\n";
    }
  }
  o << ind << "return NOT_VALID;\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o << "void Module::read_voltages() {\n";
  for(auto x : _probes){
    Probe const* p = x.second;
    assert(p);
    o << ind << p->name() << " = volts_limited(_n[n_"<< p->pname() <<"], _n[n_"<< p->nname() <<"]);\n";
  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";

  assert(front());
  front()->dump(o); // the analog block, for now.
} // Module::dump
/*--------------------------------------------------------------------------*/
void AnalogBlock::dump(std::ostream& o)const
{
  o << "bool Module::do_tr(){ // AnalogBlock\n";
  o << ind << "clear_branch_contributions();\n";
  o << ind << "read_voltages();\n";
  for(auto i : *this) {
    // need commmon baseclass...
    if(auto v=dynamic_cast<Variable const*>(i)) {
      v->dump(o);
    }else if(auto v=dynamic_cast<FlowContribution const*>(i)) { untested();
      v->dump(o);
    }else if(auto v=dynamic_cast<PotContribution const*>(i)) { untested();
      v->dump(o);
    }else{ untested();
      incomplete();
    }
  }
  o << ind << "return true;\n";
  o << "} // AnalogBlock\n";
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
      E.push_back(new Token_VARIABLE(n, v));
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
void Variable::dump(std::ostream& o)const
{
  o << ind << "ddouble " << _name << "; // Variable";
  for(auto i : _deps) { untested();
    o << ind << " Dep: " << i->name();
  }
  o << ind << "\n";
}
/*--------------------------------------------------------------------------*/
void FlowContribution::dump(std::ostream& o)const
{
  o << ind << "{ // FlowContribution " << _lhsname << "\n";
  {
    indent x("  ");
    if(_rhs) {
      ::dump(*_rhs, o);
    }else{ untested();
    }

    assert(_branch);
    o << ind << ind << "_branch" << _branch->name() << ".value() += t0.value();\n";
    for(auto v : deps()) {
      o << ind << ind << "_branch" << _branch->name()
      // << "._" << v->name() << " += " << "t0._" << v->name() << ";\n";
	<< "[p_" << v->name() << "] += " << "t0[p_" << v->name() << "];\n";
    }
  }
  o << ind <<  "}\n";
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
/*--------------------------------------------------------------------------*/
void Assignment::dump(std::ostream& o)const
{
  // out << "{ // RPN ";
  // for (const_iterator i = e.begin(); i != e.end(); ++i) { untested();
  //   out << "" << (*i)->full_name() << " ";
  // }
  o << ind << "{ // Assignment '" << _lhsname << "'.";
  for(auto i : _deps) {
    o << " Dep: " << i->name();
  }
  o << "\n";

  if(_rhs) {
    indent x("  ");
    //      _rhs->dump(o);
    ::dump(*_rhs, o);
    o << ind << _lhsname << ".value() = t0.value();\n";
    for(auto v : deps()) {
      // o << ind << _lhsname << "._" << v->name() << " = " << "t0._" << v->name() << ";\n";
      o << ind << _lhsname << "[p_" << v->name() << "] = " << "t0[p_" << v->name() << "];\n";
    }
  }else{ untested();
  }
  o << ind << "}\n";
}
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class CMD_ : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST*)override {
    Module mock;

    std::ofstream o;
    std::string name;
    cmd >> name;
    o.open(name + ".cc");

    mock.parse_ports(cmd);
    cmd.get_line("");

    AnalogBlock* ab = new AnalogBlock();
    ab->set_ctx(&mock);
    ab->parse(cmd);
    mock.push_back(ab);

    cmd.get_line("");
    cmd >> "endmodule ";

    o << "#include <gnucap/globals.h>\n"
         "#include <gnucap/e_compon.h>\n"
         "#include <gnucap/e_node.h>\n"
         "#include \"../va.h\"\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "namespace {\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    mock.dump(o);
    o << "DISPATCHER<CMD>::INSTALL d0(&device_dispatcher, \""
      << name << "\", &m);\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "} // namespace\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o.close();
  }
} p0;
DISPATCHER<CMD>::INSTALL d0(&command_dispatcher, "demo_module", &p0);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
