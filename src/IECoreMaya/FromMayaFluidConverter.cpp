//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaFluidConverter.h"
#include "IECoreMaya/Convert.h"

#include "IECore/CompoundParameter.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/MatrixMultiplyOp.h"
#include "IECore/SimpleTypedData.h"

#include "maya/MFn.h"
#include "maya/MFnFluid.h"

using namespace IECoreMaya;
using namespace IECore;
using namespace std;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( FromMayaFluidConverter );

FromMayaShapeConverter::Description<FromMayaFluidConverter> FromMayaFluidConverter::m_description( MFn::kFluid, PointsPrimitive::staticTypeId() );

FromMayaFluidConverter::FromMayaFluidConverter( const MObject &object )
	:	FromMayaShapeConverter( staticTypeName(), "Converts maya fluid data to IECore::PointsPrimitive Object", object )
{
	constructCommon();
}

FromMayaFluidConverter::FromMayaFluidConverter( const MDagPath &dagPath )
	:	FromMayaShapeConverter( staticTypeName(), "Converts maya fluid data to IECore::PointsPrimitive Object", dagPath )
{
	constructCommon();
}

FromMayaFluidConverter::~FromMayaFluidConverter()
{
}

void FromMayaFluidConverter::constructCommon()
{
	m_velocityParameter = new BoolParameter(
		"velocity",
		"When this is on the fluid's velocities are added to the result as a primitive variable named \"velocity\".",
		true
	);
	parameters()->addParameter( m_velocityParameter );

	m_densityParameter = new BoolParameter(
		"density",
		"When this is on the fluid's densities are added to the result as a primitive variable named \"density\".",
		true
	);
	parameters()->addParameter( m_densityParameter );

	m_pressureParameter = new BoolParameter(
		"pressure",
		"When this is on the fluid's pressures are added to the result as a primitive variable named \"pressure\".",
		true
	);
	parameters()->addParameter( m_pressureParameter );

	m_temperatureParameter = new BoolParameter(
		"temperature",
		"When this is on the fluid's temperatures are added to the result as a primitive variable named \"temperature\".",
		true
	);
	parameters()->addParameter( m_temperatureParameter );

	m_fuelParameter = new BoolParameter(
		"fuel",
		"When this is on the fluid's fuel is added to the result as a primitive variable named \"fuel\".",
		true
	);
	parameters()->addParameter( m_fuelParameter );

	m_falloffParameter = new BoolParameter(
		"falloff",
		"When this is on the fluid's falloff is added to the result as a primitive variable named \"falloff\".",
		true
	);
	parameters()->addParameter( m_falloffParameter );

	m_colorParameter = new BoolParameter(
		"color",
		"When this is on the fluid's colors are added to the result as a primitive variable named \"color\".",
		true
	);
	parameters()->addParameter( m_colorParameter );

	m_textureCoordinatesParameter = new BoolParameter(
		"textureCoordinates",
		"When this is on the fluid's texture coordinates are added to the result as a primitive variable named \"uvw\" or \"uv\".",
		true
	);
	parameters()->addParameter( m_textureCoordinatesParameter );
}

void FromMayaFluidConverter::addPrimVar( IECore::PrimitivePtr primitive, const std::string &name, size_t numPoints, MFnFluid &fnFluid, float *(MFnFluid::*fn)( MStatus * ) ) const
{
	MStatus s;
	float *mayaPtr = ((fnFluid).*(fn))( &s );
	if ( s )																	
	{					
		assert( mayaPtr );														
		FloatVectorDataPtr floatData = new FloatVectorData();					
		floatData->writable().resize( numPoints );								
		std::copy( mayaPtr, mayaPtr + numPoints, &(floatData->writable()[0]) );	
		PrimitiveVariable newVar( PrimitiveVariable::Vertex, floatData );		
		primitive->variables[ name ] = newVar;											
	}
}

IECore::PrimitivePtr FromMayaFluidConverter::doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnFluid fnFluid( object );
	if( !fnFluid.hasObj( object ) )
	{
		return 0;
	}
	return doPrimitiveConversion( fnFluid );
}

IECore::PrimitivePtr FromMayaFluidConverter::doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnFluid fnFluid( dagPath );
	if( !fnFluid.hasObj( dagPath.node() ) )
	{
		return 0;
	}
	return doPrimitiveConversion( fnFluid );
}

