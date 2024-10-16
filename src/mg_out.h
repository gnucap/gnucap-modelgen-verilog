/*$Id: mg_out.h,v 26.128 2009/11/10 04:21:03 al Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
//#include "mg_.h"
/*--------------------------------------------------------------------------*/
extern std::string ind;
#define o__ o << ind <<
#define o____ o__ "  " <<
#define o______ o____ "  " <<
#define o________ o______ "  " <<
#define o__________ o________ "  " <<
/*--------------------------------------------------------------------------*/
#ifdef DO_TRACE_TAGS
#define make_tag(o) (o << "//" << __FILE__ ":" << __func__ << ":" << __LINE__ << "\n")
#else
#define make_tag(o)
#endif
/*--------------------------------------------------------------------------*/
struct indent{
  explicit indent(size_t i=2){
    _old = ind;
    ind = ind + std::string(i, ' ');
  }
  explicit indent(std::string s){ untested();
    _old = ind;
    ind = ind + s;
  }
  ~indent(){
    ind = _old;
  }
  std::string _old;
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
inline std::string baseclass(Module const&)
{
  return "BASE_SUBCKT";
  // return "COMPONENT";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif // guard
// vim:ts=8:sw=2:noet
