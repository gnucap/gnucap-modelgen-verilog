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
#include "m_tokens.h"
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
  if (len < 3) {
    untested();
    len = 3;
  }
  if (len > MAXLENGTH-6) {
    untested();
    len = MAXLENGTH-6;
  }
  if (fieldwidth > MAXLENGTH-1) {
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
  if (num == std::numeric_limits<double>::infinity()) {
    untested();
    memcpy(str, " Over", 5);
  }else if (num == -std::numeric_limits<double>::infinity()) {
    untested();
    memcpy(str, "-Over", 5);
  }else if (num == std::numeric_limits<double>::quiet_NaN()) {
    untested();
    memcpy(str, " NaN", 4);
  }else if (num == std::numeric_limits<double>::signaling_NaN()) {
    untested();
    memcpy(str, " NaN", 4);
  }else
#endif
  if (num == NOT_VALID) {
    memcpy(str, " ??", 3);
  }else if (num == NOT_INPUT) {
    memcpy(str, " NA", 3);
  }else if (num >= BIGBIG) {
    memcpy(str, " Inf", 4);
  }else if (num <= -BIGBIG) {
    memcpy(str, "-Inf", 4);
  }else if (num != num) {
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
	}else if (fmt & ftos_SIGN) {
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
	if (len < 3) {
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
	if (num >= 1.) {
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
      }else{
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
      if (str[nnn-1] == 'M') {
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
    // t_int,
    t_str,
    t_ref,
  } type;
  std::stack<type> _types;
  std::stack<int> _args;
  std::stack<std::string> _refs;
//  int _idx_alloc{0};

  int _flt_idx{-1};
  int _flt_alloc{0};
  int _str_idx{-1};
  int _str_alloc{0};
public:
  ~RPN_VARS(){
    assert(_flt_idx == -1);
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
    case t_ref:
      _refs.pop();
      break;
    case t_str:
      assert(_str_idx>-1);
      --_str_idx;
      break;
    default:
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
  void new_float(std::ostream& o){
    ++_flt_idx;
    if(_flt_idx < _flt_alloc){
    }else{
      assert(_flt_idx==_flt_alloc);
      ++_flt_alloc;
      o__ "ddouble t" << _flt_idx << ";\n";
    }
    _types.push(t_flt);
  }
  bool is_ref() const{
    assert(!_types.empty());
    return _types.top() == t_ref;
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
    switch(_types.top()) {
    case t_flt:
      return "t" + std::to_string(_flt_idx);
    case t_str:
      return "s" + std::to_string(_str_idx);
    case t_ref:
      return _refs.top();
    default:
      unreachable();
      return "";
    }
  }
};
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
static void make_cc_expression_(std::ostream& o, Expression const& e, RPN_VARS& s)
{
  typedef Expression::const_iterator const_iterator;

#if 0
  if(auto se = dynamic_cast<Expression_ const*>(&e)){
    o << "/* ";
    se->dump(o);
    o << "\n";
    for(auto i : se->deps()) {
      o << "// Dep: " << i->code_name();
      o << "\n";
    }
    o << "*/\n";
  }else{
  }
#endif

  int have_parlist = false;
  // The _list is the expression in RPN.
  // print a program that computes the function and the derivatives.
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    trace2("mg_out_expr loop", (*i)->name(), s.size());

    if (auto var = dynamic_cast<const Token_VAR_REF*>(*i)) {
      s.new_ref((*var)->code_name());
    }else if(auto pp = dynamic_cast<const Token_ACCESS*>(*i)) {
      s.new_float(o);
//      assert((*pp)->branch());
      if(pp->is_short()){ untested();
	o__ s.code_name() << " = 0.; // short probe\n";
      }else{
	char sign = pp->is_reversed()?'-':'+';
	o__ s.code_name() << " = " << sign << "p->" << pp->code_name() << "; // "<< pp->name() <<"\n";
	o__ s.code_name() << "[d" << pp->code_name() << "] = " << sign << "1.;\n";
      }
    }else if (auto p = dynamic_cast<const Token_PAR_REF*>(*i)) {
      s.new_ref("pc->" + p->code_name());
    }else if (auto c = dynamic_cast<const Token_CONSTANT*>(*i)) {
      if(auto ff=dynamic_cast<Float const*>(c->data())){
#if 1
	s.new_ref(ftos_(ff->value(), 0, 20, ftos_EXP));
#else
	std::stringstream tmp;
	tmp << "(" << std::scientific << std::setprecision(17) << ff->value() << ")";
	s.new_ref(tmp.str());
#endif
      }else if(auto S=dynamic_cast<String const*>(c->data())){
	s.new_string(o);
	o__ s.code_name() << " = ";
	make_cc_string(o, *S);
	o << ";\n";
      }else{untested();
	unreachable();
	s.new_string(o);
	o__ s.code_name() << " = " << (*i)->name() << "; (u)\n";
      }
    }else if(auto F = dynamic_cast<const Token_CALL*>(*i)) {
      o__ "// function " << (*i)->name() << " " << s.have_args() << "\n";
      std::vector<std::string> argnames;
      if(have_parlist){
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
	o__ "/*void*/ ";
      }else{
	s.new_float(o);
	o__ s.code_name() << " = ";
      }

      if(!have_parlist){
	o << F->code_name() << "(); // no parlist\n";
      }else if(!argnames.size()){
	o << F->code_name() << "(); // no args\n";
	s.args_pop();
      }else{
	assert(F->code_name()!="");
	o << " /*(312)*/ " << F->code_name();

	o << "(";
       	std::string comma = "";
	for(size_t i=argnames.size(); i; --i){
	  o << comma << argnames[i-1];
	  comma = ", ";
	}
	o << ");\n";
	s.args_pop();
      }
    }else if(dynamic_cast<const Token_SYMBOL*>(*i)) { untested();
      o__ "// incomplete:symbol " << (*i)->name() << "\n";
      unreachable();
    }else if (dynamic_cast<const Token_PARLIST_*>(*i)) {
      if(auto se = dynamic_cast<Expression const*>((*i)->data())){
	s.stop();
	make_cc_expression_(o, *se, s);
      }else{ untested();
	unreachable(); // ?
      }
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) {
    }else if (dynamic_cast<const Token_STOP*>(*i)) {
      s.stop();
    }else if (dynamic_cast<const Token_BINOP*>(*i)) {
      assert((*i)->name().size());
      std::string idy = s.code_name();
      s.pop();
      std::string arg1 = s.code_name();
      s.pop();
      s.new_float(o);

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
      }else if(op == '%'){ itested();
	o__ s.code_name() << " = va::fmod(" << arg1 << ", " << idy << ");\n";
      }else{ untested();
	unreachable();
	assert(false);
	throw Exception("run time error in make_cc_expression: " + (*i)->name());
      }
    }else if (dynamic_cast<const Token_UNARY*>(*i)) {
      std::string arg1 = s.code_name();
      s.pop();
      s.new_float(o);

      auto op = (*i)->name()[0];
      if(op == '-' || op == '!') {
	o__ s.code_name() << " = " << op << arg1 << ";\n";
      }else{ untested();
	incomplete();
	unreachable();
	o__ s.code_name() << " INCOMPLETE = " << op << arg1 << ";\n";
      }
    }else if (auto t = dynamic_cast<const Token_TERNARY*>(*i)) {itested();
      std::string arg1 = s.code_name();
      s.pop();
      s.new_float(o);

      o__ "{\n";
      {itested();
	indent y;
	o__ "ddouble& tt0 = t0;\n"; // BUG: float??
	o__ "if(" << s.code_name() << "){\n";
	{itested();
	  indent x;
	  make_cc_expression(o, *t->true_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}else{\n";
	{itested();
	  indent x;
	  make_cc_expression(o, *t->false_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}\n";
      }
      o__ "}\n";
    }else{ untested();
      incomplete();
      unreachable();
      assert(false);
    }

    // TODO: use token.
    if (dynamic_cast<const Token_PARLIST*>(*i)) {
      have_parlist = true;
    }else{
      have_parlist = false;
    }
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_expression(std::ostream& o, Expression const& e)
{
  RPN_VARS s;
  make_cc_expression_(o, e, s);

  if(s.is_ref()){
    s.new_float(o);
    s.pop();
    o__ "t0 = " << s.code_name() << ";\n";
  }else{
  }
  s.pop();
}
/*--------------------------------------------------------------------------*/
void make_cc_event_cond(std::ostream& o, Expression const& e)
{
  typedef Expression::const_iterator const_iterator;
  // TODO: var stack.
  o__ "bool evt = false\n;";
  o__ "{\n";
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    if((*i)->name()=="initial_step"){
      o__ "evt = _sim->is_initial_step();\n";
    }else if((*i)->name()=="initial_model"){ itested();
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
