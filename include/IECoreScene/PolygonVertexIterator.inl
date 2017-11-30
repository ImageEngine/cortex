//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_POLYGONVERTEXITERATOR_INL
#define IECORESCENE_POLYGONVERTEXITERATOR_INL

namespace IECoreScene
{

template<typename VertexValueIterator, typename VertexIndexIterator>
PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::PolygonVertexIterator()
{
}

template<typename VertexValueIterator, typename VertexIndexIterator>
PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::PolygonVertexIterator( VertexIndexIterator vertexIndexIterator, VertexValueIterator vertexValuesBegin )
	:	m_vertexValuesBegin( vertexValuesBegin ), m_vertexIndexIterator( vertexIndexIterator )
{
}

template<typename VertexValueIterator, typename VertexIndexIterator>
PolygonVertexIterator<VertexValueIterator, VertexIndexIterator> &PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::operator++()
{
	m_vertexIndexIterator++;
	return *this;
}

template<typename VertexValueIterator, typename VertexIndexIterator>
PolygonVertexIterator<VertexValueIterator, VertexIndexIterator> PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::operator++( int )
{
	PolygonVertexIterator r( *this );
	++(*this);
	return r;
}

template<typename VertexValueIterator, typename VertexIndexIterator>
typename PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::reference PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::operator*() const
{
	return *( m_vertexValuesBegin + *m_vertexIndexIterator );
}

template<typename VertexValueIterator, typename VertexIndexIterator>
typename PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::pointer PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::operator->() const
{
	return &( operator*() );
}

template<typename VertexValueIterator, typename VertexIndexIterator>
bool PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::operator==( const PolygonVertexIterator &rhs ) const
{
	return m_vertexIndexIterator==rhs.m_vertexIndexIterator && m_vertexValuesBegin==rhs.m_vertexValuesBegin;
}

template<typename VertexValueIterator, typename VertexIndexIterator>
bool PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::operator!=( const PolygonVertexIterator &rhs ) const
{
	return ! operator==( rhs );
}

template<typename VertexValueIterator, typename VertexIndexIterator>
PolygonVertexIterator<VertexValueIterator, VertexIndexIterator> &PolygonVertexIterator<VertexValueIterator, VertexIndexIterator>::operator=( const PolygonVertexIterator &rhs )
{
	m_vertexValuesBegin = rhs.m_vertexValuesBegin;
	m_vertexIndexIterator = rhs.m_vertexIndexIterator;
	return *this;
}

} // namespace IECoreScene

#endif // IECORESCENE_POLYGONVERTEXITERATOR_INL
