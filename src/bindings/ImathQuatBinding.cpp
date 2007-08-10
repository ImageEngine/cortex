//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

// This include needs to be the very first to prevent problems with warnings 
// regarding redefinition of _POSIX_C_SOURCE
#include <boost/python.hpp>

// System includes
#include <string>
#include <stdexcept>

#include <OpenEXR/ImathQuat.h>

#include "IECore/bindings/ImathQuatBinding.h"
#include "IECore/bindings/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;

namespace IECore 
{
	
template<typename T>
void bindQuat(const char *bindName);

void bindImathQuat()
{	
	bindQuat<float>("Quatf");
	bindQuat<double>("Quatd");	
}

template<typename T>
struct QuatIndexer
{	
	static T get(const Quat<T> &x, int i)
	{	
		// Do we want to handle backward indexing?
		if ( i >= 0 && i < 4 ) 
		{
			return x[i];
		}
		else
		{
			throw std::out_of_range("");	
		}
		
	}
	
	static void set(Quat<T>  &x, int i, const T &v)
	{
		if ( i >= 0 && i < 4 ) 
		{
			x[i] = v;
		}
		else
		{
			throw std::out_of_range("");
		}
	}
	

};

#define DEFINEQUATSTRSPECIALISATION( QUAT )				\
														\
template<>												\
std::string repr<QUAT>( QUAT &x )						\
{														\
	std::stringstream s;								\
	s << #QUAT << "( ";									\
	for( unsigned i=0; i<4; i++ )						\
	{													\
		s << x[i];										\
		if( i!=3 )										\
		{												\
			s << ", ";									\
		}												\
	}													\
	s << " )";											\
	return s.str();										\
}														\
														\
template<>												\
std::string str<QUAT>( QUAT &x )						\
{														\
	std::stringstream s;								\
	for( unsigned i=0; i<4; i++ )						\
	{													\
		s << x[i];										\
		if( i!=3 )										\
		{												\
			s << " ";									\
		}												\
	}													\
	return s.str();										\
}														\

DEFINEQUATSTRSPECIALISATION( Quatf );
DEFINEQUATSTRSPECIALISATION( Quatd );

template<typename T>
void bindQuat(const char *bindName)
{
	class_< Quat<T> >(bindName)	
		.def_readwrite("r", &Quat<T>::r)
		.def_readwrite("v", &Quat<T>::v)
	
		// [] operator support
		.def("__getitem__", &QuatIndexer<T>::get)
		.def("__setitem__", &QuatIndexer<T>::set)
	
		.def(init<>())
		.def(init<Quat<T> >())
		.def(init<T, T, T, T>())
		.def(init<T, Vec3<T> >())
	
		.def("identity", &Quat<T>::identity).staticmethod("identity")
		
		.def(self ^ self)
	
		.def(self *= self)
		.def(self *= T())
		.def(self /= self)
		.def(self /= T())
		.def(self += self)
		.def(self -= self)
	
		.def(self == self)
		.def(self != self)
	
		.def("invert", &Quat<T>::invert, return_self<>())
		.def("inverse", &Quat<T>::inverse)		
		.def("normalize", &Quat<T>::normalize, return_self<>())
		.def("normalized", &Quat<T>::normalized)
		.def("length", &Quat<T>::length)
		
		.def("setAxisAngle", &Quat<T>::setAxisAngle, return_self<>())
		.def("setRotation", &Quat<T>::setRotation, return_self<>())
		
		.def("angle", &Quat<T>::angle)
		.def("axis", &Quat<T>::axis)
		
		.def("toMatrix33", &Quat<T>::toMatrix33)
		.def("toMatrix44", &Quat<T>::toMatrix44)
		
		.def("log", &Quat<T>::log)
		.def("exp", &Quat<T>::exp)
		
		.def("__str__", IECore::str<Quat<T> >)
		.def("__repr__", IECore::repr<Quat<T> >)
	;
			
	def("slerp", slerp<T>);
	def("squad", squad<T>);
	def("spline", spline<T>);
}

}
