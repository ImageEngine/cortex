//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PatchMeshPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/Renderer.h"
#include "IECore/MurmurHash.h"

using namespace IECore;

static IndexedIO::EntryID g_uPointsEntry("uPoints");
static IndexedIO::EntryID g_vPointsEntry("vPoints");
static IndexedIO::EntryID g_uBasisMatrixEntry("uBasisMatrix");
static IndexedIO::EntryID g_uBasisStepEntry("uBasisStep");
static IndexedIO::EntryID g_vBasisMatrixEntry("vBasisMatrix");
static IndexedIO::EntryID g_vBasisStepEntry("vBasisStep");
static IndexedIO::EntryID g_uPeriodicEntry("uPeriodic");
static IndexedIO::EntryID g_vPeriodicEntry("vPeriodic");

const unsigned int PatchMeshPrimitive::m_ioVersion = 1;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( PatchMeshPrimitive );

PatchMeshPrimitive::PatchMeshPrimitive()
		:
		m_uLinear( true ), m_vLinear( true ),
		m_uPoints( 0 ), m_vPoints( 0 ),
		m_uBasis( CubicBasisf::linear() ), m_vBasis( CubicBasisf::linear() ),
		m_uPeriodic( false ), m_vPeriodic( false )
{
}

PatchMeshPrimitive::PatchMeshPrimitive(
        unsigned int uPoints,
        unsigned int vPoints,
        const CubicBasisf &uBasis,
        const CubicBasisf &vBasis,
        bool uPeriodic,
        bool vPeriodic,
        ConstV3fVectorDataPtr p
) : m_uPoints( uPoints ), m_vPoints( vPoints ), m_uBasis( uBasis ), m_vBasis( vBasis ), m_uPeriodic( uPeriodic ),
		m_vPeriodic( vPeriodic )
{
	m_uLinear = m_uBasis==CubicBasisf::linear();
	m_vLinear = m_vBasis==CubicBasisf::linear();

	if ( !uPoints )
	{
		throw InvalidArgumentException( "PatchMeshPrimitive: Insufficient control points in u" );
	}
	if ( !vPoints )
	{
		throw InvalidArgumentException( "PatchMeshPrimitive: Insufficient control points in v" );
	}

	if ( m_uLinear )
	{
		if ( !m_uPeriodic && !m_uPoints )
		{
			throw InvalidArgumentException( "PatchMeshPrimitive: Insufficient control points in u" );
		}
	}
	else
	{
		if ( !m_uPeriodic && m_uPoints < 4 )
		{
			throw InvalidArgumentException( "PatchMeshPrimitive: Insufficient control points in u" );
		}
	}

	if ( m_vLinear )
	{
		if ( !m_vPeriodic && !m_vPoints )
		{
			throw InvalidArgumentException( "PatchMeshPrimitive: Insufficient control points in v" );
		}
	}
	else
	{
		if ( !m_vPeriodic && m_vPoints < 4 )
		{
			throw InvalidArgumentException( "PatchMeshPrimitive: Insufficient control points in v" );
		}
	}

	if ( p )
	{
		if ( p->readable().size() != m_uPoints * m_vPoints )
		{
			throw InvalidArgumentException( "PatchMeshPrimitive: Invalid length of primitive variable P" );
		}
		
		V3fVectorDataPtr pData = p->copy();
		pData->setInterpretation( GeometricData::Point );
		variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, pData );
	}
}

PatchMeshPrimitive::~PatchMeshPrimitive()
{
}

bool PatchMeshPrimitive::isEqualTo( const Object *other ) const
{
	if ( !Primitive::isEqualTo( other ) )
	{
		return false;
	}
	const PatchMeshPrimitive *tOther = static_cast<const PatchMeshPrimitive *>( other );
	if ( m_uPoints!=tOther->m_uPoints || m_vPoints!=tOther->m_vPoints )
	{
		return false;
	}
	if ( m_uLinear!=tOther->m_uLinear )
	{
		return false;
	}
	if ( m_vLinear!=tOther->m_vLinear )
	{
		return false;
	}
	if ( m_uBasis!=tOther->m_uBasis || m_vBasis!=tOther->m_vBasis )
	{
		return false;
	}
	if ( m_uPeriodic!=tOther->m_uPeriodic || m_vPeriodic!=tOther->m_vPeriodic )
	{
		return false;
	}

	return true;
}

void PatchMeshPrimitive::copyFrom( const Object *other, CopyContext *context )
{
	Primitive::copyFrom( other, context );
	const PatchMeshPrimitive *tOther = static_cast<const PatchMeshPrimitive *>( other );
	m_uPoints = tOther->m_uPoints;
	m_vPoints = tOther->m_vPoints;
	m_uBasis = tOther->m_uBasis;
	m_vBasis = tOther->m_vBasis;
	m_uPeriodic = tOther->m_uPeriodic;
	m_vPeriodic = tOther->m_vPeriodic;
	m_uLinear = tOther->m_uLinear;
	m_vLinear = tOther->m_vLinear;
}

