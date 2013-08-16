//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
#include <cassert>

#include "OpenEXR/ImathBox.h"

#include "IECore/Exception.h"
#include "IECore/VectorOps.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/Reader.h"
#include "IECore/ParticleReader.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/MeshPrimitiveBuilder.h"
#include "IECore/CachedImplicitSurfaceFunction.h"
#include "IECore/VectorTraits.h"
#include "IECore/MarchingCubes.h"
#include "IECore/BlobbyImplicitSurfaceFunction.h"
#include "IECore/ParticleMeshOp.h"
#include "IECore/PointMeshOp.h"
#include "IECore/PointBoundsOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

static TypeId resultTypes[] = { MeshPrimitiveTypeId, InvalidTypeId };

IE_CORE_DEFINERUNTIMETYPED( ParticleMeshOp );

ParticleMeshOp::ParticleMeshOp()
	:	Op(
		"Calculates mesh from an isosurface defined by a set of points.",
		new ObjectParameter(
			"result",
			"Mesh calculated from the points.",
			new MeshPrimitive(),
			resultTypes
		)
	)
{

	m_fileNameParameter = new FileNameParameter(
		"filename",
		"Filename of PDC to generate mesh from",
		""
	);

	m_positionAttributeParameter = new StringParameter(
		"positionAttribute",
		"Name of attribute specifying particle positions",
		"worldPosition"
	);

	m_useRadiusAttributeParameter = new BoolParameter(
		"useRadiusAttribute",
		"Use per-particle radii",
		true
	);

	m_radiusAttributeParameter = new StringParameter(
		"radiusAttribute",
		"Name of attribute specifying radii",
		"radiusPP"
	);

	m_radiusParameter = new FloatParameter(
		"radius",
		"Radius to use when not reading an attribute",
		1.0
	);

	m_radiusScaleParameter = new FloatParameter(
		"radiusScale",
		"Factor to multiply all radii by",
		1.0
	);

	m_useStrengthAttributeParameter = new BoolParameter(
		"useStrengthAttribute",
		"Use per-particle strength",
		false
	);

	m_strengthAttributeParameter = new StringParameter(
		"strengthAttribute",
		"Name of attribute specifying strength",
		"strengthPP"
	);

	m_strengthParameter = new FloatParameter(
		"strength",
		"Strength to use when not reading an attribute",
		1.0
	);

	m_strengthScaleParameter = new FloatParameter(
		"strengthScale",
		"Factor to multiply all strength by",
		1.0
	);

	m_thresholdParameter = new FloatParameter(
		"threshold",
		"The threshold at which to generate the surface.",
		0.0
	);

	m_resolutionParameter = new V3iParameter(
		"resolution",
		"The resolution",
		V3i( 10, 10, 10 )
	);

	m_automaticBoundParameter = new BoolParameter(
		"automaticBound",
		"Enable to calculate the bound automatically. Disable to specify an explicit bound.",
		true
	);

	m_boundExtendParameter = new FloatParameter(
		"boundExtend",
		"The bound's radius, even if calculated by automatic bounding, is increased by this amount.",
		0.0,
		0.0
	);

	m_boundParameter = new Box3fParameter(
		"bound",
		"The bound",
		Box3f( V3f( -1, -1, -1 ), V3f( 1, 1, 1 ) )
	);

	IntParameter::PresetsContainer gridMethodPresets;
	gridMethodPresets.push_back( IntParameter::Preset( "Resolution", Resolution ) );
	gridMethodPresets.push_back( IntParameter::Preset( "Division Size", DivisionSize ) );

	m_gridMethodParameter = new IntParameter(
		"gridMethod",
		"s",
		Resolution,
		Resolution,
		DivisionSize,
		gridMethodPresets,
		true
	);

	m_divisionSizeParameter = new V3fParameter(
		"divisionSize",
		"The dimensions of each element in the grid",
		V3f( 1, 1, 1 )
	);

	parameters()->addParameter( m_fileNameParameter );
	parameters()->addParameter( m_positionAttributeParameter );
	parameters()->addParameter( m_useRadiusAttributeParameter );
	parameters()->addParameter( m_radiusAttributeParameter );
	parameters()->addParameter( m_radiusParameter );
	parameters()->addParameter( m_radiusScaleParameter );
	parameters()->addParameter( m_useStrengthAttributeParameter );
	parameters()->addParameter( m_strengthAttributeParameter );
	parameters()->addParameter( m_strengthParameter );
	parameters()->addParameter( m_strengthScaleParameter );
	parameters()->addParameter( m_thresholdParameter );
	parameters()->addParameter( m_gridMethodParameter );
	parameters()->addParameter( m_resolutionParameter );
	parameters()->addParameter( m_divisionSizeParameter );
	parameters()->addParameter( m_automaticBoundParameter );
	parameters()->addParameter( m_boundExtendParameter );
	parameters()->addParameter( m_boundParameter );

	/// \todo Allow use of particle cache sequence, rather than single file
}

