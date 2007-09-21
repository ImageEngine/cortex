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
#include <sstream>

#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathBoxAlgo.h>

#include "IECore/VectorTraits.h"
#include "IECore/bindings/ImathBoxBinding.h"
#include "IECore/bindings/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;

namespace IECore 
{
		
template<typename T>
class_< Box<T> > bindBox(const char *bindName);
	
void bindImathBox()
{
	bindBox<V2f>("Box2f");
	bindBox<V2d>("Box2d");

	class_< Box3f > cf = bindBox<V3f>("Box3f");
	cf.def("transform", &transform<float,float>);
	cf.def("transform", &transform<float,double>);

	class_< Box3d > cd = bindBox<V3d>("Box3d");
	cd.def("transform", &transform<double,float>);
	cd.def("transform", &transform<double,double>);

	bindBox<V2i>("Box2i");
	bindBox<V3i>("Box3i");
}

#define DEFINEBOXSTRSPECIALISATION( BOX )				\
														\
template<>												\
std::string repr<BOX>( BOX &x )							\
{														\
	std::stringstream s;								\
	s << #BOX << "( ";									\
	s << repr( x.min ) << ", ";							\
	s << repr( x.max );									\
	s << " )";											\
	return s.str();										\
}														\
														\
template<>												\
std::string str<BOX>( BOX &x )							\
{														\
	std::stringstream s;								\
	s << str( x.min ) << " " << str( x.max );			\
	return s.str();										\
}														\

DEFINEBOXSTRSPECIALISATION( Box2i );
DEFINEBOXSTRSPECIALISATION( Box3i );
DEFINEBOXSTRSPECIALISATION( Box2f );
DEFINEBOXSTRSPECIALISATION( Box3f );
DEFINEBOXSTRSPECIALISATION( Box2d );
DEFINEBOXSTRSPECIALISATION( Box3d );
	
template<typename T>
class_< Box<T> > bindBox(const char *bindName)
{	
	void (Box<T>::*eb1)(const T&) = &Box<T>::extendBy;
	void (Box<T>::*eb2)(const Box<T>&) = &Box<T>::extendBy;
	
	bool (Box<T>::*i1)(const T&) const = &Box<T>::intersects;
	bool (Box<T>::*i2)(const Box<T>&) const = &Box<T>::intersects;	
	
	class_< Box<T> > myClass(bindName);
	myClass.def_readwrite("min", &Box<T>::min)
		.def_readwrite("max", &Box<T>::max)		
	
		.def(init<>())
		.def(init<T>())
		.def(init<T, T>())
	
		.def(self == self)
		.def(self != self)
	
		.def("makeEmpty", &Box<T>::makeEmpty)
		.def("extendBy", eb1)
		.def("extendBy", eb2)
	
		.def("size", &Box<T>::size)
		.def("center", &Box<T>::center)
		.def("intersects", i1)
		.def("intersects", i2)

		.def("majorAxis", &Box<T>::majorAxis)
		
		.def("isEmpty", &Box<T>::isEmpty)
		.def("hasVolume", &Box<T>::hasVolume)

		.def("dimensions", &VectorTraits<T>::dimensions).staticmethod("dimensions")

		.def( "__str__", &IECore::str<Box<T> > )
		.def( "__repr__", &IECore::repr<Box<T> > )
	;
	return myClass;
}	

}
