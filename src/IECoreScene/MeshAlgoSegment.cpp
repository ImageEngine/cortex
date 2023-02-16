//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/DataAlgo.h"

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/private/PrimitiveAlgoUtils.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

std::vector<MeshPrimitivePtr> IECoreScene::MeshAlgo::segment( const MeshPrimitive *mesh, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues, const Canceller *canceller )
{
	MeshSplitter ms( mesh, primitiveVariable, canceller );
	int numMeshes = ms.numMeshes();
	std::vector<MeshPrimitivePtr> ret;
	ret.reserve( numMeshes );

	if( !segmentValues )
	{
		for( int i = 0; i < numMeshes; i++ )
		{
			ret.push_back( ms.mesh( i ) );
		}
	}
	else
	{
		IECore::dispatch( segmentValues,
			[ ms, numMeshes, &ret, canceller ]( auto *typedSegmentValues )
			{
				using DataType = typename std::remove_pointer_t< decltype( typedSegmentValues ) >;
				if constexpr ( TypeTraits::IsVectorTypedData<DataType>::value )
				{
					using ElementType = typename DataType::ValueType::value_type;

					std::unordered_map< ElementType, int > idMap;
					Canceller::check( canceller );
					for( int i = 0; i < numMeshes; i++ )
					{
						idMap[ ms.value<ElementType>( i ) ] = i;
					}

					Canceller::check( canceller );
					for( const auto &i : typedSegmentValues->readable() )
					{
						auto f = idMap.find( i );
						if( f == idMap.end() )
						{
							ret.push_back( nullptr );
						}
						else
						{
							ret.push_back( ms.mesh( idMap[i] ) );
						}
					}
				}
				else
				{
					throw IECore::Exception( "Invalid Primitive Variable with non-vector typed data." );
				}
			}
		);
	}

	return ret;
}
