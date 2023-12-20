/*                        -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 * Author: Felix Salfelder
 *
 * This file is part of \"Gnucap\", the Gnu Circuit Analysis Package
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
#include <e_base.h> // CKT_BASE
#include <u_sim_data.h> // see simparam

// attribute index
typedef int aidx;

namespace notstd {
/*--------------------------------------------------------------------------*/
template <class InputIter, class Size, class OutputIter>
void add_n(InputIter first, Size count, OutputIter result)
{
  for ( ; count > 0; --count) {
    *result++ += *first++;
  }
}
/*--------------------------------------------------------------------------*/
}

namespace{

typedef double real;
typedef int integer;
// double inf = std::numeric_limits<double>::infinity();
#define inf std::numeric_limits<double>::infinity()
#define Inf inf

template<int numderiv>
class ddouble_ {
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
	/*explicit*/ ddouble_(PARAMETER<double> const& d) { clear(); *_data = d; }
	void	set_all_deps() {
	  std::fill_n(_data+1, numderiv, 0.);
	}
	void	set_no_deps() {
	  std::fill_n(_data+1, numderiv, std::numeric_limits<double>::quiet_NaN());
	}
private:
	void	reset_deriv(){
	  set_all_deps();
	}
public:
	virtual double const& value()const {return _data[0];}
	double d(int i)const {return _data[i];}
	double const& operator[](int i)const {return _data[i+1];}
	double& operator[](int i){return _data[i+1];}
	operator double()const { return *_data; }
#if 1
 	ddouble_& operator=(ddouble_ const& o) {
 		assert(o == o);
 		notstd::copy_n(o._data, numderiv+1, _data);
 		return *this;
 	}
#else
	ddouble_& operator=(ddouble_ o) {
		assert(o == o);
		notstd::copy_n(o._data, numderiv+1, _data);
		return *this;
	}
#endif
	ddouble_& operator=(PARAMETER<double> const& o) {
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
	bool is_same(const ddouble_& o)const {
		for(int i=0; i<=numderiv; ++i){
			if(_data[i] != _data[i]){ itested();
			  if( o._data[i] == o._data[i] ){
				return false;
			  }else{
			  }
			}else if(_data[i] != o._data[i]){ untested();
				return false;
			}else{
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
	bool operator<(PARAMETER<double> const& o)const {
		return *_data < double(o);
	}

	bool operator<=(const ddouble_& o)const { itested();
		return *_data <= *o._data;
	}
	bool operator<=(double const& o)const { itested();
		return *_data <= o;
	}
	bool operator<=(int const& o)const { itested();
		return *_data <= o;
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
	bool operator>=(const double& o)const {itested();
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
	ddouble_ operator*(const double& o) const {itested();
		ddouble_ ret = *this;
		ret *= o;
		return ret;
	}
	ddouble_ operator*(PARAMETER<double> const& o) const {
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
//	ddouble_ operator/(double const& o) {
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

template<int T>
ddouble_<T> operator/(ddouble_<T> a, ddouble_<T> b)
{
	a /= b;
	return a;
}
template<int T>
ddouble_<T> operator/(int const& a, ddouble_<T> b)
{
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
{
	ddouble_<T> c(a);
	c /= b;
	return c;
}
template<int T>
ddouble_<T> operator/(PARAMETER<int> const& a, ddouble_<T> const& b)
{
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
{ untested();
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
{
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
	c *= a;
	return c;
}

template<int T>
ddouble_<T> operator+(double const& a, ddouble_<T> const& b)
{ itested();
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
inline bool operator==(PARAMETER<double> const& b, int const& a)
{ untested();
	return int(b) == a;
}
// inline bool operator==(PARAMETER<int> const& b, int const& a)
// { untested();
// 	return int(b) == a;
// }

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
{
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
{ itested();
	// no-op
	return t;
}
template<>
inline PARAMETER<int>& chain(PARAMETER<int>& t, double const&)
{ untested();
	// no-op
	return t;
}


}

/*--------------------------------------------------------------------------*/
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
{ untested();
  return std::fmod(d, e);
}
/*--------------------------------------------------------------------------*/
inline int fmod(int d, int e)
{ untested();
  return d % e;
}
/*--------------------------------------------------------------------------*/
inline int fmod(PARAMETER<int> const& d, int e)
{itested();
  return d % e;
}
/*--------------------------------------------------------------------------*/
  template<class T>
  T log10(T& d)
{ untested();
  incomplete();
  // chain(d, std::cos(d));
  set_value(d, std::log10(double(d)));
  return d;
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
template<class S, class T>
typename ddouble_if<T, S>::type max(T d, S e)
{
  typedef typename ddouble_if<T, S>::type ret_t;
  if(double(d) <= double(e)){ itested();
    return ret_t(e);
  }else{itested();
    return ret_t(d);
  }
}
/*--------------------------------------------------------------------------*/
template<class T, class S>
typename ddouble_if<T, S>::type min(T d, S e)
{itested();
  typedef typename ddouble_if<T, S>::type ret_t;
  if(double(d) <= double(e)){itested();
    return ret_t(d);
  }else{ itested();
    return ret_t(e);
  }
}
/*--------------------------------------------------------------------------*/
template<class T, class S=T>
T white_noise(T, S=0.)
{ itested();
  //incomplete();
  return T(0.);
}

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
/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
