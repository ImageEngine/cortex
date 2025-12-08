//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/RampBinding.h"

#include "IECorePython/IECoreBinding.h"

#include "IECore/Ramp.h"

using namespace boost::python;
using namespace Imath;
using namespace std;
using namespace IECore;

namespace IECorePython
{

template<typename T>
std::string rampRepr( object x )
{
	std::stringstream s;
	const std::string name = extract<std::string>( x.attr( "__class__").attr( "__name__" ) );
	s << "IECore." << name << "( ";
	const T ramp = extract<T>( x );
	s << "(";
	int i = 0;
	int l = ramp.points.size();
	typename T::PointContainer::const_iterator it;
	for( it = ramp.points.begin(); it != ramp.points.end(); it++, i++ )
	{
		// TODO - without this const_cast I get a link error because the const version of repr<Color3f>
		// hasn't been defined
		s << " ( " << it->first << ", " << IECorePython::repr( const_cast<typename T::YType&>( it->second ) ) << " )";
		if( i!=l-1 )
		{
			s << ",";
		}
	}
	s << "), ";
	const std::string interpStr = extract<std::string>( object( ramp.interpolation ).attr( "__str__")() );
	s << "IECore.RampInterpolation." << interpStr;
	s << ")";
	return s.str();
}

template<typename T>
T *rampConstruct( object o, const RampInterpolation &interpolation )
{
	typename T::PointContainer points;
	int s = extract<int>( o.attr( "__len__" )() );
	for( int i=0; i<s; i++ )
	{
		object e = o[i];
		int es = extract<int>( e.attr( "__len__" )() );
		if( es!=2 )
		{
			throw IECore::Exception( "Each entry in the point sequence must contain two values." );
		}
		object xo = e[0];
		object yo = e[1];
		float x = extract<float>( xo );
		typename T::YType y = extract<typename T::YType>( yo );
		points.insert( typename T::PointContainer::value_type( x, y ) );
	}
	return new T( points, interpolation );
}

template<typename T>
boost::python::tuple rampPoints( const T &s )
{
	boost::python::list p;
	typename T::PointContainer::const_iterator it;
	for( it=s.points.begin(); it!=s.points.end(); it++ )
	{
		p.append( make_tuple( it->first, it->second ) );
	}
	return boost::python::tuple( p );
}

template<typename T>
void bindRampTemplate( const char *name)
{
    class_<T>( name )
        .def( "__init__", make_constructor( &rampConstruct<T> ) )
        .def( "__repr__", &rampRepr<T> )
        .def( "points", &rampPoints<T>, "Read only access to the control points as a tuple of tuples of ( x, y ) pairs." )
        .def_readwrite("interpolation", &T::interpolation)
        .def( self==self )
        .def( self!=self )
        .def( "evaluator", &T::evaluator )
    ;
}


void bindRamp()
{
    enum_<RampInterpolation>( "RampInterpolation" )
        .value( "Linear", RampInterpolation::Linear )
        .value( "CatmullRom", RampInterpolation::CatmullRom )
        .value( "BSpline", RampInterpolation::BSpline )
        .value( "MonotoneCubic", RampInterpolation::MonotoneCubic )
        .value( "Constant", RampInterpolation::Constant )
    ;

    bindRampTemplate<Rampff >( "Rampff" );
    bindRampTemplate<RampfColor3f >( "RampfColor3f" );
    bindRampTemplate<RampfColor4f >( "RampfColor4f" );
}

}
