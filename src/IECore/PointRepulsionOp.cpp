//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/Reader.h"
#include "IECore/ImagePrimitive.h"

#include "IECore/Random.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/BoundedKDTree.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/TriangulateOp.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/ImagePrimitiveEvaluator.h"
#include "IECore/KDTree.h"
#include "IECore/MessageHandler.h"
#include "IECore/SweepAndPrune.h"

#include "IECore/PointRepulsionOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PointRepulsionOp );

PointRepulsionOp::PointRepulsionOp()
		:	ModifyOp(
		        staticTypeName(),
		        "The PointRepulsionOp pushes points around a surface to give them a more even distribution.",
		        new PointsPrimitiveParameter(
		                "result",
		                "Resulting points after repulsion over mesh.",
		                new PointsPrimitive()
		        ),
		        new PointsPrimitiveParameter(
		                "input",
		                "The input points to relax by near-neighbour repulsion",
		                new PointsPrimitive()
		        )
		)
{

	m_meshParameter = new MeshPrimitiveParameter(
	        "mesh",
	        "The mesh over which the points have been distributed",
	        new MeshPrimitive()
	);

	m_imageParameter = new ImagePrimitiveParameter(
	        "image",
	        "The image specifying the density map.",
	        new ImagePrimitive()
	);

	StringParameter::PresetsContainer channelNamePresets;
	channelNamePresets.push_back( StringParameter::Preset( "R", "R" ) );
	channelNamePresets.push_back( StringParameter::Preset( "G", "G" ) );
	channelNamePresets.push_back( StringParameter::Preset( "B", "B" ) );
	channelNamePresets.push_back( StringParameter::Preset( "A", "A" ) );
	channelNamePresets.push_back( StringParameter::Preset( "Y", "Y" ) );

	m_channelNameParameter = new StringParameter(
	        "channelName",
	        "The name of the floating point channel in the image to use as the density map.",
	        "Y",
	        channelNamePresets,
	        false
	);

	m_numIterationsParameter = new IntParameter(
	        "numIterations",
	        "The number of repulsion iterations to apply.",
	        5000,
	        1
	);

	m_magnitudeParameter = new FloatParameter(
	        "magnitude",
	        "The magnitude of the force applied by each individual repulsion.",
	        0.01f,
	        1.e-10f
	);

	m_weightsNameParameter = new StringParameter(
	        "weightsName",
	        "The name of the primvar in the PointsPrimitive to use as the per-point weights",
	        ""
	);

	parameters()->addParameter( m_imageParameter );
	parameters()->addParameter( m_channelNameParameter );
	parameters()->addParameter( m_meshParameter );
	parameters()->addParameter( m_numIterationsParameter );
	parameters()->addParameter( m_magnitudeParameter );
	parameters()->addParameter( m_weightsNameParameter );
}

PointRepulsionOp::~PointRepulsionOp()
{
}

MeshPrimitiveParameterPtr PointRepulsionOp::meshParameter()
{
	return m_meshParameter;
}

ConstMeshPrimitiveParameterPtr PointRepulsionOp::meshParameter() const
{
	return m_meshParameter;
}


ImagePrimitiveParameterPtr PointRepulsionOp::imageParameter()
{
	return m_imageParameter;
}

ConstImagePrimitiveParameterPtr PointRepulsionOp::imageParameter() const
{
	return m_imageParameter;
}

StringParameterPtr PointRepulsionOp::channelNameParameter()
{
	return m_channelNameParameter;
}

ConstStringParameterPtr PointRepulsionOp::channelNameParameter() const
{
	return m_channelNameParameter;
}

IntParameterPtr PointRepulsionOp::numIterationsParameter()
{
	return m_numIterationsParameter;
}

ConstIntParameterPtr PointRepulsionOp::numIterationsParameter() const
{
	return m_numIterationsParameter;
}

FloatParameterPtr PointRepulsionOp::magnitudeParameter()
{
	return m_magnitudeParameter;
}

ConstFloatParameterPtr PointRepulsionOp::magnitudeParameter() const
{
	return m_magnitudeParameter;
}

StringParameterPtr PointRepulsionOp::weightsNameParameter()
{
	return m_weightsNameParameter;
}

ConstStringParameterPtr PointRepulsionOp::weightsNameParameter() const
{
	return m_weightsNameParameter;
}

