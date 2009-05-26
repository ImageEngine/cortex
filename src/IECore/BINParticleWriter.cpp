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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/BINParticleWriter.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/FileNameParameter.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/DataCastOp.h"

using namespace IECore;
using namespace std;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( BINParticleWriter )

const Writer::WriterDescription<BINParticleWriter> BINParticleWriter::m_writerDescription( "bin" );

BINParticleWriter::BINParticleWriter( )
		:	ParticleWriter( "BINParticleWriter", "Creates particle files in Realflow binary format" )
{
	constructParameters();
}

BINParticleWriter::BINParticleWriter( ObjectPtr object, const std::string &fileName )
		:	ParticleWriter( "BINParticleWriter", "Creates particle files in Realflow binary format" )
{

	constructParameters();

	m_objectParameter->setValue( object );
	m_fileNameParameter->setTypedValue( fileName );
}

void BINParticleWriter::constructParameters()
{
	/// \todo Presets and correct values for fluid type
	m_fluidTypeParameter = new IntParameter(
	        "fluidType",
	        "Fluid type",
	        0
	);

	m_frameNumberParameter = new IntParameter(
	        "frameNumber",
	        "Frame number",
	        0
	);

	m_radiusParameter = new FloatParameter(
	        "radius",
	        "radius",
	        1.0f
	);

	m_scaleSceneParameter = new FloatParameter(
	        "scaleScene",
	        "Scale scene",
	        1.0f
	);

	m_elapsedSimulationTimeParameter = new FloatParameter(
	        "elapsedSimulationTime",
	        "Elapsed simulation time",
	        0.0f
	);

	m_frameRateParameter = new IntParameter(
	        "frameRate",
	        "Frame rate",
	        24
	);

	m_emitterPositionParameter  = new V3fParameter(
	        "emitterPosition",
	        "Emitter position",
	        V3f( 0.0f, 0.0f, 0.0f )
	);

	m_emitterRotationParameter  = new V3fParameter(
	        "emitterRotation",
	        "Emitter rotation",
	        V3f( 0.0f, 0.0f, 0.0f )
	);

	m_emitterScaleParameter  = new V3fParameter(
	        "emitterScale",
	        "Emitter scale",
	        V3f( 1.0f, 1.0f, 1.0f )
	);

	m_positionPrimVarParameter = new StringParameter(
	        "positionPrimVar",
	        "Name of the attribute containing position data",
	        "position"
	);

	m_velocityPrimVarParameter = new StringParameter(
	        "velocityPrimVar",
	        "Name of the attribute containing velocity data",
	        "velocity"
	);

	m_forcePrimVarParameter = new StringParameter(
	        "forcePrimVar",
	        "Name of the attribute containing force data",
	        ""
	);

	m_vortisityPrimVarParameter = new StringParameter(
	        "vortisityPrimVar",
	        "Name of the attribute containing vortisity data",
	        ""
	);

	m_normalPrimVarParameter = new StringParameter(
	        "normalPrimVar",
	        "Name of the attribute containing normal data",
	        ""
	);

	m_numNeighboursPrimVarParameter = new StringParameter(
	        "numNeighboursPrimVar",
	        "Name of the attribute containing numNeighbours data",
	        ""
	);

	m_uvwPrimVarParameter = new StringParameter(
	        "uvwPrimVar",
	        "Name of the attribute containing UVW texture data",
	        ""
	);

	m_agePrimVarParameter = new StringParameter(
	        "agePrimVar",
	        "Name of the attribute containing particle age data",
	        ""
	);

	m_isolationTimePrimVarParameter = new StringParameter(
	        "isolationTimePrimVar",
	        "Name of the attribute containing isolation time data",
	        ""
	);


	m_viscosityPrimVarParameter = new StringParameter(
	        "viscosityPrimVar",
	        "Name of the attribute containing viscosity data",
	        ""
	);

	m_densityPrimVarParameter = new StringParameter(
	        "densityPrimVar",
	        "Name of the attribute containing density data",
	        ""
	);

	m_pressurePrimVarParameter = new StringParameter(
	        "pressurePrimVar",
	        "Name of the attribute containing pressure data",
	        ""
	);

	m_massPrimVarParameter = new StringParameter(
	        "massPrimVar",
	        "Name of the attribute containing mass data",
	        "mass"
	);

	m_temperaturePrimVarParameter = new StringParameter(
	        "temperaturePrimVar",
	        "Name of the attribute containing temperature data",
	        ""
	);

	m_particleIdPrimVarParameter = new StringParameter(
	        "particleIdPrimVar",
	        "Name of the attribute containing particleId data",
	        "particleId"
	);

	parameters()->addParameter( m_fluidTypeParameter );
	parameters()->addParameter( m_frameNumberParameter );
	parameters()->addParameter( m_radiusParameter );
	parameters()->addParameter( m_scaleSceneParameter );
	parameters()->addParameter( m_elapsedSimulationTimeParameter );
	parameters()->addParameter( m_frameRateParameter );

	parameters()->addParameter( m_emitterPositionParameter );
	parameters()->addParameter( m_emitterRotationParameter );
	parameters()->addParameter( m_emitterScaleParameter );

	parameters()->addParameter( m_positionPrimVarParameter );
	parameters()->addParameter( m_velocityPrimVarParameter );
	parameters()->addParameter( m_forcePrimVarParameter );
	parameters()->addParameter( m_vortisityPrimVarParameter );
	parameters()->addParameter( m_normalPrimVarParameter );
	parameters()->addParameter( m_numNeighboursPrimVarParameter );
	parameters()->addParameter( m_uvwPrimVarParameter );
	parameters()->addParameter( m_agePrimVarParameter );
	parameters()->addParameter( m_isolationTimePrimVarParameter );
	parameters()->addParameter( m_viscosityPrimVarParameter );
	parameters()->addParameter( m_densityPrimVarParameter );
	parameters()->addParameter( m_pressurePrimVarParameter );
	parameters()->addParameter( m_massPrimVarParameter );
	parameters()->addParameter( m_temperaturePrimVarParameter );
	parameters()->addParameter( m_particleIdPrimVarParameter );
}

