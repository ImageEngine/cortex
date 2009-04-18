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

#ifndef IE_COREPYTHON_PARAMETERBINDING_H
#define IE_COREPYTHON_PARAMETERBINDING_H

#include <boost/python.hpp>

#include "IECore/Object.h"
#include "IECore/Parameter.h"

#include "IECore/bindings/RunTimeTypedBinding.h"

namespace IECore
{

void bindParameter();

// Exposed so it can be used in the bindings for the other Parameter types.
template<class T>
T parameterPresets( const boost::python::object &o );

/// The following macros and functions provide a good example to follow in trying to
/// wrap non trivial C++ objects so they can be derived from in Python, as they solve
/// some issues the boost python documentation is silent about.
///
/// The first macro simply defines a virtual override for valueValid(), so that calls coming
/// from the C++ side will be forwarded on to the python reimplementation in the python
/// derived class. This is pretty standard and follows the boost documentation examples.
/// The macro is provided as every class to be used as a base class for python subclassing
/// must define the override, and we don't want to define the same code over and over in
/// each of the wrapping classes.
///
/// The next bit is more interesting. We'd like to simply bind the virtual Parameter::valueValid()
/// function in to python at the Parameter base class, and leave it at that. That works fine
/// until it becomes necessary for a python subclass to call the base class implementation
/// of valueValid() (this is part of the definition for how valueValid() should be implemented).
/// At this point the binding would resolve to the virtual function, which would fall through
/// to the most derived class implementation, which would forward it into python, creating
/// an infinite loop of recursive calls to the python valueValid() implementation. So instead
/// of binding the virtual valueValid() function at the base class level, we have to bind
/// valueValid repeatedly in every derived class, this time binding a direct call to the appropriate
/// derived class function (statically, not virtually). The second macro is used to achieve that.

/// Use this within the definition of a Wrapper class for a Parameter derived class.
/// It defines virtual overrides to forward calls into python as appropriate.
/// See src/bindings/PathParameterBinding.cpp for an example of use, and the discussion above
/// for a description of what is going on.
#define IE_COREPYTHON_PARAMETERWRAPPERFNS( CLASSNAME )								\
	virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const	\
	{																				\
		if( override f = this->get_override( "valueValid" ) )								\
		{																			\
			boost::python::tuple r = f( boost::const_pointer_cast<Object>( value ) );		\
			if( reason )															\
			{																		\
				*reason = extract<std::string>( r[1] );								\
			}																		\
			return extract<bool>( r[0] );											\
		}																			\
		return CLASSNAME::valueValid( value, reason );								\
	}

/// Use this within the class bindings to define the valueValid functions in python.
/// See src/bindings/PathParameterBinding.cpp for an example of use, and the discussion above
/// for a description of what is going on.
#define IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( CLASSNAME )																								\
	def( "valueValid", &valueValid<CLASSNAME>, "Returns a tuple containing a bool specifying validity and a string giving a reason for invalidity." )	\
	.def( "valueValid", &valueValid2, "Returns a tuple containing a bool specifying validity and a string giving a reason for invalidity." )			\
		
template<typename T>
boost::python::tuple valueValid( const T &that, ConstObjectPtr value );

boost::python::tuple valueValid2( const Parameter &that );

}

#include "IECore/bindings/ParameterBinding.inl"

#endif // IE_COREPYTHON_PARAMETERBINDING_H
