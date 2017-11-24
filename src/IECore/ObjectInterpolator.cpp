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

#include <unordered_map>

#include "IECore/Object.h"
#include "IECore/Interpolator.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/CompoundData.h"
#include "IECore/CompoundObject.h"
#include "IECore/DespatchTypedData.h"

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

} // namespace

//////////////////////////////////////////////////////////////////////////
// Public bits
//////////////////////////////////////////////////////////////////////////

namespace IECore
{

struct LinearInterpolator< Object >::Adaptor
{
	typedef ObjectPtr ReturnType;

	const Data *m_y0;
	const Data *m_y1;
	double m_x;

	Adaptor( const Object *y0, const Object *y1, double x ) : m_x( x )
	{
		m_y0 = assertedStaticCast< const Data >( y0 );
		m_y1 = assertedStaticCast< const Data >( y1 );
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr result )
	{
		const T *y0 = assertedStaticCast< const T>( m_y0 );
		const T *y1 = assertedStaticCast< const T>( m_y1 );

		LinearInterpolator<T>()( y0, y1, m_x, result );

		return result;
	};
};

void LinearInterpolator< Object >::operator()( const Object *y0, const Object *y1, double x, ObjectPtr &result ) const
{
	if ( y0->typeId() != y1->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}

	if ( y0->isInstanceOf( CompoundDataTypeId ) )
	{
		const CompoundData *x0 = assertedStaticCast<const CompoundData>( y0 );
		const CompoundData *x1 = assertedStaticCast<const CompoundData>( y1 );
		CompoundDataPtr xRes = assertedStaticCast<CompoundData>( result );
		for ( CompoundDataMap::const_iterator it0 = x0->readable().begin(); it0 != x0->readable().end(); it0++ )
		{
			CompoundDataMap::const_iterator it1 = x1->readable().find( it0->first );
			if ( it1 != x1->readable().end() && it0->second->typeId() == it1->second->typeId() )
			{
				ObjectPtr resultObj = Object::create( it0->second->typeId() );

				LinearInterpolator< Object >()( it0->second.get(), it1->second.get(), x, resultObj );

				if ( resultObj )
				{
					xRes->writable()[ it0->first ] = assertedStaticCast<Data>( resultObj );
				}
				else
				{
					xRes->writable()[ it0->first ] = it0->second;
				}
			}
		}
	}
	else if ( y0->isInstanceOf( CompoundObjectTypeId ) )
	{
		const CompoundObject *x0 = assertedStaticCast<const CompoundObject>( y0 );
		const CompoundObject *x1 = assertedStaticCast<const CompoundObject>( y1 );
		CompoundObjectPtr xRes = assertedStaticCast<CompoundObject>( result );
		for ( CompoundObject::ObjectMap::const_iterator it0 = x0->members().begin(); it0 != x0->members().end(); it0++ )
		{
			CompoundObject::ObjectMap::const_iterator it1 = x1->members().find( it0->first );
			if ( it1 != x1->members().end() && it0->second->typeId() == it1->second->typeId() )
			{
				ObjectPtr resultObj = Object::create( it0->second->typeId() );
				LinearInterpolator< Object >()( it0->second.get(), it1->second.get(), x, resultObj );

				if ( resultObj )
				{
					xRes->members()[ it0->first ] = resultObj;
				}
				else
				{
					xRes->members()[ it0->first ] = it0->second;
				}
			}
		}
	}
	else if ( result->isInstanceOf( DataTypeId ) )
	{
		DataPtr data = runTimeCast< Data >( result );
		Adaptor adaptor( y0, y1, x );

		result = despatchTypedData<
		         Adaptor,
		         TypeTraits::IsStrictlyInterpolable,
		         DespatchTypedDataIgnoreError
		         >( data.get(), adaptor );
	}
	else
	{
		const Registry &r = registry();

		ObjectInterpolator interpolator = nullptr;
		TypeId typeId = y0->typeId();
		while( typeId != InvalidTypeId )
		{
			Registry::const_iterator it = r.find( typeId );
			if( it != r.end() )
			{
				interpolator = it->second;
				break;
			}
			typeId = RunTimeTyped::baseTypeId( typeId );
		}
		if( interpolator )
		{
			result = interpolator( y0, y1, x );
		}
		else
		{
			result = nullptr;
		}
	}
}

ObjectPtr linearObjectInterpolation( const Object *y0, const Object *y1, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	LinearInterpolator<Object>()( y0, y1, x, result );
	return result;
}

void registerInterpolator( IECore::TypeId objectType, ObjectInterpolator interpolator )
{
	registry()[objectType] = interpolator;
}


}
