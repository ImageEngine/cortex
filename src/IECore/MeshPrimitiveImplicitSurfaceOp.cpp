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
#include "IECore/MeshPrimitiveImplicitSurfaceFunction.h"
#include "IECore/CachedImplicitSurfaceFunction.h"
#include "IECore/MarchingCubes.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/VectorTypedData.h"
#include "IECore/RunTimeTyped.h"

using namespace IECore;
using namespace Imath;

MeshPrimitiveImplicitSurfaceOp::MeshPrimitiveImplicitSurfaceOp() : MeshPrimitiveOp( staticTypeName(), "A MeshPrimitiveOp to make an offset mesh using an implicit surface" )
{
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
        
	m_boundExtendParameter = new FloatParameter(
		"boundExtend",
		"The bound's radius, even if calculated by automatic bounding, is increased by this amount.",
		0.05,
		0.0
	);
	
	m_automaticBoundParameter = new BoolParameter(
		"automaticBound",
		"Enable to calculate the bound automatically. Disable to specify an explicit bound.",
		true
	);		
	
	m_boundParameter = new Box3fParameter(
		"bound",
		"The bound",
		Box3f( V3d( -1, -1, -1 ), V3d( 1, 1, 1 ) )
	);
	
	IntParameter::PresetsMap gridMethodPresets;
	gridMethodPresets["Resolution"] = Resolution;
	gridMethodPresets["Division Size"] = DivisionSize;
	
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
		V3f( 10, 10, 10 )
	);	
		
	parameters()->addParameter( m_thresholdParameter );
	parameters()->addParameter( m_gridMethodParameter );	
	parameters()->addParameter( m_resolutionParameter );
	parameters()->addParameter( m_divisionSizeParameter );	
	parameters()->addParameter( m_automaticBoundParameter );
	parameters()->addParameter( m_boundExtendParameter );					
	parameters()->addParameter( m_boundParameter );	
}

MeshPrimitiveImplicitSurfaceOp::~MeshPrimitiveImplicitSurfaceOp()
{
}

FloatParameterPtr MeshPrimitiveImplicitSurfaceOp::thresholdParameter()
{
	return m_thresholdParameter;
}

ConstFloatParameterPtr MeshPrimitiveImplicitSurfaceOp::thresholdParameter() const
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

Box3fParameterPtr MeshPrimitiveImplicitSurfaceOp::boundParameter()
{
	return m_boundParameter;
}

ConstBox3fParameterPtr MeshPrimitiveImplicitSurfaceOp::boundParameter() const
{
	return m_boundParameter;
}

BoolParameterPtr MeshPrimitiveImplicitSurfaceOp::automaticBoundParameter()
{
	return m_automaticBoundParameter;
}

BoolParameterPtr MeshPrimitiveImplicitSurfaceOp::automaticBoundParameter() const
{
	return m_automaticBoundParameter;
}

IntParameterPtr MeshPrimitiveImplicitSurfaceOp::gridMethodParameter()
{
	return m_gridMethodParameter;
}

IntParameterPtr MeshPrimitiveImplicitSurfaceOp::gridMethodParameter() const
{
	return m_gridMethodParameter;
}

V3fParameterPtr MeshPrimitiveImplicitSurfaceOp::divisionSizeParameter()
{
	return m_divisionSizeParameter;
}

ConstV3fParameterPtr MeshPrimitiveImplicitSurfaceOp::divisionSizeParameter() const
{
	return m_divisionSizeParameter;
}

FloatParameterPtr MeshPrimitiveImplicitSurfaceOp::boundExtendParameter()
{
	return m_boundExtendParameter;
}

FloatParameterPtr MeshPrimitiveImplicitSurfaceOp::boundExtendParameter() const
{
	return m_boundExtendParameter;
}

void MeshPrimitiveImplicitSurfaceOp::modifyTypedPrimitive( MeshPrimitivePtr typedPrimitive, ConstCompoundObjectPtr operands )
{
	const float threshold = m_thresholdParameter->getNumericValue();

	bool automaticBound = boost::static_pointer_cast<const BoolData>(m_automaticBoundParameter->getValue())->readable();
	Box3f bound;
	
	if (automaticBound)
	{	
		bound.makeEmpty();
		
		PrimitiveVariableMap::const_iterator it = typedPrimitive->variables.find("P");
		
		if (it != typedPrimitive->variables.end())
		{
			const DataPtr &verticesData = it->second.data;
						
			/// \todo Use depatchTypedData
			if (runTimeCast<V3fVectorData>(verticesData))
			{
				ConstV3fVectorDataPtr p = runTimeCast<V3fVectorData>(verticesData);
				
				for ( V3fVectorData::ValueType::const_iterator it = p->readable().begin(); 
					it != p->readable().end(); ++it)
				{
					bound.extendBy( *it );
				}
			}
			else if (runTimeCast<V3dVectorData>(verticesData))
			{		
				ConstV3dVectorDataPtr p = runTimeCast<V3dVectorData>(verticesData);
				
				for ( V3dVectorData::ValueType::const_iterator it = p->readable().begin(); 
					it != p->readable().end(); ++it)
				{
					bound.extendBy( *it );
				}
			}
			else
			{
				throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" of type V3fVectorData/V3dVectorData in MeshPrimitiveImplicitSurfaceOp");
			}
		}
		else
		{
			throw InvalidArgumentException("MeshPrimitive has no primitive variable \"P\" in MeshPrimitiveImplicitSurfaceOp");
		}
	}
	else
	{
		bound = boost::static_pointer_cast<const Box3fData>(m_boundParameter->getValue())->readable();
	}
	
	float boundExtend = m_boundExtendParameter->getNumericValue();
	bound.min -= V3f( boundExtend, boundExtend, boundExtend );
	bound.max += V3f( boundExtend, boundExtend, boundExtend );	
		
	
	V3i resolution;
	int gridMethod = m_gridMethodParameter->getNumericValue();
	if ( gridMethod == Resolution )
	{	
		resolution = boost::static_pointer_cast<const V3iData>(m_resolutionParameter->getValue())->readable();	
	}
	else if ( gridMethod == DivisionSize )
	{
		V3f divisionSize = boost::static_pointer_cast<const V3fData>(m_divisionSizeParameter->getValue())->readable();
		
		resolution.x = (int)((bound.max.x - bound.min.x) / divisionSize.x);
		resolution.y = (int)((bound.max.y - bound.min.y) / divisionSize.y);
		resolution.z = (int)((bound.max.z - bound.min.z) / divisionSize.z);				
		
	}
	else
	{
		assert( false );
	}	
	
	
	resolution.x = std::max( 1, resolution.x );
	resolution.y = std::max( 1, resolution.y );
	resolution.z = std::max( 1, resolution.z );		
	
	/// Calculate a tolerance which is half the size of the smallest grid division
	double cacheTolerance = ((bound.max.x - bound.min.x) / (double)resolution.x) / 2.0;
	cacheTolerance = std::min(cacheTolerance, ((bound.max.y - bound.min.y) / (double)resolution.y) / 2.0 );
	cacheTolerance = std::min(cacheTolerance, ((bound.max.z - bound.min.z) / (double)resolution.z) / 2.0 );	

	MeshPrimitiveBuilderPtr builder = new MeshPrimitiveBuilder();

	typedef MarchingCubes< CachedImplicitSurfaceFunction< V3f, float > > Marcher ;								
				
	MeshPrimitiveImplicitSurfaceFunctionPtr fn = new MeshPrimitiveImplicitSurfaceFunction( typedPrimitive );

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
