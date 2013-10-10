//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SpherePrimitive.h"
#include "IECore/Renderer.h"
#include "IECore/Exception.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MurmurHash.h"

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

static IndexedIO::EntryID g_radiusEntry("radius" );
static IndexedIO::EntryID g_zMinEntry("zMin" );
static IndexedIO::EntryID g_zMaxEntry("zMax" );
static IndexedIO::EntryID g_thetaMaxEntry("thetaMax" );

const unsigned int SpherePrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(SpherePrimitive);

SpherePrimitive::SpherePrimitive() : m_radius( 1.0f ), m_zMin( -1.0f ), m_zMax( 1.0f ), m_thetaMax( 360.0f )
{
}

SpherePrimitive::SpherePrimitive( float radius, float zMin, float zMax, float thetaMax ) : m_radius( radius), m_zMin( zMin ), m_zMax( zMax ), m_thetaMax( thetaMax )
{
	if ( radius <= Imath::limits<float>::epsilon() )
	{
		throw InvalidArgumentException( "Invalid radius specified for SpherePrimitive" );
	}

	if ( zMin < -1.0f )
	{
		throw InvalidArgumentException( "Invalid zMin specified for SpherePrimitive" );
	}

	if ( zMax > 1.0f )
	{
		throw InvalidArgumentException( "Invalid zMax specified for SpherePrimitive" );
	}

	if ( zMax <= zMin )
	{
		throw InvalidArgumentException( "Invalid zMin/zMax specified for SpherePrimitive" );
	}

	if ( fabsf(thetaMax) < 1.e-6 )
	{
		throw InvalidArgumentException( "Invalid thetaMax specified for SpherePrimitive" );
	}
}

float SpherePrimitive::radius() const
{
	return m_radius;
}

float SpherePrimitive::zMin() const
{
	return m_zMin;
}

float SpherePrimitive::zMax() const
{
	return m_zMax;
}

float SpherePrimitive::thetaMax() const
{
	return m_thetaMax;
}

void SpherePrimitive::setRadius( float r )
{
	m_radius = r;
}

void SpherePrimitive::setZMin( float zm )
{
	m_zMin = zm;
}

void SpherePrimitive::setZMax( float zm )
{
	m_zMax = zm;
}

void SpherePrimitive::setThetaMax( float tm )
{
	m_thetaMax = tm;
}

size_t SpherePrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
{
	switch(interpolation)
	{
		case PrimitiveVariable::Constant :
		case PrimitiveVariable::Uniform :
			return 1;

		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying:
		case PrimitiveVariable::FaceVarying:
			return 4;

		default :
			assert( false );
			return 0;

	}
}

Imath::Box3f SpherePrimitive::bound() const
{
	/// \todo Take into account thetamax
	return Box3f(
		V3f( -m_radius, -m_radius, m_radius * m_zMin ),
		V3f( m_radius, m_radius, m_radius * m_zMax )
	);
}

void SpherePrimitive::render( Renderer *renderer ) const
{
	assert( renderer );
	renderer->sphere( m_radius, m_zMin, m_zMax, m_thetaMax, variables );
}

void SpherePrimitive::copyFrom( const Object *other, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const SpherePrimitive *tOther = static_cast<const SpherePrimitive *>( other );
	m_radius = tOther->m_radius;
	m_zMin = tOther->m_zMin;
	m_zMax = tOther->m_zMax;
	m_thetaMax = tOther->m_thetaMax;
}

void SpherePrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save(context);
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );

	container->write( g_radiusEntry, m_radius );
	container->write( g_zMinEntry, m_zMin );
	container->write( g_zMaxEntry, m_zMax );
	container->write( g_thetaMaxEntry, m_thetaMax );
}

void SpherePrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load(context);
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	container->read( g_radiusEntry, m_radius );
	container->read( g_zMinEntry, m_zMin );
	container->read( g_zMaxEntry, m_zMax );
	container->read( g_thetaMaxEntry, m_thetaMax );
}

bool SpherePrimitive::isEqualTo( const Object *other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}

	const SpherePrimitive *tOther = static_cast<const SpherePrimitive *>( other );

	if( m_radius!=tOther->m_radius )
	{
		return false;
	}
	if( m_zMin!=tOther->m_zMin )
	{
		return false;
	}
	if( m_zMax!=tOther->m_zMax )
	{
		return false;
	}
	if( m_thetaMax!=tOther->m_thetaMax )
	{
		return false;
	}

	return true;
}

void SpherePrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( m_radius ) );
	a.accumulate( sizeof( m_zMin ) );
	a.accumulate( sizeof( m_zMax ) );
	a.accumulate( sizeof( m_thetaMax ) );
}

void SpherePrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
}

void SpherePrimitive::topologyHash( MurmurHash &h ) const
{
	h.append( m_radius );
	h.append( m_zMin );
	h.append( m_zMax );
	h.append( m_thetaMax );
}
