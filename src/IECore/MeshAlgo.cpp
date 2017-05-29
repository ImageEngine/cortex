#include <assert.h>
#include <algorithm>

#include "boost/mpl/and.hpp"
#include "OpenEXR/ImathVec.h"

#include "IECore/MeshAlgo.h"
#include "IECore/FaceVaryingPromotionOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"
#include "IECore/private/PrimitiveAlgoUtils.h"

using namespace IECore;
using namespace Imath;

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
		trg.resize( numVerts );

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
		trg.resize( numVerts );

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
		IECore::MeshPrimitivePtr tmpMesh = m_mesh->copy();
		// cast OK due to read-only access.
		tmpMesh->variables["tmpPrimVar"] = IECore::PrimitiveVariable( m_srcInterpolation, const_cast< From * >(data) );
		IECore::FaceVaryingPromotionOpPtr promoteOp = new IECore::FaceVaryingPromotionOp();
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

class TexturePrimVarNames
{
	public:
		TexturePrimVarNames( const std::string &uvSetName );

		std::string uvSetName;

		std::string uName() const;
		std::string vName() const;

		std::string indicesName() const;
};

TexturePrimVarNames::TexturePrimVarNames( const std::string &uvSetName ) : uvSetName( uvSetName )
{
};

std::string TexturePrimVarNames::uName() const
{
	if( uvSetName == "st" )
	{
		return "s";
	}

	return uvSetName + "_s";
}

std::string TexturePrimVarNames::vName() const
{
	if( uvSetName == "st" )
	{
		return "t";
	}

	return uvSetName + "_t";
}

std::string TexturePrimVarNames::indicesName() const
{
	return uvSetName + "Indices";
}

} // anonymous namespace

namespace IECore
{

namespace MeshAlgo
{

std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangents(
	const MeshPrimitive *mesh,
	const std::string &uvSet, /* = "st" */
	bool orthoTangents, /* = true */
	const std::string &position /* = "P" */
)
{
	if( mesh->minVerticesPerFace() != 3 || mesh->maxVerticesPerFace() != 3 )
	{
		throw InvalidArgumentException( "MeshAlgo::calculateTangents : MeshPrimitive must only contain triangles" );
	}

	const TexturePrimVarNames texturePrimVarNames( uvSet );

	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position );
	if( !positionData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangents : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}

	const V3fVectorData::ValueType &points = positionData->readable();

	const IntVectorData *vertsPerFaceData = mesh->verticesPerFace();
	const IntVectorData::ValueType &vertsPerFace = vertsPerFaceData->readable();

	const IntVectorData *vertIdsData = mesh->vertexIds();
	const IntVectorData::ValueType &vertIds = vertIdsData->readable();

	const IntVectorData *stIndicesData = mesh->variableData<IntVectorData>( texturePrimVarNames.indicesName() );
	if( !stIndicesData )
	{
		// I'm a little unsure about using the vertIds for the stIndices.
		stIndicesData = vertIdsData;
	}
	const IntVectorData::ValueType &stIndices = stIndicesData->readable();