ParticleMeshOp::~ParticleMeshOp()
{
}

FileNameParameter * ParticleMeshOp::fileNameParameter()
{
	return m_fileNameParameter;
}

const FileNameParameter * ParticleMeshOp::fileNameParameter() const
{
	return m_fileNameParameter;
}

StringParameter * ParticleMeshOp::positionAttributeParameter()
{
	return m_positionAttributeParameter;
}

const StringParameter * ParticleMeshOp::positionAttributeParameter() const
{
	return m_positionAttributeParameter;
}

BoolParameter * ParticleMeshOp::useRadiusAttributeParameter()
{
	return m_useRadiusAttributeParameter;
}

const BoolParameter * ParticleMeshOp::useRadiusAttributeParameter() const
{
	return m_useRadiusAttributeParameter;
}

StringParameter * ParticleMeshOp::radiusAttributeParameter()
{
	return m_radiusAttributeParameter;
}

const StringParameter * ParticleMeshOp::radiusAttributeParameter() const
{
	return m_radiusAttributeParameter;
}

FloatParameter * ParticleMeshOp::radiusParameter()
{
	return m_radiusParameter;
}

const FloatParameter * ParticleMeshOp::radiusParameter() const
{
	return m_radiusParameter;
}

FloatParameter * ParticleMeshOp::radiusScaleParameter()
{
	return m_radiusScaleParameter;
}

const FloatParameter * ParticleMeshOp::radiusScaleParameter() const
{
	return m_radiusScaleParameter;
}

BoolParameter * ParticleMeshOp::useStrengthAttributeParameter()
{
	return m_useStrengthAttributeParameter;
}

const BoolParameter * ParticleMeshOp::useStrengthAttributeParameter() const
{
	return m_useStrengthAttributeParameter;
}

StringParameter * ParticleMeshOp::strengthAttributeParameter()
{
	return m_strengthAttributeParameter;
}

const StringParameter * ParticleMeshOp::strengthAttributeParameter() const
{
	return m_strengthAttributeParameter;
}

FloatParameter * ParticleMeshOp::strengthParameter()
{
	return m_strengthParameter;
}

const FloatParameter * ParticleMeshOp::strengthParameter() const
{
	return m_strengthParameter;
}

FloatParameter * ParticleMeshOp::strengthScaleParameter()
{
	return m_strengthScaleParameter;
}

const FloatParameter * ParticleMeshOp::strengthScaleParameter() const
{
	return m_strengthScaleParameter;
}

FloatParameter * ParticleMeshOp::thresholdParameter()
{
	return m_thresholdParameter;
}

const FloatParameter * ParticleMeshOp::thresholdParameter() const
{
	return m_thresholdParameter;
}

V3iParameter * ParticleMeshOp::resolutionParameter()
{
	return m_resolutionParameter;
}

const V3iParameter * ParticleMeshOp::resolutionParameter() const
{
	return m_resolutionParameter;
}

Box3fParameter * ParticleMeshOp::boundParameter()
{
	return m_boundParameter;
}

const Box3fParameter * ParticleMeshOp::boundParameter() const
{
	return m_boundParameter;
}

BoolParameter * ParticleMeshOp::automaticBoundParameter()
{
	return m_automaticBoundParameter;
}

const BoolParameter * ParticleMeshOp::automaticBoundParameter() const
{
	return m_automaticBoundParameter;
}

IntParameter * ParticleMeshOp::gridMethodParameter()
{
	return m_gridMethodParameter;
}

const IntParameter * ParticleMeshOp::gridMethodParameter() const
{
	return m_gridMethodParameter;
}

V3fParameter * ParticleMeshOp::divisionSizeParameter()
{
	return m_divisionSizeParameter;
}

const V3fParameter * ParticleMeshOp::divisionSizeParameter() const
{
	return m_divisionSizeParameter;
}

FloatParameter * ParticleMeshOp::boundExtendParameter()
{
	return m_boundExtendParameter;
}

const FloatParameter * ParticleMeshOp::boundExtendParameter() const
{
	return m_boundExtendParameter;
}

