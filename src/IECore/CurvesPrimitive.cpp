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

#include "IECore/CurvesPrimitive.h"
#include "IECore/Renderer.h"
#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace Imath;

static IndexedIO::EntryID g_basisMatrixEntry("basisMatrix");
static IndexedIO::EntryID g_basisStepEntry("basisStep");
static IndexedIO::EntryID g_periodicEntry("periodic");
static IndexedIO::EntryID g_verticesPerCurveEntry("verticesPerCurve");
static IndexedIO::EntryID g_numVertsEntry("numVerts");
static IndexedIO::EntryID g_numFaceVaryingEntry("numFaceVarying");

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
	: 	m_basis( CubicBasisf::linear() )
{
	setTopology( vertsPerCurve, basis, periodic );

	if( p )
	{
		V3fVectorDataPtr pData = p->copy();
		pData->setInterpretation( GeometricData::Point );
		variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, pData );
	}
}

CurvesPrimitive::~CurvesPrimitive()
{
}

bool CurvesPrimitive::isEqualTo( const Object *other ) const
{
	if( !Primitive::isEqualTo( other ) )
	{
		return false;
	}
	const CurvesPrimitive *tOther = static_cast<const CurvesPrimitive *>( other );
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

void CurvesPrimitive::copyFrom( const Object *other, CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const CurvesPrimitive *tOther = static_cast<const CurvesPrimitive *>( other );
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

	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( g_basisMatrixEntry, m_basis.matrix.getValue(), 16 );
	container->write( g_basisStepEntry, m_basis.step );
	int p = m_periodic;
	container->write( g_periodicEntry, p );
	context->save( m_vertsPerCurve, container, g_verticesPerCurveEntry );
	// we could recompute these on loading, but it'd take a while and the overhead
	// of storing them isn't great.
	container->write( g_numVertsEntry, m_numVerts );
	container->write( g_numFaceVaryingEntry, m_numFaceVarying );
}

void CurvesPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load( context );
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	float *f = m_basis.matrix.getValue();
	container->read( g_basisMatrixEntry, f, 16 );
	container->read( g_basisStepEntry, m_basis.step );
	m_linear = m_basis==CubicBasisf::linear();
	int p = 0;
	container->read( g_periodicEntry, p );
	m_periodic = p;
	m_vertsPerCurve = context->load<IntVectorData>( container, g_verticesPerCurveEntry );
	container->read( g_numVertsEntry, m_numVerts );
	container->read( g_numFaceVaryingEntry, m_numFaceVarying );
}

void CurvesPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( CubicBasisf ) + sizeof( bool ) * 2 + sizeof( unsigned ) * 2 );
	a.accumulate( m_vertsPerCurve );
}

void CurvesPrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
}

void CurvesPrimitive::topologyHash( MurmurHash &h ) const
{
	h.append( m_basis.matrix );
	h.append( m_basis.step );
	h.append( m_linear );
	h.append( m_periodic );
	m_vertsPerCurve->hash( h );
}

size_t CurvesPrimitive::numCurves() const
{
	return m_vertsPerCurve->readable().size();
}

const IntVectorData *CurvesPrimitive::verticesPerCurve() const
{
	return m_vertsPerCurve.get();
}

const CubicBasisf &CurvesPrimitive::basis() const
{
	return m_basis;
}

bool CurvesPrimitive::periodic() const
{
	return m_periodic;
}

void CurvesPrimitive::setTopology( ConstIntVectorDataPtr verticesPerCurve, const CubicBasisf &basis, bool periodic )
{
	m_basis = basis;
	m_linear = m_basis==CubicBasisf::linear();
	m_periodic = periodic;
	m_vertsPerCurve = verticesPerCurve->copy();

	m_numVerts = 0;
	m_numFaceVarying = 0;
	const std::vector<int> &v = m_vertsPerCurve->readable();
	for( size_t i=0; i<v.size(); i++ )
	{
		if( m_periodic )
		{
			m_numFaceVarying += numSegments( i );
		}
		else
		{
			m_numFaceVarying += numSegments( i ) + 1;
		}

		m_numVerts += v[i];
	}
}

