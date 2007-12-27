//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

using namespace IECore;
using namespace Imath;
using namespace std;

static TypeId resultTypes[] = { MeshPrimitiveTypeId, DoubleVectorDataTypeId, InvalidTypeId };

ParticleMeshOp::ParticleMeshOp()
	:	Op(
		staticTypeName(),
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
	
	m_radiusParameter = new DoubleParameter(
		"radius",
		"Radius to use when not reading an attribute",
		1.0
	);
	
	m_radiusScaleParameter = new DoubleParameter(
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
	
	m_strengthParameter = new DoubleParameter(
		"strength",
		"Strength to use when not reading an attribute",
		1.0
	);
	
	m_strengthScaleParameter = new DoubleParameter(
		"strengthScale",
		"Factor to multiply all strength by",
		1.0
	);	
	
	m_thresholdParameter = new DoubleParameter(
		"threshold",
		"The threshold at which to generate the surface.",
		0.0
	);
	
	m_resolutionParameter = new V3iParameter(
		"resolution",
		"The resolution",
		V3i( 1, 1, 1 )
	);
	
	m_boundParameter = new Box3dParameter(
		"bound",
		"The bound",
		Box3d( V3d( -1, -1, -1 ), V3d( 1, 1, 1 ) )
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
	parameters()->addParameter( m_resolutionParameter );
	parameters()->addParameter( m_boundParameter );				
}

ParticleMeshOp::~ParticleMeshOp()
{
}

FileNameParameterPtr ParticleMeshOp::fileNameParameter()
{
	return m_fileNameParameter;
}

ConstFileNameParameterPtr ParticleMeshOp::fileNameParameter() const
{
	return m_fileNameParameter;
}

StringParameterPtr ParticleMeshOp::positionAttributeParameter()
{
	return m_positionAttributeParameter;
}

ConstStringParameterPtr ParticleMeshOp::positionAttributeParameter() const
{
	return m_positionAttributeParameter;
}

BoolParameterPtr ParticleMeshOp::useRadiusAttributeParameter()
{
	return m_useRadiusAttributeParameter;
}

ConstBoolParameterPtr ParticleMeshOp::useRadiusAttributeParameter() const
{
	return m_useRadiusAttributeParameter;
}

StringParameterPtr ParticleMeshOp::radiusAttributeParameter()
{
	return m_radiusAttributeParameter;
}

ConstStringParameterPtr ParticleMeshOp::radiusAttributeParameter() const
{
	return m_radiusAttributeParameter;
}

DoubleParameterPtr ParticleMeshOp::radiusParameter()
{
	return m_radiusParameter;
}

ConstDoubleParameterPtr ParticleMeshOp::radiusParameter() const
{
	return m_radiusParameter;
}

DoubleParameterPtr ParticleMeshOp::radiusScaleParameter()
{
	return m_radiusScaleParameter;
}

ConstDoubleParameterPtr ParticleMeshOp::radiusScaleParameter() const
{
	return m_radiusScaleParameter;
}

BoolParameterPtr ParticleMeshOp::useStrengthAttributeParameter()
{
	return m_useStrengthAttributeParameter;
}

ConstBoolParameterPtr ParticleMeshOp::useStrengthAttributeParameter() const
{
	return m_useStrengthAttributeParameter;
}

StringParameterPtr ParticleMeshOp::strengthAttributeParameter()
{
	return m_strengthAttributeParameter;
}

ConstStringParameterPtr ParticleMeshOp::strengthAttributeParameter() const
{
	return m_strengthAttributeParameter;
}

DoubleParameterPtr ParticleMeshOp::strengthParameter()
{
	return m_strengthParameter;
}

ConstDoubleParameterPtr ParticleMeshOp::strengthParameter() const
{
	return m_strengthParameter;
}

DoubleParameterPtr ParticleMeshOp::strengthScaleParameter()
{
	return m_strengthScaleParameter;
}

ConstDoubleParameterPtr ParticleMeshOp::strengthScaleParameter() const
{
	return m_strengthScaleParameter;
}

DoubleParameterPtr ParticleMeshOp::thresholdParameter()
{
	return m_thresholdParameter;
}

ConstDoubleParameterPtr ParticleMeshOp::thresholdParameter() const
{
	return m_thresholdParameter;
}

V3iParameterPtr ParticleMeshOp::resolutionParameter()
{
	return m_resolutionParameter;
}

ConstV3iParameterPtr ParticleMeshOp::resolutionParameter() const
{
	return m_resolutionParameter;
}

Box3dParameterPtr ParticleMeshOp::boundParameter()
{
	return m_boundParameter;
}

ConstBox3dParameterPtr ParticleMeshOp::boundParameter() const
{
	return m_boundParameter;
}

ObjectPtr ParticleMeshOp::doOperation( ConstCompoundObjectPtr operands )
{	
	MeshPrimitiveBuilder<float>::Ptr builder = new MeshPrimitiveBuilder<float>();
	
	ConstObjectPtr fileNameData = fileNameParameter()->getValue();
	const std::string &fileName = boost::static_pointer_cast<const StringData>(fileNameData)->readable();
	
	ParticleReaderPtr reader = boost::static_pointer_cast< ParticleReader > ( Reader::create( fileName ) );
	if (!reader)
	{
		throw IOException("");
	}	
	
	ConstObjectPtr positionAttributeData = positionAttributeParameter()->getValue();
	const std::string &positionAttribute = boost::static_pointer_cast<const StringData>(positionAttributeData)->readable();	
	DataPtr positionData = reader->readAttribute( positionAttribute );
	
	/// \todo Detect V3fVectorData for positional information then automatically allow FloatVectorData for radius/strength
	V3dVectorDataPtr position = boost::static_pointer_cast< V3dVectorData >( positionData );
	if (!position)
	{
		throw InvalidArgumentException("");
	}
		
	DoubleVectorDataPtr radius;
	bool useRadiusAttribute = boost::static_pointer_cast<const BoolData>(m_useRadiusAttributeParameter->getValue())->readable();
	if ( useRadiusAttribute )
	{
		ConstObjectPtr radiusAttributeData = radiusAttributeParameter()->getValue();
		const std::string &radiusAttribute = boost::static_pointer_cast<const StringData>(radiusAttributeData)->readable();	
		DataPtr radiusData = reader->readAttribute( radiusAttribute );
		radius = boost::static_pointer_cast< DoubleVectorData >( radiusData );
		if (!radius)
		{
			throw InvalidArgumentException("");
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
	bool useStrengthAttribute = boost::static_pointer_cast<const BoolData>(m_useStrengthAttributeParameter->getValue())->readable();
	if ( useStrengthAttribute )
	{
		ConstObjectPtr strengthAttributeData = strengthAttributeParameter()->getValue();
		const std::string &strengthAttribute = boost::static_pointer_cast<const StringData>(strengthAttributeData)->readable();	
		DataPtr strengthData = reader->readAttribute( strengthAttribute );
		strength = boost::static_pointer_cast< DoubleVectorData >( strengthData );
		if (!strength)
		{
			throw InvalidArgumentException("");
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
		throw InvalidArgumentException("");
	}
	
	PointMeshOpPtr pointMeshOp = new PointMeshOp();	
		
	pointMeshOp->pointParameter()->setValue( position->copy() );
	pointMeshOp->radiusParameter()->setValue( radius );
	pointMeshOp->strengthParameter()->setValue( strength );		
	pointMeshOp->thresholdParameter()->setNumericValue( m_thresholdParameter->getNumericValue() );
	pointMeshOp->resolutionParameter()->setValue( resolutionParameter()->getValue()->copy() );
	pointMeshOp->boundParameter()->setValue( boundParameter()->getValue()->copy() );	
	
	return pointMeshOp->operate();
}