IntParameterPtr BINParticleWriter::fluidTypeParameter()
{
	return m_fluidTypeParameter;
}

ConstIntParameterPtr BINParticleWriter::fluidTypeParameter() const
{
	return m_fluidTypeParameter;
}

IntParameterPtr BINParticleWriter::frameNumberParameter()
{
	return m_frameNumberParameter;
}

ConstIntParameterPtr BINParticleWriter::frameNumberParameter() const
{
	return m_frameNumberParameter;
}

FloatParameterPtr BINParticleWriter::radiusParameter()
{
	return m_radiusParameter;
}

ConstFloatParameterPtr BINParticleWriter::radiusParameter() const
{
	return m_radiusParameter;
}

FloatParameterPtr BINParticleWriter::scaleSceneParameter()
{
	return m_scaleSceneParameter;
}

ConstFloatParameterPtr BINParticleWriter::scaleSceneParameter() const
{
	return m_scaleSceneParameter;
}

FloatParameterPtr BINParticleWriter::elapsedSimulationTimeParameter()
{
	return m_elapsedSimulationTimeParameter;
}

ConstFloatParameterPtr BINParticleWriter::elapsedSimulationTimeParameter() const
{
	return m_elapsedSimulationTimeParameter;
}

IntParameterPtr BINParticleWriter::frameRateParameter()
{
	return m_frameRateParameter;
}

ConstIntParameterPtr BINParticleWriter::frameRateParameter() const
{
	return m_frameRateParameter;
}

