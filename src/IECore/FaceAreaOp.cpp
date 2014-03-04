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

#include "IECore/FaceAreaOp.h"
#include "IECore/CompoundParameter.h"
#include "IECore/PolygonAlgo.h"
#include "IECore/PolygonIterator.h"

#include "boost/format.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/iterator/zip_iterator.hpp"
#include "boost/iterator/transform_iterator.hpp"

using namespace IECore;
using namespace std;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( FaceAreaOp );

FaceAreaOp::FaceAreaOp() : MeshPrimitiveOp( "Calculates face areas for a mesh." )
{
	StringParameterPtr pointPrimVarParameter = new StringParameter(
		"pointPrimVar",
		"The name of the primitive variable storing the mesh points to use for face area computation. "
		"This might be used to compute the areas based on Pref geometry for instance.",
		"P"
	);
	parameters()->addParameter( pointPrimVarParameter );

	StringParameterPtr areaPrimVarParameter = new StringParameter(
		"areaPrimVar",
		"The name of the the primitive variable used to store the calculated areas."
		"This can be set to the empty string to disable the computation of the face areas.",
		"faceArea"
	);
	parameters()->addParameter( areaPrimVarParameter );
	
	StringParameterPtr sPrimVarParameter = new StringParameter(
		"sPrimVar",
		"The name of the primitive variable storing the s coordinates to use for texture area computation.",
		"s"
	);
	parameters()->addParameter( sPrimVarParameter );
	
	StringParameterPtr tPrimVarParameter = new StringParameter(
		"tPrimVar",
		"The name of the primitive variable storing the t coordinates to use for texture area computation.",
		"t"
	);
	parameters()->addParameter( tPrimVarParameter );

	StringParameterPtr textureAreaPrimVarParameter = new StringParameter(
		"textureAreaPrimVar",
		"The name of the the primitive variable used to store the calculated per-face texture area."
		"This can be set to the empty string to disable the computation.",
		"textureArea"
	);
	parameters()->addParameter( textureAreaPrimVarParameter );

}

FaceAreaOp::~FaceAreaOp()
{
}

typedef boost::tuple<float, float> STTuple;

struct STTupleToV3f
{
	typedef V3f result_type;

	V3f operator()( const STTuple &t ) const
	{
		V3f result( t.get<0>(), t.get<1>(), 0.0f );
		return result;
	}
};

void FaceAreaOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	string areaPrimVarName = parameters()->parameter<StringParameter>( "areaPrimVar" )->getTypedValue();
	if( areaPrimVarName!="" )
	{
	
		const string &pName = parameters()->parameter<StringParameter>( "pointPrimVar" )->getTypedValue();
		ConstV3fVectorDataPtr pData = mesh->variableData<V3fVectorData>( pName, PrimitiveVariable::Vertex );
		if( !pData )
		{
			throw InvalidArgumentException( boost::str( boost::format( "FaceAreaOp : MeshPrimitive has no \"%s\" primitive variable." ) % pName ) );
		}
		const vector<V3f> &p = pData->readable();

		FloatVectorDataPtr areasData = new FloatVectorData;
		vector<float> &areas = areasData->writable();
		areas.reserve( mesh->variableSize( PrimitiveVariable::Uniform ) );
		PolygonIterator faceEnd = mesh->faceEnd();
		for( PolygonIterator pIt = mesh->faceBegin(); pIt!=faceEnd; pIt++ )
		{
			areas.push_back( polygonArea( pIt.vertexBegin( p.begin() ), pIt.vertexEnd( p.begin() ) ) );
		}

		mesh->variables[areaPrimVarName] = PrimitiveVariable( PrimitiveVariable::Uniform, areasData );
	}
	
	string textureAreaPrimVarName = parameters()->parameter<StringParameter>( "textureAreaPrimVar" )->getTypedValue();
	if( textureAreaPrimVarName!="" )
	{
	
		const string &sName = parameters()->parameter<StringParameter>( "sPrimVar" )->getTypedValue();
		PrimitiveVariable::Interpolation sInterpolation = PrimitiveVariable::Vertex;
		ConstFloatVectorDataPtr sData = mesh->variableData<FloatVectorData>( sName, PrimitiveVariable::Vertex );
		if( !sData )
		{
			sData = mesh->variableData<FloatVectorData>( sName, PrimitiveVariable::FaceVarying );
			if( !sData )
			{
				throw InvalidArgumentException( boost::str( boost::format( "FaceAreaOp : MeshPrimitive has no suitable \"%s\" primitive variable." ) % sName ) );
			}
			sInterpolation = PrimitiveVariable::FaceVarying;
		}
		const vector<float> &s = sData->readable();

		const string &tName = parameters()->parameter<StringParameter>( "tPrimVar" )->getTypedValue();
		PrimitiveVariable::Interpolation tInterpolation = PrimitiveVariable::Vertex;
		ConstFloatVectorDataPtr tData = mesh->variableData<FloatVectorData>( tName, PrimitiveVariable::Vertex );
		if( !tData )
		{
			tData = mesh->variableData<FloatVectorData>( tName, PrimitiveVariable::FaceVarying );
			if( !tData )
			{
				throw InvalidArgumentException( boost::str( boost::format( "FaceAreaOp : MeshPrimitive has no suitable \"%s\" primitive variable." ) % tName ) );
			}
			tInterpolation = PrimitiveVariable::FaceVarying;
		}
		const vector<float> &t = tData->readable();

		if( sInterpolation!=tInterpolation )
		{
			throw InvalidArgumentException( boost::str( boost::format( "FaceAreaOp : interpolation for \"%s\" and \"%s\" primitive variables don't match." ) % sName % tName ) );
		}

		FloatVectorDataPtr textureAreasData = new FloatVectorData;
		vector<float> &textureAreas = textureAreasData->writable();
		textureAreas.reserve( mesh->variableSize( PrimitiveVariable::Uniform ) );
		PolygonIterator faceEnd = mesh->faceEnd();
		for( PolygonIterator pIt = mesh->faceBegin(); pIt!=faceEnd; pIt++ )
		{
			if( sInterpolation==PrimitiveVariable::Vertex )
			{
				typedef PolygonVertexIterator<vector<float>::const_iterator> VertexIterator;
				typedef boost::tuple<VertexIterator, VertexIterator> IteratorTuple;
				typedef boost::zip_iterator<IteratorTuple> ZipIterator;
				typedef boost::transform_iterator<STTupleToV3f, ZipIterator> STIterator;
			
				STIterator begin( ZipIterator( IteratorTuple( pIt.vertexBegin( s.begin() ), pIt.vertexBegin( t.begin() ) ) ) );
				STIterator end( ZipIterator( IteratorTuple( pIt.vertexEnd( s.begin() ), pIt.vertexEnd( t.begin() ) ) ) );
			
				textureAreas.push_back( polygonArea( begin, end ) );
			}
			else
			{
				assert( sInterpolation==PrimitiveVariable::FaceVarying );
				typedef boost::tuple<vector<float>::const_iterator, vector<float>::const_iterator> IteratorTuple;
				typedef boost::zip_iterator<IteratorTuple> ZipIterator;
				typedef boost::transform_iterator<STTupleToV3f, ZipIterator> STIterator;
			
				STIterator begin( ZipIterator( IteratorTuple( pIt.faceVaryingBegin( s.begin() ), pIt.faceVaryingBegin( t.begin() ) ) ) );
				STIterator end( ZipIterator( IteratorTuple( pIt.faceVaryingEnd( s.begin() ), pIt.faceVaryingEnd( t.begin() ) ) ) );
			
				textureAreas.push_back( polygonArea( begin, end ) );
			}
		}

		mesh->variables[textureAreaPrimVarName] = PrimitiveVariable( PrimitiveVariable::Uniform, textureAreasData );
	
	}
	
}
