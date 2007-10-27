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

#include "IECore/NURBSPrimitive.h"
#include "IECore/Renderer.h"

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

const unsigned int NURBSPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(NURBSPrimitive);

NURBSPrimitive::NURBSPrimitive()
{
	vector<float> knot;
	knot.push_back( 0 );
	knot.push_back( 0 );
	knot.push_back( 0 );
	knot.push_back( 0.333 );
	knot.push_back( 0.666 );
	knot.push_back( 1 );
	knot.push_back( 1 );
	knot.push_back( 1 );
	setTopology( 4, new FloatVectorData( knot ), 0, 1, 4, new FloatVectorData( knot ), 0, 1 );
}

NURBSPrimitive::NURBSPrimitive( int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax,
	int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax, ConstV3fVectorDataPtr p )
{
	setTopology( uOrder, uKnot, uMin, uMax, vOrder, vKnot, vMin, vMax );
	if( p )
	{
		variables.insert( PrimitiveVariableMap::value_type( "P", PrimitiveVariable( PrimitiveVariable::Vertex,
			p->copy() ) ) );
	}
}

int NURBSPrimitive::uOrder() const
{
	return m_uOrder;
}

ConstFloatVectorDataPtr NURBSPrimitive::uKnot() const
{
	return m_uKnot;
}

float NURBSPrimitive::uMin() const
{
	return m_uMin;
}

float NURBSPrimitive::uMax() const
{
	return m_uMax;
}

int NURBSPrimitive::uVertices() const
{
	return m_uKnot->readable().size() - m_uOrder;
}

int NURBSPrimitive::uSegments() const
{
	return 1 + uVertices() - m_uOrder;
}

int NURBSPrimitive::vOrder() const
{
	return m_vOrder;
}

ConstFloatVectorDataPtr NURBSPrimitive::vKnot() const
{
	return m_vKnot;
}

float NURBSPrimitive::vMin() const
{
	return m_vMin;
}

float NURBSPrimitive::vMax() const
{
	return m_vMax;
}

int NURBSPrimitive::vVertices() const
{
	return m_vKnot->readable().size() - m_vOrder;
}

int NURBSPrimitive::vSegments() const
{
	return 1 + vVertices() - m_vOrder;
}

void NURBSPrimitive::setTopology(  int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax,
	int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax )
{
	// check order isn't too small
	if( uOrder<2 )
	{
		throw Exception( "Order in u direction too small." );
	}
	if( vOrder<2 )
	{
		throw Exception( "Order in v direction too small." );
	}

	// check knots have enough entries for the order.
	// an order of N demands at least N control points
	// and numKnots==numControlPoints + order
	// so we need numKnots>=2*order
	if( uKnot->readable().size() < uOrder * 2 )
	{
		throw Exception( "Not enough knot values in u direction." );
	}
	if( vKnot->readable().size() < vOrder * 2 )
	{
		throw Exception( "Not enough knot values in v direction." );
	}
	
	// check knots are monotonically increasing
	const vector<float> &u = uKnot->readable();
	float previous = u[0];
	for( unsigned int i=0; i<u.size(); i++ )
	{
		if( u[i]<previous )
		{
			throw Exception( "Knots not monotonically increasing in u direction." );
		}
		previous = u[i];
	}
	const vector<float> &v = vKnot->readable();
	previous = v[0];
	for( unsigned int i=0; i<v.size(); i++ )
	{
		if( v[i]<previous )
		{
			throw Exception( "Knots not monotonically increasing in v direction." );
		}
		previous = v[i];
	}
	
	// check min and max parametric values are in range
	if( uMin > uMax )
	{
		throw Exception( "uMin greater than uMax." );
	}
	if( vMin > vMax )
	{
		throw Exception( "vMin greater than vMax." );
	}
	
	if( uMin < u[uOrder-2] )
	{
		throw Exception( "uMin too small." );
	}
	if( uMax > u[u.size()-uOrder+1] )
	{
		throw Exception( "uMax too great." );
	}
	
	if( vMin < v[vOrder-2] )
	{
		throw Exception( "vMin too small." );
	}
	if( vMax > v[v.size()-vOrder+1] )
	{
		throw Exception( "vMax too great." );
	}

	// set everything (taking copies of the data)
	
	m_uOrder = uOrder;
	m_uKnot = uKnot->copy();
	m_uMin = uMin;
	m_uMax = uMax;
	m_vOrder = vOrder;
	m_vKnot = vKnot->copy();
	m_vMin = vMin;
	m_vMax = vMax;
}
		
