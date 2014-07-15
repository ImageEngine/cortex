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

#include "IECore/Object.h"
#include "IECore/Interpolator.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/CompoundData.h"
#include "IECore/CompoundObject.h"
#include "IECore/Primitive.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;

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
	else if ( y0->isInstanceOf( PrimitiveTypeId ) )
	{
		const Primitive *x0 = assertedStaticCast<const Primitive>( y0 );
		const Primitive *x1 = assertedStaticCast<const Primitive>( y1 );
		
		if( x0->variableSize( PrimitiveVariable::Uniform ) == x1->variableSize( PrimitiveVariable::Uniform ) &&
			x0->variableSize( PrimitiveVariable::Varying ) == x1->variableSize( PrimitiveVariable::Varying ) &&
			x0->variableSize( PrimitiveVariable::Vertex ) == x1->variableSize( PrimitiveVariable::Vertex ) &&
			x0->variableSize( PrimitiveVariable::FaceVarying ) == x1->variableSize( PrimitiveVariable::FaceVarying )
		)
		{
			PrimitivePtr xRes = assertedStaticCast<Primitive>( result );
			xRes->Object::copyFrom( (const Object *)x0 ); // to get topology and suchlike copied over
			// interpolate blindData
			const Object *bd0 = x0->blindData();
			const Object *bd1 = x1->blindData();
			ObjectPtr bdr = xRes->blindData();
			LinearInterpolator<Object>()( bd0, bd1, x, bdr );			
			// interpolate primitive variables
			for( PrimitiveVariableMap::const_iterator it0 = x0->variables.begin(); it0 != x0->variables.end(); it0++ )
			{
				PrimitiveVariableMap::const_iterator it1 = x1->variables.find( it0->first );
				if( it1 != x1->variables.end() &&
					it0->second.data->typeId() == it1->second.data->typeId() &&
					it0->second.interpolation == it1->second.interpolation
				)
				{
					PrimitiveVariableMap::iterator itRes = xRes->variables.find( it0->first );
					ObjectPtr resultData = linearObjectInterpolation( it0->second.data.get(), it1->second.data.get(), x );
					if( resultData )
					{
						itRes->second.data = boost::static_pointer_cast<Data>( resultData );
					}
				}
			}
		}
		else
		{
			// primitive topologies don't match
			result = 0;		
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
		result = 0;
	}
}

struct CubicInterpolator< Object >::Adaptor
{
	typedef ObjectPtr ReturnType;

	const Data *m_y0;
	const Data *m_y1;
	const Data *m_y2;
	const Data *m_y3;
	double m_x;

	Adaptor( const Object *y0, const Object *y1, const Object *y2, const Object *y3, double x ) : m_x( x )
	{
		m_y0 = assertedStaticCast< const Data >( y0 );
		m_y1 = assertedStaticCast< const Data >( y1 );
		m_y2 = assertedStaticCast< const Data >( y2 );
		m_y3 = assertedStaticCast< const Data >( y3 );
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr result )
	{
		const T *y0 = assertedStaticCast< const T >( m_y0 );
		const T *y1 = assertedStaticCast< const T >( m_y1 );
		const T *y2 = assertedStaticCast< const T >( m_y2 );
		const T *y3 = assertedStaticCast< const T >( m_y3 );

		CubicInterpolator<T>()( y0, y1, y2, y3, m_x, result );

		return result;
	}
};


void CubicInterpolator<Object>::operator()( const Object *y0, const Object *y1, const Object *y2, const Object *y3, double x, ObjectPtr &result ) const
{
	if ( y0->typeId() != y1->typeId() || y0->typeId() != y2->typeId() || y0->typeId() != y3->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}

	if ( y0->isInstanceOf( CompoundDataTypeId ) )
	{
		const CompoundData *x0 = assertedStaticCast<const CompoundData>( y0 );
		const CompoundData *x1 = assertedStaticCast<const CompoundData>( y1 );
		const CompoundData *x2 = assertedStaticCast<const CompoundData>( y2 );
		const CompoundData *x3 = assertedStaticCast<const CompoundData>( y3 );
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
						CubicInterpolator< Object >()( it0->second.get(), it1->second.get(), it2->second.get(), it3->second.get(), x, resultObj );
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
		const CompoundObject *x0 = assertedStaticCast<const CompoundObject>( y0 );
		const CompoundObject *x1 = assertedStaticCast<const CompoundObject>( y1 );
		const CompoundObject *x2 = assertedStaticCast<const CompoundObject>( y2 );
		const CompoundObject *x3 = assertedStaticCast<const CompoundObject>( y3 );
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
						CubicInterpolator< Object >()( it0->second.get(), it1->second.get(), it2->second.get(), it3->second.get(), x, resultObj );

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
	else if ( result->isInstanceOf( DataTypeId ) )
	{
		DataPtr data = runTimeCast< Data >( result );
		Adaptor converter( y0, y1, y2, y3, x );

		result = despatchTypedData<
		         Adaptor,
		         TypeTraits::IsStrictlyInterpolable,
		         DespatchTypedDataIgnoreError
		         >( data.get(), converter );
	}
	else
	{
		result = 0;
	}
}

ObjectPtr linearObjectInterpolation( const Object *y0, const Object *y1, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	LinearInterpolator<Object>()( y0, y1, x, result );
	return result;
}

ObjectPtr cubicObjectInterpolation( const Object *y0, const Object *y1, const Object *y2, const Object *y3, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	CubicInterpolator<Object>()( y0, y1, y2, y3, x, result );
	return result;
}

}
