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

#ifndef GNUCAP_VA_H
#define GNUCAP_VA_H

#include <md.h>
#include <u_parameter.h>
#include <l_stlextra.h>
#include <e_storag.h>
#include <e_base.h> // CKT_BASE
#include <u_sim_data.h> // see simparam
/*--------------------------------------------------------------------------*/
inline int simulatorVersion()
{
  std::string lv = lib_version();
  int l = int(lv.size());
  if(l < 10){ untested();
    return 0;
  }else{ itested();
    return atoi(lv.substr(l-2,l).c_str())
         + atoi(lv.substr(l-5,l-3).c_str()) * 100
         + atoi(lv.substr(l-10,l-6).c_str()) * 10000;
  }
}
/*--------------------------------------------------------------------------*/
namespace notstd {
/*--------------------------------------------------------------------------*/
template <class InputIter, class Size, class OutputIter>
void add_n(InputIter first, Size count, OutputIter result)
{ untested();
  for ( ; count > 0; --count) { untested();
    *result++ += *first++;
  }
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
struct Exception_OutOfRange_ :public Exception{
  std::string _key, _range;
  Exception_OutOfRange_(std::string const& key, std::string const& range)
    :Exception(key + " is not " + std::string(1,'"') + range + std::string(1,'"')),
     _key(key), _range(range) {
  }
};
/*--------------------------------------------------------------------------*/

namespace{

typedef double real;
typedef int integer;
typedef vString string;
/*--------------------------------------------------------------------------*/
template<class T>
double plain_value(T const& x) { return x; }
inline int plain_value(int const& x) { return x; }
inline char const* plain_value(std::string const& x) { return x.c_str(); }
/*--------------------------------------------------------------------------*/
// double inf = std::numeric_limits<double>::infinity();
#define inf std::numeric_limits<double>::infinity()
#define Inf inf

template<int numderiv>
class ddouble_ {
  typedef PARAMETER<double> Pdbl;
  typedef PARAMETER<int> Pint;
protected:
  double _data[1+numderiv];
public:
  ddouble_(ddouble_ const& o) {
    notstd::copy_n(o._data, numderiv+1, _data);
  }
  explicit ddouble_() { clear(); }
  // fixme: implicit conversion in af args..
  /*explicit*/ ddouble_(int const& d) { clear(); *_data = d; }
  /*explicit*/ ddouble_(double const& d) { clear(); *_data = d; }
  /*explicit*/ ddouble_(Pdbl const& d) { clear(); *_data = d; }
  /*explicit*/ ddouble_(Pint const& d) { untested(); clear(); *_data = d; }
  void set_all_deps() {
    std::fill_n(_data+1, numderiv, 0.);
  }
  void set_no_deps() {
    std::fill_n(_data+1, numderiv, std::numeric_limits<double>::quiet_NaN());
  }
private:
  void reset_deriv(){
    set_all_deps();
  }
public:
  virtual double const& value()const {return _data[0];}
  double d(int i)const { untested();return _data[i];}
  double const& operator[](int i)const { untested();return _data[i+1];}
  double& operator[](int i){return _data[i+1];}
  operator double()const { return *_data; }
#if 1
  ddouble_& operator=(ddouble_ const& o) {
    assert(o == o);
    notstd::copy_n(o._data, numderiv+1, _data);
    return *this;
  }
#else
  ddouble_& operator=(ddouble_ o) { untested();
    assert(o == o);
    notstd::copy_n(o._data, numderiv+1, _data);
    return *this;
  }
#endif
  ddouble_& operator=(Pdbl const& o) {
    assert(o == o);
    *_data = o;
    reset_deriv();
    return *this;
  }
  ddouble_& operator=(Pint const& o) {
    assert(o == o);
    *_data = o;
    reset_deriv();
    return *this;
  }
  ddouble_& operator=(const double& o) {
    assert(o == o);
    *_data = o;
    reset_deriv();
    return *this;
  }
  bool is_same(const ddouble_& o)const { untested();
    for(int i=0; i<=numderiv; ++i){ untested();
      if(_data[i] != _data[i]){ itested();
	if( o._data[i] == o._data[i] ){ untested();
	  return false;
	}else{ untested();
	}
      }else if(_data[i] != o._data[i]){ untested();
	return false;
      }else{ untested();
      }
    }
    return true;
  }
  bool operator==(const ddouble_& o)const {
#if 0 // really?
    for(int i=0; i<=numderiv; ++i){ untested();
      if(_data[i] != o._data[i]){ untested();
	return false;
      }else{ untested();
      }
    }
#else
    return *_data == *o._data;
#endif
    return true;
  }
  bool operator==(const double& o)const {
    assert(_data);
    return *_data == o;
  }
  bool operator==(const int& o)const {
    double x = o;
    return *_data == x;
  }

  bool operator<(const ddouble_& o)const {
    return *_data < *o._data;
  }
  bool operator<(double const& o)const {
    return *_data < o;
  }
  bool operator<(int const& o)const {
    return *_data < o;
  }
  bool operator<(PARAMETER<double> const& o)const { untested();
    return *_data < double(o);
  }

  bool operator<=(const ddouble_& o)const { itested();
    return *_data <= *o._data;
  }
  bool operator<=(double const& o)const {
    return *_data <= o;
  }
  bool operator<=(int const& o)const { itested();
    return *_data <= o;
  }
  bool operator<=(PARAMETER<double> const& o)const { itested();
    return *_data <= double(o);
  }

  bool operator>(ddouble_ const& o)const {
    return *_data > *o._data;
  }
  bool operator>(double const& o)const {
    return *_data > o;
  }
  bool operator>(int const& o)const {
    return *_data > o;
  }
  bool operator>(PARAMETER<double> const& o)const {
    return *_data > double(o);
  }

  bool operator>=(const ddouble_& o)const { itested();
    return *_data >= *o._data;
  }
  bool operator>=(const int& o)const { itested();
    return *_data >= o;
  }
  bool operator>=(const double& o)const {
    return *_data >= o;
  }
  bool operator>=(PARAMETER<double> const& o)const {itested();
    return *_data >= double(o);
  }

  ddouble_& operator+=(const ddouble_& o) {
    for(int i=0; i<=numderiv; ++i){
      _data[i] += o._data[i];
    }
    return *this;
  }
  ddouble_ operator+(ddouble_ const& o) const {
    ddouble_ ret = *this;
    ret += o;
    return ret;
  }
  ddouble_ operator+(double const& o) const {
    ddouble_ ret = *this;
    *ret._data += o;
    return ret;
  }
  ddouble_ operator+(int const& o) const {
    ddouble_ ret = *this;
    *ret._data += o;
    return ret;
  }
  ddouble_ operator+(PARAMETER<double> const& o) const {
    ddouble_ ret = *this;
    *ret._data += o;
    return ret;
  }
  ddouble_ operator+()const {
    return ddouble_(*this);
  }

  ddouble_& operator-=(const ddouble_& o) {
    for(int i=0; i<=numderiv; ++i){
      _data[i] -= o._data[i];
    }
    return *this;
  }
  ddouble_ operator-(ddouble_ const& o) const {
    ddouble_ ret = *this;
    ret -= o;
    return ret;
  }
  ddouble_ operator-(double const& o) const {
    ddouble_ ret = *this;
    ret -= ddouble_(o);
    return ret;
  }
  ddouble_ operator-(int const& o) const {
    ddouble_ ret = *this;
    ret -= o;
    return ret;
  }
  ddouble_ operator-(PARAMETER<double> const& o) const {
    ddouble_ ret = *this;
    ret -= double(o);
    return ret;
  }
  ddouble_ operator-() const {
    ddouble_ ret;
    for(int i=0; i<=numderiv; ++i){
      ret._data[i] = - _data[i];
    }
    return ret;
  }

  ddouble_& operator*=(const ddouble_& o) {
    assert(*o._data == *o._data);
    assert(*_data == *_data);
    for(int i=1; i<=numderiv; ++i){
      // assert(_data[i] == _data[i]);
      // assert(o._data[i] == o._data[i]);
      _data[i] *= *o._data;
      _data[i] += *_data * o._data[i];
    }
    *_data *= *o._data;
    return *this;
  }
  ddouble_ operator*(const ddouble_& o) const {
    ddouble_ ret = *this;
    ret *= o;
    return ret;
  }
  ddouble_& operator*=(const double& o) {
    for(int i=0; i<=numderiv; ++i){
      _data[i] *= o;
    }
    return *this;
  }
  ddouble_ operator*(const int& o) const { itested();
    ddouble_ ret = *this;
    ret *= double(o);
    return ret;
  }
  ddouble_ operator*(const double& o)const {
    ddouble_ ret = *this;
    ret *= o;
    return ret;
  }
  ddouble_ operator*(PARAMETER<double> const& o) const {
    ddouble_ ret = *this;
    ret *= (double)o;
    return ret;
  }
  ddouble_ operator*(Pint const& o) const { untested();
    ddouble_ ret = *this;
    ret *= (double)o;
    return ret;
  }


  ddouble_& operator/=(ddouble_ const& o) {
    if(*o._data){
      for(int i=1; i<=numderiv; ++i){
	_data[i] *= o.value();
	_data[i] -= value() * o._data[i];
	_data[i] /= o.value() * o.value();
      }
      *_data /= *o._data;
    }else{ itested();
      *this /= (o + 1e-20);
    }
    return *this;
  }
  ddouble_ operator/(ddouble_ o) const {
    ddouble_ ret = *this;
    ret /= o;
    return ret;
  }
  ddouble_& operator/=(double const& o) {
    assert(_data);
    assert(o);
    for(int i=0; i<=numderiv; ++i){
      _data[i] /= o;
    }
    return *this;
  }
  //	ddouble_ operator/(double const& o) { untested();
  //		assert(o);
  //		ddouble_ ret = *this;
  //		ret /= o;
  //		return ret;
  //	}
  ddouble_ operator/(double const& o) const {
    assert(o);
    ddouble_ ret = *this;
    ret /= o;
    return ret;
  }
  ddouble_ operator/(int const& o) const {
    assert(o);
    ddouble_ ret = *this;
    ret /= o;
    return ret;
  }
  ddouble_ operator/(PARAMETER<double> const& o) const {
    assert(o);
    ddouble_ ret = *this;
    ret /= double(o);
    return ret;
  }
  ddouble_ operator/(PARAMETER<int> const& o) const {
    assert(o);
    ddouble_ ret = *this;
    ret /= int(o);
    return ret;
  }
  bool operator!() const { itested();
    assert(_data);
    return *_data == 0.;
  }

  ddouble_& chain(double const& d) {
    for(int i=1; i<=numderiv; ++i){
      _data[i] *= d;
    }
    return *this;
  } /*chain*/
  ddouble_& set_value(double const& v){
    *_data = v;
    return *this;
  }
  void clear() {
    std::fill_n(_data, numderiv+1, 0.);
  } /*clear*/
}; /*ddouble_*/
/*--------------------------------------------------------------------------*/
template<int T>
ddouble_<T> operator/(ddouble_<T> a, ddouble_<T> b)
{ untested();
  a /= b;
  return a;
}
template<int T>
ddouble_<T> operator/(int const& a, ddouble_<T> b)
{ untested();
  ddouble_<T> c(a);
  c /= b;
  return c;
}
template<int T>
ddouble_<T> operator/(double const& a, ddouble_<T> b)
{
  ddouble_<T> c(a);
  c /= b;
  return c;
}
template<int T>
ddouble_<T> operator/(PARAMETER<double> const& a, ddouble_<T> const& b)
{ untested();
  ddouble_<T> c(a);
  c /= b;
  return c;
}
template<int T>
ddouble_<T> operator/(PARAMETER<int> const& a, ddouble_<T> const& b)
{ itested();
  ddouble_<T> c(a);
  c /= b;
  return c;
}

template<int T>
ddouble_<T> operator-(double const& a, ddouble_<T> b)
{
  ddouble_<T> c(a);
  c -= b;
  return c;
}
template<int T>
ddouble_<T> operator-(int const& a, ddouble_<T> b)
{
  ddouble_<T> c(a);
  c -= b;
  return c;
}
template<int T>
ddouble_<T> operator-(PARAMETER<double> const& a, ddouble_<T> b)
{itested();
  ddouble_<T> c(a);
  c -= b;
  return c;
}
template<int T>
ddouble_<T> operator-(PARAMETER<int> const& a, ddouble_<T> b)
{ untested();
  ddouble_<T> c(a);
  c -= b;
  return c;
}

template<int T>
ddouble_<T> operator*(double const& a, ddouble_<T> const& b)
{
  ddouble_<T> c(b);
  c *= a;
  return c;
}
template<int T>
ddouble_<T> operator*(int const& a, ddouble_<T> const& b)
{ itested();
  ddouble_<T> c(b);
  c *= a;
  return c;
}
template<int T>
ddouble_<T> operator*(PARAMETER<double> const& a, ddouble_<T> const& b)
{
  ddouble_<T> c(a);
  return c * b;
}
template<int T>
ddouble_<T> operator*(PARAMETER<int> const& a, ddouble_<T> const& b)
{
  ddouble_<T> c(b);
  c *= double(a);
  return c;
}

template<int T>
ddouble_<T> operator+(double const& a, ddouble_<T> const& b)
{
  ddouble_<T> c(b);
  return c + a;
}
template<int T>
ddouble_<T> operator+(int const& a, ddouble_<T> const& b)
{ itested();
  ddouble_<T> c(b);
  return c + a;
}
template<int T>
ddouble_<T> operator+(PARAMETER<double> const& a, ddouble_<T> const& b)
{ itested();
  ddouble_<T> c(b);
  return c + a;
}
template<int T>
ddouble_<T> operator+(PARAMETER<int> const& a, ddouble_<T> const& b)
{ untested();
  return b + double(a);
}

template<int T>
bool operator==(ddouble_<T> const& a, PARAMETER<int> const& b)
{ untested();
  return double(b) == a.value();
}
template<int T>
bool operator==(PARAMETER<int> const& b, ddouble_<T> const& a)
{ untested();
  return double(b) == a.value();
}
inline bool operator==(PARAMETER<int> const& b, double const& a)
{ itested();
  return double(b) == a;
}

template<class T>
T& set_value(T& t, double const& d)
{
  t.set_value(d);
  return t;
}

template<>
inline int& set_value(int& t, double const& v)
{
  return t = int(v);
}

template<>
inline double& set_value(double& t, double const& v)
{
  return t = v;
}
  template<>
  inline PARAMETER<int>& set_value(PARAMETER<int>& t, double const& v)
{ untested();
  t = int(v);
  return t;
}
  template<>
  inline PARAMETER<double>& set_value(PARAMETER<double>& t, double const& v)
{
  t = v;
  return t;
}

template<class T>
T& chain(T& t, double const& d)
{
  t.chain(d);
  return t;
}
template<>
inline double& chain(double& t, double const&)
{
  // no-op
  return t;
}
template<>
inline int& chain(int& t, double const&)
{
  // no-op
  return t;
}
template<>
inline PARAMETER<double>& chain(PARAMETER<double>& t, double const&)
{
  // no-op
  return t;
}
template<>
inline PARAMETER<int>& chain(PARAMETER<int>& t, double const&)
{ untested();
  // no-op
  return t;
}
/*--------------------------------------------------------------------------*/
class array_{
  std::vector<double> _v;
public:
  array_() {}
  array_(double a0) {
    _v.push_back(a0);
  }
  array_(double a0, double a1) {
    _v.push_back(a0);
    _v.push_back(a1);
  }
  template<typename... Args>
  array_(Args... args) {
    push_back(args...);
  }

  size_t size() const{return _v.size();}
  double const& operator[](int x)const {return _v[x];}
private:
  template<typename T>
  void push_back(T t) {
    _v.push_back(t);
  }
  template<typename T, typename... Args>
  void push_back(T t, Args... args) {
    _v.push_back(t);
    push_back(args...);
  }
};
/*--------------------------------------------------------------------------*/
template<class A, class T>
class wrap_arg{
   A _a;
   T& _d;
public:
   wrap_arg(T& d) : _d(d) { _a = d; }
   operator A&() { return _a; }
   ~wrap_arg() { _d = std::move(T(_a)); }
};
/*--------------------------------------------------------------------------*/
template<class S, class T>
wrap_arg<S, T> io_arg(S, T& d) { return wrap_arg<S, T>(d); }
/*--------------------------------------------------------------------------*/
class Href_{
  CARD const* _card{nullptr};
  Base const* _ref{nullptr};
  std::string _tail;
  // const std::string _path;
public:
  explicit Href_() {}
 // explicit Href_(std::string const& path) : _path(path) { untested();}
  explicit Href_(Href_ const&) = default;
  void link_card(CARD const* scope, std::string const& path);
  Base const* link_item(){ untested();
    return _ref = find_item(_card?_card->subckt():&CARD_LIST::card_list, _tail);
  }
  double get_double()const {
    Base const* ref = _ref;
    CARD_LIST const* scope = _card?_card->subckt():&CARD_LIST::card_list;
    if(ref){ untested();
    }else{
      // incomplete();
      ref = find_item(scope, _tail);
    }
    if(auto f = dynamic_cast<Float const*>(ref)) {
      trace1("href float", *f);
      return *f;
    }else if(scope){
      trace1("href no float", _tail);
      PARAMETER<double> pp; pp = _tail;
      pp.e_val(NOT_VALID, scope->params());
      return pp;
    }else{ untested();
      return NOT_VALID;
    }
  }
  bool operator==(Href_ const& o)const {
    return _card == o._card && _ref == o._ref && _tail == o._tail;
  }
private:
  void link_device_(CARD const*, std::string const& path);
  bool find_device_up(CARD const*, std::string const&, std::string const& tail);
  bool find_device_down(CARD_LIST const*, std::string const& path);
  bool find_device_down(CARD_LIST const*, std::string const& dev, std::string const& tail);
  Base const* find_item(CARD_LIST const* scope, std::string const& path)const;
};
/*--------------------------------------------------------------------------*/
// found a device. store the remainder of the path.
inline void Href_::link_device_(CARD const* device, std::string const& path)
{ untested();
  if(!device){ untested();
    // top level thing?
    incomplete();
  }else{ untested();
    trace2("found link device", device->long_label(), path);
  }

  _card = device;
  _tail = path;
}
/*--------------------------------------------------------------------------*/
inline Base const* Href_::find_item(CARD_LIST const* scope, std::string const& path) const
{
  // trace2("find_item", path, _card?_card->long_label():"(null)");
  auto dotplace = path.find(".");
  if(dotplace == std::string::npos){
    assert(scope);
    PARAM_LIST const* p = scope->params();
    assert(p);
    PARAM_LIST::const_iterator it = p->find(path);
    if(it == p->end()){
    }else{
      PARAM_INSTANCE const& pi = it.ref();
      return pi.value();
    }
  }else{ untested();
    incomplete();
  }
  return nullptr;
}
/*--------------------------------------------------------------------------*/
inline bool Href_::find_device_up(CARD const* device,
     std::string const& name, std::string const& tail)
{
  assert(device);
  trace3("find_device_up", device->long_label(), name, tail);
  if(*device != name){
    if(find_device_down(device->subckt(), name, tail)){
      return true;
    }else if(CARD const* o = device->owner()){ untested();
      return find_device_up(o, name, tail);
    }else if(find_device_down(device->scope(), name, tail)){
      return true;
    }else{ untested();
      _card = device;
      _tail = tail;
      return true;
    }
  }else{ untested();
    return find_device_down(device->subckt(), tail);
  }
}
/*--------------------------------------------------------------------------*/
inline bool Href_::find_device_down(CARD_LIST const* scope, std::string const& dev,
    std::string const& tail)
{
  assert(scope);
  if(scope == &CARD_LIST::card_list){
  }else{
  }
  auto it = scope->find_(dev);
  if(it == scope->end()){
    trace1("href::find_dev_down, miss", dev);
    return false;
  }else if(find_device_down((*it)->subckt(), tail)) {
    return true;
  }else{
    _card = *it;
    _tail = tail;
    return true;
  }
}
inline bool Href_::find_device_down(CARD_LIST const* scope, std::string const& path)
{
  trace2("href::find_dev_down", scope, path);
  auto dotplace = path.find(".");
  if(!scope){ untested();
    return false;
  }else if(dotplace == std::string::npos) {
    return false;
  }else{
    std::string tail = path.substr(dotplace+1, std::string::npos);
    std::string dev = path.substr(0, dotplace);
    return find_device_down(scope, dev, tail);
  }
}
/*--------------------------------------------------------------------------*/
inline void Href_::link_card(CARD const* dev, std::string const& path)
{
  assert(dev);
  trace2("href::link_card", path, dev->long_label());
  auto dotplace = path.find(".");
  std::string name = path.substr(0, dotplace);
  std::string tail = path.substr(dotplace+1, std::string::npos);

  bool ret;
  if(name == "$root") {
    ret = find_device_down(&CARD_LIST::card_list, tail);
  }else if(CARD const* o = dev->owner()){
    ret = find_device_up(o, name, tail);
  }else{
    ret = find_device_down(&CARD_LIST::card_list, path);
  }

  if(ret){
  }else{ untested();
    throw Exception_Cant_Find(name, tail);
  }
}
/*--------------------------------------------------------------------------*/
} // namespace
namespace va {
// some builtin numerical functions according to verilog standard
// TODO: move to plugins, like the others.
/*--------------------------------------------------------------------------*/
template<class T>
T fmod(T d, T e)
{ untested();
  return std::fmod(d, e);
}
/*--------------------------------------------------------------------------*/
inline double fmod(double d, int e)
{
  return std::fmod(d, e);
}
/*--------------------------------------------------------------------------*/
inline int fmod(int d, int e)
{
  return d % e;
}
/*--------------------------------------------------------------------------*/
inline int fmod(PARAMETER<int> const& d, int e)
{itested();
  return d % e;
}
/*--------------------------------------------------------------------------*/
template<class T, class S, class X=void>
struct ddouble_if{
  typedef T type;
};
/*--------------------------------------------------------------------------*/
struct ddouble_tag{};
/*--------------------------------------------------------------------------*/
template<class T, class S>
struct ddouble_if<S, T, typename std::enable_if< std::is_same<typename T::base_tag, ddouble_tag>::value >::type > {
  // typedef typename T::base type;
  typedef T type;
};
/*--------------------------------------------------------------------------*/
template<class T, class S>
struct ddouble_if<T, S, typename T::base> {
  typedef typename T::base type;
};
/*--------------------------------------------------------------------------*/
template<class T, class S>
struct ddouble_if<T, S, typename S::base> {
  typedef typename S::base type;
};
/*--------------------------------------------------------------------------*/
template<class S>
struct ddouble_if<PARAMETER<double>, S> {
  typedef double type;
};
/*--------------------------------------------------------------------------*/
template<>
struct ddouble_if<PARAMETER<double>, double> {
  typedef double type;
};
/*--------------------------------------------------------------------------*/
template<>
struct ddouble_if<PARAMETER<int>, double> {
  typedef double type;
};
/*--------------------------------------------------------------------------*/
template<>
struct ddouble_if<PARAMETER<int>, int> {
  typedef double type;
};
/*--------------------------------------------------------------------------*/
template<class T, int a>
struct ddouble_if<T, ddouble_<a>>{
  typedef ddouble_<a> type;
};
/*--------------------------------------------------------------------------*/
template<>
struct ddouble_if<double, double>{
  typedef double type;
};
/*--------------------------------------------------------------------------*/
template<class T>
struct ddouble_if<T, double>{
  typedef typename ddouble_if<double, T>::type type;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
template<class T, class S=T>
T white_noise(T, S=0.)
{ itested();
  //incomplete();
  return T(0.);
}
/*--------------------------------------------------------------------------*/
template<class T, class T2, class S=T>
T flicker_noise(T, T2, S=0.)
{ itested();
  //incomplete();
  return T(0.);
}
/*--------------------------------------------------------------------------*/
class EVT{
public:
  virtual void operator()() const = 0;
};
/*--------------------------------------------------------------------------*/
} // va
/*--------------------------------------------------------------------------*/
class FILTER {
public:
   virtual double read_filter()const { untested();unreachable(); return 0;}
};
/*--------------------------------------------------------------------------*/
namespace va {
/*--------------------------------------------------------------------------*/
inline double PORT_FLOW(int i, BASE_SUBCKT const* m)
{
  incomplete();
  node_t const& n = m->n_(i);
  double I(0.);
  assert(m->subckt());
  for(CARD const* c : *m->subckt()){
    auto e = dynamic_cast<ELEMENT const*>(c);
    if(!e){ untested();
    }else if(e->n_(1).m_() == e->n_(0).m_()){ untested();
    }else if(e->n_(0).m_() == n.m_()){
      I+= e->tr_amps();
    }else if(e->n_(1).m_() == n.m_()){
      I-= e->tr_amps();
    }else{ untested();
    }
  }
  return I;
}
/*--------------------------------------------------------------------------*/
inline node_t& ground()
{ untested();
  assert(CARD_LIST::card_list.nodes());
 // assert(CARD_LIST::card_list.nodes()->at(0).m_()==0);
  return CARD_LIST::card_list.nodes()->at(0);
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
