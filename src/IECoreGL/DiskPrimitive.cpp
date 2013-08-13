//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathMath.h"

#include "IECore/MessageHandler.h"

#include "IECoreGL/DiskPrimitive.h"
#include "IECoreGL/GL.h"

using namespace std;
using namespace Imath;
using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( DiskPrimitive );

DiskPrimitive::DiskPrimitive( float radius, float z, float thetaMax )
	:	m_radius( radius ), m_z( z ), m_thetaMax( thetaMax ), m_nPoints( 0 )
{
	
	// build vertex attributes for P, N and st, and indexes for triangles.
	
	IECore::V3fVectorDataPtr pData = new IECore::V3fVectorData;
	IECore::V3fVectorDataPtr nData = new IECore::V3fVectorData;
	IECore::V2fVectorDataPtr stData = new IECore::V2fVectorData;
	
	vector<V3f> &pVector = pData->writable();
	vector<V3f> &nVector = nData->writable();
	vector<V2f> &stVector = stData->writable();

	// centre point
	pVector.push_back( V3f( 0.0f, 0.0f, m_z ) );
	nVector.push_back( V3f( 0.0f, 0.0f, 1.0f ) );
	stVector.push_back( V2f( 0.5f, 0.5f ) );

	const unsigned int n = 20;
	float thetaMaxRadians = m_thetaMax/180.0f * M_PI;
	for( unsigned int i=0; i<n; i++ )
	{
		float t = thetaMaxRadians * i/(n-1);
		float x = Math<float>::cos( t );
		float y = Math<float>::sin( t );
		pVector.push_back( V3f( m_radius * x, m_radius * y, m_z ) );
		nVector.push_back( V3f( 0.0f, 0.0f, 1.0f ) );
		stVector.push_back( V2f( x/2.0f + 0.5f, y/2.0f + 0.5f ) );
	}

	m_nPoints = n + 1;
	
	addVertexAttribute( "P", pData );
	addVertexAttribute( "N", nData );
	addVertexAttribute( "st", stData );
}

DiskPrimitive::~DiskPrimitive()
{
}

void DiskPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
	switch( primVar.interpolation )
	{
		case IECore::PrimitiveVariable::Constant :
		case IECore::PrimitiveVariable::Uniform :
			addUniformAttribute( name, primVar.data );
			break;
		default :
			IECore::msg( IECore::Msg::Warning, "DiskPrimitive::addPrimitiveVariable", boost::format( "Primitive variable \"%s\" has unsupported interpolation." ) % name );
	}
}

void DiskPrimitive::renderInstances( size_t numInstances ) const
{
	glDrawArraysInstancedARB( GL_TRIANGLE_FAN, 0, m_nPoints, numInstances );
}

Imath::Box3f DiskPrimitive::bound() const
{
	return Imath::Box3f( V3f( -m_radius, -m_radius, 0 ), V3f( m_radius, m_radius, 0 ) );
}
