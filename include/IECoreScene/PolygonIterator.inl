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

#ifndef IECORESCENE_POLYGONITERATOR_INL
#define IECORESCENE_POLYGONITERATOR_INL

namespace IECoreScene
{

inline PolygonIterator::PolygonIterator()
{
}

inline PolygonIterator::PolygonIterator( NumVerticesIterator numVerticesIterator, VertexIndexIterator vertexIndexIterator, int faceVaryingOffset )
	:	m_numVerticesIterator( numVerticesIterator ), m_vertexIndexIterator( vertexIndexIterator ), m_faceVaryingOffset( faceVaryingOffset )
{
}

inline PolygonIterator &PolygonIterator::operator++()
{
	m_vertexIndexIterator += *m_numVerticesIterator;
	m_faceVaryingOffset += *m_numVerticesIterator++;
	return *this;
}

inline PolygonIterator PolygonIterator::operator++( int )
{
	PolygonIterator r( *this );
	++(*this);
	return r;
}

inline bool PolygonIterator::operator==( const PolygonIterator &rhs ) const
{
	return m_vertexIndexIterator==rhs.m_vertexIndexIterator && m_numVerticesIterator==rhs.m_numVerticesIterator && m_faceVaryingOffset==rhs.m_faceVaryingOffset;
}

inline bool PolygonIterator::operator!=( const PolygonIterator &rhs ) const
{
	return ! operator==( rhs );
}

inline PolygonIterator &PolygonIterator::operator=( const PolygonIterator &rhs )
{
	m_vertexIndexIterator = rhs.m_vertexIndexIterator;
	m_numVerticesIterator = rhs.m_numVerticesIterator;
	m_faceVaryingOffset = rhs.m_faceVaryingOffset;
	return *this;
}

template<typename ValueIterator>
PolygonVertexIterator<ValueIterator> PolygonIterator::vertexBegin( ValueIterator valuesBegin ) const
{
	return PolygonVertexIterator<ValueIterator>( m_vertexIndexIterator, valuesBegin );
}

template<typename ValueIterator>
PolygonVertexIterator<ValueIterator> PolygonIterator::vertexEnd( ValueIterator valuesBegin ) const
{
	return PolygonVertexIterator<ValueIterator>( m_vertexIndexIterator + *m_numVerticesIterator, valuesBegin );
}

template<typename ValueIterator>
ValueIterator PolygonIterator::faceVaryingBegin( ValueIterator valuesBegin ) const
{
	return valuesBegin + m_faceVaryingOffset;
}

template<typename ValueIterator>
ValueIterator PolygonIterator::faceVaryingEnd( ValueIterator valuesBegin ) const
{
	return valuesBegin + m_faceVaryingOffset + *m_numVerticesIterator;
}

} // namespace IECoreScene


#endif // IECORESCENE_POLYGONITERATOR_INL
