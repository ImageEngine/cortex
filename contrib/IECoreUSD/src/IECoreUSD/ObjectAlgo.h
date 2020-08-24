//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#ifndef IECOREUSD_OBJECTALGO_H
#define IECOREUSD_OBJECTALGO_H

#include "IECore/Object.h"

#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"

namespace IECoreUSD
{

namespace ObjectAlgo
{

// Reading from USD
// ================

bool canReadObject( const pxr::UsdPrim &prim );
IECore::ObjectPtr readObject( const pxr::UsdPrim &prim, pxr::UsdTimeCode time );
bool objectMightBeTimeVarying( const pxr::UsdPrim &prim );

// Writing to USD
// ==============

void writeObject( const IECore::Object *object, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time );

// Reader/Writer registrations
// ===========================

using Reader = std::function<IECore::ObjectPtr ( const pxr::UsdPrim &prim, pxr::UsdTimeCode time )>;
using MightBeTimeVarying = std::function<bool ( const pxr::UsdPrim &prim )>;
using Writer = std::function<void ( const IECore::Object *object, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )>;

void registerReader( const pxr::TfToken &schemaType, Reader reader, MightBeTimeVarying mightBeTimeVarying );
void registerWriter( const IECore::TypeId typeId, Writer writer );

template<typename SchemaType>
struct ReaderDescription
{

	using Reader = IECore::ObjectPtr (*)( SchemaType &schema, pxr::UsdTimeCode time );
	using MightBeTimeVarying = bool (*)( SchemaType &schema );

	ReaderDescription( pxr::TfToken schemaType, Reader reader, MightBeTimeVarying mightBeTimeVarying )
	{
		registerReader(
			/// \todo Query the name automatically when we're on modern enough USD :
			/// ```
			/// pxr::UsdSchemaRegistry::GetSchemaTypeName(
			/// 	pxr::TfType::Find<SchemaType>().GetTypeName()
			/// )
			///	```
			schemaType,
			[reader]( const pxr::UsdPrim &prim, pxr::UsdTimeCode time )
			{
				SchemaType schema( prim );
				return reader( schema, time );
			},
			[mightBeTimeVarying]( const pxr::UsdPrim &prim )
			{
				SchemaType schema( prim );
				return mightBeTimeVarying( schema );
			}
		);
	}

};

template<typename ObjectType>
struct WriterDescription
{

	using Writer = void (*)( const ObjectType *object, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time );

	WriterDescription( Writer writer )
	{
		registerWriter(
			ObjectType::staticTypeId(),
			[writer]( const IECore::Object *object, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
			{
				writer(
					static_cast<const ObjectType *>( object ),
					stage, path, time
				);
			}
		);
	}

};

} // namespace ObjectAlgo

} // namespace IECoreUSD

#endif // IECOREUSD_OBJECTALGO_H
