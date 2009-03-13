//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include <iostream>
#include <algorithm>
#include <cassert>

#include "OpenEXR/ImathFrame.h"

#include "IECore/Object.h"
#include "IECore/Group.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/CompoundParameter.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/PatchMeshPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/Interpolator.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"

#include "IECore/CurvesToPatchMeshGroupOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

CurvesToPatchMeshGroupOp::CurvesToPatchMeshGroupOp()
	:	Op(
		staticTypeName(),
		"The CurvesToPatchMeshGroupOp creates a group of PatchMesh geometries by lofting a circle along each given CurvesPrimitive.",
		new GroupParameter(
			"result",
			"Resulting group of patch meshes.",
			new Group()					
		)
	)
{
	m_curvesParameter = new CurvesPrimitiveParameter(
		"curves",
		"The curves to convert into patch meshes.",
		new CurvesPrimitive()
	);
	
	m_resolutionParameter = new V2iParameter(
		"resolution",
		"The resolution in U and V for each generated PatchMesh. U goes around the curve, V goes along the curve.",
		V2i( 6, 30 )
	);
	
	parameters()->addParameter( m_curvesParameter );
	parameters()->addParameter( m_resolutionParameter );
}

CurvesToPatchMeshGroupOp::~CurvesToPatchMeshGroupOp()
{
}

CurvesPrimitiveParameterPtr CurvesToPatchMeshGroupOp::curvesParameter()
{
	return m_curvesParameter;
}

ConstCurvesPrimitiveParameterPtr CurvesToPatchMeshGroupOp::curvesParameter() const
{
	return m_curvesParameter;
}

void CurvesToPatchMeshGroupOp::buildReferenceFrames( const std::vector< Imath::V3f > &points, std::vector< Imath::V3f > &tangents, std::vector< M44f > &frames ) const
{	
	/// \todo This disregads the "N" primvar which is possibly specified on the CurvesPrimitive
	std::vector< Imath::V3f >::size_type numPoints = points.size();
	assert( tangents.size() == numPoints );
	
	frames.resize( numPoints );
	
	frames[ 0 ] = Imath::firstFrame(
		points[ 0 ], 
		points[ 1 ], 
		points[ 2 ]
	);
	
	for( unsigned int i = 1; i < numPoints - 1; i++ )
	{
		frames[ i ] = Imath::nextFrame( 
			frames[ i - 1 ], 
			points[ i - 1 ], 
			points[ i ], 
			tangents[ i - 1 ], 
			tangents[ i ]
		);
	}
	
	frames[ numPoints - 1 ] = Imath::lastFrame( 
		frames[ numPoints - 2 ], 
		points[ numPoints - 2 ], 
		points[ numPoints - 1 ]
	);	
}

struct CurvesToPatchMeshGroupOp::VaryingFn
{
	typedef DataPtr ReturnType;
	
	const std::string &m_primVarName;
	ConstCurvesPrimitivePtr m_curves;
	const unsigned m_curveIndex;
	const unsigned m_varyingOffset;
	const V2i &m_resolution;
	
	VaryingFn( const std::string &primVarName, ConstCurvesPrimitivePtr curves, unsigned curveIndex, unsigned varyingOffset, const V2i &resolution ) : m_primVarName( primVarName ), m_curves( curves ), m_curveIndex( curveIndex ), m_varyingOffset( varyingOffset ), m_resolution( resolution )
	{
	}
	
	template<typename T>
	DataPtr operator() ( typename T::Ptr data ) const
	{
		assert( data );
		typedef typename T::ValueType::value_type Value;	
	
		const unsigned vPoints = m_resolution.y;
		const unsigned uPoints = m_resolution.x;
		
		typename T::Ptr newData = new T();
		newData->writable().reserve( ( vPoints ) * uPoints );		
		
		for ( unsigned int v = 0; v < vPoints; v++ )
		{
			int iSeg;
			float fSeg;
			
			if ( v == vPoints - 1 )
			{
				iSeg = m_curves->numSegments( m_curveIndex ) - 1;
				fSeg = 0.9999f;
			}
			else
			{
				float curveParam = float(v) / ( vPoints - 1 );
				fSeg = curveParam * m_curves->numSegments( m_curveIndex );
				iSeg = (int)floor( fSeg );
				fSeg = fSeg - iSeg;
			}
			
			for ( unsigned u = 0; u < uPoints ; u ++)
			{
				Value value;
				LinearInterpolator<Value>()(
					data->readable()[ m_varyingOffset + iSeg ], 
					data->readable()[ m_varyingOffset + iSeg + 1], 
					fSeg, 
					value
				);
				newData->writable().push_back( value );			
			}
		}
		
		return newData;	
	}
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			assert( data );
			throw InvalidArgumentException( ( boost::format( "CurvesToPatchMeshGroupOp: Invalid data type \"%s\" for primitive variable \"%s\"." ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_primVarName ).str() );            
                }
        };
};

