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


#include <numeric>

#include "IECore/PointsAlgo.h"
#include "IECore/DespatchTypedData.h"

#include "IECore/private/PrimitiveAlgoUtils.h"

using namespace IECore;
using namespace Imath;

namespace
{

struct PointsVertexToUniform
{
	typedef DataPtr ReturnType;

	PointsVertexToUniform( const PointsPrimitive *points ) : m_points( points )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		trg.push_back( std::accumulate( src.begin() + 1, src.end(), *src.begin() ) / src.size() );

		return result;
	}

	const PointsPrimitive *m_points;
};

struct PointsUniformToVertex
{
	typedef DataPtr ReturnType;

	PointsUniformToVertex( const PointsPrimitive *points ) : m_points( points )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType::const_iterator srcIt = data->readable().begin();

		trg.resize( m_points->variableSize( PrimitiveVariable::Vertex ), *srcIt );

		return result;
	}

	const PointsPrimitive *m_points;
};

} // anonymous namespace

namespace IECore
{

namespace PointsAlgo
{

void resamplePrimitiveVariable( const PointsPrimitive *points, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation )
{
	DataPtr result;

	if ( primitiveVariable.interpolation == interpolation )
	{
		return;
	}

	if ( interpolation == PrimitiveVariable::Constant )
	{
		IECore::Detail::AverageValueFromVector fn;
		result = despatchTypedData<IECore::Detail::AverageValueFromVector, IECore::Detail::IsArithmeticVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		primitiveVariable = PrimitiveVariable(PrimitiveVariable::Constant, result );
		return;
	}

	if ( primitiveVariable.interpolation == PrimitiveVariable::Constant )
	{
		size_t len = points->variableSize( interpolation );
		switch( primitiveVariable.data->typeId() )
		{
			case IntDataTypeId:
			{
				IntVectorDataPtr newData = new IntVectorData();
				newData->writable().resize( len, static_cast< const IntData * >( primitiveVariable.data.get() )->readable() );
				result = newData;
			}
				break;
			case FloatDataTypeId:
			{
				FloatVectorDataPtr newData = new FloatVectorData();
				newData->writable().resize( len, static_cast< const FloatData * >(primitiveVariable.data.get())->readable() );
				result = newData;
			}
				break;
			case V2fDataTypeId:
			{
				V2fVectorDataPtr newData = new V2fVectorData();
				newData->writable().resize( len, static_cast< const V2fData * >(primitiveVariable.data.get())->readable() );
				result = newData;
			}
				break;
			case V3fDataTypeId:
			{
				V3fVectorDataPtr newData = new V3fVectorData();
				newData->writable().resize( len, static_cast< const V3fData * >(primitiveVariable.data.get())->readable() );
				result = newData;
			}
				break;
			case Color3fDataTypeId:
			{
				Color3fVectorDataPtr newData = new Color3fVectorData();
				newData->writable().resize( len, static_cast< const Color3fData * >(primitiveVariable.data.get())->readable() );
				result = newData;
			}
				break;
			default:
				return;
		}

		primitiveVariable = PrimitiveVariable( interpolation, result );
		return;
	}

	if( interpolation == PrimitiveVariable::Uniform )
	{
		if( primitiveVariable.interpolation == PrimitiveVariable::Vertex || primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			PointsVertexToUniform fn( points );
			result = despatchTypedData<PointsVertexToUniform, IECore::Detail::IsArithmeticVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		}
	}
	else if( interpolation == PrimitiveVariable::Vertex || interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::FaceVarying )
	{
		if( primitiveVariable.interpolation == PrimitiveVariable::Uniform )
		{
			PointsUniformToVertex fn( points );
			result = despatchTypedData<PointsUniformToVertex, TypeTraits::IsNumericBasedVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		}
		else if( primitiveVariable.interpolation == PrimitiveVariable::Vertex || primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			result = primitiveVariable.data;
		}
	}

	primitiveVariable = PrimitiveVariable( interpolation, result );
}

} //namespace PointsAlgo
} //namespace IECore
