//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include <cassert>
#include <algorithm>

#include "boost/format.hpp"
#include "tbb/parallel_sort.h"

#include "IECore/MeshDistortionsOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshDistortionsOp );

MeshDistortionsOp::MeshDistortionsOp() : MeshPrimitiveOp( "Computes distortions( expansions and contractions ) on the mesh edges by comparing P and Pref prim vars. Adds vertex primVar with undirectional distortion, and if the s and t primitive variables are specified then it also computes face-varying primVars for UV mapped distortions. The distortion values are computed as the ratio from the averaged edge lengths and the averaged reference lengths. The values range from (-INF,0) for contraction and (0,INF) for expansion." )
{
	m_pPrimVarNameParameter = new StringParameter(
		"pPrimVarName",	
		"Name of the primitive variable that holds the vertex positions.",
		"P"
	);

	m_pRefPrimVarNameParameter = new StringParameter(
		"pRefPrimVarName",	
		"Name of the primitive variable that holds the reference vertex positions.",
		"Pref"
	);

	m_uPrimVarNameParameter = new StringParameter(
		"uPrimVarName",
		"Name of the primitive variable for u.",
		"s"
	);

	m_vPrimVarNameParameter = new StringParameter(
		"vPrimVarName",
		"Name of the primitive variable for v.",
		"t"
	);
	
	StringParameterPtr uvIndicesPrimVarNameParameter = new StringParameter(
		"uvIndicesPrimVarName",
		"This primitive variable must be FaceVarying IntVectorData, and store indices for the uvs referenced "
		"in the u and v primvars. See IECoreMaya::FromMayaMeshConverter for an example of getting such indices. "
		"You may set this to the empty string if no indices are available, but this will mean that the tangents will "
		"be incorrect across uv boundaries.",
		"stIndices"
	);

	m_distortionPrimVarNameParameter = new StringParameter(
		"distortionPrimVarName",
		"Defines the name of the primvar to receive the undirectional distortion information.",
		"distortion"
	);

	m_uDistortionPrimVarNameParameter = new StringParameter(
		"uDistortionPrimVarName",
		"Defines the name of the primvar to receive the U distortion information.",
		"uDistortion"
	);

	m_vDistortionPrimVarNameParameter = new StringParameter(
		"vDistortionPrimVarName",
		"Defines the name of the primvar to receive the V distortion information.",
		"vDistortion"
	);

	parameters()->addParameter( m_pPrimVarNameParameter );
	parameters()->addParameter( m_pRefPrimVarNameParameter );
	parameters()->addParameter( m_uPrimVarNameParameter );
	parameters()->addParameter( m_vPrimVarNameParameter );
	parameters()->addParameter( uvIndicesPrimVarNameParameter );
	parameters()->addParameter( m_distortionPrimVarNameParameter );
	parameters()->addParameter( m_uDistortionPrimVarNameParameter );
	parameters()->addParameter( m_vDistortionPrimVarNameParameter );
	
}

MeshDistortionsOp::~MeshDistortionsOp()
{
}

StringParameter * MeshDistortionsOp::pPrimVarNameParameter()
{
	return m_pPrimVarNameParameter.get();
}

const StringParameter * MeshDistortionsOp::pPrimVarNameParameter() const
{
	return m_pPrimVarNameParameter.get();
}

StringParameter * MeshDistortionsOp::pRefPrimVarNameParameter()
{
	return m_pRefPrimVarNameParameter.get();
}

const StringParameter * MeshDistortionsOp::pRefPrimVarNameParameter() const
{
	return m_pRefPrimVarNameParameter.get();
}

StringParameter * MeshDistortionsOp::uPrimVarNameParameter()
{
	return m_uPrimVarNameParameter.get();
}

const StringParameter * MeshDistortionsOp::uPrimVarNameParameter() const
{
	return m_uPrimVarNameParameter.get();
}

StringParameter * MeshDistortionsOp::vPrimVarNameParameter()
{
	return m_vPrimVarNameParameter.get();
}

const StringParameter * MeshDistortionsOp::vPrimVarNameParameter() const
{
	return m_vPrimVarNameParameter.get();
}

