//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREPYTHON_PARAMETERBINDING_H
#define IECOREPYTHON_PARAMETERBINDING_H

#include <boost/python.hpp>

#include "IECore/Object.h"
#include "IECore/Parameter.h"

#include "IECorePython/Export.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILLock.h"

namespace IECorePython
{

// Exposed so it can be used in the bindings for the other Parameter types.
template<class T>
IECOREPYTHON_API T parameterPresets( const boost::python::object &o );

/// A class to simplify the binding of Parameter derived classes.
template<typename T, typename TWrapper=T>
class ParameterClass : public IECorePython::RunTimeTypedClass<T, TWrapper>
{
	public :

		ParameterClass( const char *docString = 0 );

};

/// A class for wrapping Parameter objects to allow overriding in Python.
template<typename T>
class ParameterWrapper : public IECorePython::RunTimeTypedWrapper<T>
{

	public :

		ParameterWrapper(
			PyObject *self, const std::string &name, const std::string &description, IECore::ObjectPtr defaultValue,
			const boost::python::object &presets = boost::python::tuple(), bool presetsOnly = false, IECore::CompoundObjectPtr userData = 0
		)
			:	RunTimeTypedWrapper<T>( self, name, description, defaultValue, parameterPresets<typename T::PresetsContainer>( presets ), presetsOnly, userData )
		{
		};

		ParameterWrapper( PyObject *self, const std::string &name, const std::string &description, IECore::ObjectPtr defaultValue, IECore::CompoundObjectPtr userData )
			:	RunTimeTypedWrapper<T>( self, name, description, defaultValue, IECore::Parameter::PresetsContainer(), false, userData )
		{
		};

		ParameterWrapper( PyObject *self )
			:	IECorePython::RunTimeTypedWrapper<T>( self )
		{
		}

		/// \todo Once we require c++11, replace all this with a single variadic template.
		template<typename Arg1>
		ParameterWrapper( PyObject *self, Arg1 arg1 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1 )
		{
		}

		template<typename Arg1, typename Arg2>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3, arg4 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3, arg4, arg5 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3, arg4, arg5, arg6 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3, arg4, arg5, arg6, arg7 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 )
		{
		}

		template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
		ParameterWrapper( PyObject *self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10 )
			:	IECorePython::RunTimeTypedWrapper<T>( self, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 )
		{
		}

		virtual bool valueValid( const IECore::Object *value, std::string *reason = NULL ) const
		{
			if( this->isSubclassed() )
			{
				ScopedGILLock gilLock;
				if( boost::python::object f = this->methodOverride( "valueValid" ) )
				{
					boost::python::tuple r = boost::python::extract<boost::python::tuple>( f( IECore::ObjectPtr( const_cast<IECore::Object *>( value ) ) ) );
					if( reason )
					{
						*reason = boost::python::extract<std::string>( r[1] );
					}
					return boost::python::extract<bool>( r[0] );
				}
			}

			return T::valueValid( value, reason );
		}

};

IECOREPYTHON_API void bindParameter();

}

#include "IECorePython/ParameterBinding.inl"

#endif // IECOREPYTHON_PARAMETERBINDING_H
