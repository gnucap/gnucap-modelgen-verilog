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
    if (poolindex >= POOLSIZE) {itested();
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
class RPN_VARS {
  typedef enum{
    t_flt,
    t_ddo,
    // t_int,
    t_str,
    t_ref,
    t_arr,
  } type;
  std::stack<type> _types;
  std::stack<int> _args;
  std::stack<std::string> _refs;
//  int _idx_alloc{0};

  int _flt_idx{-1};
  int _flt_alloc{0};
  int _ddo_idx{-1};
  int _ddo_alloc{0};
  int _str_idx{-1};
  int _str_alloc{0};
  int _arr_idx{-1};
  int _arr_alloc{0};
  TData const* _deps;
public:
  explicit RPN_VARS(TData const* d) : _deps(d) {}
  ~RPN_VARS(){
    assert(_flt_idx == -1);
    assert(_ddo_idx == -1);
    assert(_arr_idx == -1);
    assert(_str_idx == -1);
    assert(_refs.empty());
  }
  void pop() {
    assert(!_types.empty());
    switch(_types.top()){
    case t_flt:
      assert(_flt_idx>-1);
      --_flt_idx;
      break;
    case t_arr:
      assert(_arr_idx>-1);
      --_arr_idx;
      break;
    case t_ddo:
      assert(_ddo_idx>-1);
      --_ddo_idx;
      break;
    case t_ref:
      _refs.pop();
      break;
    case t_str:
      assert(_str_idx>-1);
      --_str_idx;
      break;
    default:untested();
      unreachable();
    }
    _types.pop();
  }
  void new_string(std::ostream& o){
    ++_str_idx;
    if(_str_idx < _str_alloc){
    }else{
      assert(_str_idx==_str_alloc);
      ++_str_alloc;
      o__ "std::string s" << _str_idx << ";\n";
    }
    _types.push(t_str);
  }
  void new_array(std::ostream& o, Token_ARRAY_ const& /*TODO*/){
    ++_arr_idx;
    _types.push(t_arr);
    if(_arr_idx < _arr_alloc){ untested();
//      o__ "a" << _arr_idx << "= array_";
      o__ code_name() << "= array_";
    }else{
      o__ "array_ " << code_name();
      ++_arr_alloc;
    }
  }
  void new_ddouble(std::ostream& o){
    ++_ddo_idx;
    if(_ddo_idx < _ddo_alloc){
    }else{
      assert(_ddo_idx==_ddo_alloc);
      ++_ddo_alloc;
      assert(_ddo_idx>=0);
      if(_deps){
	o__ "ddouble t" << _ddo_idx << ";\n";
      }else{
	o__ "ddouble t" << _ddo_idx << ";\n"; // TODO? some deps?
      }
      if(!_deps){
      }else if(!options().optimize_deriv()){ untested();
	o__ "t" << _ddo_idx << ".set_all_deps(); // (all deriv)\n"; // code_name??
      }else{
	for(auto i: _deps->ddeps()){
	  o__ "// t" << _ddo_idx << "[d" << i->code_name() << "] = 0.; // (output dep)\n";
	}
      }
    }
    _types.push(t_ddo);
  }
  void new_float(std::ostream& o){
    ++_flt_idx;
    if(_flt_idx < _flt_alloc){ untested();
    }else{
      assert(_flt_idx==_flt_alloc);
      ++_flt_alloc;
      assert(_flt_idx>=0);
      o__ "double f" << _flt_idx << ";\n"; // code_name?
    }
    _types.push(t_flt);
  }
  bool is_ref() const{
    assert(!_types.empty());
    return _types.top() == t_ref;
  }
  void new_constant(std::ostream& o, Token_CONSTANT const& c);
  void new_rhs(Token_VAR_REF const* v){
    // TODO: linear?
    // if(0 && (*v)->is_real()){ untested();
    //   // crash?
    //   _refs.push("ddouble(" + (*v)->code_name() + ")/*rhsvar*/");
    // }else
    {
      _refs.push("" + v->code_name() + "/*rhsvar*/");
    }
    _types.push(t_ref);
  }
  void new_rhs(Token_PAR_REF const* v){
    // _refs.push("ddouble(" + (*v)->code_name() + ")/*rhsvar*/");
    // _refs.push((*v)->type() + "(" + (*v)->code_name() + ") /*rhspar*/");
    _refs.push("(" + (*v)->code_name() + ") /*rhspar*/");
    _types.push(t_ref);
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
  void args_pop(){
    assert(!_args.empty());
    _args.pop();
  }
  size_t size() const{
    return _refs.size();
  }
  std::string code_name() const{
    assert(_types.size());
    switch(_types.top()) {
    case t_flt:
      return "f" + std::to_string(_flt_idx);
    case t_ddo:
      return "t" + std::to_string(_ddo_idx);
    case t_arr:
      return "a" + std::to_string(_arr_idx);
    case t_str:
      return "s" + std::to_string(_str_idx);
    case t_ref:
      return _refs.top();
    default:untested();
      unreachable();
      return "";
    }
  }
  bool has_deps()const { return _deps; }
  TData const& deps()const { assert(_deps); return *_deps; }
}; // RPN_VARS
/*--------------------------------------------------------------------------*/
static void make_cc_string(std::ostream& o, String const& e)
{
  o << '"';
  for(char c : e.val_string()){
    if(c=='\n'){itested();
//      o << '\\';
    }else{
    }
    o << c;
  }
  o << '"';
}
/*--------------------------------------------------------------------------*/
void RPN_VARS::new_constant(std::ostream& o, Token_CONSTANT const& c)
{
  if(auto ff=dynamic_cast<Float const*>(c.data())){
#if 1
    new_ref(ftos_(ff->value(), 0, 20, ftos_EXP));
#else
    std::stringstream tmp;
    tmp << "(" << std::scientific << std::setprecision(17) << ff->value() << ")";
    s.new_ref(tmp.str());
#endif
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
static void make_cc_expression_(std::ostream& o, Expression const& e, RPN_VARS& s);
/*--------------------------------------------------------------------------*/
static void make_cc_expression_(std::ostream& o, Token const* t, RPN_VARS& s)
{
  Expression_ e;
  e.push_back(const_cast<Token*>(t));
  make_cc_expression_(o, e, s);
  e.pop_back();
}
/*--------------------------------------------------------------------------*/
static void make_cc_expression_(std::ostream& o, Expression const& e, RPN_VARS& s)
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
    trace3("mg_out_expr loop", (*i)->name(), (*i)->data(), s.size());

    if (auto var = dynamic_cast<const Token_VAR_REF*>(*i)) {
      s.new_rhs(var); // if linear?
//    }else if (auto t = dynamic_cast<const Token_OUT_VAR*>(*i)) {
//      s.new_rhs(t); // if linear?
//      //incomplete();
//      //o__ "0.; // OUTVAR?!\n";
    }else if(auto pp = dynamic_cast<const Token_ACCESS*>(*i)) {
      s.new_ddouble(o);
      if(!s.has_deps()){
      }else if(options().optimize_deriv()){
	o__ s.code_name() << ".set_no_deps();\n";
	// for(auto i: s.deps()){ untested();
	//   o__ s.code_name() << "[d" << i->code_name() << "] = 0.; // (output dep)\n";
	// }
      }else{itested();
      }

      if(!s.has_deps()){
	o__ s.code_name() << " = 0.; // static?\n";
      }else if(pp->is_short()){
	o__ s.code_name() << " = 0.; // short probe\n";
      }else{
	o__ s.code_name() << " = p->xs" << pp->code_name_() << "();\n";
      }
    }else if (auto p = dynamic_cast<const Token_PAR_REF*>(*i)) {
      s.new_rhs(p);
    }else if (auto pb = dynamic_cast<const Token_PORT_BRANCH*>(*i)) {
      incomplete();
      s.new_ref("0"); //port number here?
    }else if (auto A = dynamic_cast<const Token_ARRAY_*>(*i)) {
      if(A->args()){
	auto se = prechecked_cast<Expression const*>(A->args());
	assert(se);
	s.stop();
	make_cc_expression_(o, *se, s);
      }else{ untested();
      }
      // o__ "// array " << (*i)->name() << " " << s.have_args() << "\n";
      // o__ s.code_name() << " = ";
      std::vector<std::string> argnames;
      if(A->args()) {
	assert(s.have_args());
	argnames.resize(s.num_args());
	for(auto n=argnames.begin(); n!=argnames.end(); ++n){
	  *n = s.code_name();
	  s.pop();
	}
      }else{ untested();
      }
      s.new_array(o, *A);

      if(!A->args()) { untested();
//	o << A->code_name() << "(); // no parlist\n";
	assert(!argnames.size());
	o << " /*(312b)*/ "; //  << A->code_name();
      }else if(!argnames.size()){
//	o << A->code_name() << "(); // no args\n";
	s.args_pop();
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
	s.args_pop();
      }
    }else if (auto c = dynamic_cast<const Token_CONSTANT*>(*i)) {
      s.new_constant(o, *c);
    }else if(auto F = dynamic_cast<const Token_CALL*>(*i)) {
      if(F->args()){
	auto se = prechecked_cast<Expression const*>(F->args());
	assert(se);
	s.stop();
	make_cc_expression_(o, *se, s);
      }else{
      }

      o__ "// function " << (*i)->name() << " " << s.have_args() << "\n";
      std::vector<std::string> argnames;
      if(F->args()) {
	assert(s.have_args());
	argnames.resize(s.num_args());
	for(auto n=argnames.begin(); n!=argnames.end(); ++n){
	  *n = s.code_name();
	  s.pop();
	}
      }else{
      }

      if(F->returns_void()) {
	s.new_float(o); // TODO
	o__"(void)" <<  s.code_name() << ";\n";
      }else{
	s.new_ddouble(o);
	o__ s.code_name() << " = ";
      }

      if(!F->args()) {
	o << F->code_name() << "(); // no parlist\n";
	assert(!argnames.size());
      }else if(!argnames.size()){
	o << F->code_name() << "(); // no args\n";
	s.args_pop();
      }else{
	assert(F->code_name()!="");
	o << " /*(312)*/ " << F->code_name();

	o << "(";
       	std::string comma = "";
	for(size_t ii=argnames.size(); ii; --ii){
	  o << comma << argnames[ii-1];
	  comma = ", ";
	}
	o << ");\n";
	s.args_pop();
      }
    }else if (auto pl=dynamic_cast<const Token_PARLIST_*>(*i)) { untested();
      if(auto se = dynamic_cast<Expression const*>(pl->args())){ untested();
	o__ "// start parlist\n";
	s.stop();
	make_cc_expression_(o, *se, s);
	o__ "// end parlist\n";
      }else if(auto eee = dynamic_cast<Expression const*>((*i)->data())){ untested();
	o__ "// start parlist\n";
	s.stop();
	make_cc_expression_(o, *eee, s);
	o__ "// end parlist\n";
      }else{ untested();
	unreachable(); // ?
      }
    }else if (auto bo = dynamic_cast<const Token_BINOP_*>(*i)) {

      assert(bo->op1());
      assert(bo->op2());
      make_cc_expression_(o, bo->op1(), s);
      make_cc_expression_(o, bo->op2(), s);

      assert((*i)->name().size());
      std::string idy = s.code_name();
      s.pop();
      std::string arg1 = s.code_name();
      s.pop();
      s.new_ddouble(o);

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
	o__ s.code_name() << " = " << arg1 << " " << (*i)->name() << " " << idy << ";\n";
      }else if(op == '%'){itested();
	o__ s.code_name() << " = va::fmod(" << arg1 << ", " << idy << ");\n";
      }else{ untested();
	unreachable();
	assert(false);
	throw Exception("run time error in make_cc_expression: " + (*i)->name());
      }
    }else if (auto u = dynamic_cast<const Token_UNARY_*>(*i)) {
      assert(u->op1());
      make_cc_expression_(o, u->op1(), s);

      std::string arg1 = s.code_name();
      s.pop();
      s.new_ddouble(o);

      auto op = (*i)->name()[0];
      if(op == '-' || op == '!' || op == '+') {
	o__ s.code_name() << " = " << op << arg1 << ";\n";
      }else{ untested();
	incomplete();
	unreachable();
	o__ s.code_name() << " INCOMPLETE = " << op << arg1 << ";\n";
      }
    }else if (auto t = dynamic_cast<const Token_TERNARY_*>(*i)) {
      assert(t->cond());
      make_cc_expression_(o, t->cond(), s);

      std::string arg1 = s.code_name();
      s.pop();
      s.new_ddouble(o);

      o__ "{\n";
      {
	indent y;
	o__ "ddouble& tt0 = t0;\n"; // BUG: float??
	o__ "if(" << arg1 << "){\n";
	{
	  indent x;
	  make_cc_expression(o, *t->true_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}else{\n";
	{
	  indent x;
	  make_cc_expression(o, *t->false_part());
	  o__ "tt0 = t0;\n";
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
      s.stop();
      incomplete();
      unreachable();
    }
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_expression(std::ostream& o, Expression const& e, bool dynamic)
{
  TData const* deps = NULL;
  if(!dynamic){
  }else if(auto ex = dynamic_cast<Expression_ const*>(&e)){
    deps = &ex->deps();
  }else{ untested();
  }
  RPN_VARS s(deps);
  make_cc_expression_(o, e, s);

  if(s.is_ref()){
    s.new_ddouble(o);
    s.pop();
    o__ "t0 = " << s.code_name() << ";\n";
  }else{
  }
  s.pop();
}
/*--------------------------------------------------------------------------*/
// TODO: use FUNCTION_?
void make_cc_event_cond(std::ostream& o, Expression const& e)
{
  typedef Expression::const_iterator const_iterator;
  // TODO: var stack.
  o__ "bool evt = false\n;";
  o__ "{\n";
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    if((*i)->name()=="initial_step"){
      o__ "evt = _sim->_phase == p_INIT_DC;\n";
//    o__ "evt = _sim->vams_initial_step();\n"; // TODO
    }else if((*i)->name()=="initial_model"){ untested();
      std::cerr << "WARNING: ADMS style keyword encountered\n";
      o__ "evt = _sim->is_initial_step();\n";
    }else{ untested();
      incomplete();
      o << "--> " << (*i)->name() << " <--\n";
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
