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

#include "IECore/PointMeshOp.h"

using namespace IECore;
using namespace Imath;
using namespace std;

static TypeId pointTypes[] = { V3fVectorDataTypeId, V3dVectorDataTypeId, InvalidTypeId };
static TypeId resultTypes[] = { MeshPrimitiveTypeId, DoubleVectorDataTypeId, InvalidTypeId };

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


DoubleParameterPtr PointMeshOp::thresholdParameter()
{
	return m_thresholdParameter;
}

ConstDoubleParameterPtr PointMeshOp::thresholdParameter() const
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


Box3dParameterPtr PointMeshOp::boundParameter()
{
	return m_boundParameter;
}

ConstBox3dParameterPtr PointMeshOp::boundParameter() const
{
	return m_boundParameter;
}


template<typename P, typename V>
class BlobbyParticleFunction : public ImplicitSurfaceFunction<P, V>
{
	public:
		typedef P Point;
		typedef VectorTraits<P> PointTraits;
		typedef typename VectorTraits<P>::BaseType PointBaseType;
		typedef V Value;
		typedef VectorTraits<V> ValueTraits;
		typedef typename VectorTraits<V>::BaseType ValueBaseType;
		
		typedef std::vector<P> PointVector;
		typedef TypedData<PointVector> PointVectorData;
		
		typedef Imath::Box<P> Bound;
		
		typedef boost::intrusive_ptr< const BlobbyParticleFunction<P, V> > ConstPtr;
		typedef boost::intrusive_ptr< BlobbyParticleFunction<P, V> > Ptr;
		
		BlobbyParticleFunction( typename PointVectorData::ConstPtr p, ConstDoubleVectorDataPtr r, ConstDoubleVectorDataPtr s ) : m_p( p ), m_radius( r ), m_strength( s )
		{		
			assert( m_p );
			assert( m_radius );
			assert( m_strength );
			
			if (m_p->readable().size() != m_radius->readable().size())
			{
				throw InvalidArgumentException("Incompatible point/radius data given to PointMeshOp");
			}
			if (m_p->readable().size() != m_strength->readable().size())
			{
				throw InvalidArgumentException("Incompatible point/strength data given to PointMeshOp");
			}
			
			typename PointVector::const_iterator pit = m_p->readable().begin();
						
			std::vector<double>::const_iterator rit = m_radius->readable().begin();			
			
			for (; pit != m_p->readable().end(); ++pit, ++rit)
			{
				m_bounds.push_back(
					Bound(
						*pit - Point( *rit ),
						*pit + Point( *rit )
					)
				);
			}
			
			m_tree = new Tree( m_bounds.begin(), m_bounds.end() );
			
			assert( m_tree );			
		}
		
		virtual ~BlobbyParticleFunction()
		{
			assert( m_tree );
			delete m_tree;
		}

		inline Value operator()( const Point &p )
		{
			assert( m_tree );
			assert( m_p );
			assert( m_radius );
			assert( m_strength );
			
			std::vector<BoundVectorConstIterator> intersecting;
			
			unsigned int numInteresecting = m_tree->intersectingBounds( p, intersecting );
			(void)numInteresecting;
			
			Value totalInfluence = 0.0;
			
			for (typename std::vector<BoundVectorConstIterator>::const_iterator it = intersecting.begin(); it != intersecting.end(); ++it)
			{
				const int j = (*it - m_bounds.begin() );
				Point sep = m_p->readable()[ j ] - p;
				PointBaseType distSqrd = vecDot( sep, sep );
				PointBaseType dist = sqrt( distSqrd );
				
				/// \todo Allow falloff types other than linear
				PointBaseType invDistNorm = 1.0 - dist / m_radius->readable()[ j ];
				
				/// \todo Allow other compositing types
				totalInfluence = std::max( totalInfluence, Value( invDistNorm * m_strength->readable()[ j ] ) );
			}
			
			return totalInfluence;
		}
		
		virtual Value getValue( const Point &p )
		{
			return this->operator()(p);
		}
		
	protected:

		typedef std::vector< Bound > BoundVector;
		typedef typename BoundVector::const_iterator BoundVectorConstIterator;
		typedef BoundedKDTree< BoundVectorConstIterator > Tree;
		
		typename PointVectorData::ConstPtr m_p;		
		ConstDoubleVectorDataPtr m_radius;		
		ConstDoubleVectorDataPtr m_strength;

		BoundVector m_bounds;
		Tree *m_tree;
};

ObjectPtr PointMeshOp::doOperation( ConstCompoundObjectPtr operands )
{	
	const double threshold = m_thresholdParameter->getNumericValue();

	ConstObjectPtr points = pointParameter()->getValue();
	ConstObjectPtr radius = radiusParameter()->getValue();
	ConstObjectPtr strength = strengthParameter()->getValue();
	ConstObjectPtr resolutionData = resolutionParameter()->getValue();
	ConstObjectPtr boundData = boundParameter()->getValue();
	
	V3i resolution = boost::static_pointer_cast<const V3iData>( resolutionData )->readable();
	Box< V3d > bound = boost::static_pointer_cast<const Box3dData>( boundData )->readable();
	
	/// Calculate a tolerance which is half the size of the smallest grid division
	double cacheTolerance = ((bound.max.x - bound.min.x) / (double)resolution.x) / 2.0;
	cacheTolerance = std::min(cacheTolerance, ((bound.max.y - bound.min.y) / (double)resolution.y) / 2.0 );
	cacheTolerance = std::min(cacheTolerance, ((bound.max.z - bound.min.z) / (double)resolution.z) / 2.0 );	

	MeshPrimitiveBuilder<float>::Ptr builder = new MeshPrimitiveBuilder<float>();

	switch( points->typeId() )
	{
		case V3fVectorDataTypeId :
			{				
				typedef MarchingCubes< CachedImplicitSurfaceFunction< V3f, float > > Marcher ;								
				
				BlobbyParticleFunction< V3f, float >::Ptr fn = new BlobbyParticleFunction< V3f, float > 
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
				
				m->march( Box3f( bound.min, bound.max ), resolution, threshold );
			}
			break;
		case V3dVectorDataTypeId :
			{
				typedef MarchingCubes< CachedImplicitSurfaceFunction< V3d, double > > Marcher ;
				
				BlobbyParticleFunction< V3d, double >::Ptr fn = new BlobbyParticleFunction< V3d, double > 
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
				
				m->march( bound, resolution, threshold );
			}
			break;	
		default :
			// should never get here
			assert( 0 );
	}

	return builder->mesh();
}
