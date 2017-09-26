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

#include "IECoreGL/QuadPrimitive.h"
#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/Buffer.h"
#include "IECoreGL/GL.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( QuadPrimitive );

QuadPrimitive::QuadPrimitive( float width, float height )
	:	m_width( width ), m_height( height )
{
	IECore::V3fVectorDataPtr pData = new IECore::V3fVectorData;
	IECore::V3fVectorDataPtr nData = new IECore::V3fVectorData;
	IECore::V2fVectorDataPtr uvData = new IECore::V2fVectorData;
	m_vertIds = new IECore::UIntVectorData;
	
	vector<V3f> &pVector = pData->writable();
	vector<V3f> &nVector = nData->writable();
	vector<V2f> &uvVector = uvData->writable();
	vector<unsigned int> &vertIdsVector = m_vertIds->writable();
	
	nVector.push_back( V3f( 0, 0, 1 ) );
	nVector.push_back( V3f( 0, 0, 1 ) );
	nVector.push_back( V3f( 0, 0, 1 ) );
	nVector.push_back( V3f( 0, 0, 1 ) );

	pVector.push_back( V3f( -m_width/2.0f, -m_height/2.0f, 0 ) );
	pVector.push_back( V3f( m_width/2.0f, -m_height/2.0f, 0 ) );
	pVector.push_back( V3f( m_width/2.0f, m_height/2.0f, 0 ) );
	pVector.push_back( V3f( -m_width/2.0f, m_height/2.0f, 0 ) );

	uvVector.push_back( V2f( 0.0f, 0.0f ) );
	uvVector.push_back( V2f( 1.0f, 0.0f ) );
	uvVector.push_back( V2f( 1.0f, 1.0f ) );
	uvVector.push_back( V2f( 0.0f, 1.0f ) );
	
	vertIdsVector.push_back( 0 );
	vertIdsVector.push_back( 1 );
	vertIdsVector.push_back( 2 );
	
	vertIdsVector.push_back( 0 );
	vertIdsVector.push_back( 2 );
	vertIdsVector.push_back( 3 );
	
	addVertexAttribute( "P", pData );
	addVertexAttribute( "N", nData );
	addVertexAttribute( "uv", uvData );
}

QuadPrimitive::~QuadPrimitive()
{	
}

void QuadPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
	if ( primVar.interpolation==IECore::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.data );
	}
}

void QuadPrimitive::renderInstances( size_t numInstances ) const
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

Imath::Box3f QuadPrimitive::bound() const
{
	return Imath::Box3f( V3f( -m_width/2, -m_height/2, 0 ), V3f( m_width/2, m_height/2, 0 ) );
}
