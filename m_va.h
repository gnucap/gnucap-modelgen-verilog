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
#include <l_stlextra.h>

namespace{

typedef double real;
typedef int integer;
// double inf = std::numeric_limits<double>::infinity();
#define inf std::numeric_limits<double>::infinity()
#define Inf inf

template<int numderiv>
class ddouble_ {
	double _data[1+numderiv];
public:
	ddouble_(ddouble_ const& o) {
		notstd::copy_n(o._data, numderiv+1, _data);
	}
	explicit ddouble_() { clear(); }
	/*explicit*/ ddouble_(int const& d) { clear(); value() = d; }
	/*explicit*/ ddouble_(double const& d) { clear(); value() = d; }
	/*explicit*/ ddouble_(PARAMETER<double> const& d) { clear(); value() = d; }
public:
	double const& value() const{return _data[0];}
	double& value(){return _data[0];}
	double d(int i) const{return _data[i];}
	double const& operator[](int i) const{return _data[i+1];}
	double& operator[](int i){return _data[i+1];}
	operator double() const { return *_data; }
//	operator double() { return *_data; }
//	operator integer() const { return *_data; }
// 	ddouble_& operator=(ddouble_ const& o) { untested();
// 		assert(o == o);
// 		notstd::copy_n(o._data, numderiv+1, _data);
// 		return *this;
// 	}
	ddouble_& operator=(ddouble_ o) {
		assert(o == o);
		notstd::copy_n(o._data, numderiv+1, _data);
		return *this;
	}
	ddouble_& operator=(PARAMETER<double> const& o) {
		assert(o == o);
		*_data = o;
		std::fill_n(_data+1, numderiv, 0.);
		return *this;
	}
	ddouble_& operator=(const double& o) {
		assert(o == o);
		*_data = o;
		std::fill_n(_data+1, numderiv, 0.);
		return *this;
	}
	bool is_same(const ddouble_& o)const {
		for(int i=0; i<=numderiv; ++i){
			if(_data[i] != o._data[i]){ untested();
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
		return *_data < o;
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
	bool operator>=(PARAMETER<double> const& o)const {untested();
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
		ret.value() += o;
		return ret;
	}
	ddouble_ operator+(int const& o) const {
		ddouble_ ret = *this;
		ret.value() += o;
		return ret;
	}
	ddouble_ operator+(PARAMETER<double> const& o) const {
		ddouble_ ret = *this;
		ret.value() += o;
		return ret;
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
		ret -= o;
		return ret;
	}
	ddouble_ operator-(int const& o) const {
		ddouble_ ret = *this;
		ret -= o;
		return ret;
	}
	ddouble_ operator-(PARAMETER<double> const& o) const {
		ddouble_ ret = *this;
		ret -= o;
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
		trace2("op*", *o._data, *_data);
		assert(*o._data == *o._data);
		assert(*_data == *_data);
		for(int i=1; i<=numderiv; ++i){
			assert(_data[i] == _data[i]);
			assert(o._data[i] == o._data[i]);
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
		}else{
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
		ret /= o;
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
{ itested();
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
	c += a;
	return c;
}
template<int T>
ddouble_<T> operator+(int const& a, ddouble_<T> const& b)
{ itested();
	ddouble_<T> c(b);
	c += a;
	return c;
}
template<int T>
ddouble_<T> operator+(PARAMETER<double> const& a, ddouble_<T> const& b)
{ itested();
	ddouble_<T> c(b);
	c += a;
	return c;
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
	return t.set_value(d);
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
	return t.chain(d);
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

namespace va {
// some builtin numerical functions according to verilog standard

template<class T>
T cos(T d)
{
	chain(d, -std::sin(d));
	return set_value(d, std::cos(d));
}

template<class T>
T cosh(T d)
{itested();
	chain(d, std::sinh(d));
	return set_value(d, std::cosh(d));
}

template<class T>
T exp(T d)
{
	set_value(d, std::exp(d));
	return chain(d, d);
}

// DUP
double exp(PARAMETER<double> const& d)
{
	return std::exp(d);
}

template<class T>
T atan(T d)
{ itested();
	chain(d, 1./(1.+double(d)*double(d)));
	set_value(d, std::atan(d));
	return d;
}

template<class T>
T atanh(T d)
{ untested();
	incomplete();
	return d;
}

template<class T>
T sinh(T d)
{ untested();
	incomplete();
	return d;
}

template<class T>
T abs(T d)
{itested();
	if(d>=0.){itested();
	}else{itested();
		d *= -1.;
	}
	return d;
}

template<class T>
int floor(T d)
{itested();
	return int(std::floor(d));
}

template<class T>
T fmod(T d, T e)
{ untested();
	return std::fmod(d, e);
}
double fmod(double d, int e)
{ untested();
	return std::fmod(d, e);
}
int fmod(int d, int e)
{ untested();
	return d % e;
}
int fmod(PARAMETER<int> const& d, int e)
{itested();
	return d % e;
}

template<class T>
T limexp(T& d)
{ untested();
	return exp(d);
}


template<class T>
T log10(T& d)
{ untested();
	incomplete();
	// chain(d, std::cos(d));
	d.value() = std::log10(double(d));
	return d;
}

template<class T>
T ln(T d)
{itested();
	double l=-1e99;
	if(d>1e-40){itested();
		l = std::log(double(d));
		chain(d, 1./double(d));
	}else if(d>0){ untested();
		l=-1e40;
		chain(d, 1e40);
	}else{ untested();
		chain(d, 1e40);
	}
	set_value(d, l);
//	assert(d.is_same(d));
	return d;
}

template<class T, class S>
struct ddouble_if{
	typedef T type;
};

template<class T, int a>
struct ddouble_if<T, ddouble_<a>>{
	typedef ddouble_<a> type;
};

template<class S, class T>
typename ddouble_if<T, S>::type max(T d, S e)
{
	if(double(d) <= double(e)){ itested();
		return e;
	}else{itested();
		return d;
	}
}

template<class T, class S>
typename ddouble_if<T, S>::type min(T d, S e)
{itested();
	if(double(d) <= double(e)){itested();
		return d;
	}else{ untested();
		return e;
	}
}

template<class T, class S>
typename ddouble_if<T, S>::type pow(T b, S e)
{
	assert(b==b);
	assert(e==e);
	double p;
	if(double(b) != 0.){
		p = std::pow(b, e);
		chain(b, double(e)/double(b)*p);
	}else{
		p = 0;
		chain(b, 0.);
	}

	if(double(b) > 0.){
		double l = std::log(b);
		assert(l==l);
		chain(e, l*p);
		assert(e==e);
	}else{
		// unreachable(); // numerical nonsense, sometimes
		chain(e, 0.);
	}

	set_value(b, p);
	set_value(e, 0.);
	typedef typename ddouble_if<T, S>::type ret_t;
	ret_t ret;
	ret = b;
   ret += ret_t(e);
	assert(b==b);
	return ret;
}

template<class T>
T asinh(T d)
{itested();
	double b = sqrt(1.+double(d)*double(d));
	chain(d, 1./b);
	set_value(d, std::asinh(d));
	return d;
}

template<class T>
T sin(T d)
{
	chain(d, std::cos(d));
	return set_value(d, std::sin(d));
}

template<class T>
T sqrt(T d)
{ itested();
	if(double(d)>1e-20){ itested();
		double s = std::sqrt(d);
		d.value() = s;
		chain(d, -.5/s);
	}else if(d>0){ untested();
		chain(d, -5e21);
		d.value() = std::sqrt(d);
	}else{ untested();
		chain(d, -.5e99);
		d.value() = 1e-99;
	}
	return d;
}

template<class T>
T tanh(T d)
{itested();
	double t = std::tanh(d);
	set_value(d, t);
	chain(d, 1.-double(t)*double(t));
	assert(d.is_same(d));
	return d;
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
double simparam(std::string const& what, double def=0)
{
	if(what=="gmin") {itested();
		return OPT::gmin;
	}else{ untested();
		return def;
	}
}

} // va
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
