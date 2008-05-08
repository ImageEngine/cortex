//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

#ifndef IE_CORE_OBJECT_INL
#define IE_CORE_OBJECT_INL

#include "IECore/Exception.h"

namespace IECore
{

template<class T>
boost::intrusive_ptr<T> Object::CopyContext::copy( boost::intrusive_ptr<const T> toCopy )
{
	std::map<ConstObjectPtr, ObjectPtr>::const_iterator it = m_copies.find( toCopy );
	if( it!=m_copies.end() )
	{
		return boost::static_pointer_cast<T>( it->second );
	}
	ObjectPtr copy = create( toCopy->typeId() );
	copy->copyFrom( toCopy, this );
	m_copies.insert( std::pair<ConstObjectPtr, ObjectPtr>( toCopy, copy ) );
	return boost::static_pointer_cast<T>( copy );
}

template<class T>
Object::TypeDescription<T>::TypeDescription()
{
	Object::registerType( T::staticTypeId(), T::staticTypeName(), creator );
}

template<class T>
Object::TypeDescription<T>::TypeDescription( TypeId alternateTypeId, const std::string &alternateTypeName )
{
	Object::registerType( alternateTypeId, alternateTypeName, creator );
}


template<class T>
ObjectPtr Object::TypeDescription<T>::creator()
{
	return new T;
}

template<class T>
Object::AbstractTypeDescription<T>::AbstractTypeDescription()
{
	Object::registerType( T::staticTypeId(), T::staticTypeName(), 0 );
}

template<class T>
typename T::Ptr Object::LoadContext::load( IndexedIOInterfacePtr i, const IndexedIO::EntryID &name )
{
	return runTimeCast<T>( loadObjectOrReference( i, name ) );
}

} // namespace IECore

#endif // IE_CORE_OBJECT_INL