IECore::PrimitivePtr FromMayaFluidConverter::doPrimitiveConversion( MFnFluid &fnFluid ) const
{	
	MStatus s;
	unsigned int nPoints = fnFluid.gridSize( &s );
	if ( !s )
	{
		return 0;
	}
	unsigned int xRes, yRes, zRes;
	int xVel, yVel, zVel;
	if ( !fnFluid.getResolution( xRes, yRes, zRes ) )
	{
		return 0;
	}

	V3fVectorDataPtr positionsData = new V3fVectorData;
	V3fVectorDataPtr velocitiesData = new V3fVectorData;
	V3fVectorData::ValueType positions = positionsData->writable();
	V3fVectorData::ValueType velocities = velocitiesData->writable();
	positions.resize( nPoints );
	velocities.resize( nPoints );

	float *velX, *velY, *velZ;
	if ( !fnFluid.getVelocity( velX, velY, velZ ) )
	{
		return 0;
	}
	
	MPoint centerPos;
	for ( unsigned int x = 0; x < xRes; x++ )
	{
		for ( unsigned int y = 0; y < yRes; y++ )
		{
			for ( unsigned int z = 0; z < zRes; z++ )
			{
				int p = fnFluid.index( x, y, z );
				if ( !fnFluid.voxelCenterPosition( x, y, z, centerPos ) )
				{
					return 0;
				}
				positions[p] = IECore::convert< V3f >( centerPos );
				fnFluid.index( p, xRes, yRes, zRes, xVel, yVel, zVel );
				
				/// \todo Does this work for 2D fluids? And wouldn't we like the option to convert those to ImagePrimitives instead?
				assert( velX );
				assert( velY );
				assert( velZ );
				velocities[p] = (V3f( velX[xVel], velY[yVel], velZ[zVel] ) + V3f( velX[xVel+1], velY[yVel+1], velZ[zVel+1] )) / 2;
			}
		}
	}

	// make a matrix multiply op if we need it
	// and if we have the right information available
	MatrixMultiplyOpPtr matrixMultiplier = 0;
	if( space()==MSpace::kWorld )
	{
		const MDagPath *d = dagPath( true );
		if( d )
		{
			matrixMultiplier = new MatrixMultiplyOp;
			matrixMultiplier->copyParameter()->setTypedValue( false ); // modify input in place
			matrixMultiplier->matrixParameter()->setValue( new M44fData( IECore::convert<M44f>( d->inclusiveMatrix() ) ) );
		}
	}

	PointsPrimitivePtr pp = new PointsPrimitive( nPoints );
	
	if( matrixMultiplier )
	{
		matrixMultiplier->inputParameter()->setValue( positionsData );
		matrixMultiplier->operate();
	}
	pp->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, positionsData );

	if( m_velocityParameter->getTypedValue() )
	{
		if( matrixMultiplier )
		{
			matrixMultiplier->inputParameter()->setValue( velocitiesData );
			matrixMultiplier->modeParameter()->setNumericValue( MatrixMultiplyOp::Vector );
			matrixMultiplier->operate();
		}
		pp->variables["velocity"] = PrimitiveVariable( PrimitiveVariable::Vertex, velocitiesData );
	}

	if( m_densityParameter->getTypedValue() )
	{
		addPrimVar( pp, "density", nPoints, fnFluid, &MFnFluid::density );
	}
	
	if( m_pressureParameter->getTypedValue() )
	{
		addPrimVar( pp, "pressure", nPoints, fnFluid, &MFnFluid::pressure );
	}

	if( m_temperatureParameter->getTypedValue() )
	{
		addPrimVar( pp, "temperature", nPoints, fnFluid, &MFnFluid::temperature );
	}

	if( m_fuelParameter->getTypedValue() )
	{
		addPrimVar( pp, "fuel", nPoints, fnFluid, &MFnFluid::fuel );
	}

	if( m_falloffParameter->getTypedValue() )
	{
		addPrimVar( pp, "falloff", nPoints, fnFluid, &MFnFluid::falloff );
	}

	if( m_colorParameter->getTypedValue() )
	{
		float *redPtr;
		float *greenPtr;
		float *bluePtr;
		if ( fnFluid.getColors( redPtr, greenPtr, bluePtr ) )
		{
			assert( redPtr );
			assert( greenPtr );
			assert( bluePtr );
			V3fVectorDataPtr colorData = new V3fVectorData();
			V3fVectorData::ValueType &colorVector = colorData->writable();
			colorVector.resize( nPoints );
			for ( unsigned i = 0; i < nPoints; i++ )
			{
				colorVector[i] = V3f( redPtr[i], greenPtr[i], bluePtr[i] );
			}
			PrimitiveVariable varColor( PrimitiveVariable::Vertex, colorData );
			
			/// \todo Why not "Cs"? 
			pp->variables["color"] = varColor;
		}
	}

	if(m_textureCoordinatesParameter->getTypedValue() )
	{
		float *uPtr;
		float *vPtr;
		float *wPtr;
		if ( fnFluid.getCoordinates( uPtr, vPtr, wPtr ) )
		{
			if ( wPtr == NULL )
			{
				assert( uPtr );
				assert( vPtr );
				V2fVectorDataPtr uvData = new V2fVectorData();
				V2fVectorData::ValueType &uvVector = uvData->writable();
				uvVector.resize( nPoints );
				for ( unsigned i = 0; i < nPoints; i++ )
				{
					uvVector[i] = V2f( uPtr[i], vPtr[i] );
				}
				PrimitiveVariable varUV( PrimitiveVariable::Vertex, uvData );
				pp->variables["uv"] = varUV; /// \todo Why not s,t?
			}
			else
			{
				assert( uPtr );
				assert( vPtr );
				V3fVectorDataPtr uvwData = new V3fVectorData();
				V3fVectorData::ValueType &uvwVector = uvwData->writable();
				uvwVector.resize( nPoints );
				for ( unsigned i = 0; i < nPoints; i++ )
				{
					uvwVector[i] = V3f( uPtr[i], vPtr[i], wPtr[i] );
				}
				PrimitiveVariable varUVW( PrimitiveVariable::Vertex, uvwData );
				pp->variables["uvw"] = varUVW; /// \todo Why not s,t,u or perhaps __Pref?
			}
		}
	}

	return pp;
}
