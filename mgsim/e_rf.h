/*                        -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
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
 * various rational function structures
 */
#include <e_compon.h>
/*--------------------------------------------------------------------------*/
template<class C>
COMPLEX evalp(COMPLEX x, C const c, size_t d)
{
  if(d==0){
    return 0.;
  }else{
    return double(*c) + x * evalp(x, c+1, d-1);
  }
}
/*--------------------------------------------------------------------------*/
class poly{
  typedef std::vector<COMPLEX> c_t;
  typedef c_t::const_iterator const_iterator;
  c_t _c;
public:
  explicit poly() : _c(1) { _c[0] = 1.; }

  // multiply by x-r
  void add_root(COMPLEX const& r){
    assert(_c.back() == 1.);
    _c.push_back(1.);
    for(size_t i = _c.size()-1; i>1; ){
      --i;
      _c[i] *= -r;
      _c[i] += _c[i-1];
    }
    _c[0] *= -r;

    trace1("new root: ", r);
    for(auto x : _c){
      trace1("new root", x);
    }
  }
  // constant part is 1, where possible.
  void normalise(){
    size_t j;
    for(j = 0; j<_c.size(); ++j){
      if(fabs(_c[j])>1e-15){
	break;
      }else{
      }
    }
    if( j<_c.size() ){
      size_t p = j;
      ++j;
      for(; j<_c.size(); ++j){
	_c[j] /= _c[p];
      }
      _c[p] = 1.;
    }else{ untested();
      // trailing zeroes. OK in numerator...
    }
    for(auto x : _c){
      trace1("normalised: ", x);
    }
  }
  COMPLEX const& operator[](int i){ untested(); return _c[i]; }
  const_iterator begin()const {return _c.begin();}
  const_iterator end()const {return _c.end();}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class COMMON_RF_BASE :public COMMON_COMPONENT{
public:
  std::vector<PARAMETER<double> > _p_num;
  std::vector<PARAMETER<double> > _p_den;
  int _num_type{0};
  int _den_type{0};
public:
  explicit COMMON_RF_BASE(const COMMON_RF_BASE& p);
  explicit COMMON_RF_BASE(int c=0);
           ~COMMON_RF_BASE() {}
  bool     operator==(const COMMON_COMPONENT&)const override;
  void     set_param_by_index(int, std::string&, int)override;
  int      set_param_by_name(std::string, std::string)override;
  bool     is_valid()const;
  bool     param_is_printable(int)const override;
  std::string param_name(int)const override;
  std::string param_name(int,int)const override;
  std::string param_value(int)const override;
  int param_count()const override {
    int sum = 0;
    if(_num_type < 0){
      sum += int(_p_num.size());
    }else{
      sum += _num_type;
    }
    if(_den_type < 0){
      sum += int(_p_den.size());
    }else{
      sum += _den_type;
    }
    trace2("param_count", this, sum);
    return sum + COMMON_COMPONENT::param_count();
  }
  void precalc_first(const CARD_LIST*)override;
  void precalc_last(const CARD_LIST*)override;
  virtual int args(int)const {unreachable(); return 0;} // needed?
  int num_size() const{return (_num_type>0)? _num_type:int(_p_num.size());}
  int den_size() const{return (_den_type>0)? _den_type:int(_p_num.size());}
protected:
  void reduce_shift();
private:
  void precalc(const CARD_LIST*);
  template<class IN, class T>
  void convert(IN& data, T& type, std::string const& what)const;
};
/*--------------------------------------------------------------------------*/
COMMON_RF_BASE::COMMON_RF_BASE(int c)
  :COMMON_COMPONENT(c),
   _p_num(/*default*/),
   _p_den(/*default*/)
{
}
/*--------------------------------------------------------------------------*/
COMMON_RF_BASE::COMMON_RF_BASE(const COMMON_RF_BASE& p)
  :COMMON_COMPONENT(p),
   _p_num(p._p_num),
   _p_den(p._p_den),
   _num_type(p._num_type),
   _den_type(p._den_type)
{
}
/*--------------------------------------------------------------------------*/
bool COMMON_RF_BASE::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_RF_BASE* p = dynamic_cast<const COMMON_RF_BASE*>(&x);
  return (p
    && _p_num == p->_p_num
    && _p_den == p->_p_den
    && _num_type == p->_num_type
    && _den_type == p->_den_type
    && COMMON_COMPONENT::operator==(x));
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::set_param_by_index(int I, std::string& Value, int Offset)
{ untested();
	incomplete();
  switch (COMMON_RF_BASE::param_count() - 1 - I) {
  default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset);
  }
}
/*--------------------------------------------------------------------------*/
int COMMON_RF_BASE::set_param_by_name(std::string Name, std::string Value)
{
  if(Name == "$mfactor"){ untested();
    incomplete();
    Name = "m";
  }else{
  }
  trace2("e_rf spbn", Name, Value);
  CS cmd(CS::_STRING, Name);
  int idx;
  if(Name.size()<2){ untested();
  }else if(cmd.skip1('d')) {
    cmd >> idx;
    if(idx < 0){ untested();
      throw Exception_No_Match(Name);
    }else if(cmd.more()){ untested();
      throw Exception_No_Match(Name);
    }else if(_den_type < 0){ untested();
      throw Exception_No_Match(Name);
    }else if(size_t(idx) < _p_den.size()){
    }else{
      _p_den.resize(idx + 1);
    }
    _den_type = idx+1;
    _p_den[idx] = Value;
  }else if(cmd.skip1('n')) {
    cmd >> idx;

    if(idx < 0){ untested();
      throw Exception_No_Match(Name);
    }else if(cmd.more()){ untested();
      throw Exception_No_Match(Name);
    }else if(_num_type < 0){ untested();
      throw Exception_No_Match(Name);
    }else{
    }

    if(size_t(idx) < _p_num.size()){
    }else{
      _p_num.resize(idx + 1);
    }
    _num_type = idx+1;
    _p_num[idx] = Value;
  }else if(Name.size()>2){
    if(Name[0] == 'r' && (Name[1] == 'r' || Name[1] == 'i')){
      idx = atoi(Name.substr(2).c_str()); // error handling?
      if(_den_type > 0 && !_p_den[0].is_constant()){ untested();
	throw Exception_No_Match(Name);
      }else if(size_t(2*idx + 1) < _p_den.size()){
      }else{
	_p_den.resize(2*(idx + 1));
      }
      _den_type = -1;
      _p_den[2*idx + (Name[1] == 'i')] = Value;
    }else if(Name[0] == 'x' && (Name[1] == 'r' || Name[1] == 'i')){
      idx = atoi(Name.substr(2).c_str()); // error handling?
      if(_num_type > 0 && !_p_num[0].is_constant()){
	throw Exception_No_Match(Name);
      }else if(size_t(2*idx + 1) < _p_num.size()){
      }else{
	_p_num.resize(2*(idx + 1));
      }
      _num_type = -1;
      _p_num[2*idx + (Name[1] == 'i')] = Value;
    }else{
    }
  }else{
    throw Exception_No_Match(Name);
  }

  return 0; // incomplete();
}
/*--------------------------------------------------------------------------*/
bool COMMON_RF_BASE::param_is_printable(int i)const
{
  int idx = COMMON_RF_BASE::param_count() - 1 - i;

  int nn = _num_type;
  if(nn < 0) {
    nn = int(_p_num.size());
  }else{
  }
//  return true;

  if(idx < 0){
  }else if(idx < _num_type){
    return _p_num[idx].has_hard_value();
  }else if(_num_type<0 && idx<int(_p_num.size())){
    return _p_num[idx].has_hard_value();
  }else if(idx - nn < _den_type){
    return _p_den[idx-nn].has_hard_value();
  }else if(_den_type<0 && idx-nn<int(_p_den.size())){
    return _p_num[idx-nn].has_hard_value();
  }else{
  }
  return COMMON_COMPONENT::param_is_printable(i);
}
/*--------------------------------------------------------------------------*/
std::string COMMON_RF_BASE::param_name(int i)const
{

  int idx = COMMON_RF_BASE::param_count() - 1 - i;
    // return "n" + to_string(int(idx));
  if(idx < 0){ untested();
    return COMMON_COMPONENT::param_name(i);
  }else if(idx < _num_type){
    return "n" + to_string(int(idx));
  }else if(_num_type<0 && idx<int(_p_num.size())){
    return std::string("x") + ((idx%2)?'i':'r') + to_string(idx/2);
  }else{
    int nn = _num_type;
    if(nn < 0) {
      nn = int(_p_num.size());
    }else{
    }


    int a = idx - nn;

    if(a < _den_type){
      return "d" + to_string(a);
    }else if(_den_type<0 && a<int(_p_den.size())){
      return std::string("r") + ((a%2)?'i':'r') + to_string(a/2);
    }else{ untested();
    }
  }
  return COMMON_COMPONENT::param_name(i);
}
/*--------------------------------------------------------------------------*/
std::string COMMON_RF_BASE::param_name(int i, int j)const
{ untested();
  if(j==0){ untested();
    return param_name(i);
  }else{ untested();
    return "";
  }
}
/*--------------------------------------------------------------------------*/
std::string COMMON_RF_BASE::param_value(int i)const
{
  int idx = COMMON_RF_BASE::param_count() - 1 - i;
  int nn = _num_type;
  if(nn < 0) {
    nn = int(_p_num.size());
  }else{
  }
  if (idx < 0) { untested();
  }else if (idx < _num_type) {
    return _p_num[idx].string();
  }else if (_num_type<0 && idx < int(_p_num.size())) {
    return _p_num[idx].string();
  }else if (idx -nn < _den_type) {
    return _p_den[idx - nn].string();
  }else if (_den_type<0 && idx - nn < int(_p_den.size())) {
    return _p_den[idx-nn].string();
  }else{ untested();
  }
  return COMMON_COMPONENT::param_value(i);
}
/*--------------------------------------------------------------------------*/
bool COMMON_RF_BASE::is_valid() const
{ untested();
  return true; //COMMON_COMPONENT::is_valid();
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::precalc_first(const CARD_LIST* par_scope)
{
  assert(par_scope);
  COMMON_COMPONENT::precalc_first(par_scope);
  precalc(par_scope);
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::precalc(const CARD_LIST* par_scope)
{
  for(auto &pp : _p_num) {
    e_val(&(pp),     0. , par_scope);
  }
  for(auto &pp : _p_den) {
    e_val(&(pp),     0. , par_scope);
  }
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::reduce_shift()
{
  _p_num.resize(_num_type);
  _p_den.resize(_den_type);

  auto &a = _p_den;
  auto &b = _p_num;

  while(a.size() && b.size()){
    if(!a[0] && !b[0]){
      a.erase(a.begin());
      b.erase(b.begin());
    }else{
      break;
    }
  }

  _num_type = int(_p_num.size());
  _den_type = int(_p_den.size());
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::precalc_last(const CARD_LIST* par_scope)
{
  assert(par_scope);
  COMMON_COMPONENT::precalc_last(par_scope);
  precalc(par_scope);

  convert(_p_num, _num_type, "numerator");
  convert(_p_den, _den_type, "denominator");

  if(_num_type && _den_type){
  }else{ untested();
    incomplete();
  }

  if(_num_type && _den_type){
  }else{ untested();
    assert(0);
    incomplete();
  }

}
/*--------------------------------------------------------------------------*/
template<class IN, class T>
void COMMON_RF_BASE::convert(IN& data, T& type, std::string const& what) const
{
  trace3("convert", data.size(), type, what);
  // assert(type);
  if(!type || !data.size()){
    error(bPICKY, "no data, assuming no root, constant 1.\n");
    incomplete(); // fall through
    data.resize(1);
//    data.push_back(PARAMETER<double>());
    data.back() = 1.;
    type = 1;
  }else if(type < 0) {
    assert(!(data.size()%2));
    poly p;
    for(auto i = data.begin(); i!= data.end(); ++i) {
      trace1("convert data r", *i);
      double real = *i;
      i++;
      trace1("convert data i", *i);
      assert(i!=data.end());
      p.add_root(COMPLEX(real, *i));
    }
    p.normalise();
    type = 0;
    for(auto j : p){
      if(fabs(std::imag(j))>1e-16){ untested();
	error(bDANGER, name() + ": unreal coefficient in "+what+"\n");
      }else{
      }
      data[type] = std::real(j);
      ++type;
    }
  }else{
  }
  trace3("convert out", data.size(), type, what);

  assert( data.size());
  for(auto i = data.begin(); i!= data.end(); ++i) {
    trace1("convert out", double(*i));
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
