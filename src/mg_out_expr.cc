/*                                      -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
/*--------------------------------------------------------------------------*/
#include "mg_out.h"
#include "mg_func.h"
#include "mg_token.h"
#include "mg_options.h"
#include "mg_analog.h" // BUG. Probe
#include <globals.h>
#include <stack>
//#include <iomanip>
/*--------------------------------------------------------------------------*/
const int POOLSIZE = 100;
const int MAXLENGTH = 40;
static double ftos_floor = 1e-101;
static char* ftos_(double num, int fieldwidth, int len, int fmt)
	// num = number to convert
	// fieldwidth = size for fixed width, 0 for variable width
	// len = max length of new string
	// fmt = how to format it
{
  if (len < 3) { untested();
    untested();
    len = 3;
  }
  if (len > MAXLENGTH-6) { untested();
    untested();
    len = MAXLENGTH-6;
  }
  if (fieldwidth > MAXLENGTH-1) { untested();
    untested();
    fieldwidth = MAXLENGTH-1;
  }
  
  char *str;
  { /* get a buffer from the pool */
    //BUG// It is possible to have too many buffers active
    // then the extras are overwritten, giving bad output
    // There are no known cases, but it is not checked.
    static char strpool[POOLSIZE][MAXLENGTH];
    static int poolindex = 0;
    ++poolindex;
    if (poolindex >= POOLSIZE) {
      poolindex = 0;
    }
    str = strpool[poolindex];
  }
  
  { /* build a clean blank string */
    int string_size = std::max(fieldwidth, len+6);
    for (int iii=0; iii<string_size; ++iii) {
      str[iii] = ' ';
    }
    for (int iii=string_size; iii<MAXLENGTH; ++iii) {
      str[iii] = '\0';
    }
  }
  
#ifdef HAS_NUMERIC_LIMITS
  if (num == std::numeric_limits<double>::infinity()) { untested();
    untested();
    memcpy(str, " Over", 5);
  }else if (num == -std::numeric_limits<double>::infinity()) { untested();
    untested();
    memcpy(str, "-Over", 5);
  }else if (num == std::numeric_limits<double>::quiet_NaN()) { untested();
    untested();
    memcpy(str, " NaN", 4);
  }else if (num == std::numeric_limits<double>::signaling_NaN()) { untested();
    untested();
    memcpy(str, " NaN", 4);
  }else
#endif
  if (num == NOT_VALID) { untested();
    memcpy(str, " ??", 3);
  }else if (num == NOT_INPUT) { untested();
    memcpy(str, " NA", 3);
  }else if (num >= BIGBIG) {
    memcpy(str, " Inf", 4);
  }else if (num <= -BIGBIG) {itested();
    memcpy(str, "-Inf", 4);
  }else if (num != num) { untested();
    memcpy(str, " NaN", 4);
  }else{
    if (std::abs(num) < ftos_floor) {	/* hide noise */
      num = 0.;
    }
    
    int expo = 0;	/* exponent				*/
    int nnn = 0; 	/* char counter -- pos in string	*/
    if (num == 0.) {
      strcpy(str, " 0.");
      nnn = static_cast<int>(strlen(str)); /* num==0 .. build string 0.000...  */
      while (--len) {
	str[nnn++] = '0';
      }
      assert(expo == 0);
    }else{				/* num != 0 */
      { // insert sign
	if (num < 0.) {
	  str[0] = '-';
	  num = -num;
	}else if (fmt & ftos_SIGN) { untested();
	  untested();
	  str[0] = '+';
	}else{
	  assert(str[0] == ' ');
	}
      }
      { // scale to .001 - 1.0.  adjust expo.
	expo = -3;
	while (num < .001) {
	  num *= 1000.;
	  expo -= 3;
	}
	while (num >= 1.) {
	  num *= .001;
	  expo += 3;
	}
      }
      { // adjust len to compensate for length of printed exponent
	if ((fmt&ftos_EXP && expo<-9) || expo>10 || expo<-16) {
	  --len;			/* one less digit if 'e' notation */
	}				/* and exp is 2 digits */
	if (len < 3) { untested();
	  untested();
	  ++len;
	}
      }
      { // round to correct number of digits
	double rnd = .5 / pow(10., len); /* find amt to add to round */
	if (num < .01) {
	  rnd /= 100.;
	}else if (num < .1) {
	  rnd /= 10.;
	}
	num += rnd;			/* add it */
	if (num >= 1.) { untested();
	  num *= .001;			/* created an extra digit: rescale */
	  expo += 3;
	}
      }
      { // build mantissa
	nnn = 1;
	if (expo == -3) {		/* .001 is preferable to 1e-3 */
	  int flg = 0;			/* print in fixed point, no exponent*/
	  expo = 0;
	  str[nnn++] = '0';
	  str[nnn++] = '.';
	  while (len > 0) {
	    num *= 10.;
	    int digit = static_cast<int>(floor(num));
	    num -= static_cast<double>(digit);
	    str[nnn++] = static_cast<char>(digit + '0');
	    if ((flg += digit)) {
	      --len;
	    }
	  }
	}else{
	  int flg = 0;
	  for (int iii=2; len>0; --iii) {/* mantissa			    */
	    num *= 10.;			/* get next digit		    */
	    int digit = static_cast<int>(floor(num));
	    num -= static_cast<double>(digit);/* subtract off last digit    */
	    if ((flg += digit)) {	/* if int part !=0		    */
	      str[nnn++]=static_cast<char>(digit+'0');/*(not all zeros so far)*/
	      --len;			/* stuff the digit into the string  */
	    }
	    if (iii==0) {		/* if we found the dec.pt. and	    */
	      str[nnn++] = '.';		/*   haven't used up all the space  */
	    }				/* put a dec.pt. in the string	    */
	  }
	}
      }
    }
    assert(nnn > 0);
    assert(str[nnn] == ' ' || str[nnn] == '\0');
    
    { // suppress trailing zeros
      if (!(fmt&ftos_FILL)) {
	while (str[--nnn]=='0') {
	  str[nnn] = static_cast<char>((nnn < fieldwidth) ? ' ' : '\0');
	}
	++nnn;
      }else{ untested();
	untested();
      }
    }
    
    { // append exponent
      if (expo == 0) {
	// nothing;
      }else if (fmt&ftos_EXP || expo>10 || expo<-16) {/* exponential format  */
	char c = str[nnn+4];
	sprintf(&str[nnn], ((expo < 100) ? "E%+-3d" : "E%3u"), expo);
	nnn+=4;
	str[nnn++] = c;
      }else{				   /* if letter-scale format	    */
	str[nnn++] = "fpnum KMGT"[(expo+15)/3];/* put the appropriate letter*/
      }				/* note that letter-scale is not valid	    */
				/* for exp==-3 or exp not in -15..+12	    */
				/* this is trapped but letter-scale is also */
				/* not valid if exp not divisible by 3.     */
				/* This is not trapped, since it supposedly */
				/* cant happen.				    */
      if (str[nnn-1] == 'M') { untested();
	str[nnn++] = 'e';	/* Spice kluge "MEG" */
	str[nnn++] = 'g';
      }
    }
  }
  { // clean up trailing blanks
    if (fieldwidth==0) {
      trim(str);
    }
  }
  
  return str;
}
/*--------------------------------------------------------------------------*/
bool is_cc_ref(Token const* t)
{
  if (dynamic_cast<Token_PAR_REF const*>(t)) {
    return true;
  }else if (dynamic_cast<Token_VAR_REF const*>(t)) {
    return true;
  }else if (dynamic_cast<Token_PORT_BRANCH const*>(t)) {
    return true;
  }else if (dynamic_cast<Token_CONSTANT const*>(t)) {
    return true;
  }else if (dynamic_cast<Token_HIER_REF const*>(t)) {
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
bool is_cc_ref(Expression const* e)
{
  assert(e);
  assert(e->size());

  if(e->size()==1){
    return is_cc_ref(*e->begin());
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
class RPN_VARS {
  typedef enum{
    t_log = 0,
    t_flt,
    t_ddo,
    // t_int,
    t_str,
    t_ref,
    t_arr,
    t_count,
  } type;
  typedef std::stack<int> stack;
  std::stack<type> _types;
  std::stack<int> _args;
  std::stack<std::string> _refs;

  stack _stack[t_count];
  int _idx[t_count];
  int _alloc[t_count];
  TData const* _deps;
public:
  explicit RPN_VARS(TData const* d) : _deps(d) {
    std::fill(_idx, _idx+t_count, -1);
    std::fill(_alloc, _alloc+t_count, 0);
    assert(_idx[0] == -1);
    assert(_idx[1] == -1);
    assert(_alloc[0] == 0);
    assert(_refs.empty());
  }
  ~RPN_VARS(){
    assert(_idx[0] == -1);
    assert(_idx[1] == -1);
    assert(_refs.empty());
  }
  void pop() {
    assert(!_types.empty());
    int t = _types.top();
    if(t == t_ref){
      _refs.pop();
    }else{
      assert(_idx[t]>-1);
      --_idx[t];
    }
    _types.pop();
  }
  void new_string(std::ostream& o) { new_(o, t_str); }
  void new_array(std::ostream& o, Token_ARRAY_ const& /*TODO*/){
    ++_idx[t_arr];
    _types.push(t_arr);
    if(_idx[t_arr] < _alloc[t_arr]){ untested();
      o__ code_name() << "= array_";
    }else{
      o__ "array_ " << code_name();
      ++_alloc[t_arr];
    }
  }
  void new_ddouble(std::ostream& o){
    ++_idx[t_ddo];
    if(_idx[t_ddo] < _alloc[t_ddo]){
    }else{
      assert(_idx[t_ddo]==_alloc[t_ddo]);
      ++_alloc[t_ddo];
      assert(_idx[t_ddo]>=0);
      if(_deps){
	o__ "ddouble t" << _idx[t_ddo] << ";\n";
      }else{
	o__ "ddouble t" << _idx[t_ddo] << ";\n"; // TODO? some deps?
      }
      if(!_deps){
      }else if(!options().optimize_deriv()){ untested();
	o__ "t" << _idx[t_ddo] << ".set_all_deps(); // (all deriv)\n"; // code_name??
      }else{
	o__ "//t" << _idx[t_ddo] << ".set_no_deps();\n"; // ...
	for(Dep const& i: _deps->ddeps()){
	  o__ "//t" << _idx[t_ddo] << "[d" << probe(i)->code_name() << "] = 0.; // (output dep)\n";
	}
      }
    }
    _types.push(t_ddo);
  }
  void new_logic(std::ostream& o) { new_(o, t_log); }
  void new_float(std::ostream& o) { new_(o, t_flt); }
private:
  void new_(std::ostream& o, int t){
    ++_idx[t];
    if(_idx[t] < _alloc[t]){
      _types.push(type(t));
    }else{
      trace3("DBG??", t, _idx[t], _alloc[t]);
      assert(_idx[t]==_alloc[t]);
      ++_alloc[t];
      assert(_idx[t]>=0);
      _types.push(type(t));
      o__ "/*361, "<<t<<"*/" << cpptype() << " " << code_name() << ";\n"; // code_name?
    }
  }
public:
  bool is_ref() const{
    assert(!_types.empty());
    return _types.top() == t_ref;
  }
  void new_literal(std::ostream& o, Token_CONSTANT const& c);
  void new_rhs(Token_NODE const* v){
    new_ref("MOD::n_" + v->code_name() + "/*node*/");
  }
  void new_rhs(Token_VAR_REF const* v){
    // TODO: linear?
    // if(0 && (*v)->is_real()){ untested();
    //   // crash?
    //   _refs.push("ddouble(" + (*v)->code_name() + ")/*rhsvar*/");
    // }else
    new_ref(v->code_name() + "/*rhsvar*/");
  }
  void new_rhs(Token_HIER_REF const* v){
    // TODO: what type is it??
    new_ref("pc->" + v->code_name() + ".get_double()");
  }
  void new_rhs(Token_PAR_REF const* v){
    // _refs.push("ddouble(" + (*v)->code_name() + ")/*rhsvar*/");
    // _refs.push((*v)->type() + "(" + (*v)->code_name() + ") /*rhspar*/");
    new_ref("(" + (*v)->code_name() + ") /*rhspar*/");
  }
  void new_ref(std::string name){
    _refs.push(name);
    _types.push(t_ref);
  }
  void stop(){
    _args.push(int(_types.size())-1);
  }
  bool have_args()const{
    return !_args.empty();
  }
  size_t num_args() const{
    assert(!_args.empty());
    return _types.size() - 1 - size_t(_args.top());
  }
  void enter_scope(){
    for(int i = 0 ; i<t_count; ++i){
      _stack[i].push(_alloc[i]);
    }
  }
  void leave_scope(){
    for(int i = 0 ; i<t_count; ++i){
      _alloc[i] = _stack[i].top();
      _stack[i].pop();
    }
  }
  void args_pop(){
    assert(!_args.empty());
    _args.pop();
  }
  size_t size() const{ untested();
    return _refs.size();
  }
  std::string cpptype()const {
    assert(_types.size());
    int t = _types.top();
    assert(t<t_count);
    static std::string names[] = {
       "LOGICVAL", "double", "ddouble",
       "string", "auto&",
       "incomplete_cpptype"
    };
    return names[t];
  }
private:
  char var_name(int i)const {
    static char names[] = "lftsra";
    assert(i<t_count);
    return names[i];
  }
public:
  std::string code_name() const{
    assert(_types.size());
    int t = _types.top();
    if(t == t_ref) {
      return _refs.top();
    }else{
      return std::string(1, var_name(t)) + std::to_string(_idx[t]);
    }
  }
  bool has_deps()const { return _deps; }
  TData const& deps()const { untested(); assert(_deps); return *_deps; }
}; // RPN_VARS
/*--------------------------------------------------------------------------*/
class OUT_EXPRESSION {
  RPN_VARS& _s;
  std::string _ctx;
public:
  explicit OUT_EXPRESSION(RPN_VARS& r, std::string ctx)
    : _s(r), _ctx(ctx) {}

  std::string make_cc_expression_(std::ostream& o, Expression const& e);
private:
  RPN_VARS& vars() {return _s;}
  void new_variable(std::ostream& o, Token const* t);
  std::string make_cc_expression_(std::ostream& o, Token const* t) {
    Expression_ e;
    e.push_back(const_cast<Token*>(t));
    std::string name = make_cc_expression_(o, e);
    e.pop_back();
    return name;
  }
  void make_cc_array(std::ostream& o, Token_ARRAY_ const* t);
  void make_cc_call(std::ostream& o, Token_CALL const* t);
  bool is_precalc()const {return _ctx == "precalc";}
  bool is_adjust()const {return _ctx == "adjust";}
};
/*--------------------------------------------------------------------------*/
void OUT_EXPRESSION::new_variable(std::ostream& o, Token const* t)
{
  assert(t);
  Data_Type const* d = nullptr;
  auto td = dynamic_cast<TData const*>(t->data());

  if(auto F = dynamic_cast<const Token_CALL*>(t)){
    d = (*F)->return_type(); // BUG.
  }else if(td){
    d = &td->type();
  }else{
  }

  if(_ctx == "logic") {
    // hack
    vars().new_logic(o);
  }else if(!d) {
    o__ "/* void? */\n";
    vars().new_ddouble(o); // TODO
  }else if(d->is_real()){
    o__ "//real? ddouble?\n";
    vars().new_ddouble(o);
  }else if(d->is_int()){
    o__ "//int, incomplete\n";
    vars().new_float(o);
  }else if(d->is_string()){
    vars().new_string(o);
  }else if(td){
    if(td->ddeps().size()){ untested();
      o__ "//ddeps. ddouble??\n";
      vars().new_ddouble(o); // TODO
    }else{ untested();
      incomplete();
      o__ "//no ddeps. float??\n";
      vars().new_float(o); // TODO
    }
  }else{ untested();
    o__ "/* what is it? " << *d << " */\n";
    vars().new_float(o); // TODO
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_string(std::ostream& o, String const& e)
{
 // o << '"';
  for(char c : e.val_string()){
    if(c=='\n'){itested();
//      o << '\\';
    }else{
    }
    o << c;
  }
  // o << '"';
}
/*--------------------------------------------------------------------------*/
void RPN_VARS::new_literal(std::ostream& o, Token_CONSTANT const& c)
{
  if(auto ff=dynamic_cast<Float const*>(c.data())){
#if 1
    new_ref(ftos_(ff->value(), 0, 20, ftos_EXP));
#else
    std::stringstream tmp;
    tmp << "(" << std::scientific << std::setprecision(17) << ff->value() << ")";
    s.new_ref(tmp.str());
#endif
  }else if(auto ii=dynamic_cast<Integer const*>(c.data())) {
    new_ref(to_string(ii->value()));
  }else if(auto S=dynamic_cast<String const*>(c.data())){
    new_string(o);
    o__ code_name() << " = ";
    make_cc_string(o, *S);
    o << ";\n";
  }else{untested();
    unreachable();
    new_string(o);
    o__ code_name() << " = " << c.name() << "; (u)\n";
  }
}
/*--------------------------------------------------------------------------*/
void OUT_EXPRESSION::make_cc_array(std::ostream& o, Token_ARRAY_ const* A)
{
  if(A->args()){
    auto se = prechecked_cast<Expression const*>(A->args());
    assert(se);
    vars().stop();
    make_cc_expression_(o, *se);
  }else{ untested();
  }
  std::vector<std::string> argnames;
  if(A->args()) {
    assert(vars().have_args());
    argnames.resize(vars().num_args());
    for(auto n=argnames.begin(); n!=argnames.end(); ++n){
      *n = vars().code_name();
      vars().pop();
    }
  }else{ untested();
  }
  vars().new_array(o, *A);

  if(!A->args()) { untested();
    //	o << A->code_name() << "(); // no parlist\n";
    assert(!argnames.size());
    o << " /*(312b)*/ "; //  << A->code_name();
  }else if(!argnames.size()){
    //	o << A->code_name() << "(); // no args\n";
    o << "; /*(312a)*/ "; //  << A->code_name();
  }else{
    o << " /*(312)*/ "; //  << A->code_name();

    o << "(";
    std::string comma = "";
    for(size_t ii=argnames.size(); ii; --ii){
      o << comma << argnames[ii-1];
      comma = ", ";
    }
    o << ");\n";
  }
  vars().args_pop();
}
/*--------------------------------------------------------------------------*/
void OUT_EXPRESSION::make_cc_call(std::ostream& o, Token_CALL const* F)
{
  new_variable(o, F);
  vars().stop();
  vars().enter_scope();
  o__ "{ // scope\n"; {
  indent x;
  if(F->args()){
    o__ "// F " << F->name() << " args:" << vars().have_args() << "\n";
    auto se = prechecked_cast<Expression const*>(F->args());
    assert(se);
    make_cc_expression_(o, *se);
  }else{
    o__ "// function " << F->name() << " args:" << vars().have_args() << "\n";
  }

  std::vector<std::string> argnames;
  if(F->args()) {
    assert(vars().have_args());
    argnames.resize(vars().num_args());
    for(auto n=argnames.begin(); n!=argnames.end(); ++n){
      *n = vars().code_name();
      vars().pop();
    }
  }else{
  }
  o__ "// --- \n";
  vars().args_pop();
  vars().leave_scope();
  Data_Type const* rt = (*F)->return_type();
  if(!rt) {
    o__"(void)" <<  vars().code_name() << ";\n";
  }else{
    o__ vars().code_name() << " = ";
  }

  if(is_adjust()) {
  }else if((*F)->is_in_common()) {
  }else{
    o << "/*"<<_ctx<<"*/ d->";
  }

  o << F->code_name();
  if((*F)->has_modes()){
    if(is_adjust()) {
      o << "precalc";
    }else{
      o << _ctx;
    }
  }else if(is_precalc() && (*F)->has_precalc()){
    // TODO: cleanup.
    o << "__" + _ctx;
  }else{
  }

  o << "(";
  std::string comma = "";
  // if(_ctx=="precalc"){ untested();
  // }else
  if(is_adjust()) {
    // there is no context in adjust
  }else if((*F)->needs_context()){
    o << "d /* "<<_ctx<<"*/";
    comma = ", ";
  }else{
  }
  if(!F->args()) {
    assert(!argnames.size());
  }else{
    assert(F->code_name()!="");
    for(int nn=0; nn<int(argnames.size()); ++nn) {
      int ii = int(argnames.size())-nn-1;
      o << comma; //  << "/* arg " << nn << "*/";
      if((*F)->is_output_arg(nn)){
	o << "io_arg(" << ::code_name((*F)->arg_type(nn)) << "(), " << argnames[ii] << ")";
      }else{
	o << argnames[ii];
      }
      comma = ", ";
    }
  }
  o << "); // (659)\n";
  } o__ "} // scope\n";
}
/*--------------------------------------------------------------------------*/
static Data_Type data_type(Base const* d)
{
  if(auto td = dynamic_cast<TData const*>(d)){
    return td->type();
  }else if(dynamic_cast<Integer const*>(d)){
    return Data_Type_Int();
  }else{ untested();
    incomplete();
    return Data_Type();
  }
}
/*--------------------------------------------------------------------------*/
std::string OUT_EXPRESSION::make_cc_expression_(std::ostream& o, Expression const& e)
{
  typedef Expression::const_iterator const_iterator;

#if 0
  if(auto se = dynamic_cast<Expression_ const*>(&e)){ untested();
    o << "/* ";
    se->dump(o);
    o << "\n";
    o << "is_offset " << se->deps().is_offset() << "\n";
    for(auto i : se->deps()) { untested();
      o << "// Dep: " << i->code_name() << " order: " << i.order() << " ";
      o << "\n";
    }
    o << "*/\n";
  }else{ untested();
  }
#endif

  // The _list is the expression in RPN.
  // print a program that computes the function and the derivatives.
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    trace3("mg_out_expr loop", (*i)->name(), (*i)->data(), vars().size());

    if (auto n = dynamic_cast<const Token_NODE*>(*i)) {
      if(_ctx != "logic") {
	vars().new_rhs(n);
      }else{
	Token_VAR_REF const* r = n;
	vars().new_rhs(r);
      }
    }else if (auto var = dynamic_cast<const Token_VAR_REF*>(*i)) {
      vars().new_rhs(var); // if linear?
//    }else if (auto t = dynamic_cast<const Token_OUT_VAR*>(*i)) { untested();
//      vars().new_rhs(t); // if linear?
//      //incomplete();
//      //o__ "0.; // OUTVAR?!\n";
    }else if (auto hh = dynamic_cast<const Token_HIER_REF*>(*i)) {
      vars().new_rhs(hh);
    }else if(auto pp = dynamic_cast<const Token_ACCESS*>(*i)) {
      vars().new_ddouble(o);
      std::string lhsname = vars().code_name();
      if(!vars().has_deps()){ untested();
      }else if(options().optimize_deriv()){
	o__ lhsname << ".set_no_deps();\n";
	// for(auto i: vars().deps()){ untested();
	//   o__ vars().code_name() << "[d" << i->code_name() << "] = 0.; // (output dep)\n";
	// }
      }else{itested();
      }

      if(!vars().has_deps()){ untested();
        o__ lhsname << " = 0.; // no deps.\n";
      }else if(pp->is_short()){
	o__ lhsname << " = 0.; // short probe\n";
      }else if(is_precalc()){
        o__ lhsname << " = 0.; // precalc.\n";
      }else{
	o__ lhsname << " = p->xs" << pp->code_name_() << "();\n";
      }
    }else if (auto p = dynamic_cast<const Token_PAR_REF*>(*i)) {
      vars().new_rhs(p);
    }else if (auto pb = dynamic_cast<const Token_PORT_BRANCH*>(*i)) {
      vars().new_ref(pb->code_name());
    }else if (auto A = dynamic_cast<const Token_ARRAY_*>(*i)) {
      make_cc_array(o, A);
    }else if (auto c = dynamic_cast<const Token_CONSTANT*>(*i)) {
      vars().new_literal(o, *c);
    }else if(auto F = dynamic_cast<const Token_CALL*>(*i)) {
      make_cc_call(o, F);
    }else if (auto ff=dynamic_cast<const Token_FUNCTION*>(*i)) {
      vars().new_ref("&COMMON::" + ff->code_name());
#if 0
    }else if (auto pl=dynamic_cast<const Token_PARLIST_*>(*i)) { untested();
      if(auto se = dynamic_cast<Expression const*>(pl->args())){ untested();
	o__ "// start parlist\n";
	vars().stop();
	make_cc_expression_(o, *se);
	o__ "// end parlist\n";
      }else if(auto eee = dynamic_cast<Expression const*>((*i)->data())){ untested();
	o__ "// start parlist\n";
	vars().stop();
	make_cc_expression_(o, *eee);
	o__ "// end parlist\n";
      }else{ untested();
	unreachable(); // ?
      }
#endif
    }else if (auto bo = dynamic_cast<const Token_BINOP_*>(*i)) {
      // make_cc_binop(..);

      assert(bo->op1());
      // assert(bo->op1()->data());
      assert(bo->op2());
      // assert(bo->op2()->data());
      make_cc_expression_(o, bo->op1());
      make_cc_expression_(o, bo->op2());

      assert((*i)->name().size());
      std::string idy = vars().code_name();
      vars().pop();
      std::string arg1 = vars().code_name();
      vars().pop();

      new_variable(o, *i);

      auto op = (*i)->name()[0];
      if ( op == '-'
	|| op == '+'
	|| op == '*'
	|| op == '/'
	|| op == '<'
	|| op == '>'
	|| op == '='
	|| op == '&'
	|| op == '|'
	|| op == '!' ){
	o__ vars().code_name() << " = " << arg1 << " " << (*i)->name() << " " << idy << "; // (703)\n";
      }else if(op == '%'){itested();
	Data_Type type_1 = data_type(bo->op1()->data());
	Data_Type type_2 = data_type(bo->op2()->data());
//	o__ "// " << bo->op1()->name() << " " <<  type_1 << "\n";
//	o__ "// " << bo->op2()->name() << " " <<  type_2 << "\n";
	o__ vars().code_name() << " = ";
        if(type_1.is_int() && type_2.is_int()){ untested();
	  // BUG: cast should not be needed.
	  o << "( int(" << arg1 << ") \% int(" << idy << "));\n";
	}else{
	  o << "va::fmod(" << arg1 << ", " << idy << ");\n";
	}
      }else{ untested();
	unreachable();
	throw Exception("run time error in make_cc_expression: " + (*i)->name());
      }
    }else if (auto u = dynamic_cast<const Token_UNARY_*>(*i)) {
      // make_cc_unary(..);
      assert(u->op1());
      make_cc_expression_(o, u->op1());

      std::string arg1 = vars().code_name();
      vars().pop();
      new_variable(o, *i);

      auto op = (*i)->name()[0];
      if(op == '-' || op == '!' || op == '+') {
	o__ vars().code_name() << " = " << op << arg1 << ";\n";
      }else{ untested();
	incomplete();
	unreachable();
	o__ vars().code_name() << " INCOMPLETE = " << op << arg1 << ";\n";
      }
    }else if (auto t = dynamic_cast<const Token_TERNARY_*>(*i)) {
      assert(t->cond());
      make_cc_expression_(o, t->cond());

      std::string arg1 = vars().code_name();
      vars().pop();
      vars().new_ddouble(o);

      o__ "{ // ternary " << _ctx << "\n";
      {
	indent y;
	o__ "/*tt*/" << vars().cpptype() << "& tt0 = " << vars().code_name() << ";\n"; // BUG: float??
	o__ "if(" << arg1 << "){ // true part\n";
	{
	  indent x;
	  // BUG: nest?
	  std::string name = make_cc_expression(o, *t->true_part(), true, _ctx);
	  o__ "tt0 = " << name << ";\n";
	}
	o__ "}else{ // false part\n";
	{
	  indent x;
	  // BUG: nest?
	  std::string name = make_cc_expression(o, *t->false_part(), true, _ctx);
	  o__ "tt0 = " << name << ";\n";
	}
	o__ "}\n";
      }
      o__ "}\n";
    }else{ untested();
      assert(!dynamic_cast<const Token_UNARY*>(*i));
      assert(!dynamic_cast<const Token_SYMBOL*>(*i));
      assert(!dynamic_cast<const Token_BINOP*>(*i));
      assert(!dynamic_cast<const Token_TERNARY*>(*i));
      assert(!dynamic_cast<const Token_PARLIST*>(*i));
      assert(!dynamic_cast<const Token_STOP*>(*i));
      vars().stop();
      incomplete();
      unreachable();
    }
  }
  return vars().code_name();
}
/*--------------------------------------------------------------------------*/
std::string make_cc_expression(std::ostream& o, Expression const& e, bool dynamic,
    std::string ctx)
{
  TData const* deps = nullptr;
  if(ctx=="precalc"){
    // assert(!dynamic);
  }else{
  }
  if(ctx == "af"){
  }else if(ctx == "logic"){
    // no deps-> use ddouble..
  }else if(!dynamic && ctx!="precalc"){
  }else if(auto ex = dynamic_cast<Expression_ const*>(&e)){
    deps = &ex->data();
  }else{ untested();
  }
  RPN_VARS s(deps);
  OUT_EXPRESSION ex(s, ctx);
  // o__ "// expression\n";
  std::string name = ex.make_cc_expression_(o, e);
  // o__ "// /expression\n";

  // BUG: this does not belong here.
  if(ctx == "adjust"){
    o__ "(void)" << name << ";\n";
  }else if(deps && deps->size() && (dynamic || ctx == "precalc")){
    o__ "// dynamic & deps " << ctx << " " << deps->size() << "\n";
    s.new_ddouble(o);
    s.pop();
    o__ "t0 = " << s.code_name() << "; // " << ctx << "\n";
  }else if(s.is_ref()){
    o__ "// ref. type?\n";
    s.new_float(o);
    s.pop();
    o__ "f0 = " << s.code_name() << "; // " << ctx << "\n";
    o__ "(void) f0;\n"; // BUG
  }else{
  }
  s.pop();
  return name;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
