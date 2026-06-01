/*                             -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023-26 Felix Salfelder
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
 */
#ifndef MG_OUT_H
#define MG_OUT_H
/*--------------------------------------------------------------------------*/
#include <fstream>
#include "mg_base.h"
#include "mg_dump.h"
/*--------------------------------------------------------------------------*/
#ifdef DO_TRACE_TAGS
#define make_tag(o) (o << "//" << __FILE__ ":" << __func__ << ":" << __LINE__ << "\n")
#else
#define make_tag(o)
#endif
/*--------------------------------------------------------------------------*/
// obsolete.
struct indent{
  std::string _old;
  explicit indent(int i=2){
    _old = ind;
    if(i<0){ untested();
      assert(ind.size()>=size_t(-i));
      ind = ind.substr(i, ind.size()+i);
    }else{
      ind = ind + std::string(i, ' ');
    }
  }
  explicit indent(std::string s){ untested();
    _old = ind;
    ind = ind + s;
  }
  ~indent(){ unindent(); }
  void unindent(){ ind = _old; }
};
/*--------------------------------------------------------------------------*/
class oindent : private std::streambuf, public std::ostream{
  std::ostream& _o;
  int _indent;
  std::string _old_ind; // obsolete.
public:
  oindent(std::ostream&o, int indent=2) : std::ostream(this), _o(o), _indent(indent) {
    _old_ind = ind;
    ind = "";
  }
  ~oindent(){
    ind = _old_ind;
  }

  int overflow(int c)override {
    while(_indent) {
      _o << " ";
      --_indent;
    }
    if(c=='\n'){
      _indent = 2;
    }else{
    }
    _o << char(c);
    return 0;
  }
};
/*--------------------------------------------------------------------------*/
class File;
/* mg_out_root.cc */
//void make_h_file(const File&);
void make_cc_file(const File&);
void make_dump_file(const File&);
void make_cc(std::ostream&, const File&);
/*--------------------------------------------------------------------------*/
/* mg_out_lib.cc */
class Parameter_List_Collection;
class Parameter_1;
typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
class Parameter_2;
class Parameter_Block;
void make_final_adjust_eval_parameter_list(std::ostream&, const Parameter_List_Collection&);
void make_final_adjust_value(std::ostream&, const Parameter_2&);
void make_final_adjust_value_list(std::ostream&, const Parameter_List_Collection&);
void make_final_adjust_parameter(std::ostream&, const Parameter_1&);
void make_final_adjust_parameter_list(std::ostream&, const Parameter_1_List&);
void make_final_adjust(std::ostream&, const Parameter_Block&);
void make_construct_parameter_list(std::ostream&, const Parameter_List_Collection&);
void make_get_param_list(std::ostream&, const Parameter_1_List&);
void make_print_param_list(std::ostream&, const Parameter_1_List&);
void make_print_calc_param_list(std::ostream&, const Parameter_1_List&);
void make_copy_construct_parameter_list(std::ostream&, const Parameter_List_Collection&);
/*--------------------------------------------------------------------------*/
/* mg_out_h.cc */
class Module;
void make_cc_decl(std::ostream&, const Module&);
/* mg_out_module.cc */
void make_cc_module(std::ostream&, const Module&);
/* mg_out_analog.cc */
void make_cc_analog(std::ostream&, const Module&);
//void make_cc_func(std::ostream&, const Module&); // ?
void make_cc_analog_functions(std::ostream&, const Module&);
/* mg_out_common.cc */
void make_cc_common(std::ostream&, const Module&);
/*--------------------------------------------------------------------------*/
/* mg_out_primitive.cc */
class Primitive;
void make_cc_primitive(std::ostream&, const Primitive&);
/*--------------------------------------------------------------------------*/
inline std::string baseclass(Module const&)
{
  return "BASE_SUBCKT";
  // return "COMPONENT";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif // guard
// vim:ts=8:sw=2:noet
