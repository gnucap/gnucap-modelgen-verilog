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
 * residue/pole representation is nonstandard and incomplete.
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
namespace { // local
/*--------------------------------------------------------------------------*/
class poly {
  typedef std::vector<COMPLEX> c_t;
  typedef c_t::const_iterator const_iterator;
  c_t _c;
public:
  explicit poly(poly const& p) : _c(p._c) {}
  explicit poly(int degree) : _c(degree+1, 0.) { }
  explicit poly(COMPLEX c0=1.) : _c(1) { _c[0] = c0; }

  int size()const { return int(_c.size()); }

  // multiply by x-r
  void add_root(COMPLEX const& r){
    assert(r==r);
    assert(_c.back() == 1.);
    _c.push_back(1.);
    for(size_t i = _c.size()-1; i>1; ){
      --i;
      _c[i] *= -r;
      _c[i] += _c[i-1];
    }
    _c[0] *= -r;
  }
  void add_root(double const& r, double const& i){
    add_root(COMPLEX(r,i));
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
  }
  COMPLEX& operator[](size_t i){ assert(i<_c.size()); return _c[i]; }
  COMPLEX const& operator[](size_t i) const{ assert(i<_c.size()); return _c[i]; }
  poly operator*(poly const& other);
  poly& operator+=(poly const& other);
  poly& operator=(poly const& other);
  poly operator*(COMPLEX const& other);
  const_iterator begin()const {return _c.begin();}
  const_iterator end()const {return _c.end();}
};
/*--------------------------------------------------------------------------*/
poly& poly::operator=(poly const& o)
{ untested();
//  assert(o.size() == size());
  _c.resize(o.size());

  for(int i=0; i<size(); ++i){ untested();
    (*this)[i] = o[i];
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
poly& poly::operator+=(poly const& o)
{
  assert(o.size() == size());

  for(int i=0; i<size(); ++i){
    _c[i] += o[i];
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
poly poly::operator*(COMPLEX const& o)
{
  poly r(*this);

  for(int i=0; i<size(); ++i){
    r[i] *= o;
  }
  return poly(r);
}
/*--------------------------------------------------------------------------*/
poly poly::operator*(poly const& other)
{
  poly r(size() + other.size() - 2);

  for(int i=0; i<size(); ++i) {
    for(int j=0; j<other.size(); ++j) {
      r[i+j] += _c[i] * other[j];
    }
  }
  return poly(r);
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
class COMMON_RF_BASE :public COMMON_COMPONENT{
  typedef enum { rf_unknown = 0,
		 rf_nx = 1,
		 rf_zx = 2,
		 rf_xd =     4,
		 rf_xp =     8,
                 rf_nd = 1 | 4,
		 rf_np = 1 | 8,
		 rf_zd = 2 | 4,
		 rf_zp = 2 | 8,
		 rf_rp =     8 | 16
               }  rf_type;
public:
  std::vector<PARAMETER<double> > _p_num;
  std::vector<PARAMETER<double> > _p_den;
  rf_type _type{rf_unknown};
private:
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
    int sum = int(_p_num.size()) + int(_p_den.size());
    return sum + COMMON_COMPONENT::param_count();
  }
  void precalc_first(const CARD_LIST*)override;
  void precalc_last(const CARD_LIST*)override;
  virtual int args(int)const {unreachable(); return 0;} // needed?
  void set_rp() { _type = rf_rp; }
  void set_zx() { _type = rf_type(_type | rf_zx); }
  void set_nx() { _type = rf_type(_type | rf_nx); }
  void set_xp() { _type = rf_type(_type | rf_xp); }
  void set_xd() { _type = rf_type(_type | rf_xd); }
  bool is_rp()const { return _type == rf_rp; }
  bool num_is_n()const { return _type & rf_nx; }
  bool num_is_z()const { return _type & rf_zx; }
  bool den_is_d()const { return _type & rf_xd; }
  bool den_is_p()const { return _type & rf_xp; }
  int num_size()const {
    if(is_rp()){
      return int(_p_num.size())/2;
    }else if(num_is_z()){
      return int(_p_num.size()/2 + 1);
    }else{
      return int(_p_num.size());
    }
  }
  int den_size()const {
    if(is_rp()){
      return int(_p_den.size())/2 + 1;
    }else if(den_is_p()){
      return int(_p_den.size()/2 + 1);
    }else{
      return int(_p_den.size());
    }
  }
protected:
  void reduce_shift();
  void convert_nd();
private:
  void precalc(const CARD_LIST*);
  template<class IN>
  void convert(IN& data, std::string const& what)const;
  template<class IN>
  void invres(IN& data, IN& type)const;
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
   _type(p._type)
{
  trace1("COMMON_RF_BASE", _type);
}
/*--------------------------------------------------------------------------*/
bool COMMON_RF_BASE::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_RF_BASE* p = dynamic_cast<const COMMON_RF_BASE*>(&x);
  return (p
    && _p_num == p->_p_num
    && _p_den == p->_p_den
    && _type == p->_type
    && COMMON_COMPONENT::operator==(x));
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::set_param_by_index(int I, std::string& Value, int Offset)
{
  if(I==-1){
    // reset;
    _type = rf_unknown;
  }else{
    incomplete();
    switch (COMMON_RF_BASE::param_count() - 1 - I) {
    default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset);
    }
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
  CS cmd(CS::_STRING, Name);
  int idx;
  if(Name.size()<2){ untested();
  }else if(cmd.skip1('d')) {
    cmd >> idx;
    if(idx < 0){ untested();
      throw Exception_No_Match(Name);
    }else if(cmd.more()){ untested();
      throw Exception_No_Match(Name);
    }else if(is_rp() || den_is_p()){ untested();
      throw Exception_No_Match(Name);
    }else if(size_t(idx) < _p_den.size()){
    }else{
      _p_den.resize(idx + 1);
    }
    _type = rf_type(_type | rf_xd);
    _p_den[idx] = Value;
  }else if(cmd.skip1('n')) {
    cmd >> idx;

    if(idx < 0){ untested();
      throw Exception_No_Match(Name);
    }else if(cmd.more()){ untested();
      throw Exception_No_Match(Name);
    }else if(is_rp() || num_is_z()){ untested();
      throw Exception_No_Match(Name);
    }else{
    }

    if(size_t(idx) < _p_num.size()){
    }else{
      _p_num.resize(idx + 1);
    }
    //_type |= rf_nx; //??
    _type = rf_type(_type | rf_nx);
    _p_num[idx] = Value;
  }else if(Name.size()>2){
    if(Name[0] == 'p' && (Name[1] == 'r' || Name[1] == 'i')){
      idx = atoi(Name.substr(2).c_str()); // error handling?
      if(den_is_d()) { untested();
	throw Exception_No_Match(Name);
      }else if(size_t(2*idx + 1) < _p_den.size()){
      }else if(is_rp()) { untested();
	size_t m = std::max(_p_den.size(), _p_num.size());
	m = std::max(m, size_t(2*(idx + 1)));
	_p_den.resize(m);
	_p_num.resize(m);
      }else{
	_p_den.resize(2*(idx + 1));
      }
      if(is_rp()){
      }else{
	set_xp();
      }
      _p_den[2*idx + (Name[1] == 'i')] = Value;
      assert(den_is_p());
    }else if(Name[0] == 'z' && (Name[1] == 'r' || Name[1] == 'i')){
      idx = atoi(Name.substr(2).c_str()); // error handling?
      if(num_is_n()) {
	throw Exception_No_Match(Name);
      }else if(size_t(2*idx + 1) < _p_num.size()){
      }else{
	_p_num.resize(2*(idx + 1));
      }
      _type = rf_type(_type | rf_zx);
      _p_num[2*idx + (Name[1] == 'i')] = Value;
      assert(!is_rp());
      assert(num_is_z());
    }else if(Name[0] == 'r' && (Name[1] == 'r' || Name[1] == 'i')){
      set_rp();
      idx = atoi(Name.substr(2).c_str()); // error handling?
      if(num_is_z() || num_is_n()){ untested();
	throw Exception_No_Match(Name);
//      }else if(_num_type > 0 && !_p_num[0].is_constant()){ untested();
//	throw Exception_No_Match(Name);
//      }else if(size_t(2*idx + 1) < _p_num.size()){ untested();
      }else{
	size_t m = std::max(_p_den.size(), _p_num.size());
	m = std::max(m, size_t(2*(idx + 1)));
	_p_den.resize(m);
	_p_num.resize(m);
      }
      assert(is_rp());
      _p_num[2*idx + (Name[1] == 'i')] = Value;

    }else{
    }
    if(is_rp()){
      size_t m = std::max(_p_den.size(), _p_num.size());
      _p_num.resize(m);
      _p_den.resize(m);
    }else{
    }
  }else{
    throw Exception_No_Match(Name);
  }

  if(num_is_z()){
     assert(!(_p_num.size()%2));
  }else{
  }
  if(den_is_p()){
     assert(!(_p_den.size()%2));
  }else{
  }


  return 0; // incomplete();
}
/*--------------------------------------------------------------------------*/
bool COMMON_RF_BASE::param_is_printable(int i)const
{
  int idx = COMMON_RF_BASE::param_count() - 1 - i;

  int nn = int(_p_num.size());
  int pp = int(_p_den.size());

  if(idx < 0){
  }else if(size_t(idx) < _p_num.size() && _p_num[idx].is_constant()){
    return true;
  }else if(is_rp() && size_t(idx) < _p_num.size()){
    return _p_num[idx].has_hard_value();
  }else if(size_t(idx-nn) < _p_den.size() && _p_den[idx-nn].is_constant()){
    return true;
  }else if(is_rp() && size_t(idx) < _p_num.size() + _p_den.size()){
    return _p_den[idx-nn].has_hard_value();
  }else if(num_is_n() && idx < nn){
    return _p_num[idx].has_hard_value();
  }else if(idx < nn) {
    return _p_num[idx].has_hard_value();
  }else if(idx - nn < pp){
    return _p_den[idx-nn].has_hard_value();
  }else{
    trace3("is_printable", _p_num.size(), _p_den.size(), is_rp());
  }
  return COMMON_COMPONENT::param_is_printable(i);
}
/*--------------------------------------------------------------------------*/
std::string COMMON_RF_BASE::param_name(int i)const
{

  int idx = COMMON_RF_BASE::param_count() - 1 - i;
  trace4("param_name", idx, is_rp(), _p_num.size(), _p_den.size());
    // return "n" + to_string(int(idx));
  if(idx < 0){ untested();
    return COMMON_COMPONENT::param_name(i);
  }else if(idx < int(_p_num.size())){
    if(num_is_n()) {
      return "n" + to_string(idx);
    }else if(is_rp()) {
      return std::string("r") + ((idx%2)?'i':'r') + to_string(idx/2);
    }else if(num_is_z()) {
      return std::string("z") + ((idx%2)?'i':'r') + to_string(idx/2);
    }else{ untested();
      unreachable();
    }
  }else if(idx <  int(_p_num.size()+_p_den.size())){
    int a = idx - int(_p_num.size());
    if(is_rp()){
      return std::string("p") + ((a%2)?'i':'r') + to_string(a/2);
    }else if(den_is_d()){
      return "d" + to_string(a);
    }else if(den_is_p()){
      return std::string("p") + ((a%2)?'i':'r') + to_string(a/2);
    }else{ untested();
      unreachable();
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
  if (idx < 0) { untested();
  }else if (idx < int(_p_num.size())) {
    return _p_num[idx].string();
  }else if(idx <  int(_p_num.size()+_p_den.size())){
    int a = idx - int(_p_num.size());
    return _p_den[a].string();
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
  assert(num_is_n());
  assert(den_is_d());
//  _p_num.resize(_num_type);
//  _p_den.resize(_den_type);
  incomplete();

  auto &a = _p_den;
  auto &b = _p_num;
  trace2("reduce_shift", a.size(), b.size());

  while(a.size() && b.size()){
    if(!a[0] && !b[0]){
      a.erase(a.begin());
      b.erase(b.begin());
    }else{
      break;
    }
  }
  trace2("reduce_shift done", a.size(), b.size());
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::precalc_last(const CARD_LIST* par_scope)
{
  assert(par_scope);
  COMMON_COMPONENT::precalc_last(par_scope);
  precalc(par_scope);
}
/*--------------------------------------------------------------------------*/
void COMMON_RF_BASE::convert_nd()
{
  if(!_type){
    assert(1==_p_num.size());
    assert(1==_p_den.size());
    _p_num[0] = 1.;
    _p_den[0] = 1.;
    _type = rf_nd;
  }else if(is_rp()) {
    invres(_p_num, _p_den);
    _type = rf_nd;
    assert(!is_rp());
  }else if ( !num_is_n() || !den_is_p() ){
    trace3("convert", _type, _p_num.size(), _p_den.size());
    if(!num_is_n()){
      // assert(num_is_z());
      convert(_p_num, "numerator");
      assert(_p_num.size());
    }else{
    }
    if(!den_is_d()){
      // assert(den_is_p());
      convert(_p_den, "denominator");
    }else{
    }
    _type = rf_nd;
    assert(num_is_n());
    assert(den_is_d());
  }else{ untested();
    incomplete();
    assert(0);
  }

  assert(_p_num.size());
  assert(_p_den.size());
}
/*--------------------------------------------------------------------------*/
template<class IN>
void COMMON_RF_BASE::convert(IN& data, std::string const& what) const
{
  trace2("convert", data.size(), what);
  if(!data.size()){ untested();
    error(bPICKY, "no data, assuming no root, constant 1.\n");
    incomplete(); // fall through
    data.resize(1);
//    data.push_back(PARAMETER<double>());
    data.back() = 1.;
  }else{
    assert(!(data.size()%2));
    poly p;
    for(auto i = data.begin(); i!= data.end(); ++i) {
      trace1("convert data r", *i);
      double real = *i;
      i++;
      trace1("convert data i", *i);
      assert(i!=data.end());
      p.add_root(COMPLEX(real, *i));
      trace1("convert", p.size());
    }
    p.normalise();
    data.resize(p.size());
    int type=0;
    for(auto j : p){
      if(fabs(std::imag(j))>1e-16){ untested();
	error(bDANGER, name() + ": unreal coefficient in "+what+"\n");
      }else{
      }
      data[type] = std::real(j);
      ++type;
    }
  }

  assert( data.size());
  for(auto i = data.begin(); i!= data.end(); ++i) {
    trace1("convert out", double(*i));
  }
}
/*--------------------------------------------------------------------------*/
template<class DATA>
void COMMON_RF_BASE::invres(DATA& num, DATA& denom) const
{
  assert(num.size()==denom.size());
  std::vector<poly> pp(denom.size()/2-1);
  poly new_num(int(num.size())/2 - 1);

  if(pp.size()){
    pp[0].add_root(denom[0], denom[1]);
  }else{
  }
  for(int i=1; i<int(pp.size()); ++i) {untested();
    pp[i] = pp[i-1];
    assert(2*i+1<int(denom.size()));
    pp[i].add_root(denom[2*i], denom[2*i+1]);
  }

  poly rr;
  for(int i=int(pp.size()); i>0; --i ){
    COMPLEX residue(num[2*i], num[2*i+1]);
    new_num += rr * pp[i-1] * residue;
    assert(2*i+1<int(denom.size()));
    rr.add_root(denom[2*i], denom[2*i+1]);
  }
  new_num += rr * COMPLEX(num[0], num[1]);
  rr.add_root(denom[0], denom[1]);

  num.resize(denom.size()/2);
  denom.resize(denom.size()/2+1);
  assert(rr.size()==int(denom.size()));
  for(int i=0; i<int(denom.size()); ++i){
    denom[i] = std::real(rr[i]);
    if(std::abs(std::imag(rr[i]))>1e-10){ untested();
      incomplete();
      error(bWARNING, "unreal denominator\n");
    }else{
    }
  }
  for(int i=0; i<int(num.size()); ++i){
    num[i] = std::real(new_num[i]);
    if(std::abs(std::imag(new_num[i]))>1e-10){ untested();
      incomplete();
      error(bWARNING, "unreal numerator\n");
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
