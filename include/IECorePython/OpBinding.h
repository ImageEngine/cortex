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

#ifndef IECOREPYTHON_OPBINDING_H
#define IECOREPYTHON_OPBINDING_H

#include "IECorePython/Export.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "IECore/CompoundParameter.h"

namespace IECorePython
{

/// A class to simplify the binding of Op derived classes.
template<typename T, typename TWrapper=T>
class OpClass : public IECorePython::RunTimeTypedClass<T, TWrapper>
{
	public :

		OpClass( const char *docString = nullptr )
			:	IECorePython::RunTimeTypedClass<T, TWrapper>( docString )
		{
		}

};

/// A class for wrapping Ops to allow overriding in Python.
template<typename T>
class OpWrapper : public IECorePython::RunTimeTypedWrapper<T>
{

	public :

		OpWrapper( PyObject *self, const std::string &description )
			:	IECorePython::RunTimeTypedWrapper<T>( self, description )
		{
		}

		OpWrapper( PyObject *self, const std::string &description, IECore::ParameterPtr resultParameter )
			:	IECorePython::RunTimeTypedWrapper<T>( self, description, resultParameter )
		{
		}

		OpWrapper( PyObject *self, const std::string &description, IECore::CompoundParameterPtr compoundParameter, IECore::ParameterPtr resultParameter )
			:	IECorePython::RunTimeTypedWrapper<T>( self, description, compoundParameter, resultParameter )
		{
		}

		IECore::ObjectPtr doOperation( const IECore::CompoundObject * operands ) override
		{
			ScopedGILLock gilLock;
			boost::python::object o = this->methodOverride( "doOperation" );
			if( o )
			{
				IECore::ObjectPtr r = boost::python::extract<IECore::ObjectPtr>( o( IECore::CompoundObjectPtr( const_cast<IECore::CompoundObject *>( operands ) ) ) );
				if( !r )
				{
					throw IECore::Exception( "doOperation() python method didn't return an Object." );
				}
				return r;
			}
			else
			{
				throw IECore::Exception( "doOperation() python method not defined" );
			}
		}

};

IECOREPYTHON_API void bindOp();

}

#endif // IECOREPYTHON_OPBINDING_H