void CurvesPrimitive::render( Renderer *renderer ) const
{
	renderer->curves( m_basis, m_periodic, m_vertsPerCurve, variables );
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

size_t CurvesPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation, unsigned curveIndex ) const
{
	if( curveIndex < 0 || curveIndex >= m_vertsPerCurve->readable().size() )
	{
		throw Exception( "Curve index out of range." );
	}

	switch( interpolation )
	{
		case PrimitiveVariable::Constant :
			throw Exception( "Constant variables are not specified on a per curve basis." );
		case PrimitiveVariable::Uniform :
			return 1;
		case PrimitiveVariable::Varying :
		case PrimitiveVariable::FaceVarying :
			if( m_periodic )
			{
				return numSegments( curveIndex );
			}
			else
			{
				return 1 + numSegments( curveIndex );
			}
		case PrimitiveVariable::Vertex :
			return m_vertsPerCurve->readable()[curveIndex];
		default :
			throw Exception( "Invalid interpolation type." );
	}
}

unsigned CurvesPrimitive::numSegments( unsigned curveIndex ) const
{
	if( curveIndex < 0 || curveIndex >= m_vertsPerCurve->readable().size() )
	{
		throw Exception( "Curve index out of range." );
	}
	return numSegments( m_linear, m_basis.step, m_periodic, m_vertsPerCurve->readable()[curveIndex] );
}

unsigned CurvesPrimitive::numSegments( const CubicBasisf &basis, bool periodic, unsigned numVerts )
{
	return numSegments( basis==CubicBasisf::linear(), basis.step, periodic, numVerts );
}

unsigned int CurvesPrimitive::numSegments( bool linear, int step, bool periodic, int numVerts )
{
	if( linear )
	{
		if( periodic )
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
		if( periodic )
		{
			if( numVerts < 3 )
			{
				throw Exception( "Cubic periodic curve with less than 3 vertices." );
			}
			if( (numVerts - 1) % step )
			{
				throw Exception( "Cubic curve with extra vertices." );
			}
			return numVerts / step;
		}
		else
		{
			if( numVerts < 4 )
			{
				throw Exception( "Cubic nonperiodic curve with less than 4 vertices." );
			}
			if( (numVerts - 4) % step )
			{
				throw Exception( "Cubic curve with extra vertices." );
			}
			return (numVerts - 4 )/ step + 1;
		}
	}
}

CurvesPrimitivePtr CurvesPrimitive::createBox( const Imath::Box3f &b )
{
	IntVectorDataPtr vertsPerCurveData = new IntVectorData;
	std::vector<int> &vertsPerCurve = vertsPerCurveData->writable();
	vertsPerCurve.reserve( 6 );
	
	V3fVectorDataPtr pData = new V3fVectorData;
	std::vector<V3f> &p = pData->writable();
	p.reserve( 18 );
	
	vertsPerCurve.push_back( 5 );
	p.push_back( b.min );
	p.push_back( V3f( b.max.x, b.min.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.min.y, b.max.z ) );
	p.push_back( V3f( b.min.x, b.min.y, b.max.z ) );
	p.push_back( b.min );
	
	vertsPerCurve.push_back( 5 );
	p.push_back( V3f( b.min.x, b.max.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.max.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.max.y, b.max.z ) );
	p.push_back( V3f( b.min.x, b.max.y, b.max.z ) );
	p.push_back( V3f( b.min.x, b.max.y, b.min.z ) );

	vertsPerCurve.push_back( 2 );
	p.push_back( b.min );
	p.push_back( V3f( b.min.x, b.max.y, b.min.z ) );

	vertsPerCurve.push_back( 2 );
	p.push_back( V3f( b.max.x, b.min.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.max.y, b.min.z ) );

	vertsPerCurve.push_back( 2 );
	p.push_back( V3f( b.max.x, b.min.y, b.max.z ) );
	p.push_back( V3f( b.max.x, b.max.y, b.max.z ) );
	
	vertsPerCurve.push_back( 2 );
	p.push_back( V3f( b.min.x, b.min.y, b.max.z ) );
	p.push_back( V3f( b.min.x, b.max.y, b.max.z ) );
	
	return new CurvesPrimitive( vertsPerCurveData, CubicBasisf::linear(), false, pData );
}
