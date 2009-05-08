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

#include <cassert>

#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;
using namespace boost;
using namespace std;
using namespace Imath;

/////////////////////////////////////////////////////////////////////////////////////
// Primitive
/////////////////////////////////////////////////////////////////////////////////////

const unsigned int Primitive::m_ioVersion = 1;
IE_CORE_DEFINEABSTRACTOBJECTTYPEDESCRIPTION( Primitive );

Primitive::Primitive()
{
}

Primitive::~Primitive()
{
}

Imath::Box3f Primitive::bound() const
{
	Box3f result;
	PrimitiveVariableMap::const_iterator it = variables.find( "P" );
	if( it!=variables.end() )
	{
		ConstV3fVectorDataPtr p = runTimeCast<const V3fVectorData>( it->second.data );
		if( p )
		{
			const vector<V3f> &pp = p->readable();
			for( size_t i=0; i<pp.size(); i++ )
			{
				result.extendBy( pp[i] );
			}
		}
	}
	return result;
}

void Primitive::copyFrom( IECore::ConstObjectPtr other, IECore::Object::CopyContext *context )
{
	VisibleRenderable::copyFrom( other, context );
	const Primitive *tOther = static_cast<const Primitive *>( other.get() );
	variables.clear();
	for( PrimitiveVariableMap::const_iterator it=tOther->variables.begin(); it!=tOther->variables.end(); it++ )
	{
		variables.insert( PrimitiveVariableMap::value_type( it->first, PrimitiveVariable( it->second.interpolation, context->copy<Data>( it->second.data ) ) ) );
	}
}

void Primitive::save( IECore::Object::SaveContext *context ) const
{
	VisibleRenderable::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->mkdir( "variables" );
	container->chdir( "variables" );
		for( PrimitiveVariableMap::const_iterator it=variables.begin(); it!=variables.end(); it++ )
		{
			container->mkdir( it->first );
			container->chdir( it->first );
				const int i = it->second.interpolation;
				container->write( "interpolation", i );
				context->save( it->second.data, container, "data" );
			container->chdir( ".." );
		}
	container->chdir( ".." );
}

void Primitive::load( IECore::Object::LoadContextPtr context )
{
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );

	// we changed the inheritance hierarchy at io version 1
	if( v==0 )
	{
		Renderable::load( context );
	}
	else
	{
		VisibleRenderable::load( context );
	}

	container->chdir( "variables" );
		variables.clear();
		IndexedIO::EntryList names = container->ls();
		IndexedIO::EntryList::const_iterator it;
		for( it=names.begin(); it!=names.end(); it++ )
		{
			container->chdir( it->id() );
				int i; container->read( "interpolation", i );
				variables.insert( PrimitiveVariableMap::value_type( it->id(), PrimitiveVariable( (PrimitiveVariable::Interpolation)i, context->load<Data>( container, "data" ) ) ) );
			container->chdir( ".." );
		}
	container->chdir( ".." );
}

bool Primitive::isEqualTo( ConstObjectPtr other ) const
{
	if( !VisibleRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const Primitive *tOther = static_cast<const Primitive *>( other.get() );
	if( tOther->variables!=variables )
	{
		return false;
	}
	return true;
}

void Primitive::memoryUsage( Object::MemoryAccumulator &a ) const
{
	VisibleRenderable::memoryUsage( a );
	for( PrimitiveVariableMap::const_iterator it=variables.begin(); it!=variables.end(); it++ )
	{
		a.accumulate( it->second.data );
	}
}

struct ValidateArraySize
{
	typedef bool ReturnType;

	ValidateArraySize( size_t sz ) : m_variableSize( sz )
	{
	}

	template<typename T>
	bool operator() ( typename T::ConstPtr data )
	{
		assert( data );

		const typename T::ValueType &v = data->readable();

		return v.size() == m_variableSize;
	}

	private:

	size_t m_variableSize;
};

struct ReturnFalseErrorHandler
{
	typedef bool ReturnType;

	template<typename T, typename F>
	bool operator() ( typename T::ConstPtr data, const F &f )
	{
		return false;
	}
};

bool Primitive::isPrimitiveVariableValid( const PrimitiveVariable &pv ) const
{
	if (! pv.data || pv.interpolation==PrimitiveVariable::Invalid)
	{
		return false;
	}

	if( pv.interpolation==PrimitiveVariable::Constant )
	{
		// any data is reasonable for constant interpolation
		return true;
	}

	// all other interpolations require an array of data of the correct length. it could be argued tha
	// SimpleTypedData should be accepted in the rare case that variableSize==1, but we're rejecting that
	// argument on the grounds that it makes for a whole bunch of special cases with no gain - the general
	// cases all require arrays so that's what we require.
	size_t sz = variableSize( pv.interpolation );
	ValidateArraySize func( sz );
	return despatchTypedData<ValidateArraySize, TypeTraits::IsVectorTypedData, ReturnFalseErrorHandler>( pv.data, func );
}

bool Primitive::arePrimitiveVariablesValid() const
{
	for( PrimitiveVariableMap::const_iterator it=variables.begin(); it!=variables.end(); it++ )
	{
		if ( !isPrimitiveVariableValid( it->second ) )
		{
			return false;
		}
	}

	return true;
}

PrimitiveVariable::Interpolation Primitive::inferInterpolation( size_t numElements ) const
{
	if( variableSize( PrimitiveVariable::Constant )==numElements )
	{
		return PrimitiveVariable::Constant;
	}
	else if( variableSize( PrimitiveVariable::Uniform )==numElements )
	{
		return PrimitiveVariable::Uniform;
	}
	else if( variableSize( PrimitiveVariable::Vertex )==numElements )
	{
		return PrimitiveVariable::Vertex;
	}
	else if( variableSize( PrimitiveVariable::Varying )==numElements )
	{
		return PrimitiveVariable::Varying;
	}
	else if( variableSize( PrimitiveVariable::FaceVarying )==numElements )
	{
		return PrimitiveVariable::FaceVarying;
	}
	return PrimitiveVariable::Invalid;
}

PrimitiveVariable::Interpolation Primitive::inferInterpolation( ConstDataPtr data ) const
{
	size_t s = IECore::despatchTypedData<IECore::TypedDataSize>( boost::const_pointer_cast<Data>( data ) );
	return inferInterpolation( s );
}
