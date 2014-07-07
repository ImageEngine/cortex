//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_COMPOUNDDATA_INL
#define IECORE_COMPOUNDDATA_INL

#include "boost/format.hpp"

#include "IECore/Exception.h"

namespace IECore
{

template<typename T>
T *CompoundData::member( const InternedString &name, bool throwExceptions )
{
	return member<T>( name, throwExceptions, false );
}


template<typename T>
const T *CompoundData::member( const InternedString &name, bool throwExceptions ) const
{
	CompoundDataMap::const_iterator it = readable().find( name );
	if( it!=readable().end() )
	{
		const T *result = runTimeCast<const T>( it->second.get() );
		if( result )
		{
			return result;
		}
		else
		{
			if( throwExceptions )
			{
				throw Exception( boost::str( boost::format( "CompoundData child \"%s\" is not of type \"%s\"." ) % name.value() % T::staticTypeName() ) );
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
		if( throwExceptions )
		{
			throw Exception( boost::str( boost::format( "CompoundData has no child named \"%s\"." ) % name.value() ) );
		}
		else
		{
			return 0;
		}
	}
	return 0; // shouldn't get here anyway
}

template<typename T>
T *CompoundData::member( const InternedString &name, bool throwExceptions, bool createIfMissing )
{
	CompoundDataMap::const_iterator it = readable().find( name );
	if( it!=readable().end() )
	{
		T *result = runTimeCast<T>( it->second.get() );
		if( result )
		{
			return result;
		}
		else
		{
			if( throwExceptions )
			{
				throw Exception( boost::str( boost::format( "CompoundData child \"%s\" is not of type \"%s\"." ) % name.value() % T::staticTypeName() ) );
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
		if( createIfMissing )
		{
			typename T::Ptr member = staticPointerCast<T>( Object::create( T::staticTypeId() ) );
			writable()[name] = member;
			return member.get();
		}
		else if( throwExceptions )
		{
			throw Exception( boost::str( boost::format( "CompoundData has no child named \"%s\"." ) % name.value() ) );
		}
		else
		{
			return 0;
		}
	}
	return 0; // shouldn't get here anyway
}



}; // namespace IECore

#endif // IECORE_COMPOUNDDATA_INL
