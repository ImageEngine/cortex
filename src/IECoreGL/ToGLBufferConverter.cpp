//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/ToGLBufferConverter.h"

#include "IECoreGL/Buffer.h"

#include "IECore/Data.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;
using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLBufferConverter );

ToGLConverter::ConverterDescription<ToGLBufferConverter> ToGLBufferConverter::g_description;

namespace IECoreGL
{
namespace Detail
{

/// \todo Would this be useful in DespatchTypedData.h?
struct TypedDataBytes
{

	typedef size_t ReturnType;

	template<typename T>
	ReturnType operator()( typename T::ConstPtr data ) const
	{
		return sizeof( typename T::BaseType ) * data->baseSize();
	}

};

} // namespace Detail
} // namespace IECoreGL

ToGLBufferConverter::ToGLBufferConverter( IECore::ConstDataPtr toConvert )
	:	ToGLConverter( "Converts IECore::Data objects to IECoreGL::Buffer objects.", IECore::DataTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::Data>( toConvert ) );
}

ToGLBufferConverter::~ToGLBufferConverter()
{
}

IECore::RunTimeTypedPtr ToGLBufferConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECore::ConstDataPtr data = boost::static_pointer_cast<const IECore::Data>( src ); // safe because the parameter validated it for us

	const void *address = despatchTypedData<TypedDataAddress, TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( data.get() ) );
	size_t size = despatchTypedData<Detail::TypedDataBytes, TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( data.get() ) );

	return new Buffer( address, size );
}
