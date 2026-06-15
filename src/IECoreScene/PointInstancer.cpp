//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2026, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreScene/PointInstancer.h"

#include <numeric>
#include <unordered_set>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( PointInstancer );

PointInstancer::PointInstancer( size_t numPoints )
	:	PointsPrimitive( numPoints )
{
}

PointInstancer::~PointInstancer()
{
}

void PointInstancer::copyFrom( const Object *other, IECore::Object::CopyContext *context )
{
	PointsPrimitive::copyFrom( other, context );
}

void PointInstancer::save( IECore::Object::SaveContext *context ) const
{
	PointsPrimitive::save( context );
}

void PointInstancer::load( IECore::Object::LoadContextPtr context )
{
	PointsPrimitive::load( context );
}

bool PointInstancer::isEqualTo( const Object *other ) const
{
	return PointsPrimitive::isEqualTo( other );
}

void PointInstancer::memoryUsage( Object::MemoryAccumulator &a ) const
{
	PointsPrimitive::memoryUsage( a );
}

void PointInstancer::hash( MurmurHash &h ) const
{
	PointsPrimitive::hash( h );
}

void PointInstancer::setPrototypes( IECore::StringVectorDataPtr &prototypes )
{
	if( prototypes )
	{
		variables["prototypeRoots"] = PrimitiveVariable( PrimitiveVariable::Constant, prototypes );
	}
	else
	{
		variables.erase( "prototypeRoots" );
	}
}

PrimitiveVariable::IndexedView<std::string> PointInstancer::getPrototypes() const
{
	auto optionalView = variableIndexedView<StringVectorData>( "prototypeRoots", PrimitiveVariable::Constant );
	return optionalView ? *optionalView : PrimitiveVariable::IndexedView<std::string>();
}

void PointInstancer::setPrototypeIndex( const IECore::IntVectorDataPtr &prototypeIndex )
{
	if( prototypeIndex )
	{
		variables["prototypeIndex"] = PrimitiveVariable( PrimitiveVariable::Vertex, prototypeIndex );
	}
	else
	{
		variables.erase( "prototypeIndex" );
	}
}

PrimitiveVariable::IndexedView<int> PointInstancer::getPrototypeIndex() const
{
	auto optionalView = variableIndexedView<IntVectorData>( "prototypeIndex", PrimitiveVariable::Vertex );
	return optionalView ? *optionalView : PrimitiveVariable::IndexedView<int>();
}

void PointInstancer::setPosition( const IECore::V3fVectorDataPtr &position )
{
	if( position )
	{
		position->setInterpretation( GeometricData::Interpretation::Point );
		variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, position );
	}
	else
	{
		variables.erase( "P" );
	}
}

PrimitiveVariable::IndexedView<Imath::V3f> PointInstancer::getPosition() const
{
	auto optionalView = variableIndexedView<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	return optionalView ? *optionalView : PrimitiveVariable::IndexedView<Imath::V3f>();
}

void PointInstancer::setScale( const IECore::V3fVectorDataPtr &scale )
{
	if( scale )
	{
		variables["scale"] = PrimitiveVariable( PrimitiveVariable::Vertex, scale );
	}
	else
	{
		variables.erase( "scale" );
	}
}

PrimitiveVariable::IndexedView<Imath::V3f> PointInstancer::getScale() const
{
	auto optionalView = variableIndexedView<V3fVectorData>( "scale", PrimitiveVariable::Vertex );
	return optionalView ? *optionalView : PrimitiveVariable::IndexedView<Imath::V3f>();
}

void PointInstancer::setOrientation( const IECore::QuatfVectorDataPtr &orientation )
{
	if( orientation )
	{
		variables["orientation"] = PrimitiveVariable( PrimitiveVariable::Vertex, orientation );
	}
	else
	{
		variables.erase( "orientation" );
	}
}

PrimitiveVariable::IndexedView<Imath::Quatf> PointInstancer::getOrientation() const
{
	auto optionalView = variableIndexedView<QuatfVectorData>( "orientation", PrimitiveVariable::Vertex );
	return optionalView ? *optionalView : PrimitiveVariable::IndexedView<Imath::Quatf>();
}

void PointInstancer::setID( const IECore::Int64VectorDataPtr &ids )
{
	if( ids )
	{
		variables["instanceId"] = PrimitiveVariable( PrimitiveVariable::Vertex, ids );
	}
	else
	{
		variables.erase( "instanceId" );
	}
}

PrimitiveVariable::IndexedView<int64_t> PointInstancer::getID() const
{
	auto optionalView = variableIndexedView<Int64VectorData>( "instanceId", PrimitiveVariable::Vertex );
	return optionalView ? *optionalView : PrimitiveVariable::IndexedView<int64_t>();
}

void PointInstancer::setInvisibleIDs( const IECore::Int64VectorDataPtr &invisibleIds )
{
	if( invisibleIds )
	{
		variables["invisibleIds"] = PrimitiveVariable( PrimitiveVariable::Constant, invisibleIds );
	}
	else
	{
		variables.erase( "invisibleIds" );
	}
}

PrimitiveVariable::IndexedView<int64_t> PointInstancer::getInvisibleIDs() const
{
	auto optionalView = variableIndexedView<Int64VectorData>( "invisibleIds", PrimitiveVariable::Constant );
	return optionalView ? *optionalView : PrimitiveVariable::IndexedView<int64_t>();
}

// Query
// =====

PointInstancer::TransformQuery::TransformQuery( const PointInstancer &pointInstancer )
	:	m_position( pointInstancer.getPosition() ),
		m_orientation( pointInstancer.getOrientation() ),
		m_scale( pointInstancer.getScale() )
{
}

Imath::M44f PointInstancer::TransformQuery::transform( size_t pointIndex ) const
{
	M44f result;
	if( m_position )
	{
		result.translate( m_position[pointIndex] );
	}

	if( m_orientation )
	{
		/// \todo Gaffer's Instancer class uses a `normalizedIfNeeded()` function
		/// that avoids normalizing quaternions that can't get any closer to normalized.
		/// Is there any value in doing that here?
		result = m_orientation[pointIndex].normalized().toMatrix44() * result;
	}

	if( m_scale )
	{
		result.scale( m_scale[pointIndex] );
	}

	return result;
}