size_t NURBSPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation )
{
	switch( interpolation )
	{
		case PrimitiveVariable::Constant :
			return 1;
			
		case PrimitiveVariable::Uniform :
			return uSegments() * vSegments();
			
		case PrimitiveVariable::Vertex :
			return uVertices() * vVertices();
			
		case PrimitiveVariable::Varying:
		case PrimitiveVariable::FaceVarying:
			return (uSegments()+1) * (vSegments()+1);
						
		default :
			return 0;
		
	}
}

void NURBSPrimitive::render( RendererPtr renderer )
{
	renderer->nurbs( m_uOrder, m_uKnot, m_uMin, m_uMax, m_vOrder, m_vKnot, m_vMin, m_vMax, variables );
}

void NURBSPrimitive::copyFrom( ConstObjectPtr other, IECore::Object::CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const NURBSPrimitive *tOther = static_cast<const NURBSPrimitive *>( other.get() );
	m_uOrder = tOther->m_uOrder;
	m_uKnot = context->copy<FloatVectorData>( tOther->m_uKnot );
	m_uMin = tOther->m_uMin;
	m_uMax = tOther->m_uMax;
	m_vOrder = tOther->m_vOrder;
	m_vKnot = context->copy<FloatVectorData>( tOther->m_vKnot );
	m_vMin = tOther->m_vMin;
	m_vMax = tOther->m_vMax;
}

void NURBSPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save(context);
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	
	container->write( "uOrder", m_uOrder );
	context->save( m_uKnot, container, "uKnot" );
	container->write( "uMin", m_uMin );
	container->write( "uMax", m_uMax );

	container->write( "vOrder", m_vOrder );
	context->save( m_vKnot, container, "vKnot" );
	container->write( "vMin", m_vMin );
	container->write( "vMax", m_vMax );	
}

void NURBSPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load(context);
	unsigned int v = m_ioVersion;
	
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	container->read( "uOrder", m_uOrder );
	m_uKnot = context->load<FloatVectorData>( container, "uKnot" );
	container->read( "uMin", m_uMin );
	container->read( "uMax", m_uMax );
	
	container->read( "vOrder", m_vOrder );
	m_vKnot = context->load<FloatVectorData>( container, "vKnot" );
	container->read( "vMin", m_vMin );
	container->read( "vMax", m_vMax );
}

bool NURBSPrimitive::isEqualTo( ConstObjectPtr other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}
	
	const NURBSPrimitive *tOther = static_cast<const NURBSPrimitive *>( other.get() );
	
	if( m_uOrder!=tOther->m_uOrder )
	{
		return false;
	}
	if( m_vOrder!=tOther->m_vOrder )
	{
		return false;
	}
	
	if( m_uMin!=tOther->m_uMin )
	{
		return false;
	}
	if( m_vMin!=tOther->m_vMin )
	{
		return false;
	}
	
	if( m_uMax!=tOther->m_uMax )
	{
		return false;
	}
	if( m_vMax!=tOther->m_vMax )
	{
		return false;
	}
	
	if( !m_uKnot->isEqualTo( tOther->m_uKnot ) )
	{
		return false;
	}
	if( !m_vKnot->isEqualTo( tOther->m_vKnot ) )
	{
		return false;
	}
		
	return true;
}

void NURBSPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( m_uOrder ) * 2 );
	a.accumulate( sizeof( m_uMin ) * 4 );
	a.accumulate( m_uKnot );
	a.accumulate( m_vKnot );
}
