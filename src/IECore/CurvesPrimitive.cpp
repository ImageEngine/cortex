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

#include "IECore/CurvesPrimitive.h"
#include "IECore/Renderer.h"

using namespace IECore;

const unsigned int CurvesPrimitive::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( CurvesPrimitive );

CurvesPrimitive::CurvesPrimitive()
	:	
		m_basis( CubicBasisf::linear() ),
		m_linear( true ),
		m_periodic( false ),
		m_vertsPerCurve( new IntVectorData() ),
		m_numVerts( 0 ),
		m_numFaceVarying( 0 )
{
}

CurvesPrimitive::CurvesPrimitive( ConstIntVectorDataPtr vertsPerCurve, const CubicBasisf &basis, bool periodic, ConstV3fVectorDataPtr p )
	:	m_basis( basis ), m_periodic( periodic ), m_vertsPerCurve( vertsPerCurve->copy() ), m_numVerts( 0 ), m_numFaceVarying( 0 )
{
	m_linear = m_basis==CubicBasisf::linear();

	const std::vector<int> &v = m_vertsPerCurve->readable();
	for( std::vector<int>::const_iterator it=v.begin(); it!=v.end(); it++ )
	{
		if( m_periodic )
		{
			m_numFaceVarying += numSegments( *it );
		}
		else
		{
			m_numFaceVarying += numSegments( *it ) + 1;
		}
		
		m_numVerts += *it;
	}
	
	if( p )
	{
		variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, p->copy() );
	}
}

CurvesPrimitive::~CurvesPrimitive()
{
}

bool CurvesPrimitive::isEqualTo( ConstObjectPtr other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}
	const CurvesPrimitive *tOther = static_cast<const CurvesPrimitive *>( other.get() );
	if( m_basis!=tOther->m_basis )
	{
		return false;
	}
	if( m_periodic!=tOther->m_periodic )
	{
		return false;
	}
	if( !m_vertsPerCurve->isEqualTo( tOther->m_vertsPerCurve ) )
	{
		return false;
	}
	
	return true;
}

void CurvesPrimitive::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const CurvesPrimitive *tOther = static_cast<const CurvesPrimitive *>( other.get() );
	m_basis = tOther->m_basis;
	m_linear = tOther->m_linear;
	m_periodic = tOther->m_periodic;
	m_vertsPerCurve = tOther->m_vertsPerCurve->copy(); // don't need to use context as we don't share this data with anyone
	m_numVerts = tOther->m_numVerts;
	m_numFaceVarying = tOther->m_numFaceVarying;
}

void CurvesPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save(context);

	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( "basisMatrix", m_basis.matrix.getValue(), 16 );
	container->write( "basisStep", m_basis.step );
	int p = m_periodic;
	container->write( "periodic", p );
	context->save( m_vertsPerCurve, container, "verticesPerCurve" );
	// we could recompute these on loading, but it'd take a while and the overhead
	// of storing them isn't great.
	container->write( "numVerts", m_numVerts );
	container->write( "numFaceVarying", m_numFaceVarying );	
}

void CurvesPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load( context );
	unsigned int v = m_ioVersion;
	
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	float *f = m_basis.matrix.getValue();
	container->read( "basisMatrix", f, 16 );
	container->read( "basisStep", m_basis.step );
	m_linear = m_basis==CubicBasisf::linear();
	int p = 0;
	container->read( "periodic", p );
	m_periodic = p;
	m_vertsPerCurve = context->load<IntVectorData>( container, "verticesPerCurve" );
	container->read( "numVerts", m_numVerts );
	container->read( "numFaceVarying", m_numFaceVarying );
}

void CurvesPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( CubicBasisf ) + sizeof( bool ) * 2 + sizeof( unsigned ) * 2 );
	a.accumulate( m_vertsPerCurve );
}

ConstIntVectorDataPtr CurvesPrimitive::verticesPerCurve() const
{
	return m_vertsPerCurve;
}

const CubicBasisf &CurvesPrimitive::basis() const
{
	return m_basis;
}

bool CurvesPrimitive::periodic() const
{
	return m_periodic;
}
		
void CurvesPrimitive::render( RendererPtr renderer )
{
	std::string interpolation = "unknown";
	if( m_linear )
	{
		interpolation = "linear";
	}
	else
	{
		if( m_basis==CubicBasisf::bezier() )
		{
			interpolation = "bezier";
		}
		else if( m_basis==CubicBasisf::bSpline() )
		{
			interpolation = "bSpline";
		}
		else if( m_basis==CubicBasisf::catmullRom() )
		{
			interpolation = "catmullRom";
		}
	}
	renderer->curves( interpolation, m_periodic, m_vertsPerCurve, variables );
}

size_t CurvesPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
{
	switch( interpolation )
	{
		case PrimitiveVariable::Constant :
			return 1;
		case PrimitiveVariable::Uniform :
			return m_vertsPerCurve->readable().size();
		case PrimitiveVariable::Varying :
		case PrimitiveVariable::FaceVarying :
			return m_numFaceVarying;
		case PrimitiveVariable::Vertex :
			return m_numVerts;
		default :
			return 0;
	}
}

unsigned int CurvesPrimitive::numSegments( int numVerts )
{
	if( m_linear )
	{
		if( m_periodic )
		{
			if( numVerts < 3 )
			{
				throw Exception( "Linear periodic curve with less than 3 vertices." );
			}
			return numVerts;
		}
		else
		{
			if( numVerts < 2 )
			{
				throw Exception( "Linear non-periodic curve with less than 2 vertices." );
			}
			return numVerts - 1;
		}
	}
	else
	{
		if( m_periodic )
		{
			if( numVerts < 3 )
			{
				throw Exception( "Cubic periodic curve with less than 3 vertices." );
			}
			if( (numVerts - 1) % m_basis.step )
			{
				throw Exception( "Cubic curve with extra vertices." );
			}
			return numVerts / m_basis.step;
		}
		else
		{
			if( numVerts < 4 )
			{
				throw Exception( "Cubic nonperiodic curve with less than 4 vertices." );
			}
			if( (numVerts - 4) % m_basis.step )
			{
				throw Exception( "Cubic curve with extra vertices." );
			}
			return (numVerts - 4 )/ m_basis.step + 1;	
		}
	}
}
