//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/ObjectInterpolator.h"

#include "IECore/CompoundData.h"
#include "IECore/CompoundObject.h"
#include "IECore/DataAlgo.h"
#include "IECore/Interpolator.h"
#include "IECore/Object.h"
#include "IECore/TypeTraits.h"

#include <unordered_map>

using namespace IECore;

//////////////////////////////////////////////////////////////////////////
// Internals
//////////////////////////////////////////////////////////////////////////

namespace std
{

// Specialise hash so we can use TypeId as a key
// in unordered_map. When we move to C++14 this should
// no longer be necessary :
//
// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148
template<>
struct hash<IECore::TypeId>
{
	size_t operator()( IECore::TypeId v ) const
	{
		return hash<int>()( v );
	}
};

} // namespace std

namespace
{

typedef std::unordered_map<IECore::TypeId, ObjectInterpolator> Registry;

Registry &registry()
{
	static Registry g_registry;
	return g_registry;
}

struct DataInterpolator
{

	DataInterpolator( const Data *y1, double x )
		:	m_y1( y1 ), m_x( x )
	{
	}

	template<typename T>
	DataPtr operator()( const T *y0, typename std::enable_if<TypeTraits::IsStrictlyInterpolable<T>::value>::type *enabler = nullptr ) const
	{
		typename T::Ptr result = new T;
		LinearInterpolator<T>()(
			y0,
			static_cast<const T *>( m_y1 ),
			m_x,
			result
		);
		return result;
	}

	DataPtr operator()( const Data *y0 ) const
	{
		return nullptr;
	}

	private :

		const Data *m_y1;
		double m_x;

};

DataPtr interpolateData( const Data *y0, const Data *y1, double x )
{
	return dispatch( y0, DataInterpolator( y1, x ) );
}

CompoundDataPtr interpolateCompoundData( const CompoundData *y0, const CompoundData *y1, double x )
{
	CompoundDataPtr result = y0->copy();
	CompoundDataMap &resultWritable = result->writable();
	const CompoundDataMap y1Readable = y1->readable();
	for( const auto &member : y0->readable() )
	{
		CompoundDataMap::const_iterator it = y1Readable.find( member.first );
		if( it != y1Readable.end() && it->second->typeId() == member.second->typeId() )
		{
			DataPtr interpolatedMember = boost::static_pointer_cast<Data>(
				linearObjectInterpolation( member.second.get(), it->second.get(), x )
			);
			if( interpolatedMember )
			{
				resultWritable[member.first] = interpolatedMember;
			}
		}
	}
	return result;
}

CompoundObjectPtr interpolateCompoundObject( const CompoundObject *y0, const CompoundObject *y1, double x )
{
	CompoundObjectPtr result = y0->copy();
	CompoundObject::ObjectMap &resultWritable = result->members();
	const CompoundObject::ObjectMap y1Readable = y1->members();
	for( const auto &member : y0->members() )
	{
		CompoundObject::ObjectMap::const_iterator it = y1Readable.find( member.first );
		if( it != y1Readable.end() && it->second->typeId() == member.second->typeId() )
		{
			ObjectPtr interpolatedMember = linearObjectInterpolation( member.second.get(), it->second.get(), x );
			if( interpolatedMember )
			{
				resultWritable[member.first] = interpolatedMember;
			}
		}
	}
	return result;
}

IECore::InterpolatorDescription<IECore::Data> g_dataDescription( interpolateData );
IECore::InterpolatorDescription<IECore::CompoundData> g_compoundDataDescription( interpolateCompoundData );
IECore::InterpolatorDescription<IECore::CompoundObject> g_compoundObjectDescription( interpolateCompoundObject );

} // namespace

//////////////////////////////////////////////////////////////////////////
// Public bits
//////////////////////////////////////////////////////////////////////////

namespace IECore
{

ObjectPtr linearObjectInterpolation( const Object *y0, const Object *y1, double x )
{
	if( y0->typeId() != y1->typeId() )
	{
		throw( Exception( "Object types don't match" ) );
	}

	const Registry &r = registry();

	TypeId typeId = y0->typeId();
	while( typeId != InvalidTypeId )
	{
		Registry::const_iterator it = r.find( typeId );
		if( it != r.end() )
		{
			return it->second( y0, y1, x );
		}
		typeId = RunTimeTyped::baseTypeId( typeId );
	}

	return nullptr;
}

void registerInterpolator( IECore::TypeId objectType, ObjectInterpolator interpolator )
{
	registry()[objectType] = interpolator;
}

} // namespace IECore
