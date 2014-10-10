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
#include "OpenEXR/ImathFun.h"

#include "IECore/MessageHandler.h"

#include "IECoreGL/SpherePrimitive.h"
#include "IECoreGL/Buffer.h"
#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/GL.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( SpherePrimitive );

SpherePrimitive::SpherePrimitive( float radius, float zMin, float zMax, float thetaMax )
	:	m_radius( radius ), m_zMin( zMin ), m_zMax( zMax ), m_thetaMax( thetaMax ), m_vertIdsBuffer( 0 )
{
	// figure out bounding box
	
	thetaMax = m_thetaMax/180.0f * M_PI;
	float minX = m_radius * ( thetaMax < M_PI ? Math<float>::cos( thetaMax ) : -1.0f );
	float maxY = m_radius * ( thetaMax < M_PI/2 ? Math<float>::sin( thetaMax ) : 1.0f );
	float minY = m_radius * ( thetaMax > 3 * M_PI/2 ? -1.0f : min( 0.0f, Math<float>::sin( thetaMax ) ) );
	m_bound = Imath::Box3f( V3f( minX, minY, m_zMin * m_radius ), V3f( m_radius, maxY, m_zMax * m_radius ) );
	
	// build vertex attributes for P, N and st, and indexes for triangles.
	
	IECore::V3fVectorDataPtr pData = new IECore::V3fVectorData;
	IECore::V3fVectorDataPtr nData = new IECore::V3fVectorData;
	IECore::V2fVectorDataPtr stData = new IECore::V2fVectorData;
	m_vertIds = new IECore::UIntVectorData;
	
	vector<V3f> &pVector = pData->writable();
	vector<V3f> &nVector = nData->writable();
	vector<V2f> &stVector = stData->writable();
	vector<unsigned int> &vertIdsVector = m_vertIds->writable();
	
	float oMin = Math<float>::asin( m_zMin );
	float oMax = Math<float>::asin( m_zMax );
	const unsigned int nO = max( 4u, (unsigned int)( 20.0f * (oMax - oMin) / M_PI ) );

	thetaMax = m_thetaMax/180.0f * M_PI;
	const unsigned int nT = max( 7u, (unsigned int)( 40.0f * thetaMax / (M_PI*2) ) );

	for( unsigned int i=0; i<nO; i++ )
	{
		float v = (float)i/(float)(nO-1);
		float o = lerp( oMin, oMax, v );
		float z = m_radius * Math<float>::sin( o );
		float r = m_radius * Math<float>::cos( o );

		for( unsigned int j=0; j<nT; j++ )
		{
			float u = (float)j/(float)(nT-1);
			float theta = thetaMax * u;
			V3f p( r * Math<float>::cos( theta ), r * Math<float>::sin( theta ), z );
			stVector.push_back( V2f( u, v ) );
			pVector.push_back( p );
			nVector.push_back( p );
			if( i < nO - 1 && j < nT - 1 )
			{
				unsigned int i0 = i * nT + j;
				unsigned int i1 = i0 + 1;
				unsigned int i2 = i0 + nT;
				unsigned int i3 = i2 + 1;
				vertIdsVector.push_back( i0 );
				vertIdsVector.push_back( i1 );
				vertIdsVector.push_back( i2 );
				vertIdsVector.push_back( i1 );
				vertIdsVector.push_back( i3 );
				vertIdsVector.push_back( i2 );
			}
		}
	}
	
	addVertexAttribute( "P", pData );
	addVertexAttribute( "N", nData );
	addVertexAttribute( "st", stData );
}

SpherePrimitive::~SpherePrimitive()
{

}

void SpherePrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
	switch( primVar.interpolation )
	{
		case IECore::PrimitiveVariable::Constant :
		case IECore::PrimitiveVariable::Uniform :
			addUniformAttribute( name, primVar.data );
			break;
		default :
			IECore::msg( IECore::Msg::Warning, "SpherePrimitive::addPrimitiveVariable", boost::format( "Primitive variable \"%s\" has unsupported interpolation." ) % name );
	}
}

void SpherePrimitive::renderInstances( size_t numInstances ) const
{
	if( !m_vertIdsBuffer )
	{
		// we don't build the actual buffer until now, because in the constructor we're not guaranteed
		// a valid GL context.
		CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();
		m_vertIdsBuffer = IECore::runTimeCast<const Buffer>( cachedConverter->convert( m_vertIds.get() ) );
	}
	
	Buffer::ScopedBinding indexBinding( *m_vertIdsBuffer, GL_ELEMENT_ARRAY_BUFFER );
	glDrawElementsInstancedARB( GL_TRIANGLES, m_vertIds->readable().size(), GL_UNSIGNED_INT, 0, numInstances );
}

Imath::Box3f SpherePrimitive::bound() const
{
	return m_bound;
}