void PointRepulsionOp::getNearestPointsAndDensities( ImagePrimitiveEvaluatorPtr imageEvaluator, const PrimitiveVariable &densityPrimVar, MeshPrimitiveEvaluatorPtr meshEvaluator, const PrimitiveVariable &sPrimVar, const PrimitiveVariable &tPrimVar, std::vector<Imath::V3f> &points, std::vector<float> &densities )
{
	densities.resize( points.size() );

	PrimitiveEvaluator::ResultPtr meshResult = meshEvaluator->createResult();
	PrimitiveEvaluator::ResultPtr imageResult = imageEvaluator->createResult();

	for ( std::vector<Imath::V3f>::size_type p = 0; p < points.size(); p++ )
	{
		bool found = meshEvaluator->closestPoint( points[p], meshResult );
		if ( !found )
		{
			throw InvalidArgumentException( "PointRepulsionOp: Invaid mesh - closest point is undefined" );
		}

		points[p] = meshResult->point();

		Imath::V2f uv(
		        meshResult->floatPrimVar( sPrimVar ),
		        meshResult->floatPrimVar( tPrimVar )
		);

		/// \todo Texture repeat
		float repeatU = 1.0;
		float repeatV = 1.0;

		/// \todo Wrap modes
		bool wrapU = true;
		bool wrapV = true;

		Imath::V2f placedUv(
		        uv.x * repeatU,
		        uv.y * repeatV
		);

		if ( wrapU )
		{
			placedUv.x = fmodf( placedUv.x, 1.0f );
		}

		if ( wrapV )
		{
			placedUv.y = fmodf( placedUv.y, 1.0f );
		}

		imageEvaluator->pointAtUV( placedUv, imageResult );

		densities[p] = imageResult->floatPrimVar( densityPrimVar );
	}
}

void PointRepulsionOp::calculateForces( std::vector<V3f> &points, std::vector<float> &radii, std::vector<Imath::Box3f> &bounds, std::vector<Imath::V3f> &forces, Imath::Rand48 &generator, std::vector<float> &densities, float densityInv )
{
	unsigned numPoints = points.size();

	Box3fTree tree( bounds.begin(), bounds.end(), 16 );

	typedef std::vector< Box3fTree::Iterator> Bounds;

	for ( std::vector<V3f>::size_type p = 0; p < numPoints; p++ )
	{
		Bounds approximateBounds;
		tree.intersectingBounds( bounds[p], approximateBounds );

		for ( Bounds::const_iterator it = approximateBounds.begin(); it != approximateBounds.end(); ++it )
		{
			const std::vector<V3f>::size_type other = *it - bounds.begin();
			assert( other < numPoints );
			assert( other < radii.size() );

			if ( p != other )
			{
				Imath::V3f separation = points[p] - points[other];

				float dist = separation.length();

				float densityDiff = 1.0f - fabsf( densities[p] * densityInv - densities[other] * densityInv );

				if ( dist < radii[p] + radii[other] )
				{
					float overlap = radii[p] + radii[other] - dist;
					assert( overlap >= 0.0f );
					float overlapNorm = overlap / ( radii[p] + radii[other] );

					if ( dist < 1.e-6f )
					{
						/// Points are incident, so force acts to move current point away from neighbour in a random direction
						forces[ p ] += densityDiff * overlapNorm * solidSphereRand< V3f, Rand48 >( generator ) ;
					}
					else
					{
						/// Force acts to move current point away from neighbour along their line of separation
						forces[ p ] += densityDiff * overlapNorm * separation.normalized() ;
					}
				}
			}
		}
	}
}


