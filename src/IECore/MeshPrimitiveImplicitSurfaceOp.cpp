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

#include "IECore/MeshPrimitiveBuilder.h"
#include "IECore/MeshPrimitiveImplicitSurfaceOp.h"
#include "IECore/PrimitiveImplicitSurfaceFunction.h"
#include "IECore/CachedImplicitSurfaceFunction.h"
#include "IECore/MarchingCubes.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"

using namespace IECore;
using namespace Imath;

MeshPrimitiveImplicitSurfaceOp::MeshPrimitiveImplicitSurfaceOp() : MeshPrimitiveOp( staticTypeName(), "A MeshPrimitiveOp to make an offset mesh using an implicit surface" )
{
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
	
	/// \todo Parameters for auto-bounding, and setting resolution based on units-per-grid-division
	
	parameters()->addParameter( m_thresholdParameter );
	parameters()->addParameter( m_resolutionParameter );
	parameters()->addParameter( m_boundParameter );
}

MeshPrimitiveImplicitSurfaceOp::~MeshPrimitiveImplicitSurfaceOp()
{
}

DoubleParameterPtr MeshPrimitiveImplicitSurfaceOp::thresholdParameter()
{
	return m_thresholdParameter;
}

ConstDoubleParameterPtr MeshPrimitiveImplicitSurfaceOp::thresholdParameter() const
{
	return m_thresholdParameter;
}

V3iParameterPtr MeshPrimitiveImplicitSurfaceOp::resolutionParameter()
{
	return m_resolutionParameter;
}

ConstV3iParameterPtr MeshPrimitiveImplicitSurfaceOp::resolutionParameter() const
{
	return m_resolutionParameter;
}

Box3dParameterPtr MeshPrimitiveImplicitSurfaceOp::boundParameter()
{
	return m_boundParameter;
}

ConstBox3dParameterPtr MeshPrimitiveImplicitSurfaceOp::boundParameter() const
{
	return m_boundParameter;
}

void MeshPrimitiveImplicitSurfaceOp::modifyTypedPrimitive( MeshPrimitivePtr typedPrimitive, ConstCompoundObjectPtr operands )
{
	const double threshold = m_thresholdParameter->getNumericValue();
	ConstObjectPtr resolutionData = resolutionParameter()->getValue();
	ConstObjectPtr boundData = boundParameter()->getValue();
	V3i resolution = boost::static_pointer_cast<const V3iData>( resolutionData )->readable();
	Box< V3d > bound = boost::static_pointer_cast<const Box3dData>( boundData )->readable();
	
	resolution.x = std::max( 1, resolution.x );
	resolution.y = std::max( 1, resolution.y );
	resolution.z = std::max( 1, resolution.z );		
	
	/// Calculate a tolerance which is half the size of the smallest grid division
	double cacheTolerance = ((bound.max.x - bound.min.x) / (double)resolution.x) / 2.0;
	cacheTolerance = std::min(cacheTolerance, ((bound.max.y - bound.min.y) / (double)resolution.y) / 2.0 );
	cacheTolerance = std::min(cacheTolerance, ((bound.max.z - bound.min.z) / (double)resolution.z) / 2.0 );	

	MeshPrimitiveBuilder<float>::Ptr builder = new MeshPrimitiveBuilder<float>();

	typedef MarchingCubes< CachedImplicitSurfaceFunction< V3f, float > > Marcher ;								
				
	PrimitiveImplicitSurfaceFunctionPtr fn = new PrimitiveImplicitSurfaceFunction( typedPrimitive );

	Marcher::Ptr m = new Marcher
	( 
		new CachedImplicitSurfaceFunction< V3f, float >(
			fn,						
			cacheTolerance
		),

		builder
	);

	m->march( Box3f( bound.min, bound.max ), resolution, threshold );	
	MeshPrimitivePtr resultMesh = builder->mesh();
	typedPrimitive->variables.clear();

	typedPrimitive->setTopology( 
		resultMesh->verticesPerFace(),
		resultMesh->vertexIds()
	);

	typedPrimitive->variables["P"] = PrimitiveVariable( resultMesh->variables["P"].interpolation, resultMesh->variables["P"].data->copy() );
	typedPrimitive->variables["N"] = PrimitiveVariable( resultMesh->variables["N"].interpolation, resultMesh->variables["N"].data->copy() );	
			
}
