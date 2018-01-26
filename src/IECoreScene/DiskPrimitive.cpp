//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/DiskPrimitive.h"

#include "IECoreScene/Renderer.h"

#include "IECore/Exception.h"
#include "IECore/MurmurHash.h"

using namespace std;
using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace boost;

static IndexedIO::EntryID g_radiusEntry("radius");
static IndexedIO::EntryID g_zEntry("z");
static IndexedIO::EntryID g_thetaMaxEntry("thetaMax");

const unsigned int DiskPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(DiskPrimitive);

DiskPrimitive::DiskPrimitive( float radius, float z, float thetaMax )
{
	setRadius( radius );
	setZ( z );
	setThetaMax( thetaMax );
}

float DiskPrimitive::getRadius() const
{
	return m_radius;
}

void DiskPrimitive::setRadius( float radius )
{
	if ( radius <= Imath::limits<float>::epsilon() )
	{
		throw InvalidArgumentException( "Invalid radius specified for DiskPrimitive" );
	}
	m_radius = radius;
}

float DiskPrimitive::getZ() const
{
	return m_z;
}

void DiskPrimitive::setZ( float z )
{
	m_z = z;
}

float DiskPrimitive::getThetaMax() const
{
	return m_thetaMax;
}

void DiskPrimitive::setThetaMax( float degrees )
{
	if ( fabsf( degrees ) < 1.e-6 )
	{
		throw InvalidArgumentException( "Invalid thetaMax specified for DiskPrimitive" );
	}
	m_thetaMax = degrees;
}

size_t DiskPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
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

Imath::Box3f DiskPrimitive::bound() const
{
	/// \todo Take into account thetamax
	return Box3f(
		V3f( -m_radius, -m_radius, m_z ),
		V3f( m_radius, m_radius, m_z )
	);
}

void DiskPrimitive::render( Renderer *renderer ) const
{
	assert( renderer );
	renderer->disk( m_radius, m_z, m_thetaMax, variables );
}

void DiskPrimitive::copyFrom( const Object *other, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const DiskPrimitive *tOther = static_cast<const DiskPrimitive *>( other );
	m_radius = tOther->m_radius;
	m_z = tOther->m_z;
	m_thetaMax = tOther->m_thetaMax;
}

void DiskPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save(context);
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );

	container->write( g_radiusEntry, m_radius );
	container->write( g_zEntry, m_z );
	container->write( g_thetaMaxEntry, m_thetaMax );
}

void DiskPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load(context);
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	container->read( g_radiusEntry, m_radius );
	container->read( g_zEntry, m_z );
	container->read( g_thetaMaxEntry, m_thetaMax );
}

bool DiskPrimitive::isEqualTo( const Object *other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}

	const DiskPrimitive *tOther = static_cast<const DiskPrimitive *>( other );

	if( m_radius!=tOther->m_radius )
	{
		return false;
	}
	if( m_z!=tOther->m_z )
	{
		return false;
	}
	if( m_thetaMax!=tOther->m_thetaMax )
	{
		return false;
	}

	return true;
}

void DiskPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( m_radius ) );
	a.accumulate( sizeof( m_z ) );
	a.accumulate( sizeof( m_thetaMax ) );
}

void DiskPrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
}

void DiskPrimitive::topologyHash( MurmurHash &h ) const
{
	h.append( m_radius );
	h.append( m_z );
	h.append( m_thetaMax );
}
