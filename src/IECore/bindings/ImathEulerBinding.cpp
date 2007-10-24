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

#include <OpenEXR/ImathEuler.h>

#include "IECore/bindings/ImathVecBinding.h"
#include "IECore/bindings/IECoreBinding.h"

using namespace boost::python;
using namespace Imath;
using namespace std;

namespace IECore 
{

template<typename T>
void bindEuler(const char *bindName);

void bindImathEuler()
{
	bindEuler<float>("Eulerf");
	bindEuler<double>("Eulerd");
}

#define DEFINEEULERSTRSPECIALISATION( EULER )\
\
template<>\
std::string repr<EULER>( EULER &x )\
{\
	std::stringstream s;\
	s << #EULER << "( ";\
	for( unsigned i=0; i<EULER::dimensions(); i++ )\
	{\
		s << x[i];\
		if( i!=EULER::dimensions()-1 )\
		{\
			s << ", ";\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<>\
std::string str<EULER>( EULER &x )\
{\
	std::stringstream s;\
	for( unsigned i=0; i<EULER::dimensions(); i++ )\
	{\
		s << x[i];\
		if( i!=EULER::dimensions()-1 )\
		{\
			s << " ";\
		}\
	}\
	return s.str();\
}\

DEFINEEULERSTRSPECIALISATION( Eulerf );
DEFINEEULERSTRSPECIALISATION( Eulerd );

template<typename T>
struct EulerHelper
{
	static tuple angleOrder( Euler<T> &e )
	{
		int i, j, k;
		e.angleOrder( i, j, k );
		
		return make_tuple( i, j, k );
	}
	
	static tuple angleMapping( Euler<T> &e )
	{
		int i, j, k;
		e.angleMapping( i, j, k );
		
		return make_tuple( i, j, k );
	}
};

template<typename T>
void bindEuler(const char *bindName)
{		
	/// We need these typedefs so we can instantiate the "optional" and "enum_" templates. 
	// Without them, the enums defined in Imath::Euler aren't types.
	typedef typename Euler<T>::Order OrderType;
	typedef typename Euler<T>::InputLayout InputLayoutType;	
	typedef typename Euler<T>::Axis AxisType;		

	void (Euler<T>::*extractM33)(const Matrix33<T>&) = &Euler<T>::extract;
	void (Euler<T>::*extractM44)(const Matrix44<T>&) = &Euler<T>::extract;
	void (Euler<T>::*extractQuat)(const Quat<T>&) = &Euler<T>::extract;		

	object euler = class_< Euler<T>, bases< Vec3<T> > >(bindName)
	
		.def( init<>() )
		.def( init< const Euler<T> & >() )		
		.def( init< OrderType >() )
		.def( init< const Vec3<T> &, optional< OrderType, InputLayoutType > >() )
		.def( init< T, T, T, optional< OrderType, InputLayoutType > >() )
		.def( init< const Euler<T> &, optional< OrderType > >() )
		.def( init< const Matrix33<T> &, optional< OrderType > > () )
		.def( init< const Matrix44<T> &, optional< OrderType > > () )
		
		.def( "__str__", &IECore::str<Euler<T> > )
		.def( "__repr__", &IECore::repr<Euler<T> > )
		

		.def( "legal", &Euler<T>::legal ).staticmethod("legal")
		
		.def( "setXYZVector", &Euler<T>::setXYZVector )
		
		.def( "order", &Euler<T>::order )
		.def( "setOrder", &Euler<T>::setOrder )
		
		.def( "set", &Euler<T>::set )
		
		.def( "extract", extractM33 )
		.def( "extract", extractM44 )		
		.def( "extract", extractQuat )	
		
		.def( "toMatrix33", &Euler<T>::toMatrix33 )	
		.def( "toMatrix44", &Euler<T>::toMatrix44 )
		.def( "toQuat", &Euler<T>::toQuat )
		.def( "toXYZVector", &Euler<T>::toXYZVector )		
		
		.def( "angleOrder", &EulerHelper<T>::angleOrder )
		.def( "angleMapping", &EulerHelper<T>::angleOrder )
		
		.def( "angleMod", &Euler<T>::angleMod ).staticmethod("angleMod")
		.def( "simpleXYZRotation", &Euler<T>::simpleXYZRotation ).staticmethod("simpleXYZRotation")
		.def( "nearestRotation", &Euler<T>::simpleXYZRotation ).staticmethod("nearestRotation")		
		
		.def( "makeNear", &Euler<T>::makeNear )
		
		.def( "frameStatic", &Euler<T>::frameStatic )
		.def( "initialRepeated", &Euler<T>::initialRepeated )
		.def( "parityEven", &Euler<T>::parityEven )
		.def( "initialAxis", &Euler<T>::initialAxis )				
	;
	
	scope eulerScope (euler );
	
	enum_< OrderType >( "Order" )
		.value( "XYZ", Euler<T>::XYZ )		
		.value( "XZY", Euler<T>::XZY )
		.value( "YZX", Euler<T>::YZX )
		.value( "YXZ", Euler<T>::YXZ )
		.value( "ZXY", Euler<T>::ZXY )
		.value( "ZYX", Euler<T>::ZYX )

		.value( "XZX", Euler<T>::XZX )
		.value( "XYX", Euler<T>::XYX )
		.value( "YXY", Euler<T>::YXY )
		.value( "YZY", Euler<T>::YZY )
		.value( "ZYZ", Euler<T>::ZYZ )
		.value( "ZXZ", Euler<T>::ZXZ )

		.value( "XYZr", Euler<T>::XYZr )
		.value( "XZYr", Euler<T>::XZYr )
		.value( "YZXr", Euler<T>::YZXr )
		.value( "YXZr", Euler<T>::YXZr )
		.value( "ZXYr", Euler<T>::ZXYr )
		.value( "ZYXr", Euler<T>::ZYXr )

		.value( "XZXr", Euler<T>::XZXr )
		.value( "XYXr", Euler<T>::XYXr )
		.value( "YXYr", Euler<T>::YXYr )
		.value( "YZYr", Euler<T>::YZYr )
		.value( "ZYZr", Euler<T>::ZYZr )
		.value( "ZXZr", Euler<T>::ZXZr )
		
		.value( "Default", Euler<T>::Default )		
	;
	
	enum_< InputLayoutType >( "InputLayout" )
		.value( "XYZLayout", Euler<T>::XYZLayout )
		.value( "IJKLayout", Euler<T>::IJKLayout )
	;		
		
	enum_< AxisType >( "Axis" )
		.value( "X", Euler<T>::X )
		.value( "Y", Euler<T>::Y )		
		.value( "Z", Euler<T>::Z )		
	;
}

}