	ConstFloatVectorDataPtr uData = mesh->variableData<FloatVectorData>( texturePrimVarNames.uName(), PrimitiveVariable::FaceVarying );
	if( !uData )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::calculateTangents : MeshPrimitive has no FaceVarying FloatVectorData primitive variable named \"%s\"."  ) % ( texturePrimVarNames.uName() ) ).str() );
	}

	ConstFloatVectorDataPtr vData = mesh->variableData<FloatVectorData>( texturePrimVarNames.vName(), PrimitiveVariable::FaceVarying );
	if( !vData )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::calculateTangents : MeshPrimitive has no FaceVarying FloatVectorData primitive variable named \"%s\"."  ) % ( texturePrimVarNames.vName() ) ).str() );
	}

	const FloatVectorData::ValueType &u = uData->readable();
	const FloatVectorData::ValueType &v = vData->readable();


	// the uvIndices array is indexed as with any other facevarying data. the values in the
	// array specify the connectivity of the uvs - where two facevertices have the same index
	// they are known to be sharing a uv. for each one of these unique indices we compute
	// the tangents and normal, by accumulating all the tangents and normals for the faces
	// that reference them. we then take this data and shuffle it back into facevarying
	// primvars for the mesh.
	int numUniqueTangents = 1 + *std::max_element( stIndices.begin(), stIndices.end() );

	std::vector<V3f> uTangents( numUniqueTangents, V3f( 0 ) );
	std::vector<V3f> vTangents( numUniqueTangents, V3f( 0 ) );
	std::vector<V3f> normals( numUniqueTangents, V3f( 0 ) );

	for( size_t faceIndex = 0; faceIndex < vertsPerFace.size(); faceIndex++ )
	{
		assert( vertsPerFace[faceIndex] == 3 );

		// indices into the facevarying data for this face
		size_t fvi0 = faceIndex * 3;
		size_t fvi1 = fvi0 + 1;
		size_t fvi2 = fvi1 + 1;
		assert( fvi2 < vertIds.size() );
		assert( fvi2 < u.size() );
		assert( fvi2 < v.size() );

		// positions for each vertex of this face
		const V3f &p0 = points[vertIds[fvi0]];
		const V3f &p1 = points[vertIds[fvi1]];
		const V3f &p2 = points[vertIds[fvi2]];

		// uv coordinates for each vertex of this face
		const V2f uv0( u[fvi0], v[fvi0] );
		const V2f uv1( u[fvi1], v[fvi1] );
		const V2f uv2( u[fvi2], v[fvi2] );

		// compute tangents and normal for this face
		const V3f e0 = p1 - p0;
		const V3f e1 = p2 - p0;

		const V2f e0uv = uv1 - uv0;
		const V2f e1uv = uv2 - uv0;

		V3f tangent = ( e0 * -e1uv.y + e1 * e0uv.y ).normalized();
		V3f bitangent = ( e0 * -e1uv.x + e1 * e0uv.x ).normalized();

		V3f normal = ( p2 - p1 ).cross( p0 - p1 );
		normal.normalize();

		// and accumlate them into the computation so far
		uTangents[stIndices[fvi0]] += tangent;
		uTangents[stIndices[fvi1]] += tangent;
		uTangents[stIndices[fvi2]] += tangent;

		vTangents[stIndices[fvi0]] += bitangent;
		vTangents[stIndices[fvi1]] += bitangent;
		vTangents[stIndices[fvi2]] += bitangent;

		normals[stIndices[fvi0]] += normal;
		normals[stIndices[fvi1]] += normal;
		normals[stIndices[fvi2]] += normal;

	}

	// normalize and orthogonalize everything
	for( size_t i = 0; i < uTangents.size(); i++ )
	{
		normals[i].normalize();

		uTangents[i].normalize();
		vTangents[i].normalize();

		// Make uTangent/vTangent orthogonal to normal
		uTangents[i] -= normals[i] * uTangents[i].dot( normals[i] );
		vTangents[i] -= normals[i] * vTangents[i].dot( normals[i] );

		uTangents[i].normalize();
		vTangents[i].normalize();

		if( orthoTangents )
		{
			vTangents[i] -= uTangents[i] * vTangents[i].dot( uTangents[i] );
			vTangents[i].normalize();
		}

		// Ensure we have set of basis vectors (n, uT, vT) with the correct handedness.
		if( uTangents[i].cross( vTangents[i] ).dot( normals[i] ) < 0.0f )
		{
			uTangents[i] *= -1.0f;
		}
	}

	// convert the tangents back to facevarying data and add that to the mesh
	V3fVectorDataPtr fvUD = new V3fVectorData();
	V3fVectorDataPtr fvVD = new V3fVectorData();

	std::vector<V3f> &fvU = fvUD->writable();
	std::vector<V3f> &fvV = fvVD->writable();
	fvU.resize( stIndices.size() );
	fvV.resize( stIndices.size() );

	for( unsigned i = 0; i < stIndices.size(); i++ )
	{
		fvU[i] = uTangents[stIndices[i]];
		fvV[i] = vTangents[stIndices[i]];
	}

	PrimitiveVariable tangentPrimVar( PrimitiveVariable::FaceVarying, fvUD );
	PrimitiveVariable bitangentPrimVar( PrimitiveVariable::FaceVarying, fvVD );

	return std::make_pair( tangentPrimVar, bitangentPrimVar );
}

void resamplePrimitiveVariable( const MeshPrimitive *mesh, PrimitiveVariable& primitiveVariable, PrimitiveVariable::Interpolation interpolation )
{
	Data *srcData = primitiveVariable.data.get();
	DataPtr dstData;

	PrimitiveVariable::Interpolation srcInterpolation = primitiveVariable.interpolation;

	if ( srcInterpolation == interpolation )
	{
		return;
	}

	// average array to single value
	if ( interpolation == PrimitiveVariable::Constant )
	{
		Detail::AverageValueFromVector fn;
		dstData = despatchTypedData<Detail::AverageValueFromVector, Detail::IsArithmeticVectorTypedData>( srcData, fn );
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
			dstData = despatchTypedData<MeshVertexToUniform, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
		else if( srcInterpolation == PrimitiveVariable::FaceVarying )
		{
			MeshFaceVaryingToUniform fn( mesh );
			dstData = despatchTypedData<MeshFaceVaryingToUniform, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
	}
	else if( interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::Vertex )
	{
		if( srcInterpolation == PrimitiveVariable::Uniform )
		{
			MeshUniformToVertex fn( mesh );
			dstData = despatchTypedData<MeshUniformToVertex, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
		else if( srcInterpolation == PrimitiveVariable::FaceVarying )
		{
			MeshFaceVaryingToVertex fn( mesh );
			dstData = despatchTypedData<MeshFaceVaryingToVertex, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
		else if( srcInterpolation == PrimitiveVariable::Varying || srcInterpolation == PrimitiveVariable::Vertex )
		{
			dstData = srcData;
		}
	}
	else if( interpolation == PrimitiveVariable::FaceVarying )
	{
		MeshAnythingToFaceVarying fn( mesh, srcInterpolation );
		dstData = despatchTypedData<MeshAnythingToFaceVarying, Detail::IsArithmeticVectorTypedData>( srcData, fn );
	}

	primitiveVariable = PrimitiveVariable( interpolation, dstData );
}

} //namespace MeshAlgo
} //namespace IECore