ObjectPtr ParticleMeshOp::doOperation( const CompoundObject * operands )
{
	MeshPrimitiveBuilderPtr builder = new MeshPrimitiveBuilder();

	ConstObjectPtr fileNameData = fileNameParameter()->getValue();
	const std::string &fileName = staticPointerCast<const StringData>(fileNameData)->readable();

	ParticleReaderPtr reader = staticPointerCast< ParticleReader > ( Reader::create( fileName ) );
	if (!reader)
	{

		throw IOException( "Could not create reader for particle cache file" );
	}

	reader->realTypeParameter()->setValue( "native" );

	ConstObjectPtr positionAttributeData = positionAttributeParameter()->getValue();
	const std::string &positionAttribute = staticPointerCast<const StringData>(positionAttributeData)->readable();
	DataPtr positionData = reader->readAttribute( positionAttribute );

	/// \todo Detect V3fVectorData for positional information then automatically allow FloatVectorData for radius/strength
	V3dVectorDataPtr position = staticPointerCast< V3dVectorData >( positionData );
	if (!position)
	{
		throw InvalidArgumentException("Could not read position data");
	}

	DoubleVectorDataPtr radius;
	bool useRadiusAttribute = static_cast<const BoolData *>( m_useRadiusAttributeParameter->getValue() )->readable();
	if ( useRadiusAttribute )
	{
		ConstObjectPtr radiusAttributeData = radiusAttributeParameter()->getValue();
		const std::string &radiusAttribute = staticPointerCast<const StringData>(radiusAttributeData)->readable();
		DataPtr radiusData = reader->readAttribute( radiusAttribute );
		radius = staticPointerCast< DoubleVectorData >( radiusData );
		if (!radius)
		{
			throw InvalidArgumentException("Could not read radiusPP attribute data");
		}
		radius = radius->copy();
	}
	else
	{
		radius = new DoubleVectorData();
		radius->writable().resize( reader->numParticles(), m_radiusParameter->getNumericValue() );
	}


	double radiusScale = m_radiusScaleParameter->getNumericValue();
	for (DoubleVectorData::ValueType::iterator it = radius->writable().begin(); it != radius->writable().end(); ++it)
	{
		*it *= radiusScale;
	}

	DoubleVectorDataPtr strength;
	bool useStrengthAttribute = static_cast<const BoolData *>( m_useStrengthAttributeParameter->getValue() )->readable();
	if ( useStrengthAttribute )
	{
		ConstObjectPtr strengthAttributeData = strengthAttributeParameter()->getValue();
		const std::string &strengthAttribute = staticPointerCast<const StringData>(strengthAttributeData)->readable();
		DataPtr strengthData = reader->readAttribute( strengthAttribute );
		strength = staticPointerCast< DoubleVectorData >( strengthData );
		if (!strength)
		{
			throw InvalidArgumentException("Could not read strengthPP attribute data");
		}
		strength = strength->copy();
	}
	else
	{
		strength = new DoubleVectorData();
		strength->writable().resize( reader->numParticles(), m_strengthParameter->getNumericValue() );
	}

	double strengthScale = m_strengthScaleParameter->getNumericValue();
	for (DoubleVectorData::ValueType::iterator it = strength->writable().begin(); it != strength->writable().end(); ++it)
	{
		*it *= strengthScale;
	}

	if ( position->readable().size() != reader->numParticles()
		|| radius->readable().size() != reader->numParticles()
		|| strength->readable().size() != reader->numParticles() )
	{
		throw InvalidArgumentException("Position/radius/strength array lengths mismatch");
	}

	bool automaticBound = static_cast<const BoolData *>( m_automaticBoundParameter->getValue() )->readable();
	Box3f bound;

	if (automaticBound)
	{
		PointBoundsOpPtr pointBoundsOp = new PointBoundsOp();

		pointBoundsOp->pointParameter()->setValue( positionData );

		pointBoundsOp->radiusParameter()->setValue( radius );

		ObjectPtr result = pointBoundsOp->operate();

		Box3fDataPtr boxResult = runTimeCast<Box3fData> ( result );

		assert( boxResult );

		bound = boxResult->readable();
	}
	else
	{
		bound = static_cast<const Box3fData *>( m_boundParameter->getValue() )->readable();
	}

	double boundExtend = m_boundExtendParameter->getNumericValue();
	bound.min -= V3f( boundExtend, boundExtend, boundExtend );
	bound.max += V3f( boundExtend, boundExtend, boundExtend );


	V3i resolution;
	int gridMethod = m_gridMethodParameter->getNumericValue();
	if ( gridMethod == Resolution )
	{
		resolution = static_cast<const V3iData *>( m_resolutionParameter->getValue() )->readable();
	}
	else if ( gridMethod == DivisionSize )
	{
		V3f divisionSize = static_cast<const V3fData *>( m_divisionSizeParameter->getValue() )->readable();

		resolution.x = (int)((bound.max.x - bound.min.x) / divisionSize.x);
		resolution.y = (int)((bound.max.y - bound.min.y) / divisionSize.y);
		resolution.z = (int)((bound.max.z - bound.min.z) / divisionSize.z);

	}
	else
	{
		assert( false );
	}

	PointMeshOpPtr pointMeshOp = new PointMeshOp();

	pointMeshOp->pointParameter()->setValue( position->copy() );
	pointMeshOp->radiusParameter()->setValue( radius );
	pointMeshOp->strengthParameter()->setValue( strength );
	pointMeshOp->thresholdParameter()->setNumericValue( m_thresholdParameter->getNumericValue() );
	pointMeshOp->resolutionParameter()->setTypedValue( resolution );
	pointMeshOp->boundParameter()->setTypedValue( Box3f( bound.min, bound.max ) );

	return pointMeshOp->operate();
}
