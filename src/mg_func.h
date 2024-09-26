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
//#include <m_expression.h>
/*--------------------------------------------------------------------------*/
#ifndef MG_FUNCTION_H
#define MG_FUNCTION_H
/*--------------------------------------------------------------------------*/
class Token;
class Expression;
class Module;
class TData;
// TODO: move upstream, partly?
class FUNCTION_ : public FUNCTION {
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

public: // characteristics
  std::string const& label()const { return short_label(); }
  std::string const& key()const { return label(); } // free?
  void set_num_args(size_t n){ _num_args = n; }
  size_t num_args() const { return _num_args; }
  virtual bool has_precalc()const   {return false;}
  virtual bool has_tr_begin()const  {return false;}
  virtual bool has_tr_restore()const{return has_tr_begin();}
  virtual bool has_tr_eval()const   {return true;}
  virtual bool has_tr_review()const {return false;}
  virtual bool has_tr_accept()const {return false;}
  virtual bool has_tr_advance()const{return false;}
  virtual bool has_tr_regress()const{return has_tr_advance();}
  virtual bool has_set_event()const {return false;}

  virtual bool static_code()const {return false;}
  virtual bool is_common()const {return false;}
  virtual bool has_modes()const {return false;}
  virtual bool returns_void()const { return false; }

public: // code generation
  virtual void make_cc_impl(std::ostream&)const {}
  virtual void make_cc_common(std::ostream&)const {}
  virtual void make_cc_dev(std::ostream&)const {}
  virtual void make_cc_tr_advance(std::ostream&)const {}
  virtual void make_cc_tr_review(std::ostream&)const {}
  virtual void make_cc_tr_accept(std::ostream&)const {}

  virtual Token* new_token(Module&, size_t)const { untested();unreachable(); return NULL;}
  virtual std::string code_name()const { itested();
	  // incomplete();
	  return "";
  }
  void stack_op(Expression const& args, Expression* out) const;
  virtual double evalf(double const*)const {
    throw Exception("not implemented");
  }
  virtual void setup(Module*) {};
/*--------------------------------------------------------------------------*/
public: // use refcounter in e_base
  void	      inc_refs()const	{inc_probes();}
  void	      dec_refs()const	{dec_probes();}
  bool	      has_refs()const	{return has_probes();}
}; // FUNCTION_
/*--------------------------------------------------------------------------*/
class MGVAMS_FUNCTION : public FUNCTION_ {
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
	  unreachable();
	  return "func";
  }
public:
  ~MGVAMS_FUNCTION() {}
  virtual MGVAMS_FUNCTION* clone()const { untested();
	  unreachable();
	  return NULL;
  }
  virtual void make_cc_dev(std::ostream&)const override {}
//  Token* new_token(Module& m, size_t na) const override;
  bool static_code()const override {return true;}
};
/*--------------------------------------------------------------------------*/
class Node_Ref;
class Branch;
class MGVAMS_FILTER : public FUNCTION_ {
protected:
  mutable Branch const* _output{NULL};
public:
  ~MGVAMS_FILTER() {}
  // Token* new_token(Module& m, size_t na, Deps& d) const override;
  void make_cc_common(std::ostream&)const override {}
  std::string code_name()const override{ untested();
    return "";
  }
  virtual Node_Ref p() const;
  virtual Node_Ref n() const;
  virtual Branch const* output()const{ untested();return NULL;}
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
	  unreachable();
	  return "filt";
  }
  bool has_precalc()const override { return true;}
  bool is_standalone()const { return _output; }
private:
  virtual Branch* branch() const {return NULL;}
  void setup(Module*)override;
protected:
public: // bug
  void set_n_to_gnd(Module*)const;
  void set_p_to_gnd(Module*)const;
};
/*--------------------------------------------------------------------------*/
class MGVAMS_TASK : public FUNCTION_ {
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
	  unreachable();
	  return "task";
  }
public:
  ~MGVAMS_TASK() {}
  virtual MGVAMS_TASK* clone()const { untested();
	  unreachable();
	  return NULL;
  }
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
  std::string eval(CS&, const CARD_LIST*)const override { untested();unreachable(); return "";}
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
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
