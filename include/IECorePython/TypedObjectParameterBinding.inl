//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREPYTHON_TYPEDOBJECTPARAMETERBINDING_INL
#define IECOREPYTHON_TYPEDOBJECTPARAMETERBINDING_INL

#include <string>

#include "IECorePython/Wrapper.h"
#include "IECorePython/ParameterBinding.h"

#include "IECore/TypedObjectParameter.h"

namespace IECorePython
{

template<typename T>
class TypedObjectParameterWrap : public IECore::TypedObjectParameter<T>, public Wrapper< IECore::TypedObjectParameter<T> >
{
	public:

		IE_CORE_DECLAREMEMBERPTR( TypedObjectParameterWrap<T> );

		TypedObjectParameterWrap( PyObject *self, const std::string &n, const std::string &d, typename T::Ptr dv, const boost::python::object &p = boost::python::tuple(), bool po = false, IECore::CompoundObjectPtr ud = 0 )
			:	IECore::TypedObjectParameter<T>( n, d, dv, parameterPresets<typename IECore::TypedObjectParameter<T>::ObjectPresetsContainer>( p ), po, ud ), Wrapper< IECore::TypedObjectParameter<T> >( self, this ) {};

		IECOREPYTHON_PARAMETERWRAPPERFNS( IECore::TypedObjectParameter<T> );
};

}

#endif // IECOREPYTHON_TYPEDOBJECTPARAMETERBINDING_INL
