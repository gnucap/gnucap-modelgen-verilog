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
 * Verilog-AMS functions, tasks, calls.. under construction
 */

#include <u_function.h>
#include "mg_token.h" // BUG?
#include "mg_base.h"
#include "mg_lib.h"
//#include <m_expression.h>
/*--------------------------------------------------------------------------*/
#ifndef MG_FUNCTION_H
#define MG_FUNCTION_H
/*--------------------------------------------------------------------------*/
class Token;
class Expression;
class Module;
class TData;
class Data_Type;
// TODO: move upstream, partly?
class FUNCTION_ : public FUNCTION {
  mutable int _refs{0};
//  std::string _label; // e_base.
  size_t _num_args{size_t(-1)};
public:
  explicit FUNCTION_() : FUNCTION() {}
  explicit FUNCTION_(FUNCTION_ const& o)
  	: FUNCTION(o), _num_args(o._num_args) {
	  set_label(o.short_label()); // base?
  }

//  void set_label(std::string const& l){ untested();
//  	_label = l;
//  }
  ~FUNCTION_();
  virtual FUNCTION_* clone()const { untested();
         unreachable();
         return nullptr;
  }

public: // characteristics
  std::string const& label()const { return short_label(); }
  void set_num_args(size_t n){ _num_args = n; }
  size_t num_args() const { return _num_args; }
  virtual bool is_output_arg(int)const {return false;}
  virtual Data_Type const* arg_type(int)const{return nullptr;}
  virtual Data_Type const* return_type()const{return nullptr;}
  virtual bool has_analysis()const  {return false;}
  virtual bool has_precalc()const   {return false;}
  virtual bool has_tr_begin()const  {return false;}
  virtual bool has_tr_restore()const{return has_tr_begin();}
  virtual bool has_tr_eval()const   {return true;}
  virtual bool has_tr_review()const {return false;}
  virtual bool has_tr_accept()const {return false;}
  virtual bool has_tr_advance()const{return false;}
  virtual bool has_tr_regress()const{return has_tr_advance();}
  virtual bool has_set_event()const {return false;}
  virtual bool has_state()const {return false;}
  virtual bool has_modes()const {return false;}
  virtual bool context_arg()const {return false;}
  virtual bool is_analog_filter()const {return false;}

public: // non-virtual. TODO
  virtual bool static_code()const {return false;}
  virtual bool is_common()const {return static_code();}
  virtual bool is_in_common()const {return is_common();}
  virtual bool needs_context()const {
    return (is_common() && !static_code())
       ||  (is_common() && has_tr_accept()) ;}
  virtual bool returns_void()const { return false; } // use return_type?

public: // code generation
  virtual void make_cc_impl(std::ostream&)const {}
  virtual void make_cc_common(std::ostream&)const {}
  virtual void make_cc_dev(std::ostream&)const {}
  virtual void make_cc_tr_advance(std::ostream&)const {}
  virtual void make_cc_tr_review(std::ostream&)const {}
  virtual void make_cc_tr_accept(std::ostream&)const {}

  virtual Token* new_token(Module&, size_t)const {return nullptr;}
  virtual std::string code_name()const { itested();
	  // incomplete();
	  return "";
  }
  using FUNCTION::stack_op;
  void stack_op(Expression*)const override = 0;
  virtual double evalf(double const*)const {
    throw Exception("not implemented");
  }
  void setup(Module*) {};
/*--------------------------------------------------------------------------*/
public:
  void	      inc_refs()const	{++_refs;}
  void	      dec_refs()const	{assert(_refs); --_refs;}
  bool	      has_refs()const	{return _refs;}
}; // FUNCTION_
/*--------------------------------------------------------------------------*/
class MGVAMS_FUNCTION : public FUNCTION_ {
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
	  unreachable();
	  return "func";
  }
public:
  ~MGVAMS_FUNCTION() {}
  virtual void make_cc_dev(std::ostream&)const override {}
//  Token* new_token(Module& m, size_t na) const override;
  bool static_code()const override {return true;}
};
/*--------------------------------------------------------------------------*/
class MGVAMS_EVENT : public FUNCTION_ {
private:
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
public:
  MGVAMS_EVENT() : FUNCTION_() { }
  MGVAMS_EVENT(MGVAMS_EVENT const& p) : FUNCTION_(p) { }
  ~MGVAMS_EVENT() {} //  {delete _rdeps;}
  bool needs_context()const override{ return true; }
};
/*--------------------------------------------------------------------------*/
class Node_Ref;
class Branch;
class MGVAMS_FILTER : public FUNCTION_ {
protected: // BUG
  Module* _m{nullptr};
  Branch* _br{nullptr};
  Probe const* _prb{nullptr};
  mutable Branch const* _output{nullptr};
public:
  ~MGVAMS_FILTER() {}
  // Token* new_token(Module& m, size_t na, Deps& d) const override;
  void make_cc_common(std::ostream&)const override {}
  std::string code_name()const override{ untested();
    return "";
  }
  virtual Node_Ref p() const;
  virtual Node_Ref n() const;
  virtual Branch const* output()const{ untested();return nullptr;}
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  bool has_precalc()const override { return true;}
  bool is_standalone()const { return _output; }
 // bool is_analog_filter()const {return true;} not yet
public:
  virtual /*BUG*/ Branch* branch() const { return _br;}
private:
 // void has_setup()const override{return true;}
 // void setup(Module*)override;
protected:
public: // bug
  void set_n_to_gnd(Module*)const;
  void set_p_to_gnd(Module*)const;
  void set_output(Branch const*b){_output=b;}
  bool has_output()const {return _output;}
public: // XDT refactor
  virtual bool port_hack()const {return true;} // TODO
  virtual int max_args()const {assert(0); return 0; }
  virtual Branch* branch__() const {return _br;}
  virtual void set_n_to_gnd__()const {assert(0);}
  virtual void set_p_to_gnd__()const {assert(0);}
  virtual Probe const* prb__()const {return _prb;}
  void set_owner(Module* m) {_m=m;}
  void set_branch(Branch* br);
};
/*--------------------------------------------------------------------------*/
class MGVAMS_TASK : public FUNCTION_ {
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
public:
  ~MGVAMS_TASK() {}
  std::string code_name()const override{ untested();
	  return "";
  }
};
/*--------------------------------------------------------------------------*/
class VAMS_ACCESS : public FUNCTION_ {
  std::string _name, _arg0, _arg1;
public:
  VAMS_ACCESS(std::string n, std::string a0, std::string a1)
    : _name(n), _arg0(a0), _arg1(a1) { untested();
  }
private:
  void stack_op(Expression*)const override { untested();
    throw Exception("invalid");
  }
  Token* new_token(Module& m, size_t na)const override;
  void make_cc_common(std::ostream&)const override { untested(); unreachable(); }
};
/*--------------------------------------------------------------------------*/
// inline void MGVAMS_FUNCTION::make_cc_dev(std::ostream& o) const
// { untested();
// //   o__ "// " << label() << "\n";
// }
/*--------------------------------------------------------------------------*/
inline FUNCTION_::~FUNCTION_()
{
  if(has_refs()){
    incomplete();
    std::cerr << "stale ref " << label() << "\n";
  }else{
  }
//  assert(!has_refs()); // base class
}
/*--------------------------------------------------------------------------*/
void stack_op(FUNCTION_ const* f, Expression const& args, Expression* out);
/*--------------------------------------------------------------------------*/
template<>
inline std::string key_(FUNCTION_ const* t)
{
  return t->short_label();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
