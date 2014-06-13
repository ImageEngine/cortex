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
#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace boost;
using namespace std;
using namespace Imath;

/////////////////////////////////////////////////////////////////////////////////////
// Primitive
/////////////////////////////////////////////////////////////////////////////////////

static IndexedIO::EntryID g_variablesEntry("variables");
static IndexedIO::EntryID g_interpolationEntry("interpolation");
static IndexedIO::EntryID g_dataEntry("data");
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

void Primitive::copyFrom( const Object *other, IECore::Object::CopyContext *context )
{
	VisibleRenderable::copyFrom( other, context );
	const Primitive *tOther = static_cast<const Primitive *>( other );
	variables.clear();
	for( PrimitiveVariableMap::const_iterator it=tOther->variables.begin(); it!=tOther->variables.end(); it++ )
	{
		variables.insert( PrimitiveVariableMap::value_type( it->first, PrimitiveVariable( it->second.interpolation, context->copy<Data>( it->second.data ) ) ) );
	}
}

void Primitive::save( IECore::Object::SaveContext *context ) const
{
	VisibleRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	IndexedIOPtr ioVariables = container->subdirectory( g_variablesEntry, IndexedIO::CreateIfMissing );
	for( PrimitiveVariableMap::const_iterator it=variables.begin(); it!=variables.end(); it++ )
	{
		IndexedIOPtr ioPrimVar = ioVariables->subdirectory( it->first, IndexedIO::CreateIfMissing );
		const int i = it->second.interpolation;
		ioPrimVar->write( g_interpolationEntry, i );
		context->save( it->second.data, ioPrimVar, g_dataEntry );
	}
}

void Primitive::load( IECore::Object::LoadContextPtr context )
{
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	// we changed the inheritance hierarchy at io version 1
	if( v==0 )
	{
		Renderable::load( context );
	}
	else
	{
		VisibleRenderable::load( context );
	}

	ConstIndexedIOPtr ioVariables = container->subdirectory( g_variablesEntry );

	variables.clear();
	IndexedIO::EntryIDList names;
	ioVariables->entryIds( names, IndexedIO::Directory );
	IndexedIO::EntryIDList::const_iterator it;
	for( it=names.begin(); it!=names.end(); it++ )
	{
		ConstIndexedIOPtr ioPrimVar = ioVariables->subdirectory( *it );
		int i; 
		ioPrimVar->read( g_interpolationEntry, i );
		variables.insert( 
			PrimitiveVariableMap::value_type( *it, PrimitiveVariable( (PrimitiveVariable::Interpolation)i, context->load<Data>( ioPrimVar, g_dataEntry ) ) ) 
		);
	}
}

PrimitiveVariableMap Primitive::loadPrimitiveVariables( const IndexedIO *ioInterface, const IndexedIO::EntryID &name, const IndexedIO::EntryIDList &primVarNames )
{
	IECore::Object::LoadContextPtr context = new Object::LoadContext( ioInterface->subdirectory( name )->subdirectory( g_dataEntry ) );

	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( Primitive::staticTypeName(), v );
	if ( !container )
	{
		throw Exception( "Could not find Primitive entry in the file!" );
	}
	ConstIndexedIOPtr ioVariables = container->subdirectory( g_variablesEntry );

	PrimitiveVariableMap variables;
	IndexedIO::EntryIDList::const_iterator it;
	for( it=primVarNames.begin(); it!=primVarNames.end(); it++ )
	{
		ConstIndexedIOPtr ioPrimVar = ioVariables->subdirectory( *it, IndexedIO::NullIfMissing );
		if ( !ioPrimVar )
		{
			continue;
		}
		int i; 
		ioPrimVar->read( g_interpolationEntry, i );
		variables.insert( 
			PrimitiveVariableMap::value_type( *it, PrimitiveVariable( (PrimitiveVariable::Interpolation)i, context->load<Data>( ioPrimVar, g_dataEntry ) ) ) 
		);
	}

	return variables;
}

bool Primitive::isEqualTo( const Object *other ) const
{
	if( !VisibleRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const Primitive *tOther = static_cast<const Primitive *>( other );
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

void Primitive::hash( MurmurHash &h ) const
{
	VisibleRenderable::hash( h );
	for( PrimitiveVariableMap::const_iterator it=variables.begin(); it!=variables.end(); it++ )
	{
		h.append( it->first );
		h.append( it->second.interpolation );
		it->second.data->hash( h );
	}
	
	topologyHash( h );
}

struct ValidateArraySize
{
	typedef bool ReturnType;

	ValidateArraySize( size_t sz ) : m_variableSize( sz )
	{
	}

	template<typename T>
	bool operator() ( const T *data )
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
	bool operator() ( const T *data, const F &f )
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
	return despatchTypedData<ValidateArraySize, TypeTraits::IsVectorTypedData, ReturnFalseErrorHandler>( pv.data.get(), func );
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

PrimitiveVariable::Interpolation Primitive::inferInterpolation( const Data *data ) const
{
	size_t s = IECore::despatchTypedData<IECore::TypedDataSize>( const_cast<Data *>( data ) );
	return inferInterpolation( s );
}
