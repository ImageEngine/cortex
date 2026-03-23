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

#include "IECoreScene/CurvesPrimitive.h"

#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

namespace
{

const IndexedIO::EntryID g_basisMatrixEntry( "basisMatrix" );
const IndexedIO::EntryID g_basisStepEntry( "basisStep" );
const IndexedIO::EntryID g_periodicEntry( "periodic" );
const IndexedIO::EntryID g_wrapEntry( "wrap" );
const IndexedIO::EntryID g_verticesPerCurveEntry( "verticesPerCurve" );
const IndexedIO::EntryID g_numVertsEntry( "numVerts" );
const IndexedIO::EntryID g_numFaceVaryingEntry( "numFaceVarying" );

/// Throws an exception if numVerts is an inappropriate number for the specified basis.
unsigned int numSegmentsInternal( StandardCubicBasis basis, int step, CurvesPrimitive::Wrap wrap, int numVerts )
{
	if( basis == StandardCubicBasis::Linear )
	{
		switch( wrap )
		{
			case CurvesPrimitive::Wrap::Periodic :
				if( numVerts < 3 )
				{
					throw Exception( "Linear periodic curve with less than 3 vertices." );
				}
				return numVerts;
			case CurvesPrimitive::Wrap::NonPeriodic :
			case CurvesPrimitive::Wrap::Pinned :
				if( numVerts < 2 )
				{
					throw Exception( "Linear non-periodic curve with less than 2 vertices." );
				}
				return numVerts - 1;
			default :
				return 0;
		}
	}
	else
	{
		switch( wrap )
		{
			case CurvesPrimitive::Wrap::Periodic :
				if( numVerts < 3 )
				{
					throw Exception( "Cubic periodic curve with less than 3 vertices." );
				}
				if( (numVerts - 1) % step )
				{
					throw Exception( "Cubic curve with extra vertices." );
				}
				return numVerts / step;
			case CurvesPrimitive::Wrap::Pinned :
				if( basis == StandardCubicBasis::BSpline || basis == StandardCubicBasis::CatmullRom )
				{
					if( numVerts < 2 )
					{
						throw Exception( "Cubic pinned curve with less than 2 vertices." );
					}
					return numVerts - 1;
				}
				[[fallthrough]]; // Treat as NonPeriodic.
			case CurvesPrimitive::Wrap::NonPeriodic :
				if( numVerts < 4 )
				{
					throw Exception( "Cubic non-periodic curve with less than 4 vertices." );
				}
				if( (numVerts - 4) % step )
				{
					throw Exception( "Cubic curve with extra vertices." );
				}
				return (numVerts - 4 ) / step + 1;
			default :
				return 0;
		}
	}
}

} // namespace

const unsigned int CurvesPrimitive::m_ioVersion = 1;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( CurvesPrimitive );

CurvesPrimitive::CurvesPrimitive()
	:
		m_basis( CubicBasisf::linear() ),
		m_standardBasis( StandardCubicBasis::Linear ),
		m_wrap( Wrap::NonPeriodic ),
		m_vertsPerCurve( new IntVectorData() ),
		m_numVerts( 0 ),
		m_numFaceVarying( 0 )
{
}

CurvesPrimitive::CurvesPrimitive( const IECore::IntVectorData *vertsPerCurve, const IECore::CubicBasisf &basis, Wrap wrap, const IECore::V3fVectorData *p )
	:	m_basis( CubicBasisf::linear() )
{
	setTopology( vertsPerCurve, basis, wrap );

	if( p )
	{
		V3fVectorDataPtr pData = p->copy();
		pData->setInterpretation( GeometricData::Point );
		variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, pData );
	}
}

