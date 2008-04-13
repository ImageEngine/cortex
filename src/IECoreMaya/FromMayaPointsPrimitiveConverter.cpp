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

#include "IECoreMaya/FromMayaPointsPrimitiveConverter.h"
#include "IECoreMaya/Convert.h"

#include "IECore/CompoundParameter.h"
#include "IECore/PointsPrimitive.h"

#include "maya/MFn.h"
#include "maya/MFnFluid.h"

using namespace IECoreMaya;
using namespace IECore;
using namespace std;
using namespace Imath;

FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaPointsPrimitiveConverter > FromMayaPointsPrimitiveConverter::m_description( MFn::kFluid, PointsPrimitive::staticTypeId() );

FromMayaPointsPrimitiveConverter::FromMayaPointsPrimitiveConverter( const MObject &object )
	:	FromMayaObjectConverter( "FromMayaPointsPrimitiveConverter", "Converts maya fluid data to IECore::PointsPrimitive Object", object )
{
	m_velocity = new BoolParameter(
		"velocity",
		"When this is on the fluid's velocities are added to the result as a primitive variable named \"velocity\".",
		true
	);
	parameters()->addParameter( m_velocity );

	m_density = new BoolParameter(
		"density",
		"When this is on the fluid's densities are added to the result as a primitive variable named \"density\".",
		true
	);
	parameters()->addParameter( m_density );

	m_pressure = new BoolParameter(
		"pressure",
		"When this is on the fluid's pressures are added to the result as a primitive variable named \"pressure\".",
		true
	);
	parameters()->addParameter( m_pressure );

	m_temperature = new BoolParameter(
		"temperature",
		"When this is on the fluid's temperatures are added to the result as a primitive variable named \"temperature\".",
		true
	);
	parameters()->addParameter( m_temperature );

	m_fuel = new BoolParameter(
		"fuel",
		"When this is on the fluid's fuel is added to the result as a primitive variable named \"fuel\".",
		true
	);
	parameters()->addParameter( m_fuel );

	m_falloff = new BoolParameter(
		"falloff",
		"When this is on the fluid's falloff is added to the result as a primitive variable named \"falloff\".",
		true
	);
	parameters()->addParameter( m_falloff );

	m_color = new BoolParameter(
		"color",
		"When this is on the fluid's colors are added to the result as a primitive variable named \"color\".",
		true
	);
	parameters()->addParameter( m_color );

	m_textureCoordinates = new BoolParameter(
		"textureCoordinates",
		"When this is on the fluid's texture coordinates are added to the result as a primitive variable named \"uvw\" or \"uv\".",
		true
	);
	parameters()->addParameter( m_textureCoordinates );

}

#define ADD_PRIMVAR( NAME, MAYA_FUNC )												\
		float *mayaPtr = MAYA_FUNC( &s );											\
		if ( s )																	\
		{																			\
			FloatVectorDataPtr floatData = new FloatVectorData();					\
			floatData->writable().resize( nPoints );								\
			std::copy( mayaPtr, mayaPtr + nPoints, &(floatData->writable()[0]) );	\
			PrimitiveVariable newVar( PrimitiveVariable::Vertex, floatData );		\
			pp->variables[ NAME ] = newVar;											\
		}

IECore::ObjectPtr FromMayaPointsPrimitiveConverter::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;
	MFnFluid fnFluid( object );
	if( !fnFluid.hasObj( object ) )
	{
		return 0;
	}
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

	V3fVectorData::ValueType velocities, positions;
	velocities.resize( nPoints );
	positions.resize( nPoints );

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
				velocities[p] = (V3f( velX[xVel], velY[yVel], velZ[zVel] ) + V3f( velX[xVel+1], velY[yVel+1], velZ[zVel+1] )) / 2;
			}
		}
	}

	PointsPrimitivePtr pp = new PointsPrimitive( nPoints );
	PrimitiveVariable varP( PrimitiveVariable::Vertex, new V3fVectorData( positions ) );
	pp->variables["P"] = varP;

	if( m_velocity->getTypedValue() )
	{
		PrimitiveVariable varVelocity( PrimitiveVariable::Vertex, new V3fVectorData( velocities ) );
		pp->variables["velocity"] = varVelocity;
	}

	if( m_density->getTypedValue() )
	{
		ADD_PRIMVAR( "density", fnFluid.density )
	}
	
	if(	m_pressure->getTypedValue() )
	{
		ADD_PRIMVAR( "pressure", fnFluid.pressure )
	}

	if( m_temperature->getTypedValue() )
	{
		ADD_PRIMVAR( "temperature", fnFluid.temperature )
	}

	if( m_fuel->getTypedValue() )
	{
		ADD_PRIMVAR( "fuel", fnFluid.fuel )
	}

	if(m_falloff->getTypedValue() )
	{
		ADD_PRIMVAR( "falloff", fnFluid.falloff )
	}

	if(m_color->getTypedValue() )
	{
		float *redPtr;
		float *greenPtr;
		float *bluePtr;
		if ( fnFluid.getColors( redPtr, greenPtr, bluePtr ) )
		{
			V3fVectorDataPtr colorData = new V3fVectorData();
			V3fVectorData::ValueType &colorVector = colorData->writable();
			colorVector.resize( nPoints );
			for ( unsigned i = 0; i < nPoints; i++ )
			{
				colorVector[i] = V3f( redPtr[i], greenPtr[i], bluePtr[i] );
			}
			PrimitiveVariable varColor( PrimitiveVariable::Vertex, colorData );
			pp->variables["color"] = varColor;
		}
	}

	if(m_textureCoordinates->getTypedValue() )
	{
		float *uPtr;
		float *vPtr;
		float *wPtr;
		if ( fnFluid.getCoordinates( uPtr, vPtr, wPtr ) )
		{
			if ( wPtr == NULL )
			{
				V2fVectorDataPtr uvData = new V2fVectorData();
				V2fVectorData::ValueType &uvVector = uvData->writable();
				uvVector.resize( nPoints );
				for ( unsigned i = 0; i < nPoints; i++ )
				{
					uvVector[i] = V2f( uPtr[i], vPtr[i] );
				}
				PrimitiveVariable varUV( PrimitiveVariable::Vertex, uvData );
				pp->variables["uv"] = varUV;
			}
			else
			{
				V3fVectorDataPtr uvwData = new V3fVectorData();
				V3fVectorData::ValueType &uvwVector = uvwData->writable();
				uvwVector.resize( nPoints );
				for ( unsigned i = 0; i < nPoints; i++ )
				{
					uvwVector[i] = V3f( uPtr[i], vPtr[i], wPtr[i] );
				}
				PrimitiveVariable varUVW( PrimitiveVariable::Vertex, uvwData );
				pp->variables["uvw"] = varUVW;
			}
		}
	}

	return pp;
}
