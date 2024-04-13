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
 */
//
#ifndef MG_ATTRIB_H
#define MG_ATTRIB_H
#include "mg_base.h"
// <u_attrib.h> instead?
/*--------------------------------------------------------------------------*/
class Attribute_Spec : public Owned_Base {
  String_Arg _key;
public:
  typedef std::string value_type;
private:
  value_type* _expr{NULL};
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override;
public:
  Attribute_Spec() : Owned_Base() { untested();untested();}
  Attribute_Spec(CS& f, Block* o) : Owned_Base(o) { untested();
    set_owner(o);
    parse(f);
  }
  ~Attribute_Spec(){ untested();
    delete _expr;
    _expr = NULL;
  }
  bool operator==(Attribute_Spec const& o) const{ untested();
    return o._key == _key;
  }
  bool operator!=(String_Arg const& o) const{ untested();
    return o != _key;
  }

  String_Arg const& key() const{ untested();
    return _key;
  }
  bool has_expression() const{ untested();
    return _expr;
  }
  value_type const* expression_or_null() const{ untested();
    return _expr;
  }
  value_type const& expression() const{ untested();
    assert(_expr);
    return *_expr;
  }
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
void move_attributes(void* from, void* to);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