StringParameterPtr BINParticleWriter::positionPrimVarParameter()
{
	return m_positionPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::positionPrimVarParameter() const
{
	return m_positionPrimVarParameter;
}

StringParameterPtr BINParticleWriter::velocityPrimVarParameter()
{
	return m_velocityPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::velocityPrimVarParameter() const
{
	return m_velocityPrimVarParameter;
}

StringParameterPtr BINParticleWriter::forcePrimVarParameter()
{
	return m_forcePrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::forcePrimVarParameter() const
{
	return m_forcePrimVarParameter;
}

StringParameterPtr BINParticleWriter::vortisityPrimVarParameter()
{
	return m_vortisityPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::vortisityPrimVarParameter() const
{
	return m_vortisityPrimVarParameter;
}

StringParameterPtr BINParticleWriter::normalPrimVarParameter()
{
	return m_normalPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::normalPrimVarParameter() const
{
	return m_normalPrimVarParameter;
}

StringParameterPtr BINParticleWriter::numNeighboursPrimVarParameter()
{
	return m_numNeighboursPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::numNeighboursPrimVarParameter() const
{
	return m_numNeighboursPrimVarParameter;
}

StringParameterPtr BINParticleWriter::uvwPrimVarParameter()
{
	return m_uvwPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::uvwPrimVarParameter() const
{
	return m_uvwPrimVarParameter;
}

StringParameterPtr BINParticleWriter::agePrimVarParameter()
{
	return m_agePrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::agePrimVarParameter() const
{
	return m_agePrimVarParameter;
}

StringParameterPtr BINParticleWriter::isolationTimePrimVarParameter()
{
	return m_isolationTimePrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::isolationTimePrimVarParameter() const
{
	return m_isolationTimePrimVarParameter;
}

StringParameterPtr BINParticleWriter::viscosityPrimVarParameter()
{
	return m_viscosityPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::viscosityPrimVarParameter() const
{
	return m_viscosityPrimVarParameter;
}

StringParameterPtr BINParticleWriter::densityPrimVarParameter()
{
	return m_densityPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::densityPrimVarParameter() const
{
	return m_densityPrimVarParameter;
}

StringParameterPtr BINParticleWriter::pressurePrimVarParameter()
{
	return m_pressurePrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::pressurePrimVarParameter() const
{
	return m_pressurePrimVarParameter;
}

StringParameterPtr BINParticleWriter::massPrimVarParameter()
{
	return m_massPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::massPrimVarParameter() const
{
	return m_massPrimVarParameter;
}

StringParameterPtr BINParticleWriter::temperaturePrimVarParameter()
{
	return m_temperaturePrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::temperaturePrimVarParameter() const
{
	return m_temperaturePrimVarParameter;
}

StringParameterPtr BINParticleWriter::particleIdPrimVarParameter()
{
	return m_particleIdPrimVarParameter;
}

ConstStringParameterPtr BINParticleWriter::particleIdPrimVarParameter() const
{
	return m_particleIdPrimVarParameter;
}

template<typename T>
static void writeLittleEndian( std::ostream &f, const T &n )
{
	const T x = asLittleEndian<>( n );
	f.write(( const char* ) &x, sizeof( T ) );
}

template<>
void writeLittleEndian< Imath::V3f >( std::ostream &f, const Imath::V3f &n )
{
	for ( unsigned i = 0; i < 3; i ++ )
	{
		const float x = asLittleEndian<>( n[i] );
		f.write(( const char* ) &x, sizeof( float ) );
	}
}

template<typename T>
typename T::ConstPtr BINParticleWriter::getPrimVar( ConstStringParameterPtr parameter )
{
	assert( parameter );

	const std::string &primVarName = parameter->getTypedValue();
	if ( primVarName == "" )
	{
		return 0;
	}

	const PrimitiveVariableMap &pv = particleObject()->variables;
	PrimitiveVariableMap::const_iterator it = pv.find( primVarName );

	if ( it == pv.end() )
	{
		throw InvalidArgumentException(( boost::format( "BINParticleWriter: Couldn't find primitive variable '%s'" ) % primVarName ).str() );
	}

	DataPtr data = it->second.data;
	if ( !data )
	{
		throw InvalidArgumentException(( boost::format( "BINParticleWriter: Couldn't find data for primitive variable '%s'" ) % primVarName ).str() );
	}

	/// Cast data to the desired type
	DataCastOpPtr op = new DataCastOp();
	op->objectParameter()->setValue( data );
	op->targetTypeParameter()->setNumericValue( T::staticTypeId() );

	typename T::ConstPtr result = assertedStaticCast<const T>( op->operate() );
	if ( result->readable().size() != particleCount() )
	{
		throw InvalidArgumentException(( boost::format( "BINParticleWriter: Invalid data length for primitive variable '%s'" ) % primVarName ).str() );
	}

	return result;
}

template<typename T>
void BINParticleWriter::writeParticlePrimVar( std::ofstream &f, typename T::ConstPtr data, uint32_t i ) const
{
	typename T::ValueType::value_type v;
	if ( !data )
	{
		v = typename T::ValueType::value_type();
	}
	else
	{
		assert( data->readable().size() );
		v = data->readable().size() == 1 ? data->readable()[0] : data->readable()[i];
	}

	writeLittleEndian( f, v );
}

void BINParticleWriter::getMaxMinAvg( ConstFloatVectorDataPtr data, float &mx, float &mn, float &avg ) const
{
	if ( !data )
	{
		mx = 0.0f;
		mn = 0.0f;
		avg = 0.0f;
		return;
	}

	mx = Imath::limits<float>::min();
	mn = Imath::limits<float>::max();
	avg = 0.0;

	const FloatVectorData::ValueType &valueArray = data->readable();
	for ( FloatVectorData::ValueType::const_iterator it = valueArray.begin(); it != valueArray.end(); ++it )
	{
		if ( *it < mn ) mn = *it;
		if ( *it > mx ) mx = *it;
		avg += *it;
	}
	avg /= ( float ) valueArray.size();
}

void BINParticleWriter::doWrite()
{
	ConstV3fVectorDataPtr positionData = getPrimVar< V3fVectorData >( m_positionPrimVarParameter );
	ConstV3fVectorDataPtr velocityData = getPrimVar< V3fVectorData >( m_velocityPrimVarParameter );
	ConstV3fVectorDataPtr forceData = getPrimVar< V3fVectorData >( m_forcePrimVarParameter );
	ConstV3fVectorDataPtr vortisityData = getPrimVar< V3fVectorData >( m_vortisityPrimVarParameter );
	ConstV3fVectorDataPtr normalData = getPrimVar< V3fVectorData >( m_normalPrimVarParameter );
	ConstIntVectorDataPtr numNeighboursData = getPrimVar< IntVectorData >( m_numNeighboursPrimVarParameter );
	ConstV3fVectorDataPtr uvwData = getPrimVar< V3fVectorData >( m_uvwPrimVarParameter );
	ConstFloatVectorDataPtr ageData = getPrimVar< FloatVectorData >( m_agePrimVarParameter );
	ConstFloatVectorDataPtr isolationTimeData = getPrimVar< FloatVectorData >( m_isolationTimePrimVarParameter );
	ConstFloatVectorDataPtr viscosityData = getPrimVar< FloatVectorData >( m_viscosityPrimVarParameter );
	ConstFloatVectorDataPtr densityData = getPrimVar< FloatVectorData >( m_densityPrimVarParameter );
	ConstFloatVectorDataPtr pressureData = getPrimVar< FloatVectorData >( m_pressurePrimVarParameter );
	ConstFloatVectorDataPtr massData = getPrimVar< FloatVectorData >( m_massPrimVarParameter );
	ConstFloatVectorDataPtr temperatureData = getPrimVar< FloatVectorData >( m_temperaturePrimVarParameter );
	ConstIntVectorDataPtr particleIdData = getPrimVar< IntVectorData >( m_particleIdPrimVarParameter );

	ofstream f( fileName().c_str() );
	if ( !f.is_open() )
	{
		throw IOException(( boost::format( "Unable to open file \%s\"." ) % fileName() ).str() );
	}

	uint32_t magic = 0xFABADA;
	char fluidName[250] = { 0 };
	short version = 9;

	float scaleScene = m_scaleSceneParameter->getNumericValue();
	uint32_t fluidType = m_fluidTypeParameter->getNumericValue();
	float elapsedSimulationTime = m_elapsedSimulationTimeParameter->getNumericValue();
	uint32_t frameNumber = m_frameNumberParameter->getNumericValue();
	uint32_t framesPerSecond = m_frameRateParameter->getNumericValue();

	uint32_t numParticles = particleCount();

	float radius = m_radiusParameter->getNumericValue();

	float pressureMax = 0.0f, pressureMin = 0.0f, pressureAvg = 0.0f;
	float speedMax = 0.0f, speedMin = 0.0f, speedAvg = 0.0f;
	float temperatureMax = 0.0f, temperatureMin = 0.0f, temperatureAvg = 0.0f;

	if ( numParticles )
	{
		/// Compute speed from velocity
		speedMax = Imath::limits<float>::min();
		speedMin = Imath::limits<float>::max();
		uint32_t numVelocity = velocityData->readable().size();
		for ( uint32_t i = 0; i < numVelocity; i++ )
		{
			float speed = velocityData->readable()[i].length();
			if ( speed > speedMax ) speedMax = speed;
			if ( speed < speedMin ) speedMin = speed;
			speedAvg += speed;
		}
		speedAvg /= ( float ) numVelocity;

		getMaxMinAvg( pressureData, pressureMax, pressureMin, pressureAvg );
		getMaxMinAvg( temperatureData, temperatureMax, temperatureMin, temperatureAvg );
	}

	const V3f &emitterPosition = m_emitterPositionParameter->getTypedValue();
	const V3f &emitterRotation = m_emitterRotationParameter->getTypedValue();
	const V3f &emitterScale = m_emitterScaleParameter->getTypedValue();

	writeLittleEndian( f, magic );
	f.write( fluidName, 250 );
	writeLittleEndian( f, version );

	writeLittleEndian( f, fluidType );
	writeLittleEndian( f, scaleScene );
	writeLittleEndian( f, elapsedSimulationTime );
	writeLittleEndian( f, frameNumber );
	writeLittleEndian( f, framesPerSecond );
	writeLittleEndian( f, numParticles );
	writeLittleEndian( f, radius );

	writeLittleEndian( f, pressureMax );
	writeLittleEndian( f, pressureMin );
	writeLittleEndian( f, pressureAvg );

	writeLittleEndian( f, speedMax );
	writeLittleEndian( f, speedMin );
	writeLittleEndian( f, speedAvg );

	writeLittleEndian( f, temperatureMax );
	writeLittleEndian( f, temperatureMin );
	writeLittleEndian( f, temperatureAvg );

	writeLittleEndian( f, emitterPosition );
	writeLittleEndian( f, emitterRotation );
	writeLittleEndian( f, emitterScale );

	for ( uint32_t i = 0; i < numParticles; i ++ )
	{
		writeParticlePrimVar< V3fVectorData >( f, positionData, i );
		writeParticlePrimVar< V3fVectorData >( f, velocityData, i );
		writeParticlePrimVar< V3fVectorData >( f, forceData, i );
		writeParticlePrimVar< V3fVectorData >( f, vortisityData, i );
		writeParticlePrimVar< V3fVectorData >( f, normalData, i );
		writeParticlePrimVar< IntVectorData >( f, numNeighboursData, i );
		writeParticlePrimVar< V3fVectorData >( f, uvwData, i );
		short infoBits = 0;
		writeLittleEndian( f, infoBits );
		writeParticlePrimVar< FloatVectorData >( f, ageData, i );
		writeParticlePrimVar< FloatVectorData >( f, isolationTimeData, i );
		writeParticlePrimVar< FloatVectorData >( f, viscosityData, i );
		writeParticlePrimVar< FloatVectorData >( f, densityData, i );
		writeParticlePrimVar< FloatVectorData >( f, pressureData, i );
		writeParticlePrimVar< FloatVectorData >( f, massData, i );
		writeParticlePrimVar< FloatVectorData >( f, temperatureData, i );
		writeParticlePrimVar< IntVectorData >( f, particleIdData, i );
	}

	if ( f.fail() )
	{
		throw IOException(( boost::format( "Error writing file \%s\"." ) % fileName() ).str() );
	}
}