StringParameter * MeshDistortionsOp::distortionPrimVarNameParameter()
{
	return m_distortionPrimVarNameParameter.get();
}

const StringParameter * MeshDistortionsOp::distortionPrimVarNameParameter() const
{
	return m_distortionPrimVarNameParameter.get();
}

StringParameter * MeshDistortionsOp::uDistortionPrimVarNameParameter()
{
	return m_uDistortionPrimVarNameParameter.get();
}

const StringParameter * MeshDistortionsOp::uDistortionPrimVarNameParameter() const
{
	return m_uDistortionPrimVarNameParameter.get();
}

StringParameter * MeshDistortionsOp::vDistortionPrimVarNameParameter()
{
	return m_vDistortionPrimVarNameParameter.get();
}

const StringParameter * MeshDistortionsOp::vDistortionPrimVarNameParameter() const
{
	return m_vDistortionPrimVarNameParameter.get();
}

struct MeshDistortionsOp::CalculateDistortions
{
	public :
		typedef void ReturnType;
	
		CalculateDistortions( const vector<int> &vertsPerFace, const vector<int> &vertIds, size_t faceVaryingSize, const vector<float> *u, const vector<float> *v, const vector<int> &uvIndices, ConstDataPtr pRefData )
			:	vvDistortionsData(0), fvUDistortionsData(0), fvVDistortionsData(0),
				m_vertsPerFace( vertsPerFace ), m_vertIds( vertIds ), m_faceVaryingSize(faceVaryingSize), m_u( u ), m_v( v ), m_uvIds( uvIndices ), m_pRefData(pRefData)
		{
		}
	
		// this is the data filled in by operator() above, ready to be added onto the mesh
		FloatVectorDataPtr vvDistortionsData;
		FloatVectorDataPtr fvUDistortionsData;
		FloatVectorDataPtr fvVDistortionsData;
	
	private :

		const vector<int> &m_vertsPerFace;
		const vector<int> &m_vertIds;
		const size_t m_faceVaryingSize;
		const vector<float> *m_u;
		const vector<float> *m_v;
		const vector<int> &m_uvIds;
		ConstDataPtr m_pRefData;

		struct UVDistortion
		{
			Imath::V2f distortion;
			int counter;

			UVDistortion() : distortion(0), counter(0) 	{};

			void accumulateDistortion( float dist, const Imath::V2f &uv )
			{
				distortion.x += fabs( uv.x ) * dist;
				distortion.y += fabs( uv.y ) * dist;
				counter++;
			}
		};
		std::vector< UVDistortion > m_uvDistortions;

		struct VertexDistortion 
		{
			float distortion;
			int counter;

			VertexDistortion() : distortion(0), counter(0) 	{};

			void accumulateDistortion( float dist )
			{
				distortion += dist;
				counter++;
			}
		};
		std::vector< VertexDistortion > m_distortions;

	public :