CurvesPrimitive::CurvesPrimitive( ConstIntVectorDataPtr vertsPerCurve, const CubicBasisf &basis, bool periodic, ConstV3fVectorDataPtr p )
	:	CurvesPrimitive( vertsPerCurve.get(), basis, periodic ? Wrap::Periodic : Wrap::NonPeriodic, p.get() )
{
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
	if( m_wrap!=tOther->m_wrap )
	{
		return false;
	}
	if( !m_vertsPerCurve->isEqualTo( tOther->m_vertsPerCurve.get() ) )
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
	m_standardBasis = tOther->m_standardBasis;
	m_wrap = tOther->m_wrap;
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
	int wrap = static_cast<int>( m_wrap );
	container->write( g_wrapEntry, wrap );
	if( m_wrap != Wrap::Pinned )
	{
		// Although it is redundant, we write the `periodic` entry for the benefit
		// of old versions of the library, so that loading won't fail. We don't do
		// this for pinned curves though, since they aren't supported in old versions.
		/// \todo Remove
		int p = periodic();
		container->write( g_periodicEntry, p );
	}
	context->save( m_vertsPerCurve.get(), container.get(), g_verticesPerCurveEntry );
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
	m_standardBasis = m_basis.standardBasis();

	if( v >= 1 )
	{
		int wrap = 0;
		container->read( g_wrapEntry, wrap );
		m_wrap = static_cast<Wrap>( wrap );
	}
	else
	{
		int p = 0;
		container->read( g_periodicEntry, p );
		m_wrap = p ? Wrap::Periodic : Wrap::NonPeriodic;
	}
	m_vertsPerCurve = context->load<IntVectorData>( container.get(), g_verticesPerCurveEntry );
	container->read( g_numVertsEntry, m_numVerts );
	container->read( g_numFaceVaryingEntry, m_numFaceVarying );
}

void CurvesPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( CubicBasisf ) + sizeof( bool ) * 2 + sizeof( unsigned ) * 2 );
	a.accumulate( m_vertsPerCurve.get() );
}

void CurvesPrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
}

void CurvesPrimitive::topologyHash( MurmurHash &h ) const
{
	h.append( m_basis.matrix );
	h.append( m_basis.step );
	h.append( m_wrap );
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

CurvesPrimitive::Wrap CurvesPrimitive::wrap() const
{
	return m_wrap;
}

bool CurvesPrimitive::periodic() const
{
	return m_wrap == Wrap::Periodic;
}

void CurvesPrimitive::setTopology( const IECore::IntVectorData *verticesPerCurve, const IECore::CubicBasisf &basis, Wrap wrap )
{
	m_basis = basis;
	m_standardBasis = m_basis.standardBasis();
	m_wrap = wrap;
	m_vertsPerCurve = verticesPerCurve->copy();

	m_numVerts = 0;
	m_numFaceVarying = 0;
	const std::vector<int> &v = m_vertsPerCurve->readable();
	for( size_t i=0; i<v.size(); i++ )
	{
		m_numFaceVarying += variableSize( PrimitiveVariable::FaceVarying, i );
		m_numVerts += v[i];
	}
}

void CurvesPrimitive::setTopology( ConstIntVectorDataPtr verticesPerCurve, const CubicBasisf &basis, bool periodic )
{
	setTopology( verticesPerCurve.get(), basis, periodic ? Wrap::Periodic : Wrap::NonPeriodic );
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
	if( curveIndex >= m_vertsPerCurve->readable().size() )
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
			switch( m_wrap )
			{
				case Wrap::Periodic :
					return numSegments( curveIndex );
				case Wrap::NonPeriodic :
				case Wrap::Pinned :
					return 1 + numSegments( curveIndex );
				default :

					return 0;
			}
		case PrimitiveVariable::Vertex :
			return m_vertsPerCurve->readable()[curveIndex];
		default :
			throw Exception( "Invalid interpolation type." );
	}
}

unsigned CurvesPrimitive::numSegments( unsigned curveIndex ) const
{
	if( curveIndex >= m_vertsPerCurve->readable().size() )
	{
		throw Exception( "Curve index out of range." );
	}
	return numSegmentsInternal( m_standardBasis, m_basis.step, m_wrap, m_vertsPerCurve->readable()[curveIndex] );
}

unsigned CurvesPrimitive::numSegments( const IECore::CubicBasisf &basis, Wrap wrap, unsigned numVerts )
{
	return numSegmentsInternal( basis.standardBasis(), basis.step, wrap, numVerts );
}

unsigned CurvesPrimitive::numSegments( const CubicBasisf &basis, bool periodic, unsigned numVerts )
{
	return numSegmentsInternal( basis.standardBasis(), basis.step, periodic ? Wrap::Periodic : Wrap::NonPeriodic, numVerts );
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
