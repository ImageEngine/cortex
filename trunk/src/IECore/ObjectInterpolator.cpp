//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Object.h"
#include "IECore/Interpolator.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/CompoundData.h"
#include "IECore/CompoundObject.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;

namespace IECore
{

struct LinearInterpolator< Object >::Adaptor
{
	typedef ObjectPtr ReturnType;

	DataPtr m_y0, m_y1;
	double m_x;

	Adaptor( ObjectPtr y0, ObjectPtr y1, double x ) : m_x( x )
	{
		m_y0 = assertedStaticCast< Data >( y0 );
		m_y1 = assertedStaticCast< Data >( y1 );
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr result )
	{
		typename T::Ptr y0 = assertedStaticCast<T>( m_y0 );
		typename T::Ptr y1 = assertedStaticCast<T>( m_y1 );

		LinearInterpolator<T>()( y0, y1, m_x, result );

		return result;
	};
};

void LinearInterpolator< Object >::operator()( const ObjectPtr &y0, const ObjectPtr &y1, double x, ObjectPtr &result ) const
{
	if ( y0->typeId() != y1->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}

	if ( y0->isInstanceOf( CompoundDataTypeId ) )
	{
		CompoundDataPtr x0 = assertedStaticCast<CompoundData>( y0 );
		CompoundDataPtr x1 = assertedStaticCast<CompoundData>( y1 );
		CompoundDataPtr xRes = assertedStaticCast<CompoundData>( result );
		for ( CompoundDataMap::const_iterator it0 = x0->readable().begin(); it0 != x0->readable().end(); it0++ )
		{
			CompoundDataMap::const_iterator it1 = x1->readable().find( it0->first );
			if ( it1 != x1->readable().end() && it0->second->typeId() == it1->second->typeId() )
			{
				ObjectPtr resultObj = Object::create( it0->second->typeId() );

				LinearInterpolator< Object >()( it0->second, it1->second, x, resultObj );

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
		CompoundObjectPtr x0 = assertedStaticCast<CompoundObject>( y0 );
		CompoundObjectPtr x1 = assertedStaticCast<CompoundObject>( y1 );
		CompoundObjectPtr xRes = assertedStaticCast<CompoundObject>( result );
		for ( CompoundObject::ObjectMap::const_iterator it0 = x0->members().begin(); it0 != x0->members().end(); it0++ )
		{
			CompoundObject::ObjectMap::const_iterator it1 = x1->members().find( it0->first );
			if ( it1 != x1->members().end() && it0->second->typeId() == it1->second->typeId() )
			{
				ObjectPtr resultObj = Object::create( it0->second->typeId() );
				LinearInterpolator< Object >()( it0->second, it1->second, x, resultObj );

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
	else
	{
		DataPtr data = runTimeCast< Data >( result );
		Adaptor adaptor( y0, y1, x );

		result = despatchTypedData<
		         Adaptor,
		         TypeTraits::IsStrictlyInterpolable,
		         DespatchTypedDataIgnoreError
		         >( data, adaptor );
	}
}

struct CubicInterpolator< Object >::Adaptor
{
	typedef ObjectPtr ReturnType;

	DataPtr m_y0, m_y1, m_y2, m_y3;
	double m_x;

	Adaptor( ObjectPtr y0, ObjectPtr y1, ObjectPtr y2, ObjectPtr y3, double x ) : m_x( x )
	{
		m_y0 = assertedStaticCast< Data >( y0 );
		m_y1 = assertedStaticCast< Data >( y1 );
		m_y2 = assertedStaticCast< Data >( y2 );
		m_y3 = assertedStaticCast< Data >( y3 );
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr result )
	{
		typename T::Ptr y0 = assertedStaticCast<T>( m_y0 );
		typename T::Ptr y1 = assertedStaticCast<T>( m_y1 );
		typename T::Ptr y2 = assertedStaticCast<T>( m_y2 );
		typename T::Ptr y3 = assertedStaticCast<T>( m_y3 );

		CubicInterpolator<T>()( y0, y1, y2, y3, m_x, result );

		return result;
	}
};


void CubicInterpolator<Object>::operator()( const ObjectPtr &y0, const ObjectPtr &y1, const ObjectPtr &y2, const ObjectPtr &y3, double x, ObjectPtr &result ) const
{
	if ( y0->typeId() != y1->typeId() || y0->typeId() != y2->typeId() || y0->typeId() != y3->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}

	if ( y0->isInstanceOf( CompoundDataTypeId ) )
	{
		CompoundDataPtr x0 = assertedStaticCast<CompoundData>( y0 );
		CompoundDataPtr x1 = assertedStaticCast<CompoundData>( y1 );
		CompoundDataPtr x2 = assertedStaticCast<CompoundData>( y2 );
		CompoundDataPtr x3 = assertedStaticCast<CompoundData>( y3 );
		CompoundDataPtr xRes = assertedStaticCast<CompoundData>( result );
		for ( CompoundDataMap::const_iterator it1 = x1->readable().begin(); it1 != x1->readable().end(); it1++ )
		{
			CompoundDataMap::const_iterator it0 = x0->readable().find( it1->first );
			if ( it0 != x0->readable().end() && it0->second->typeId() == it1->second->typeId() )
			{
				CompoundDataMap::const_iterator it2 = x2->readable().find( it1->first );
				if ( it2 != x2->readable().end() && it0->second->typeId() == it2->second->typeId() )
				{
					CompoundDataMap::const_iterator it3 = x3->readable().find( it1->first );
					if ( it3 != x3->readable().end() && it0->second->typeId() == it3->second->typeId() )
					{
						ObjectPtr resultObj = Object::create( it1->second->typeId() );
						CubicInterpolator< Object >()( it0->second, it1->second, it2->second, it3->second, x, resultObj );
						if ( resultObj )
						{
							xRes->writable()[ it1->first ] = assertedStaticCast<Data>( resultObj );
						}
						else
						{
							xRes->writable()[ it1->first ] = it1->second;
						}
					}
					else
					{
						xRes->writable()[ it1->first ] = it1->second;
					}
				}
				else
				{
					xRes->writable()[ it1->first ] = it1->second;
				}
			}
			else
			{
				xRes->writable()[ it1->first ] = it1->second;
			}
		}
	}
	else if ( y0->isInstanceOf( CompoundObjectTypeId ) )
	{
		CompoundObjectPtr x0 = assertedStaticCast<CompoundObject>( y0 );
		CompoundObjectPtr x1 = assertedStaticCast<CompoundObject>( y1 );
		CompoundObjectPtr x2 = assertedStaticCast<CompoundObject>( y2 );
		CompoundObjectPtr x3 = assertedStaticCast<CompoundObject>( y3 );
		CompoundObjectPtr xRes = assertedStaticCast<CompoundObject>( result );
		for ( CompoundObject::ObjectMap::const_iterator it1 = x1->members().begin(); it1 != x1->members().end(); it1++ )
		{
			CompoundObject::ObjectMap::const_iterator it0 = x0->members().find( it1->first );
			if ( it0 != x0->members().end() && it0->second->typeId() == it1->second->typeId() )
			{
				CompoundObject::ObjectMap::const_iterator it2 = x2->members().find( it1->first );
				if ( it2 != x2->members().end() && it0->second->typeId() == it2->second->typeId() )
				{
					CompoundObject::ObjectMap::const_iterator it3 = x3->members().find( it1->first );
					if ( it3 != x3->members().end() && it0->second->typeId() == it3->second->typeId() )
					{
						ObjectPtr resultObj = Object::create( it1->second->typeId() );
						CubicInterpolator< Object >()( it0->second, it1->second, it2->second, it3->second, x, resultObj );

						if ( resultObj )
						{
							xRes->members()[ it1->first ] = resultObj;
						}
						else
						{
							xRes->members()[ it1->first ] = it1->second;
						}
					}
					else
					{
						xRes->members()[ it1->first ] = it1->second;
					}
				}
				else
				{
					xRes->members()[ it1->first ] = it1->second;
				}
			}
			else
			{
				xRes->members()[ it1->first ] = it1->second;
			}
		}
	}
	else
	{
		DataPtr data = runTimeCast< Data >( result );
		Adaptor converter( y0, y1, y2, y3, x );

		result = despatchTypedData<
		         Adaptor,
		         TypeTraits::IsStrictlyInterpolable,
		         DespatchTypedDataIgnoreError
		         >( data, converter );
	}
}

ObjectPtr linearObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	LinearInterpolator<Object>()( y0, y1, x, result );
	return result;
}

ObjectPtr cubicObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, const ObjectPtr &y2, const ObjectPtr &y3, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	CubicInterpolator<Object>()( y0, y1, y2, y3, x, result );
	return result;
}

}
