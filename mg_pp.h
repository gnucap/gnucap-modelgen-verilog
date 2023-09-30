/*                             -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023 Felix Salfelder
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
#ifndef GNUCAP_MG_PP_H
#define GNUCAP_MG_PP_H
/*--------------------------------------------------------------------------*/
class Preprocessor : public CS {
  Define_List _define_list;
  std::string _cwd;
  std::string _include_path;
  std::basic_ostream<char>* _diag{NULL};
private:
  std::string _stripped_file; // DUP?
public:
  explicit Preprocessor();
  void read(std::string const& file_name);
  void define(std::string const&);
  void dump(std::ostream&)const;
  void add_include_path(std::string const&);
  void set_diag(std::basic_ostream<char>& o){
    _diag = &o;
  }
private:
  void parse(CS& file);
  void include(const std::string& file_name); // `include?
  std::basic_ostream<char>& diag() {
    if(_diag){
      return *_diag;
    }else{
      return std::cerr;
    }
  }
  Define_List& define_list() { return _define_list; }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