struct CurvesToPatchMeshGroupOp::VertexFn
{
	typedef DataPtr ReturnType;
	
	const std::string &m_primVarName;
	ConstCurvesPrimitivePtr m_curves;
	const unsigned m_curveIndex;
	const unsigned m_varyingOffset;
	const V2i &m_resolution;	
	
	VertexFn( const std::string &primVarName, ConstCurvesPrimitivePtr curves, unsigned curveIndex, unsigned varyingOffset, const V2i &resolution ) : m_primVarName( primVarName ), m_curves( curves ), m_curveIndex( curveIndex ), m_varyingOffset( varyingOffset ), m_resolution( resolution )
	{
	}
	
	template<typename T>
	DataPtr operator() ( typename T::Ptr data ) const
	{
		assert( data );
		typedef typename T::ValueType::value_type Value;	
	
		const unsigned vPoints = m_resolution.y;
		const unsigned uPoints = m_resolution.x;
		
		typename T::Ptr newData = new T();
		newData->writable().reserve( ( vPoints + 2 ) * uPoints );		
		
		for ( unsigned int v = 0; v < vPoints; v++ )
		{
			size_t iSeg;
			float fSeg;
			
			if ( v == vPoints - 1 )
			{
				iSeg = m_curves->numSegments( m_curveIndex ) - 1;
				fSeg = 0.9999f;
			}
			else
			{
				float curveParam = float(v) / ( vPoints - 1 );
				fSeg = curveParam * m_curves->numSegments( m_curveIndex );
				iSeg = (size_t)floor( fSeg );
				fSeg = fSeg - iSeg;
			}
			
			const int num = v == 0 || v == vPoints - 1 ? 2 : 1;
			
			const size_t i0 = iSeg;
			const size_t i1 = std::min( iSeg + 1, m_curves->variableSize( PrimitiveVariable::Varying, m_curveIndex ) );
		
			for ( int x = 0; x < num; x++)
			{
				for ( unsigned u = 0; u < uPoints ; u ++)
				{
					Value value;
					LinearInterpolator<Value>()(
						data->readable()[ m_varyingOffset + i0 ], 
						data->readable()[ m_varyingOffset + i1], 
						fSeg, 
						value
					);
					newData->writable().push_back( value );
				}
			}
		}
		
		return newData;	
	}
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			assert( data );
			throw InvalidArgumentException( ( boost::format( "CurvesToPatchMeshGroupOp: Invalid data type \"%s\" for vertex primitive variable \"%s\"." ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_primVarName ).str() );            
                }
        };
};


struct CurvesToPatchMeshGroupOp::UniformFn
{
	typedef DataPtr ReturnType;
	
	const std::string &m_primVarName;		
	ConstCurvesPrimitivePtr m_curves;
	const unsigned m_curveIndex;
	
	UniformFn( const std::string &primVarName, ConstCurvesPrimitivePtr curves, unsigned curveIndex ) : m_primVarName( primVarName ), m_curves( curves ), m_curveIndex( curveIndex )
	{
	}
	
	template<typename T>
	DataPtr operator() ( typename T::Ptr data ) const
	{		
		assert( data );
		return new TypedData< typename T::ValueType::value_type >( data->readable()[ m_curveIndex ] );
	}
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			assert( data );                	
			throw InvalidArgumentException( ( boost::format( "CurvesToPatchMeshGroupOp: Invalid data type \"%s\" for uniform primitive variable \"%s\"." ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_primVarName ).str() );            
                }
        };
};


