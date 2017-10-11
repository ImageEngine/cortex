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

#ifndef IECOREPYTHON_READERBINDING_H
#define IECOREPYTHON_READERBINDING_H

#include "IECore/CompoundObject.h"
#include "IECore/Reader.h"

#include "IECorePython/Export.h"
#include "IECorePython/OpBinding.h"

namespace IECorePython
{

/// A class to simplify the binding of Reader derived classes.
template<typename T, typename TWrapper=T>
class ReaderClass : public OpClass<T, TWrapper>
{
	public :

		ReaderClass( const char *docString = nullptr );

};

/// A class for wrapping Reader to allow overriding in Python.
template<typename T>
class ReaderWrapper : public OpWrapper<IECore::Reader>
{
	public :

		ReaderWrapper( PyObject *self, const std::string &description )
			: OpWrapper<Reader>( self, description )
		{
		};

		virtual IECore::CompoundObjectPtr readHeader()
		{
			if( this->isSubclassed() )
			{
				ScopedGILLock gilLock;
				boost::python::object o = this->methodOverride( "readHeader" );
				if( o )
				{
					IECore::CompoundObjectPtr r = boost::python::extract<IECore::CompoundObjectPtr>( o() );
					if( !r )
					{
						throw IECore::Exception( "readHeader() python method didn't return a CompoundObject." );
					}
					return r;
				}
			}

			return T::readHeader();
		}

};

IECOREPYTHON_API void bindReader();

}

#include "IECorePython/ReaderBinding.inl"

#endif // IECOREPYTHON_READERBINDING_H
