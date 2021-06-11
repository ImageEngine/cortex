//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include <type_traits>

namespace IECore
{

namespace Detail
{

template<typename T>
Object::CreatorFn objectCreator( typename std::enable_if<!std::is_abstract<T>::value>::type *enabler = nullptr )
{
	// Object is not abstract - return a lambda
	// that will create a new instance.
	return [](){ return new T; };
}

template<typename T>
Object::CreatorFn objectCreator( typename std::enable_if<std::is_abstract<T>::value>::type *enabler = nullptr )
{
	// Object is abstract - return a null
	// CreatorFn.
	return Object::CreatorFn();
}

} // namespace Detail

template<class T>
typename T::Ptr Object::CopyContext::copy( const T *toCopy )
{
	return boost::static_pointer_cast<T>( copyInternal( toCopy ) );
}

template<class T>
Object::TypeDescription<T>::TypeDescription() : RunTimeTyped::TypeDescription<T>()
{
	Object::registerType( T::staticTypeId(), T::staticTypeName(), Detail::objectCreator<T>() );
}

template<class T>
Object::TypeDescription<T>::TypeDescription( TypeId alternateTypeId, const std::string &alternateTypeName ) : RunTimeTyped::TypeDescription<T>()
{
	Object::registerType( alternateTypeId, alternateTypeName, Detail::objectCreator<T> );
}

template<class T>
typename T::Ptr Object::LoadContext::load( const IndexedIO *i, const IndexedIO::EntryID &name )
{
	return runTimeCast<T>( loadObjectOrReference( i, name ) );
}

inline const Canceller *Object::LoadContext::canceller()
{
	return m_canceller;
}

} // namespace IECore

#endif // IE_CORE_OBJECT_INL