void PointRepulsionOp::modify( ObjectPtr object, ConstCompoundObjectPtr operands )
{
	MeshPrimitivePtr mesh = m_meshParameter->getTypedValue<MeshPrimitive>();
	assert( mesh );

	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( mesh );
	op->toleranceParameter()->setNumericValue( 1.e-3f );
	mesh = runTimeCast< MeshPrimitive > ( op->operate() );
	assert( mesh );

	PointsPrimitivePtr pointsPrimitive = runTimeCast< PointsPrimitive >( object );
	assert( pointsPrimitive );

	ImagePrimitivePtr image = runTimeCast< ImagePrimitive >( m_imageParameter->getValue()->copy() );
	assert( image );

	const std::string &channelName = m_channelNameParameter->getTypedValue();

	const int numIterations = m_numIterationsParameter->getNumericValue();

	const float magnitude = m_magnitudeParameter->getNumericValue();

	const std::string &weightsName = m_weightsNameParameter->getTypedValue();

	PrimitiveVariableMap::const_iterator sIt = mesh->variables.find( "s" );
	if ( sIt != mesh->variables.end() )
	{
		if ( sIt->second.interpolation != PrimitiveVariable::FaceVarying )
		{
			throw InvalidArgumentException( "PointRepulsionOp: MeshPrimitive variable 's' must have facevarying interpolation" );
		}
		if ( !runTimeCast< FloatVectorData >( sIt->second.data ) )
		{
			throw InvalidArgumentException( "PointRepulsionOp: MeshPrimitive variable 's' must be of type FloatVectorData" );
		}
	}

	PrimitiveVariableMap::const_iterator tIt = mesh->variables.find( "t" );
	if ( tIt != mesh->variables.end() )
	{
		if ( tIt->second.interpolation != PrimitiveVariable::FaceVarying )
		{
			throw InvalidArgumentException( "PointRepulsionOp: MeshPrimitive variable 't' must have facevarying interpolation" );
		}

		if ( !runTimeCast< FloatVectorData >( tIt->second.data ) )
		{
			throw InvalidArgumentException( "PointRepulsionOp: MeshPrimitive variable 't' must be of type FloatVectorData" );
		}
	}

	V3fVectorDataPtr pData = 0;
	PrimitiveVariableMap::const_iterator pIt = pointsPrimitive->variables.find( "P" );
	if ( pIt != pointsPrimitive->variables.end() )
	{
		if ( pIt->second.interpolation != PrimitiveVariable::Vertex )
		{
			throw InvalidArgumentException( "PointRepulsionOp: PointsPrimitive variable 'P' must have vertex interpolation" );
		}

		pData = runTimeCast< V3fVectorData >( pIt->second.data );
		if ( !pData )
		{
			throw InvalidArgumentException( "PointRepulsionOp: PointsPrimitive variable 'P' must be of type V3fVectorData" );
		}
	}
	else
	{
		throw InvalidArgumentException( "PointRepulsionOp: PointsPrimitive variable 'P' must be present" );
	}

	typedef V3fVectorData::ValueType PointArray;
	PointArray &points = pData->writable();
	PointArray::size_type numPoints = points.size();

	FloatVectorDataPtr weights = 0;
	if ( weightsName.size() )
	{
		PrimitiveVariableMap::const_iterator wIt = pointsPrimitive->variables.find( weightsName );
		if ( wIt != pointsPrimitive->variables.end() )
		{
			if ( wIt->second.interpolation != PrimitiveVariable::Vertex )
			{
				throw InvalidArgumentException(( boost::format( "PointRepulsionOp: PointsPrimitive variable '%s' must have vertex interpolation" ) % weightsName ).str() );
			}

			weights = runTimeCast< FloatVectorData >( wIt->second.data );
			if ( !weights )
			{
				throw InvalidArgumentException(( boost::format( "PointRepulsionOp: PointsPrimitive variable '%s' must be of type FloatVectorData" ) % weightsName ).str() );
			}

			assert( weights );
			if ( weights->readable().size() != numPoints )
			{
				throw InvalidArgumentException(( boost::format( "PointRepulsionOp: PointsPrimitive variable '%s' of incorrect size" ) % weightsName ).str() );
			}
		}
		else
		{
			throw InvalidArgumentException(( boost::format( "PointRepulsionOp: PointsPrimitive weights variable '%s' must be present" ) % weightsName ).str() );
		}
	}

	PrimitiveVariableMap::iterator cIt = image->variables.find( channelName );
	if ( cIt != image->variables.end() )
	{
		if ( !runTimeCast< FloatVectorData >( cIt->second.data ) )
		{
			throw InvalidArgumentException(( boost::format( "PointRepulsionOp: ImagePrimitive channel '%s' must be of type FloatVectorData" ) % channelName ).str() );
		}
	}
	else
	{
		throw InvalidArgumentException(( boost::format( "PointRepulsionOp: ImagePrimitive channel '%s' not found" ) % channelName ).str() );
	}
	const PrimitiveVariable &densityPrimVar = cIt->second;

	ImagePrimitiveEvaluatorPtr imageEvaluator = new ImagePrimitiveEvaluator( image );
	PrimitiveEvaluator::ResultPtr imageResult = imageEvaluator->createResult();

	MeshPrimitiveEvaluatorPtr meshEvaluator = new MeshPrimitiveEvaluator( mesh );
	PrimitiveEvaluator::ResultPtr meshResult = meshEvaluator->createResult();


	/// Convert density image texture, so that values of pixels with the original density channel
	/// represent "number of points per unit area".
	int height = image->getDisplayWindow().max.y - image->getDisplayWindow().min.y + 1;
	int width = image->getDisplayWindow().max.x - image->getDisplayWindow().min.x + 1;

	if ( width == 0 || height == 0 )
	{
		throw InvalidArgumentException( "PointRepulsionOp: ImagePrimitive has zero area" );
	}

	float du = 1.0f / width;
	float dv = 1.0f / height;

	float textureArea = 0.0f;
	for ( int y = 0; y < height; y++ )
	{
		for ( int x = 0; x < width; x++ )
		{
			//// Get point at center of pixel
			bool found = imageEvaluator->pointAtPixel( V2i( x, y ), imageResult );
			if ( found )
			{
				V2f topLeft =     imageResult->uv() + V2f( -du / 2.0f, -dv / 2.0f );
				V2f topRight =    imageResult->uv() + V2f( du / 2.0f, -dv / 2.0f );
				V2f bottomRight = imageResult->uv() + V2f( du / 2.0f,  dv / 2.0f );
				V2f bottomLeft =  imageResult->uv() + V2f( -du / 2.0f,  dv / 2.0f );

				std::vector<V2f> pixelCornersUV;
				pixelCornersUV.push_back( topLeft );
				pixelCornersUV.push_back( topRight );
				pixelCornersUV.push_back( bottomRight );
				pixelCornersUV.push_back( bottomLeft );

				std::vector<V3f> pixelCornersWorld;

				for ( unsigned c = 0; c < 4; c++ )
				{
					found = meshEvaluator->pointAtUV( pixelCornersUV[c], meshResult );

					if ( found )
					{
						pixelCornersWorld.push_back( meshResult->point() );
					}
				}

				if ( pixelCornersWorld.size() == 4 )
				{
					/// \todo optimise
					imageEvaluator->pointAtPixel( V2i( x, y ), imageResult );
					float density = imageResult->floatPrimVar( densityPrimVar );

					textureArea += density * triangleArea( pixelCornersWorld[0], pixelCornersWorld[1], pixelCornersWorld[2] ) ;
					textureArea += density * triangleArea( pixelCornersWorld[0], pixelCornersWorld[2], pixelCornersWorld[3] );
				}
				else if ( pixelCornersWorld.size() == 3 )
				{
					/// \todo optimise
					imageEvaluator->pointAtPixel( V2i( x, y ), imageResult );
					float density = imageResult->floatPrimVar( densityPrimVar );

					textureArea += density * triangleArea( pixelCornersWorld[0], pixelCornersWorld[1], pixelCornersWorld[2] );
				}
			}
		}
	}
	FloatVectorDataPtr densityData = runTimeCast< FloatVectorData >( densityPrimVar.data );
	assert( densityData );
	int dataOffset = 0;
	for ( int y = 0; y < height; y++ )
	{
		for ( int x = 0; x < width; x++ )
		{
			densityData->writable()[ dataOffset ++ ] *= ( float )numPoints / textureArea;
		}
	}

	std::vector<float> originalDensities( numPoints );
	std::vector<float> currentDensities( numPoints );
	std::vector<Imath::V3f> forces( numPoints );
	std::vector<float> radii( numPoints );
	std::vector<Imath::V3f> oldPoints( numPoints );
	std::vector<Imath::Box3f> bounds( numPoints );

	float lastEnergy = std::numeric_limits<float>::max();

	Rand48 generator( 1 );

	for ( int i = 0; i < numIterations; ++i )
	{
		assert( points.size() == originalDensities.size() );
		assert( points.size() == currentDensities.size() );
		assert( points.size() == forces.size() );
		assert( points.size() == radii.size() );
		assert( points.size() == oldPoints.size() );
		assert( points.size() == bounds.size() );

		// Snap points to mesh, and calculate new densities
		getNearestPointsAndDensities( imageEvaluator, densityPrimVar, meshEvaluator, sIt->second, tIt->second, points, currentDensities );

		if ( i == 0 )
		{
			std::copy( currentDensities.begin(), currentDensities.end(), originalDensities.begin() );
		}

		/// Update radii, bounds, and force accumulator
		for ( PointArray::size_type p = 0; p < numPoints; p++ )
		{
			float pointsPerUnitArea = originalDensities[ p ];

			/// \todo More accurately determine the minimum permissible value for "pointsPerUnitArea"
			float areaPerPoint = 1.0f / std::max( 0.01f, pointsPerUnitArea );

			assert( p < radii.size() );

			/// pi * r * r = area
			/// Compensate for the fact that even at the densest possible packing (hexagonal), we only get pi/sqrt(12) ( ~ 0.9 ) efficiency,
			/// by making each "circle" slightly larger by sqrt(12)/pi
			radii[p] = sqrt( areaPerPoint / M_PI ) * sqrt( 12.0f ) / M_PI;

			assert( p < radii.size() );
			bounds[p] = Imath::Box3f(
			                    Imath::V3f( points[p] - Imath::V3f( radii[p], radii[p], radii[p] ) ),
			                    Imath::V3f( points[p] + Imath::V3f( radii[p], radii[p], radii[p] ) )
			            );

			/// Zero force accumulator
			forces[p] = V3f( 0.0 );
		}

		calculateForces( points, radii, bounds, forces, generator, originalDensities, textureArea / ( float )numPoints );

		std::copy( points.begin(), points.end(), oldPoints.begin() );

		float totalEnergy = 0.0f;
		for ( PointArray::size_type p = 0; p < numPoints; p++ )
		{
			totalEnergy += forces[p].length();

			/// Advect point by force applied to it
			points[p] += forces[p] * magnitude;
		}

		// Snap points back to mesh, and calculate new densities
		getNearestPointsAndDensities( imageEvaluator, densityPrimVar, meshEvaluator, sIt->second, tIt->second, points, currentDensities );

		totalEnergy = 0.0f;
		for ( PointArray::size_type p = 0; p < numPoints; p++ )
		{
			float denom = std::max( originalDensities[p], currentDensities[p] );
			float scale = 1.0f;
			if ( denom > 1.0e-6f )
			{
				scale = fabsf( originalDensities[p] - currentDensities[p] ) / denom;
			}
			forces[p] *= 1.0f - scale;

			if ( weights )
			{
				forces[p] *= weights->readable()[ p ];
			}

			totalEnergy += forces[p].length();

			/// Advect point by force applied to it
			points[p] = oldPoints[p] + forces[p] * magnitude;
		}

		assert( totalEnergy >= 0.0f );

		msg( Msg::Info, "PointRepulsionOp", boost::format( "Residual error after iteration %s : %s " ) % i % totalEnergy );

		if ( totalEnergy > lastEnergy )
		{
			float percentageIncrease = ( totalEnergy / lastEnergy - 1.0f ) * 100.0f;

			if ( percentageIncrease > 1.0 )
			{
				msg( Msg::Warning, "PointRepulsionOp", boost::format( "Residual error increased by %f%% during iteration %s, consider decreasing magnitude parameter " ) % ( percentageIncrease ) % i );
			}
		}

		if ( totalEnergy < 1.e-6f )
		{
			break;
		}

		lastEnergy = totalEnergy;
	}

	getNearestPointsAndDensities( imageEvaluator, densityPrimVar, meshEvaluator, sIt->second, tIt->second, points, currentDensities );

	if ( pointsPrimitive->variables.find( "width" ) == pointsPrimitive->variables.end() )
	{
		for ( PointArray::size_type p = 0; p < numPoints; p++ )
		{
			radii[p] *= 2.0; /// now want to interpret as "width" (diameter)
		}
		pointsPrimitive->variables["width"] = PrimitiveVariable( PrimitiveVariable::Vertex, new FloatVectorData( radii ) );
	}

	/// Update s and t, if necessary
	bool pointsPrimitiveHasS = pointsPrimitive->variables.find( "s" ) != pointsPrimitive->variables.end();
	bool pointsPrimitiveHasT = pointsPrimitive->variables.find( "t" ) != pointsPrimitive->variables.end();

	if ( pointsPrimitiveHasS || pointsPrimitiveHasT )
	{
		FloatVectorDataPtr sData = 0;
		if ( pointsPrimitiveHasS )
		{
			sData = new FloatVectorData();
			sData->writable().resize( numPoints );
		}
		FloatVectorDataPtr tData = 0;
		if ( pointsPrimitiveHasT )
		{
			tData = new FloatVectorData();
			tData->writable().resize( numPoints );
		}

		assert( sData || tData );

		for ( PointArray::size_type p = 0; p < numPoints; p++ )
		{
			bool found = meshEvaluator->closestPoint( points[p], meshResult );
			assert( found );
			( void ) found;

			if ( sData )
			{
				assert( p < sData->readable().size() );
				sData->writable()[p] = meshResult->uv().x;
			}
			if ( tData )
			{
				assert( p < tData->readable().size() );
				tData->writable()[p] = meshResult->uv().y;
			}
		}

		if ( sData )
		{
			pointsPrimitive->variables["s"] = PrimitiveVariable( PrimitiveVariable::Varying, sData );
		}
		if ( tData )
		{
			pointsPrimitive->variables["t"] = PrimitiveVariable( PrimitiveVariable::Varying, tData );
		}
	}
}

