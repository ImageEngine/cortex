//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/VectorOps.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/BoundedKDTree.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/MeshPrimitiveBuilder.h"
#include "IECore/CachedImplicitSurfaceFunction.h"
#include "IECore/VectorTraits.h"
#include "IECore/MarchingCubes.h"
#include "IECore/BlobbyImplicitSurfaceFunction.h"
#include "IECore/PointMeshOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PointMeshOp );

static TypeId pointTypes[] = { V3fVectorDataTypeId, V3dVectorDataTypeId, InvalidTypeId };
static TypeId resultTypes[] = { MeshPrimitiveTypeId, InvalidTypeId };

PointMeshOp::PointMeshOp()
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
	m_pointParameter = new ObjectParameter(
		"points",
		"The points to calculate the mesh from.",
		new V3fVectorData(),
		pointTypes
	);

	m_radiusParameter = new DoubleVectorParameter(
		"radius",
		"The radius of each point",
		new DoubleVectorData()
	);

	m_strengthParameter = new DoubleVectorParameter(
		"strength",
		"The strength of each point",
		new DoubleVectorData()
	);

	m_thresholdParameter = new FloatParameter(
		"threshold",
		"The threshold at which to generate the surface.",
		0.0
	);

	m_resolutionParameter = new V3iParameter(
		"resolution",
		"The resolution",
		V3i( 1, 1, 1 )
	);

	m_boundParameter = new Box3fParameter(
		"bound",
		"The bound",
		Box3f( V3d( -1, -1, -1 ), V3d( 1, 1, 1 ) )
	);


	parameters()->addParameter( m_pointParameter );
	parameters()->addParameter( m_radiusParameter );
	parameters()->addParameter( m_strengthParameter );
	parameters()->addParameter( m_thresholdParameter );
	parameters()->addParameter( m_resolutionParameter );
	parameters()->addParameter( m_boundParameter );
}

PointMeshOp::~PointMeshOp()
{
}

ObjectParameterPtr PointMeshOp::pointParameter()
{
	return m_pointParameter;
}

ConstObjectParameterPtr PointMeshOp::pointParameter() const
{
	return m_pointParameter;
}


DoubleVectorParameterPtr PointMeshOp::radiusParameter()
{
	return m_radiusParameter;
}

ConstDoubleVectorParameterPtr PointMeshOp::radiusParameter() const
{
	return m_radiusParameter;
}


DoubleVectorParameterPtr PointMeshOp::strengthParameter()
{
	return m_strengthParameter;
}

ConstDoubleVectorParameterPtr PointMeshOp::strengthParameter() const
{
	return m_strengthParameter;
}


FloatParameterPtr PointMeshOp::thresholdParameter()
{
	return m_thresholdParameter;
}

ConstFloatParameterPtr PointMeshOp::thresholdParameter() const
{
	return m_thresholdParameter;
}


V3iParameterPtr PointMeshOp::resolutionParameter()
{
	return m_resolutionParameter;
}

ConstV3iParameterPtr PointMeshOp::resolutionParameter() const
{
	return m_resolutionParameter;
}


Box3fParameterPtr PointMeshOp::boundParameter()
{
	return m_boundParameter;
}

ConstBox3fParameterPtr PointMeshOp::boundParameter() const
{
	return m_boundParameter;
}

ObjectPtr PointMeshOp::doOperation( ConstCompoundObjectPtr operands )
{
	const float threshold = m_thresholdParameter->getNumericValue();

	ConstObjectPtr points = pointParameter()->getValue();
	ConstObjectPtr radius = radiusParameter()->getValue();
	ConstObjectPtr strength = strengthParameter()->getValue();
	ConstObjectPtr resolutionData = resolutionParameter()->getValue();
	ConstObjectPtr boundData = boundParameter()->getValue();

	V3i resolution = boost::static_pointer_cast<const V3iData>( resolutionData )->readable();
	Box3f bound = boost::static_pointer_cast<const Box3fData>( boundData )->readable();

	/// Calculate a tolerance which is half the size of the smallest grid division
	double cacheTolerance = ((bound.max.x - bound.min.x) / (double)resolution.x) / 2.0;
	cacheTolerance = std::min(cacheTolerance, ((bound.max.y - bound.min.y) / (double)resolution.y) / 2.0 );
	cacheTolerance = std::min(cacheTolerance, ((bound.max.z - bound.min.z) / (double)resolution.z) / 2.0 );

	MeshPrimitiveBuilderPtr builder = new MeshPrimitiveBuilder();

	switch( points->typeId() )
	{
		case V3fVectorDataTypeId :
			{
				typedef MarchingCubes< CachedImplicitSurfaceFunction< V3f, float > > Marcher ;

				BlobbyImplicitSurfaceFunction< V3f, float >::Ptr fn = new BlobbyImplicitSurfaceFunction< V3f, float >
				(
					boost::static_pointer_cast<const V3fVectorData>( points ),
					boost::static_pointer_cast<const DoubleVectorData>( radius ),
					boost::static_pointer_cast<const DoubleVectorData>( strength )
				);

				Marcher::Ptr m = new Marcher
				(
					new CachedImplicitSurfaceFunction< V3f, float >(
						fn,
						cacheTolerance
					),

					builder
				);

				m->march( bound, resolution, threshold );
			}
			break;
		case V3dVectorDataTypeId :
			{
				typedef MarchingCubes< CachedImplicitSurfaceFunction< V3d, double > > Marcher ;

				BlobbyImplicitSurfaceFunction< V3d, double >::Ptr fn = new BlobbyImplicitSurfaceFunction< V3d, double >
				(
					boost::static_pointer_cast<const V3dVectorData>( points ),
					boost::static_pointer_cast<const DoubleVectorData>( radius ),
					boost::static_pointer_cast<const DoubleVectorData>( strength )
				);

				Marcher::Ptr m = new Marcher
				(
					new CachedImplicitSurfaceFunction< V3d, double >(
						fn,
						cacheTolerance
					),

					builder
				);

				m->march( Box3d( bound.min, bound.max ), resolution, threshold );
			}
			break;
		default :
			// should never get here
			assert( 0 );
	}

	return builder->mesh();
}