		template<typename T>
		ReturnType operator()( T * data )
		{
			typedef typename T::ValueType VecContainer;
			typedef typename VecContainer::value_type Vec;
			const T * refData = (T*)m_pRefData.get();
			const VecContainer &points = data->readable();
			const VecContainer &refPoints = refData->readable();
			bool computeUV =  ( m_u && m_v );

			m_distortions.clear();
			m_distortions.resize( points.size() );

			// uses the uvIndices array in the same way MeshTangentsOp does...
			int numUniqueTangents = 1 + *max_element( m_uvIds.begin(), m_uvIds.end() );

			if ( computeUV )
			{
				m_uvDistortions.clear();
				m_uvDistortions.resize( numUniqueTangents );
			}

			size_t fvi0 = 0;

			for( size_t faceIndex = 0; faceIndex < m_vertsPerFace.size() ; faceIndex++ )
			{
				size_t firstFvi = fvi0;
				unsigned vertex0 = m_vertIds[ fvi0 ];
				Imath::V2f uv0(0);
				if ( computeUV )
				{
					uv0 = Imath::V2f( (*m_u)[ fvi0 ], (*m_v)[ fvi0 ] );
				}
				for ( int v = 1; v <= m_vertsPerFace[faceIndex]; v++, fvi0++ )
				{
					size_t fvi1 = fvi0 + 1;
					if ( v == m_vertsPerFace[faceIndex] )
					{
						// final edge must also be computed...
						fvi1 = firstFvi;
					}
					unsigned vertex1 = m_vertIds[ fvi1 ];
					// compute distortion along the edge					
					const Vec &p0 = points[ vertex0 ];
					const Vec &refP0 = refPoints[ vertex0 ];
					const Vec &p1 = points[ vertex1 ];
					const Vec &refP1 = refPoints[ vertex1 ];
					Vec edge = p1 - p0;
					Vec refEdge = refP1 - refP0;
					float edgeLen = edge.length();
					float refEdgeLen = refEdge.length();
					float distortion = 0;
					if ( edgeLen >= refEdgeLen )
					{
						distortion = fabs((edgeLen / refEdgeLen) - 1.0f);
					}
					else
					{
						distortion = -fabs( (refEdgeLen / edgeLen) - 1.0f );
					}

					// accumulate vertex distortions
					m_distortions[ vertex0 ].accumulateDistortion( distortion );
					m_distortions[ vertex1 ].accumulateDistortion( distortion );
					vertex0 = vertex1;

					if ( computeUV )
					{
						// compute uv vector
						const Imath::V2f uv1( (*m_u)[ fvi1 ], (*m_v)[ fvi1 ] );
						const Imath::V2f uvDir = (uv1 - uv0).normalized();
						// accumulate uv distortion
						m_uvDistortions[ m_uvIds[fvi0] ].accumulateDistortion( distortion, uvDir );
						m_uvDistortions[ m_uvIds[fvi1] ].accumulateDistortion( distortion, uvDir );
						uv0 = uv1;
					}
				}
				fvi0 = firstFvi + m_vertsPerFace[faceIndex];
			}

			// normalize distortions and build output vectors

			// create the distortion prim var.
			vvDistortionsData = new FloatVectorData();
			std::vector<float> &distortionVec = vvDistortionsData->writable();
			distortionVec.reserve( m_distortions.size() );
			for ( std::vector< VertexDistortion >::iterator vIt = m_distortions.begin(); vIt != m_distortions.end(); vIt++ )
			{
				float invCounter = 0;
				if ( vIt->counter )
					invCounter = (1.0f/vIt->counter);
				distortionVec.push_back( vIt->distortion * invCounter );
			}

			// create U and V distortions
			if ( computeUV )
			{
				fvUDistortionsData = new FloatVectorData();
				fvVDistortionsData = new FloatVectorData();
				std::vector<float> &uDistortionVec = fvUDistortionsData->writable();
				uDistortionVec.reserve( m_faceVaryingSize );
				std::vector<float> &vDistortionVec = fvVDistortionsData->writable();
				vDistortionVec.reserve( m_faceVaryingSize );
				
				vector<float>::const_iterator uIt, vIt;
				unsigned fvi = 0;

				for ( uIt = m_u->begin(), vIt = m_v->begin(); uIt != m_u->end(); uIt++, vIt++, fvi++ )
				{
					UVDistortion &uvDist = m_uvDistortions[ m_uvIds[ fvi] ];
					if ( uvDist.counter )
					{
						uvDist.distortion /= (float)uvDist.counter;
						uvDist.counter = 0;
					}
					uDistortionVec.push_back( uvDist.distortion.x );
					vDistortionVec.push_back( uvDist.distortion.y );
				}
			}

		}


};

struct MeshDistortionsOp::HandleErrors
{
	template<typename T, typename F>
	void operator()( const T *d, const F &f )
	{
		string e = boost::str( boost::format( "MeshDistortionsOp : pPrimVarName parameter has unsupported data type \"%s\"." ) % d->typeName() );
		throw InvalidArgumentException( e );
	}
};

void MeshDistortionsOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	if( !mesh->arePrimitiveVariablesValid( ) )
	{
		throw InvalidArgumentException( "MeshDistortionsOp : MeshPrimitive variables are invalid." );
	}
	
	const std::string &pPrimVarName = pPrimVarNameParameter()->getTypedValue();
	Data * pData = mesh->variableData<Data>( pPrimVarName, PrimitiveVariable::Vertex );
	if( !pData )
	{
		string e = boost::str( boost::format( "MeshDistortionsOp : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % pPrimVarName );
		throw InvalidArgumentException( e );
	}

	const std::string &pRefPrimVarName = pRefPrimVarNameParameter()->getTypedValue();
	Data * pRefData = mesh->variableData<Data>( pRefPrimVarName, PrimitiveVariable::Vertex );
	if( !pRefData )
	{
		string e = boost::str( boost::format( "MeshDistortionsOp : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % pRefPrimVarName );
		throw InvalidArgumentException( e );
	}

	if ( pData->typeId() != pRefData->typeId() )
	{
		string e = boost::str( boost::format( "MeshDistortionsOp : Type mismatch between primitive variables \"%s\" and \"%s\"." ) % pPrimVarName % pRefPrimVarName );
		throw InvalidArgumentException( e );
	}

	const std::string &uvIndicesPrimVarName = parameters()->parameter<StringParameter>( "uvIndicesPrimVarName" )->getTypedValue();
	ConstIntVectorDataPtr uvIndicesData = 0;
	if( uvIndicesPrimVarName=="" )
	{
		uvIndicesData = mesh->vertexIds();
	}
	else
	{
		uvIndicesData = mesh->variableData<IntVectorData>( uvIndicesPrimVarName, PrimitiveVariable::FaceVarying );
		if( !uvIndicesData )
		{
			throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive has no FaceVarying IntVectorData primitive variable named \"%s\"."  ) % ( uvIndicesPrimVarName ) ).str() );
		}
	}

	const IntVectorData * vertsPerFace = mesh->verticesPerFace();
	const std::string &uPrimVarName = uPrimVarNameParameter()->getTypedValue();
	const std::string &vPrimVarName = vPrimVarNameParameter()->getTypedValue();

	FloatVectorDataPtr uData = 0;
	if ( uPrimVarName.size() )
	{
		uData = mesh->variableData<FloatVectorData>( uPrimVarName, PrimitiveVariable::FaceVarying );
		if( !uData )
		{
			throw InvalidArgumentException( ( boost::format( "MeshDistortionsOp : MeshPrimitive has no FaceVarying V3fVectorData primitive variable named \"%s\"."  ) % ( uPrimVarName ) ).str() );
		}
	}

	FloatVectorDataPtr vData = 0;
	if ( vPrimVarName.size() )
	{
		vData = mesh->variableData<FloatVectorData>( vPrimVarName, PrimitiveVariable::FaceVarying );
		if( !vData )
		{
			throw InvalidArgumentException( ( boost::format( "MeshDistortionsOp : MeshPrimitive has no FaceVarying V3fVectorData primitive variable named \"%s\"."  ) % ( vPrimVarName ) ).str() );
		}
	}

	const std::string &distortionPrimVarName = distortionPrimVarNameParameter()->getTypedValue();
	const std::string &uDistortionPrimVarName = uDistortionPrimVarNameParameter()->getTypedValue();
	const std::string &vDistortionPrimVarName = vDistortionPrimVarNameParameter()->getTypedValue();

	size_t faceVaryingSize = mesh->variableSize( PrimitiveVariable::FaceVarying );

	CalculateDistortions f( vertsPerFace->readable(), mesh->vertexIds()->readable(), faceVaryingSize, (uData ? &uData->readable() : 0 ), 
			( vData ? &vData->readable() : 0 ), uvIndicesData->readable(), pRefData );

	despatchTypedData<CalculateDistortions, TypeTraits::IsVec3VectorTypedData, HandleErrors>( pData, f );

	mesh->variables[ distortionPrimVarName ] = PrimitiveVariable( PrimitiveVariable::Vertex, f.vvDistortionsData );
	if ( f.fvUDistortionsData )
	{
		mesh->variables[ uDistortionPrimVarName ] = PrimitiveVariable( PrimitiveVariable::FaceVarying, f.fvUDistortionsData );
		mesh->variables[ vDistortionPrimVarName ] = PrimitiveVariable( PrimitiveVariable::FaceVarying, f.fvVDistortionsData );
	}

	assert( mesh->arePrimitiveVariablesValid() );
}
