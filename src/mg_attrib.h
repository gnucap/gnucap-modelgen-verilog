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
 * use <u_attrib.h> instead?
 */
//
#ifndef MG_ATTRIB_H
#define MG_ATTRIB_H
#include "mg_base.h"
/*--------------------------------------------------------------------------*/
class Attribute_Spec : public Owned_Base {
  String_Arg _key;
//  typedef ConstExpression value_type; TODO
public:
//  typedef Raw_String_Arg value_type;
  // typedef String_Arg value_type;
  typedef std::string value_type;
private:
  value_type* _expr{NULL};
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override;
public:
  Attribute_Spec() : Owned_Base() {untested();}
  Attribute_Spec(CS& f, Block* o) : Owned_Base(o) {
    set_owner(o);
    parse(f);
  }
  ~Attribute_Spec(){
    delete _expr;
    _expr = NULL;
  }
  bool operator==(Attribute_Spec const& o) const{ untested();
    return o._key == _key;
  }
  bool operator!=(String_Arg const& o) const{ untested();
    return o != _key;
  }

  String_Arg const& key() const{
    return _key;
  }
  bool has_expression() const{
    return _expr;
  }
  value_type const* expression_or_null() const{
    return _expr;
  }
  value_type const& expression() const{
    assert(_expr);
    return *_expr;
  }
};
/*--------------------------------------------------------------------------*/
class Attribute_Instance : public Collection<Attribute_Spec> {
//  std::set<Attribute_Spec*, getkey>;
  typedef Attribute_Spec::value_type value_type;
public:
  Attribute_Instance() : Collection<Attribute_Spec>() { untested(); }
  Attribute_Instance(CS& f, Block* o) : Collection<Attribute_Spec>() {
    set_owner(o);
    parse(f);
  }
  void parse(CS& f)override;
  void dump(std::ostream&)const override;
  void clear();
//  bool empty() const{ return _m.empty(); }
  value_type const* find(String_Arg const&) const;
private:
};
/*--------------------------------------------------------------------------*/
class Attribute_Stash : public Owned_Base {
  Attribute_Instance* _a{NULL};
public:
  Attribute_Stash() : Owned_Base() { }
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ unreachable(); }
  bool is_empty() const{
    return !_a;
  }
  Attribute_Instance const* detach(){
    Attribute_Instance const* r = _a;
    _a = NULL;
    return r;
  }
private:
};
/*--------------------------------------------------------------------------*/
class Attribute : public Base {
  String_Arg _name;
  String_Arg _value;
public:
  void set_owner(Block const*){}
  void parse(CS& f)override {f >> _name >> '=' >> _value >> ';';}
  void dump(std::ostream& f)const override
	  {f << "  " << name() << " = \"" << value() << "\";\n";}
  Attribute() {}
  const String_Arg&  key()const	  {return _name;}
  const String_Arg&  name()const  {return _name;}
  const String_Arg&  value()const {return _value;}
};
typedef Collection<Attribute> Attribute_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
