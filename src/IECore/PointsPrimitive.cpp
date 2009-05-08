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

#include "IECore/PointsPrimitive.h"
#include "IECore/MessageHandler.h"
#include "IECore/Renderer.h"

using namespace IECore;

const unsigned int PointsPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( PointsPrimitive );

PointsPrimitive::PointsPrimitive( size_t numPoints )
	:	m_numPoints( numPoints )
{
}

PointsPrimitive::PointsPrimitive( V3fVectorDataPtr points, FloatVectorDataPtr radii )
	:	m_numPoints( points->readable().size() )
{
	variables.insert( PrimitiveVariableMap::value_type( "P", PrimitiveVariable( PrimitiveVariable::Vertex, points ) ) );
	if( radii )
	{
		/// \todo This isn't the name the renderers are looking for - why are we using it?
		variables.insert( PrimitiveVariableMap::value_type( "r", PrimitiveVariable( PrimitiveVariable::Vertex, radii ) ) );
	}
}

PointsPrimitive::~PointsPrimitive()
{
}

void PointsPrimitive::copyFrom( IECore::ConstObjectPtr other, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const PointsPrimitive *tOther = static_cast<const PointsPrimitive *>( other.get() );
	m_numPoints = tOther->getNumPoints();
}

void PointsPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( "numPoints", static_cast<unsigned int>(m_numPoints) );
}

void PointsPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	unsigned int numPoints;
	container->read( "numPoints", numPoints );
	m_numPoints = static_cast<size_t>(numPoints);
}

bool PointsPrimitive::isEqualTo( ConstObjectPtr other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}
	const PointsPrimitive *tOther = static_cast<const PointsPrimitive *>( other.get() );
	if( tOther->getNumPoints()!=getNumPoints() )
	{
		return false;
	}
	return true;
}

void PointsPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( m_numPoints ) );
}

size_t PointsPrimitive::getNumPoints() const
{
	return m_numPoints;
}

void PointsPrimitive::setNumPoints( size_t n )
{
	m_numPoints = n;
}

size_t PointsPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
{
	switch( interpolation )
	{
		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying :
		case PrimitiveVariable::FaceVarying :
			return getNumPoints();
		default :
			return 1;
	}
}

void PointsPrimitive::render( RendererPtr renderer ) const
{
	renderer->points( getNumPoints(), variables );
}
