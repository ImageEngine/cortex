//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
#include "boost/python.hpp"

// System includes
#include <string>
#include <stdexcept>

#include "OpenEXR/ImathPlane.h"
#include "OpenEXR/ImathVec.h"

#include "IECore/bindings/ImathPlaneBinding.h"
#include "IECore/bindings/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;
using namespace std;

namespace IECore 
{

template<typename T>
void bindPlane3(const char *bindName);

void bindImathPlane()
{
	bindPlane3<float>("Plane3f");
	bindPlane3<double>("Plane3d");
}

#define DEFINEPLANEPECIALISATION( PLANE )\
\
template<>\
std::string repr<PLANE>( PLANE &x )\
{\
	std::stringstream s;\
	s << "IECore." << #PLANE << "( ";\
	s << repr( x.normal ) << ", ";\
	s << x.distance;\
	s << " )";\
	return s.str();\
}\
\
template<>\
std::string str<PLANE>( PLANE &x )\
{\
	std::stringstream s;\
	s << str( x.normal ) << " " << str( x.distance );\
	return s.str();\
}\

DEFINEPLANEPECIALISATION( Plane3f );
DEFINEPLANEPECIALISATION( Plane3d );

template<typename T>
void bindPlane3(const char *bindName)
{	

	void (Plane3<T>::*set1)(const Vec3<T> &, T) = &Plane3<T>::set;
	void (Plane3<T>::*set2)(const Vec3<T> &, const Vec3<T> &) = &Plane3<T>::set;
	void (Plane3<T>::*set3)(const Vec3<T> &, const Vec3<T> &, const Vec3<T> &) = &Plane3<T>::set;		

	class_< Plane3<T> >(bindName)
		.def_readwrite("normal", &Plane3<T>::normal)
		.def_readwrite("distance", &Plane3<T>::distance)
		
		.def(init<>())
		.def(init<const Vec3<T> &, T>())
		.def(init<const Vec3<T> &, const Vec3<T> & >())
		.def(init<const Vec3<T> &, const Vec3<T> &, const Vec3<T> & >())
		
		.def( "set", set1 )
		.def( "set", set2 )
		.def( "set", set3 )				

		.def( "distanceTo",  &Plane3<T>::distanceTo )
		.def( "reflectPoint",  &Plane3<T>::reflectPoint )		
		.def( "reflectVector",  &Plane3<T>::reflectVector )		
		
		.def( "__str__", &IECore::str<Plane3<T> > )
		.def( "__repr__", &IECore::repr<Plane3<T> > )
	;
}

}