void PatchMeshPrimitive::save( IECore::Object::SaveContext *context ) const
{
	Primitive::save( context );

	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );

	container->write( g_uPointsEntry, m_uPoints );
	container->write( g_vPointsEntry, m_vPoints );
	container->write( g_uBasisMatrixEntry, m_uBasis.matrix.getValue(), 16 );
	container->write( g_uBasisStepEntry, m_uBasis.step );
	container->write( g_vBasisMatrixEntry, m_vBasis.matrix.getValue(), 16 );
	container->write( g_vBasisStepEntry, m_vBasis.step );
	container->write( g_uPeriodicEntry, (char)(m_uPeriodic) );
	container->write( g_vPeriodicEntry, (char)(m_vPeriodic) );
}

void PatchMeshPrimitive::load( IECore::Object::LoadContextPtr context )
{
	Primitive::load( context );
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	container->read( g_uPointsEntry, m_uPoints );
	container->read( g_vPointsEntry, m_vPoints );

	float *f = m_uBasis.matrix.getValue();
	container->read( g_uBasisMatrixEntry, f, 16 );
	container->read( g_uBasisStepEntry, m_uBasis.step );

	f = m_vBasis.matrix.getValue();
	container->read( g_vBasisMatrixEntry, f, 16 );
	container->read( g_vBasisStepEntry, m_vBasis.step );

	char p = 0;
	container->read( g_uPeriodicEntry, p );
	m_uPeriodic = p;
	container->read( g_vPeriodicEntry, p );
	m_vPeriodic = p;

	m_uLinear = m_uBasis==CubicBasisf::linear();
	m_vLinear = m_vBasis==CubicBasisf::linear();
}

void PatchMeshPrimitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Primitive::memoryUsage( a );
	a.accumulate( sizeof( CubicBasisf ) * 2 + sizeof( bool ) * 4 + sizeof( unsigned ) * 2 );
}

void PatchMeshPrimitive::hash( MurmurHash &h ) const
{
	Primitive::hash( h );
}

void PatchMeshPrimitive::topologyHash( MurmurHash &h ) const
{
	h.append( m_uPoints );
	h.append( m_vPoints );
	h.append( m_uBasis.matrix );
	h.append( m_uBasis.step );
	h.append( m_vBasis.matrix );
	h.append( m_vBasis.step );
	h.append( m_uPeriodic );
	h.append( m_vPeriodic );
}

unsigned int PatchMeshPrimitive::uPoints() const
{
	return m_uPoints;
}

unsigned int PatchMeshPrimitive::vPoints() const
{
	return m_vPoints;
}

unsigned int PatchMeshPrimitive::uPatches() const
{
	if ( m_uLinear )
	{
		if ( m_uPeriodic )
		{
			return m_uPoints;
		}
		else
		{
			return m_uPoints - 1;
		}
	}
	else
	{
		if ( m_uPeriodic )
		{
			return m_uPoints /  m_uBasis.step;
		}
		else
		{
			return ( ( m_uPoints - 4 ) /  m_uBasis.step ) + 1;
		}
	}
}

unsigned int PatchMeshPrimitive::vPatches() const
{
	if ( m_vLinear )
	{
		if ( m_vPeriodic )
		{
			return m_vPoints;
		}
		else
		{
			return m_vPoints - 1;
		}
	}
	else
	{
		if ( m_vPeriodic )
		{
			return m_vPoints /  m_vBasis.step;
		}
		else
		{
			return ( ( m_vPoints - 4 ) /  m_vBasis.step ) + 1;
		}
	}
}

const CubicBasisf &PatchMeshPrimitive::uBasis() const
{
	return m_uBasis;
}

const CubicBasisf &PatchMeshPrimitive::vBasis() const
{
	return m_vBasis;
}

bool PatchMeshPrimitive::uPeriodic() const
{
	return m_uPeriodic;
}

bool PatchMeshPrimitive::vPeriodic() const
{
	return m_vPeriodic;
}

void PatchMeshPrimitive::render( Renderer *renderer ) const
{
	renderer->patchMesh(
		m_uBasis,
		m_vBasis,
		m_uPoints,
		m_uPeriodic,
		m_vPoints,
		m_vPeriodic,
		Primitive::variables
	);
}

size_t PatchMeshPrimitive::variableSize( PrimitiveVariable::Interpolation interpolation ) const
{
	switch ( interpolation )
	{
		case PrimitiveVariable::Constant :
			return 1;
		case PrimitiveVariable::Uniform :
			return uPatches() * vPatches();
		case PrimitiveVariable::Vertex :
			return m_uPoints * m_vPoints;
		case PrimitiveVariable::Varying :
		case PrimitiveVariable::FaceVarying :

			{
				if ( !m_uPeriodic )
				{
					if ( m_vPeriodic )
					{
						return ( uPatches() + 1 ) * vPatches();
					}
					else
					{
						return ( uPatches() + 1 ) * ( vPatches() + 1 );
					}
				}
				else
				{
					if ( !m_vPeriodic )
					{
						return uPatches() * ( vPatches() + 1 );
					}
					else
					{
						return uPatches() * vPatches();
					}
				}
			}
			assert( false ); // Unreachable
		default :
			return 0;
	}
}