PatchMeshPrimitivePtr CurvesToPatchMeshGroupOp::buildPatchMesh( ConstCurvesPrimitivePtr curves, unsigned curveIndex, unsigned vertexOffset, unsigned varyingOffset ) const
{		
	if ( curves->periodic() )
	{
		throw InvalidArgumentException( "CurvesToPatchMeshGroupOp: Cannot convert periodic curves" );
	}
	
	PrimitiveVariableMap::const_iterator it = curves->variables.find( "P" );
	if ( it == curves->variables.end() )
	{
		throw InvalidArgumentException( "CurvesToPatchMeshGroupOp: Input curve has no 'P' primvar" );
	}
	ConstV3fVectorDataPtr pData = runTimeCast< const V3fVectorData >( it->second.data );
	if ( !pData )
	{
		throw InvalidArgumentException( "CurvesToPatchMeshGroupOp: Input curve has no 'P' primvar of type V3fVectorData" );
	}
	
	float width = 1.0f;
	it = curves->variables.find( "constantwidth" );
	if ( it != curves->variables.end() )
	{
		ConstFloatDataPtr widthData = 0;
		if ( it->second.interpolation == PrimitiveVariable::Constant )
		{	
			widthData = runTimeCast< const FloatData >( it->second.data );
		}
		
		if ( widthData )
		{
			width = widthData->readable();
		}
		else
		{
			msg( Msg::Warning, "CurvesToPatchMeshGroupOp", "Ignoring malformed primvar 'constantwidth'" );
		}
	}
	
	ConstFloatVectorDataPtr varyingWidthData = 0;
	ConstFloatVectorDataPtr vertexWidthData = 0;
	it = curves->variables.find( "width" );
	if ( it != curves->variables.end() )
	{
		if ( it->second.interpolation == PrimitiveVariable::Varying )
		{	
			varyingWidthData = runTimeCast< const FloatVectorData >( it->second.data );
		}
		else if ( it->second.interpolation == PrimitiveVariable::Vertex )
		{	
			vertexWidthData = runTimeCast< const FloatVectorData >( it->second.data );
		}
		
		if ( !varyingWidthData && !vertexWidthData)
		{
			msg( Msg::Warning, "CurvesToPatchMeshGroupOp", "Ignoring malformed primvar 'width'" );
		}
	}
	
	const V2i &resolution = m_resolutionParameter->getTypedValue();
	
	const unsigned int vPoints = resolution.y;
	const unsigned int uPoints = resolution.x;	
	
	PatchMeshPrimitivePtr patchMesh = new PatchMeshPrimitive(
		uPoints,
		vPoints + 2, // End points are duplicated
		CubicBasisf::catmullRom(), 
		CubicBasisf::catmullRom(),
		true,
		false
	);
		
	for ( PrimitiveVariableMap::const_iterator it = curves->variables.begin(); it != curves->variables.end(); ++it )
	{
		if ( it->second.interpolation == PrimitiveVariable::FaceVarying || it->second.interpolation == PrimitiveVariable::Varying )
		{
			VaryingFn varyingFn( it->first, curves, curveIndex, varyingOffset, resolution );
 			assert( it->second.data );

			patchMesh->variables[ it->first ] = PrimitiveVariable( 
				it->second.interpolation, 
				despatchTypedData<VaryingFn, TypeTraits::IsInterpolableVectorTypedData>( it->second.data, varyingFn )
			);

		} 
		else if ( it->second.interpolation == PrimitiveVariable::Vertex )
		{
			VertexFn vertexFn( it->first, curves, curveIndex, vertexOffset, resolution );
 			assert( it->second.data );

			patchMesh->variables[ it->first ] = PrimitiveVariable( 
				it->second.interpolation, 
				despatchTypedData<VertexFn, TypeTraits::IsInterpolableVectorTypedData>( it->second.data, vertexFn )
			);

		}
		else if ( it->second.interpolation == PrimitiveVariable::Constant )
		{
			patchMesh->variables[ it->first ] = PrimitiveVariable( it->second.interpolation, it->second.data->copy() );
		}
		else if ( it->second.interpolation == PrimitiveVariable::Uniform )
		{
			UniformFn uniformFn( it->first, curves, curveIndex );
			patchMesh->variables[ it->first ] = PrimitiveVariable(
				PrimitiveVariable::Constant, 
				despatchTypedData<UniformFn, TypeTraits::IsVectorTypedData>( it->second.data, uniformFn )
			);
		}
	}
	
	if ( varyingWidthData )
	{
		assert( !vertexWidthData );
		PrimitiveVariableMap::const_iterator it = patchMesh->variables.find( "width" );
		assert( it !=  patchMesh->variables.end() );
		
		varyingWidthData = runTimeCast< const FloatVectorData >( it->second.data );
		assert( varyingWidthData );
	}
	else if ( vertexWidthData )
	{	
		PrimitiveVariableMap::const_iterator it = patchMesh->variables.find( "width" );
		assert( it !=  patchMesh->variables.end() );
		
		vertexWidthData = runTimeCast< const FloatVectorData >( it->second.data );
		assert( vertexWidthData );
	}
	
	const V3fVectorData::ValueType &p = pData->readable();
	
	V3fVectorData::ValueType resampledPoints;
	resampledPoints.reserve( vPoints );
	
	V3fVectorData::ValueType resampledTangents;
	resampledPoints.reserve( vPoints );	
	
	/// \todo Make adaptive
	for ( unsigned v = 0; v < vPoints; v ++)
	{		
		size_t iSeg;
		float fSeg;
		
		/// Make sure we don't fall off the end of the curve
		if ( v == vPoints - 1 )
		{
			iSeg = curves->numSegments( curveIndex ) - 1;
			fSeg = 1.0f - std::numeric_limits<float>::epsilon();
		}
		else
		{
			float curveParam = float(v) / ( vPoints - 1 );
			fSeg = curveParam * curves->numSegments( curveIndex );
			iSeg = (size_t)floor( fSeg );
			fSeg = fSeg - iSeg;
		}
		
		size_t segmentStart = iSeg;
		
		size_t i0 = std::min( segmentStart + 0, curves->variableSize( PrimitiveVariable::Vertex, curveIndex ) );
		size_t i1 = std::min( segmentStart + 1, curves->variableSize( PrimitiveVariable::Vertex, curveIndex ) );
		size_t i2 = std::min( segmentStart + 2, curves->variableSize( PrimitiveVariable::Vertex, curveIndex ) );
		size_t i3 = std::min( segmentStart + 3, curves->variableSize( PrimitiveVariable::Vertex, curveIndex ) );
						
		const Imath::V3f &p0 = p[ vertexOffset + i0 ];
		const Imath::V3f &p1 = p[ vertexOffset + i1  ];
		const Imath::V3f &p2 = p[ vertexOffset + i2  ];
		const Imath::V3f &p3 = p[ vertexOffset + i3  ];						
		
		Imath::V3f pt = curves->basis()( 
				fSeg, 
				p0, p1, p2, p3
			);
			
		resampledPoints.push_back( pt );

		resampledTangents.push_back( 
			curves->basis().derivative( 
				fSeg, 
				p0, p1, p2, p3
			).normalized()
		);	
		
	}
	assert( resampledPoints.size() == vPoints );
	assert( resampledTangents.size() == vPoints );		
	
	std::vector< M44f > frames;

	buildReferenceFrames( resampledPoints, resampledTangents, frames );
	assert( frames.size() == vPoints );
				
	std::vector< V3f > patchP;
	patchP.reserve( uPoints * ( vPoints + 2 ) );
	
	for ( unsigned int v = 0; v < vPoints; v++ )
	{
		if ( varyingWidthData )
		{
			assert( !vertexWidthData );	
			assert( v * uPoints <  varyingWidthData->readable().size() );			
			width = varyingWidthData->readable()[v * uPoints];
		}
		else if ( vertexWidthData )
		{
			assert( (v+1) * uPoints <  vertexWidthData->readable().size() );
			width = vertexWidthData->readable()[(v+1) * uPoints];		
		}
		
		const float radius = width / 2.0f;

		/// Double up end points		
		const int num = v == 0 || v == vPoints - 1 ? 2 : 1;
		
		for ( int x = 0; x < num; x++)
		{
			for( unsigned int u = 0; u < uPoints; u++ )
			{
				/// We're periodic in 'u', so no need to close the curve.
				/// Go from -PI to PI, in order to make the periodicity work, and to give the
				/// surface the correct orientation.
				float theta = -2.0 * M_PI * float(u) / float(uPoints) - M_PI;

				V3f circlePoint( 
					0.0,
					radius * cos( theta ),				
					radius * sin( theta )
				);

				circlePoint = circlePoint * frames[v];

				patchP.push_back( circlePoint );
			}
		}		
	}
			
	patchMesh->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, new V3fVectorData( patchP ) );

	assert( patchMesh->arePrimitiveVariablesValid() );

	return patchMesh;
}

ObjectPtr CurvesToPatchMeshGroupOp::doOperation( ConstCompoundObjectPtr operands )
{	
	CurvesPrimitivePtr curves = m_curvesParameter->getTypedValue<CurvesPrimitive>();	
	assert( curves );
	assert( curves->arePrimitiveVariablesValid() );
	
	GroupPtr group = new Group();
	
	ConstIntVectorDataPtr verticesPerCurve = curves->verticesPerCurve();
	assert( verticesPerCurve );
	
	unsigned numCurves = verticesPerCurve->readable().size();
	unsigned vertexOffset = 0;
	unsigned varyingOffset = 0;
	for ( unsigned curveIndex = 0; curveIndex < numCurves; curveIndex++ )
	{
		int numVertices = curves->variableSize( PrimitiveVariable::Vertex, curveIndex );
		
		PatchMeshPrimitivePtr patchMesh = buildPatchMesh( curves, curveIndex, vertexOffset, varyingOffset );
		assert( patchMesh );
						
		group->addChild( patchMesh );
				
		vertexOffset += numVertices;	
		varyingOffset += curves->variableSize( PrimitiveVariable::Varying, curveIndex );	
	}
	
	assert( group->children().size() == numCurves );
		
	return group;
}
