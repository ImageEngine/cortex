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

#include <cassert>

#include "IECore/MeshPrimitiveBuilder.h"

using namespace IECore;
using namespace Imath;

MeshPrimitiveBuilder::MeshPrimitiveBuilder()
{
	m_P = new V3fVectorData();
	m_N = new V3fVectorData();
	m_P->setInterpretation( GeometricData::Point );
	m_N->setInterpretation( GeometricData::Normal );
	m_verticesPerFace = new IntVectorData();
	m_vertexIds = new IntVectorData();
}

namespace IECore
{

template<> IECORE_API
void MeshPrimitiveBuilder::addVertex<float>( const Imath::Vec3<float> &p, const Imath::Vec3<float> &n )
{
	assert( m_P );
	assert( m_N );

	m_P->writable().push_back( p );
	m_N->writable().push_back( n.normalized() );
}

template<> IECORE_API
void MeshPrimitiveBuilder::addVertex<double>( const Imath::Vec3<double> &p, const Imath::Vec3<double> &n )
{
	assert( m_P );
	assert( m_N );

	m_P->writable().push_back( Imath::V3f( p.x, p.y, p.z ) );
	m_N->writable().push_back( Imath::V3f( n.x, n.y, n.z ).normalized() );
}

}

void MeshPrimitiveBuilder::addTriangle( int v0, int v1, int v2 )
{
	assert( m_verticesPerFace );
	assert( m_vertexIds );

	m_verticesPerFace->writable().push_back( 3 );

	m_vertexIds->writable().push_back ( v0 );
	m_vertexIds->writable().push_back ( v1 );
	m_vertexIds->writable().push_back ( v2 );
}

MeshPrimitivePtr MeshPrimitiveBuilder::mesh() const
{
	MeshPrimitivePtr m = new MeshPrimitive( m_verticesPerFace, m_vertexIds, "linear", m_P );
	m->variables["N"] =  IECore::PrimitiveVariable( IECore::PrimitiveVariable::Varying, m_N->copy() );

	return m;
}
