//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/TypedPrimitiveOp.h"

#include "IECoreScene/Export.h"
#include "IECoreScene/MeshPrimitive.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ModifyOp.h"
#include "IECore/NullObject.h"
#include "IECore/TypedObjectParameter.h"

#include "boost/static_assert.hpp"

using namespace IECore;
using namespace IECoreScene;

template<typename T>
TypedPrimitiveOp<T>::TypedPrimitiveOp( const std::string &description )
	:	ModifyOp( description, new TypedObjectParameter<T>( "result", "The result", new T() ), new TypedObjectParameter<T>( "input", "The Primitive to modify", new T() ) )
{
}

template<typename T>
TypedPrimitiveOp<T>::~TypedPrimitiveOp()
{
}

template<typename T>
void TypedPrimitiveOp<T>::modify( Object * primitive, const CompoundObject * operands )
{
	T * typedPrimitive = runTimeCast< T, Object >( primitive );

	// Parameter validation should ensure that this is object is of the correct type, hence the assertion
	assert( typedPrimitive );

	modifyTypedPrimitive( typedPrimitive, operands );
}

template<typename T>
const RunTimeTyped::TypeDescription< TypedPrimitiveOp<T> > TypedPrimitiveOp<T>::g_typeDescription;

#define IE_CORE_DEFINETYPEDPRIMITIVEOPSPECIALISATION( T, TNAME ) \
	\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TNAME##TypeId );\
	\
	template class TypedPrimitiveOp<T>;

namespace IECoreScene
{
IE_CORE_DEFINETYPEDPRIMITIVEOPSPECIALISATION( MeshPrimitive, MeshPrimitiveOp );
IE_CORE_DEFINETYPEDPRIMITIVEOPSPECIALISATION( CurvesPrimitive, CurvesPrimitiveOp );
}
