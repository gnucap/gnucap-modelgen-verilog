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
/*--------------------------------------------------------------------------*/
class Token;
class Module;
class Deps;
// TODO: move upstream, partly?
class FUNCTION_ : public FUNCTION {
	std::string _label;
	size_t _num_args{size_t(-1)};
public:
	explicit FUNCTION_() : FUNCTION() {}
	explicit FUNCTION_(FUNCTION_ const& o)
		: FUNCTION(o), _label(o._label), _num_args(o._num_args) {}
	void set_label(std::string const& l){
		_label = l;
	}
	std::string label() const{
		return _label;
	}
	void set_num_args(size_t n){ _num_args = n; }
	size_t num_args() const { return _num_args; }

	virtual void make_cc_impl(std::ostream&)const {}
	virtual void make_cc_dev(std::ostream&) const {}

	/* TODO: some kind of
	Expression* eval(Expression const*);
	*/
  virtual Token* new_token(Module& m, size_t na, Deps& d) const = 0;
  virtual bool returns_void() const { return false; }
  virtual std::string code_name() const{ itested();
	  // incomplete();
	  return "";
  }
};
/*--------------------------------------------------------------------------*/
class MGVAMS_FUNCTION : public FUNCTION_ {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "func";
  }
public:
  virtual MGVAMS_FUNCTION* clone()const {
	  unreachable();
	  return NULL;
  }
  virtual void make_cc_dev(std::ostream& o)const override;
  virtual void make_cc_common(std::ostream& o) const = 0;
  Token* new_token(Module& m, size_t na, Deps& d) const override;
};
/*--------------------------------------------------------------------------*/
class MGVAMS_FILTER : public FUNCTION_ {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "filt";
  }
public:
  // Token* new_token(Module& m, size_t na, Deps& d) const override;
  virtual void make_cc_common(std::ostream&) const {}
  virtual std::string code_name() const{
	  return "";
  }
};
/*--------------------------------------------------------------------------*/
class MGVAMS_TASK : public FUNCTION_ {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "task";
  }
public:
  virtual MGVAMS_TASK* clone()const {
	  unreachable();
	  return NULL;
  }
  virtual void make_cc_common(std::ostream&)const {} //up?
  std::string code_name()const override{
	  return "";
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
