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

#include "IECoreScene/FaceVaryingPromotionOp.h"
#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/private/PrimitiveAlgoUtils.h"
#include "IECoreScene/private/PrimitiveVariableAlgos.h"

#include "IECore/DataAlgo.h"
#include "IECore/DespatchTypedData.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Resample Primitive Variables
//////////////////////////////////////////////////////////////////////////

namespace
{

struct MeshVertexToUniform
{
	typedef DataPtr ReturnType;

	MeshVertexToUniform( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		// TODO sipmlify this code by using polygon face & vertex iterators
		trg.reserve( m_mesh->numFaces() );

		std::vector<int>::const_iterator vId = m_mesh->vertexIds()->readable().begin();
		const std::vector<int> &verticesPerFace = m_mesh->verticesPerFace()->readable();
		for( std::vector<int>::const_iterator it = verticesPerFace.begin(); it < verticesPerFace.end(); ++it )
		{
			// initialize with the first value to avoid
			// ambiguitity during default construction
			typename From::ValueType::value_type total = src[ *vId ];
			++vId;

			for( int j = 1; j < *it; ++j, ++vId )
			{
				total += src[ *vId ];
			}

			trg.push_back( total / *it );
		}

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshUniformToVertex
{
	typedef DataPtr ReturnType;

	MeshUniformToVertex( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		size_t numVerts = m_mesh->variableSize( PrimitiveVariable::Vertex );
		std::vector<int> count( numVerts, 0 );
		trg.resize( numVerts, typename From::ValueType::value_type( 0.0f ) );

		typename From::ValueType::const_iterator srcIt = src.begin();
		std::vector<int>::const_iterator vId = m_mesh->vertexIds()->readable().begin();
		const std::vector<int> &verticesPerFace = m_mesh->verticesPerFace()->readable();
		for( std::vector<int>::const_iterator it = verticesPerFace.begin(); it < verticesPerFace.end(); ++it, ++srcIt )
		{
			for( int j = 0; j < *it; ++j, ++vId )
			{
				trg[ *vId ] += *srcIt;
				++count[ *vId ];
			}
		}

		std::vector<int>::const_iterator cIt = count.begin();
		typename From::ValueType::iterator trgIt = trg.begin(), trgEnd = trg.end();
		for( trgIt = trg.begin(); trgIt != trgEnd ; ++trgIt, ++cIt )
		{
			*trgIt /= *cIt;
		}

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshFaceVaryingToVertex
{
	typedef DataPtr ReturnType;

	MeshFaceVaryingToVertex( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		size_t numVerts = m_mesh->variableSize( PrimitiveVariable::Vertex );
		std::vector<int> count( numVerts, 0 );
		trg.resize( numVerts, typename From::ValueType::value_type( 0.0f ) );

		const std::vector<int>& vertexIds = m_mesh->vertexIds()->readable();
		std::vector<int>::const_iterator vertexIdIt = vertexIds.begin();
		typename From::ValueType::const_iterator srcIt = src.begin(), srcEnd = src.end();

		for( ; srcIt != srcEnd ; ++srcIt, ++vertexIdIt )
		{
			trg[ *vertexIdIt ] += *srcIt;
			++count[ *vertexIdIt ];
		}

		std::vector<int>::const_iterator cIt = count.begin();
		typename From::ValueType::iterator trgIt = trg.begin(), trgEnd = trg.end();
		for( trgIt = trg.begin(); trgIt != trgEnd ; ++trgIt, ++cIt )
		{
			*trgIt /= *cIt;
		}

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshFaceVaryingToUniform
{
	typedef DataPtr ReturnType;

	MeshFaceVaryingToUniform( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		trg.reserve( m_mesh->numFaces() );

		typename From::ValueType::const_iterator srcIt = src.begin();

		const std::vector<int> &verticesPerFace = m_mesh->verticesPerFace()->readable();
		for( std::vector<int>::const_iterator it = verticesPerFace.begin(); it < verticesPerFace.end(); ++it )
		{
			// initialize with the first value to avoid
			// ambiguity during default construction
			typename From::ValueType::value_type total = *srcIt;
			++srcIt;

			for( int j = 1; j < *it; ++j, ++srcIt )
			{
				total += *srcIt;
			}

			trg.push_back( total / *it );
		}

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshAnythingToFaceVarying
{
	typedef DataPtr ReturnType;

	MeshAnythingToFaceVarying( const MeshPrimitive *mesh, PrimitiveVariable::Interpolation srcInterpolation )
		: m_mesh( mesh ), m_srcInterpolation( srcInterpolation )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{

		// TODO replace the call to the IECore::FaceVaryingPromotionOpPtr and include the logic in this file.

		// we need to duplicate because the Op expects a primvar to manipulate..
		MeshPrimitivePtr tmpMesh = m_mesh->copy();
		// cast OK due to read-only access.
		tmpMesh->variables["tmpPrimVar"] = PrimitiveVariable( m_srcInterpolation, const_cast< From * >(data) );
		FaceVaryingPromotionOpPtr promoteOp = new FaceVaryingPromotionOp();
		promoteOp->inputParameter()->setValue( tmpMesh );
		IECore::StringVectorDataPtr names = new StringVectorData();
		names->writable().push_back( "tmpPrimVar" );
		promoteOp->primVarNamesParameter()->setValue( names );
		ReturnType result = runTimeCast< MeshPrimitive >( promoteOp->operate() )->variables["tmpPrimVar"].data;


		return result;
	}

	const MeshPrimitive *m_mesh;
	PrimitiveVariable::Interpolation m_srcInterpolation;
};

} // namespace

void IECoreScene::MeshAlgo::resamplePrimitiveVariable( const MeshPrimitive *mesh, PrimitiveVariable& primitiveVariable, PrimitiveVariable::Interpolation interpolation )
{
	PrimitiveVariable::Interpolation srcInterpolation = primitiveVariable.interpolation;
	if ( srcInterpolation == interpolation )
	{
		return;
	}

	DataPtr dstData = nullptr;
	DataPtr srcData = nullptr;

	if( primitiveVariable.indices )
	{
		if( srcInterpolation < interpolation )
		{
			// upsampling can be a resampling of indices
			srcData = primitiveVariable.indices;
		}
		else
		{
			// downsampling forces index expansion to
			// simplify the algorithms.
			// \todo: allow indices to be maintained.
			srcData = primitiveVariable.expandedData();
			primitiveVariable.indices = nullptr;
		}
	}
	else
	{
		// with no indices we can just resample the data
		srcData = primitiveVariable.data;
	}

	// average array to single value
	if ( interpolation == PrimitiveVariable::Constant )
	{
		Detail::AverageValueFromVector fn;
		dstData = dispatch( srcData.get(), fn );
		primitiveVariable = PrimitiveVariable( interpolation, dstData );
		return;
	}

	if ( primitiveVariable.interpolation == PrimitiveVariable::Constant )
	{
		DataPtr arrayData = Detail::createArrayData(primitiveVariable, mesh, interpolation);
		if (arrayData)
		{
			primitiveVariable = PrimitiveVariable(interpolation, arrayData);
		}
		return;
	}

	if( interpolation == PrimitiveVariable::Uniform )
	{
		if( srcInterpolation == PrimitiveVariable::Varying || srcInterpolation == PrimitiveVariable::Vertex )
		{
			MeshVertexToUniform fn( mesh );
			dstData = despatchTypedData<MeshVertexToUniform, Detail::IsArithmeticVectorTypedData>( srcData.get(), fn );
		}
		else if( srcInterpolation == PrimitiveVariable::FaceVarying )
		{
			MeshFaceVaryingToUniform fn( mesh );
			dstData = despatchTypedData<MeshFaceVaryingToUniform, Detail::IsArithmeticVectorTypedData>( srcData.get(), fn );
		}
	}
	else if( interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::Vertex )
	{
		if( srcInterpolation == PrimitiveVariable::Uniform )
		{
			MeshUniformToVertex fn( mesh );
			dstData = despatchTypedData<MeshUniformToVertex, Detail::IsArithmeticVectorTypedData>( srcData.get(), fn );
		}
		else if( srcInterpolation == PrimitiveVariable::FaceVarying )
		{
			MeshFaceVaryingToVertex fn( mesh );
			dstData = despatchTypedData<MeshFaceVaryingToVertex, Detail::IsArithmeticVectorTypedData>( srcData.get(), fn );
		}
		else if( srcInterpolation == PrimitiveVariable::Varying || srcInterpolation == PrimitiveVariable::Vertex )
		{
			dstData = srcData;
		}
	}
	else if( interpolation == PrimitiveVariable::FaceVarying )
	{
		MeshAnythingToFaceVarying fn( mesh, srcInterpolation );
		dstData = despatchTypedData<MeshAnythingToFaceVarying, Detail::IsArithmeticVectorTypedData>( srcData.get(), fn );
	}

	if( primitiveVariable.indices )
	{
		primitiveVariable = PrimitiveVariable( interpolation, primitiveVariable.data, runTimeCast<IntVectorData>( dstData ) );
	}
	else
	{
		primitiveVariable = PrimitiveVariable( interpolation, dstData );
	}
}
