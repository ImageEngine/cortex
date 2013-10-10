//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_GEOMETRICTYPEDDATA_INL
#define IECORE_GEOMETRICTYPEDDATA_INL

namespace IECore
{

template<class T>
Object::TypeDescription<GeometricTypedData<T> > GeometricTypedData<T>::m_typeDescription;

template<class T>
GeometricTypedData<T>::GeometricTypedData()
	: TypedData<T>(), m_interpretation( GeometricData::Numeric )
{
}

template<class T>
GeometricTypedData<T>::GeometricTypedData( const ValueType &data )
	: TypedData<T>( data ), m_interpretation( GeometricData::Numeric )
{
}

template<class T>
GeometricTypedData<T>::GeometricTypedData( const ValueType &data, GeometricData::Interpretation interpretation )
	: TypedData<T>( data ), m_interpretation( interpretation )
{
}

template<class T>
GeometricTypedData<T>::~GeometricTypedData()
{
}

template<class T>
GeometricData::Interpretation GeometricTypedData<T>::getInterpretation() const
{
	return m_interpretation;
}

template<class T>
void GeometricTypedData<T>::setInterpretation( GeometricData::Interpretation interpretation )
{
	m_interpretation = interpretation;
}

//////////////////////////////////////////////////////////////////////////////////////
// object interface
//////////////////////////////////////////////////////////////////////////////////////

template <class T>
typename GeometricTypedData<T>::Ptr GeometricTypedData<T>::copy() const
{
	return staticPointerCast<GeometricTypedData<T> >( TypedData<T>::copy() );
}

template <class T>
void GeometricTypedData<T>::copyFrom( const Object *other, Object::CopyContext *context )
{
	TypedData<T>::copyFrom( other, context );
	const GeometricTypedData<T> *tOther = static_cast<const GeometricTypedData<T> *>( other );
	m_interpretation = tOther->m_interpretation;
}

template <class T>
void GeometricTypedData<T>::save( Object::SaveContext *context ) const
{
	static InternedString interpretationEntry("interpretation");
	TypedData<T>::save( context );
	IndexedIO *container = context->rawContainer();
	container->write( interpretationEntry, (unsigned)m_interpretation );
}

template <class T>
void GeometricTypedData<T>::load( Object::LoadContextPtr context )
{
	static InternedString interpretationEntry("interpretation");
	TypedData<T>::load( context );
	const IndexedIO *container = context->rawContainer();
	// test for new format
	if ( container->hasEntry(interpretationEntry) )
	{
		unsigned tmp;
		container->read( interpretationEntry, tmp );
		m_interpretation = (GeometricData::Interpretation)tmp;
	}
}

template <class T>
bool GeometricTypedData<T>::isEqualTo( const Object *other ) const
{
	if( !TypedData<T>::isEqualTo( other ) )
	{
		return false;
	}
	const GeometricTypedData<T> *tOther = static_cast<const GeometricTypedData<T> *>( other );
	return m_interpretation == tOther->m_interpretation;
}

template <class T>
void GeometricTypedData<T>::hash( MurmurHash &h ) const
{
	TypedData<T>::hash( h );
	h.append( m_interpretation );
}

template <class T>
void GeometricTypedData<T>::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	TypedData<T>::memoryUsage( accumulator );
	accumulator.accumulate( sizeof( m_interpretation ) );
}

} // namespace IECore

#endif // IECORE_GEOMETRICTYPEDDATA_INL
