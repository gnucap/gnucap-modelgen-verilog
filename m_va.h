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

namespace{

typedef double real;
typedef int integer;
// double inf = std::numeric_limits<double>::infinity();
#define inf std::numeric_limits<double>::infinity()

template<int numderiv>
class ddouble_ {
	double _data[1+numderiv];
public:
	explicit ddouble_() { clear(); }
	explicit ddouble_(double const& d) { clear(); value() = d; }
public:
	double const& value() const{return _data[0];}
	double& value(){return _data[0];}
	double d(int i) const{return _data[i];}
	double const& operator[](int i) const{return _data[i+1];}
	double& operator[](int i){return _data[i+1];}
	operator double&() { return *_data; }
//	operator integer() const { return *_data; }
	ddouble_& operator=(const double& o) {
		assert(o == o);
		*_data = o;
		std::fill_n(_data+1, numderiv, 0.);
		return *this;
	}
	bool operator==(const ddouble_& o)const {
#if 0 // really?
		for(int i=0; i<=numderiv; ++i){
			if(_data[i] != o._data[i]){
				return false;
			}else{
			}
		}
#else
		return *_data == *o._data;
#endif
		return true;
	}
	bool operator<(const ddouble_& o)const {
		return *_data < *o._data;
	}
	bool operator<=(const ddouble_& o)const { untested();
		return *_data <= *o._data;
	}
	bool operator>(const ddouble_& o)const {
		return *_data > *o._data;
	}
	bool operator>=(const ddouble_& o)const { untested();
		return *_data >= *o._data;
	}
	ddouble_& operator+=(const ddouble_& o) {
		for(int i=0; i<=numderiv; ++i){
			_data[i] += o._data[i];
		}
		return *this;
	}
	ddouble_& operator-=(const ddouble_& o) {
		for(int i=0; i<=numderiv; ++i){
			_data[i] -= o._data[i];
		}
		return *this;
	}
	ddouble_& operator*=(const double& o) {
		for(int i=1; i<=numderiv; ++i){
			_data[i] *= o;
		}
		*_data *= o;
		return *this;
	}
	ddouble_& operator*=(const ddouble_& o) {
		assert(*o._data == *o._data);
		assert(*_data == *_data);
		for(int i=1; i<=numderiv; ++i){
			assert(_data[i] == _data[i]);
			assert(o._data[i] == o._data[i]);
			_data[i] *= *o._data; _data[i] += *_data * o._data[i];
		}
		*_data *= *o._data;
		return *this;
	}
	ddouble_& operator/=(const ddouble_& o) {
		assert(*o._data);
		for(int i=1; i<=numderiv; ++i){
			_data[i] *= o.value(); _data[i] -= value() * o._data[i];
			_data[i] /= o.value() * o.value();
		}
		*_data /= *o._data;
		return *this;
	}
	ddouble_& chain(double d) {
		for(int i=1; i<=numderiv; ++i){
			_data[i] *= d;
		}
		return *this;
	} /*chain*/
	void clear() {
		std::fill_n(_data, numderiv+1, 0.);
	} /*clear*/
}; /*ddouble_*/

template<class T>
T& chain(T& t, double d)
{
	t.chain(d);
	return t;
}

template<>
inline double& chain(double& t, double)
{ untested();
	// no-op
	return t;
}

}

namespace va {
// some builtin numerical functions according to verilog standard

template<class T>
T cos(T& d)
{
	chain(d, -std::sin(d));
	d.value() = std::cos(d);
	return d;
}

template<class T>
T exp(T& d)
{
	assert(d.value() == d.value());
	d.value() = std::exp(d);
	assert(d.value() == d.value());
	chain(d, d);
	return d;
}

template<class T>
T limexp(T& d)
{ untested();
	return exp(d);
}

template<class T>
T abs(T& d)
{
	if(d>=0){ untested();
		chain(d, 1.);
	}else{ untested();
		chain(d, -1.);
	}
	d.value() = std::abs(d);
	return d;
}

template<class T>
T atanh(T& d)
{
	incomplete();
	return d;
}

template<class T>
T log10(T& d)
{
	incomplete();
	// chain(d, std::cos(d));
	d.value() = std::log10(d);
	return d;
}

template<class T>
T ln(T& d)
{
	incomplete();
	// chain(d, std::cos(d));
	d.value() = std::log(d);
	return d;
}


template<class T>
T max(T& d, T& e)
{
	if(d.value() <= e.value()){ untested();
		return e;
	}else{ untested();
		return d;
	}
}

template<class T>
T min(T& d, T& e)
{
	if(d.value() <= e.value()){ untested();
		return d;
	}else{ untested();
		return e;
	}
}

template<class T>
T pow(T& b, T& e)
{
	assert(b==b);
	assert(e==e);
	double p;
	if(b.value()>0){
		p = std::pow(b, e);
		chain(b, e.value()/b.value()*p);
		double l = std::log(b);
		assert(l==l);
		chain(e, l*p);
		assert(e==e);
	}else{
		// unreachable(); // numerical nonsense, sometimes
		p = 0;
		chain(b, 0.);
		chain(e, 0.);
	}

	b.value() = p;
	e.value() = 0.;
	b += e;
	assert(b==b);
	return b;
}

template<class T>
T sin(T& d)
{
	chain(d, std::cos(d));
	d.value() = std::sin(d);
	return d;
}

template<class T>
T sinh(T& d)
{
	incomplete();
	return d;
}

template<class T>
T sqrt(T& d)
{
	incomplete();
	// chain(d, std::cos(d));
	d.value() = std::sqrt(d);
	return d;
}

template<class T>
T tanh(T& d)
{
	incomplete();
	// chain(d, std::cos(d));
	d.value() = std::tanh(d);
	return d;
}

/*--------------------------------------------------------------------------*/
template<class T, class S=T>
T white_noise(T, S=0.)
{
	//incomplete();
	return T(0.);
}

template<class T, class S=T>
T flicker_noise(T, T, S=0.)
{
	//incomplete();
	return T(0.);
}

} // va
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
