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

#include "boost/algorithm/string.hpp"

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
static IndexedIO::EntryID g_indicesEntry("indices");
const unsigned int Primitive::m_ioVersion = 2;
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
		IntVectorDataPtr indices = ( it->second.indices ) ? context->copy<IntVectorData>( it->second.indices.get() ) : nullptr;
		variables.insert( PrimitiveVariableMap::value_type( it->first, PrimitiveVariable( it->second.interpolation, context->copy<Data>( it->second.data.get() ), indices ) ) );
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
		context->save( it->second.data.get(), ioPrimVar.get(), g_dataEntry );
		if( it->second.indices )
		{
			context->save( it->second.indices.get(), ioPrimVar.get(), g_indicesEntry );
		}
	}
}

namespace
{

void flipV( FloatVectorData *v )
{
	for( auto &value : v->writable() )
	{
		value = 1.0f - value;
	}
}

V2fVectorDataPtr combineUVs( const FloatVectorData *u, const FloatVectorData *v, const IntVectorData *indices )
{
	const std::vector<float> &uValues = u->readable();
	const std::vector<float> &vValues = v->readable();
	V2fVectorDataPtr uvs = new V2fVectorData();

	if( indices )
	{
		const std::vector<int> &indexValues = indices->readable();
		int numUVs = 1 + *max_element( indexValues.begin(), indexValues.end() );
		std::vector<Imath::V2f> &uvValues = uvs->writable();
		uvValues.resize( numUVs );

		for( size_t i = 0, e = indexValues.size(); i < e; ++i )
		{
			uvValues[indexValues[i]] = Imath::V2f( uValues[i], vValues[i] );
		}
	}
	else
	{
		size_t numUVs = uValues.size();

		std::vector<Imath::V2f> &uvValues = uvs->writable();
		uvValues.reserve( numUVs );

		for( size_t i = 0; i < numUVs; ++i )
		{
			uvValues.emplace_back( uValues[i], vValues[i] );
		}
	}

	return uvs;
}

void remapToLegacyVariableNames( const IndexedIO::EntryIDList &requested, const IndexedIO::EntryIDList &existing, IndexedIO::EntryIDList &result )
{
	result.clear();

	IndexedIO::EntryIDList extraUVs;
	for( const auto &existingName : existing )
	{
		if( boost::ends_with( existingName.string(), "_s" ) || boost::ends_with( existingName.string(), "_t" ) )
		{
			extraUVs.push_back( existingName.string().substr( 0, existingName.string().length() - 2 ) );
		}
	}

	for( const auto &name : requested )
	{
		if( name == "uv" )
		{
			result.push_back( "s" );
			result.push_back( "t" );
			result.push_back( "stIndices" );
		}
		else if( std::find( extraUVs.begin(), extraUVs.end(), name ) != extraUVs.end() )
		{
			result.push_back( name.string() + "_s" );
			result.push_back( name.string() + "_t" );
			result.push_back( name.string() + "Indices" );
		}
		else
		{
			result.push_back( name );
		}
	}
}

void convertLegacyVariables( PrimitiveVariableMap &variables )
{
	std::set<Data *> processed;
	std::vector<PrimitiveVariableMap::const_iterator> variablesToErase;

	for( auto it = variables.begin(); it != variables.end(); ++it )
	{
		// We changed from s & t (FloatVectorData) to UVs (V2fVectorData)
		// in version 2, so we collect our UV sets and combine them.

		// We have 2 legacy conventions for storing UVs:
		//  (a) PrimitiveVariables named "s" and "t" represent
		//      the components of the primary UV set.
		//  (b) PrimitiveVariables named "<var>_s" and "<var>_t"
		//      represent the components of an extra UV set
		//      named "<var>".

		// We also changed from storing separate indices
		// as side-car PrimitiveVariables to storing them
		// directly on the primary PrimitiveVariable.

		// We have 3 legacy conventions for storing indices:
		//  (a) For the primary UV set, as in (a) above, the
		//      indices were named "stIndices".
		//  (b) For the extra UV sets, as in (b) above, the
		//      indices were named "<var>Indices".
		//  (c) For non-UV PrimitiveVariables named "<var>"
		//      the indices were also named "<var>Indices".

		// the name of the uvSet if there is one
		std::string uvSet;
		// the u values if we find any
		PrimitiveVariableMap::const_iterator uIt = variables.end();
		// the v values if we find any
		PrimitiveVariableMap::const_iterator vIt = variables.end();
		// the indices if we find any
		PrimitiveVariableMap::const_iterator indicesIt = variables.end();

		if( it->first == "s" )
		{
			uvSet = "uv";
			uIt = it;
			vIt = variables.find( "t" );
			indicesIt = variables.find( "stIndices" );
		}
		else if( it->first == "t" )
		{
			uvSet = "uv";
			uIt = variables.find( "s" );
			vIt = it;
			indicesIt = variables.find( "stIndices" );
		}
		else if( boost::ends_with( it->first, "_s" ) )
		{
			uvSet = it->first.substr( 0, it->first.length() - 2 );

			uIt = it;
			vIt = variables.find( uvSet + "_t" );
			indicesIt = variables.find( uvSet + "Indices" );
		}
		else if( boost::ends_with( it->first, "_t" ) )
		{
			uvSet = it->first.substr( 0, it->first.length() - 2 );

			uIt = variables.find( uvSet + "_s" );
			vIt = it;
			indicesIt = variables.find( uvSet + "Indices" );
		}
		else
		{
			indicesIt = variables.find( it->first + "Indices" );
		}

		if(
			uIt != variables.end() &&
			vIt != variables.end() &&
			uIt->second.interpolation == vIt->second.interpolation
		)
		{
			FloatVectorData *u = runTimeCast<FloatVectorData>( uIt->second.data.get() );
			FloatVectorData *v = runTimeCast<FloatVectorData>( vIt->second.data.get() );

			if( v && processed.find( v ) == processed.end() )
			{
				// We unflipped the t (v) values in version 2, so we must unflip
				// them for older files as well.
				flipV( v );
				// But we only want to flip once, incase this data is shared with
				// a PrimitiveVariable we already processed.
				processed.insert( v );
			}

			if(
				u && v &&
				std::find( variablesToErase.begin(), variablesToErase.end(), uIt ) == variablesToErase.end() &&
				std::find( variablesToErase.begin(), variablesToErase.end(), vIt ) == variablesToErase.end()
			)
			{
				// We changed UV set convention to be V2fVectorData in version 2,
				// so we must combine them for older files and associate the
				// indicies, compacting the UV data if possible.

				IntVectorData *indices = nullptr;
				PrimitiveVariable::Interpolation interpolation = uIt->second.interpolation;
				if( indicesIt != variables.end() )
				{
					indices = runTimeCast<IntVectorData>( indicesIt->second.data.get() );
					interpolation = indicesIt->second.interpolation;
					variablesToErase.push_back( indicesIt );
				}

				V2fVectorDataPtr uvData = combineUVs( u, v, indices );

				bool inserted;
				PrimitiveVariableMap::iterator uvIt;
				std::tie( uvIt, inserted ) = variables.emplace( uvSet, PrimitiveVariable( interpolation, uvData, indices ) );
				if( !inserted )
				{
					// the uvSet name already existed, but we just stomp over it.
					// in practice, there shouldn't be any files like this.
					uvIt->second = PrimitiveVariable( interpolation, uvData, indices );
				}

				variablesToErase.push_back( uIt );
				variablesToErase.push_back( vIt );
			}
		}
		else if( indicesIt != variables.end() )
		{
			// We presume that non-UV indices are already as compact as possible
			IntVectorData *indices = runTimeCast<IntVectorData>( indicesIt->second.data.get() );
			it->second.indices = indices;
			it->second.interpolation = indicesIt->second.interpolation;
			variablesToErase.push_back( indicesIt );
		}
	}

	for( auto &eIt : variablesToErase )
	{
		variables.erase( eIt );
	}
}

} // namespace

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

		IntVectorDataPtr indices = nullptr;
		if( ioPrimVar->hasEntry( g_indicesEntry ) )
		{
			indices = context->load<IntVectorData>( ioPrimVar.get(), g_indicesEntry );
		}

		variables.insert( 
			PrimitiveVariableMap::value_type( *it, PrimitiveVariable( (PrimitiveVariable::Interpolation)i, context->load<Data>( ioPrimVar.get(), g_dataEntry ), indices ) )
		);
	}

	if( v < 2 )
	{
		::convertLegacyVariables( variables );
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

	IndexedIO::EntryIDList names( primVarNames );
	if( v < 2 )
	{
		// we changed naming convention for UVs in version 2
		// so we must remap to the names that actually exist
		// in the file, assuming the user request is using
		// the new naming convention.
		IndexedIO::EntryIDList existingNames;
		ioVariables->entryIds( existingNames, IndexedIO::Directory );
		remapToLegacyVariableNames( primVarNames, existingNames, names );
	}

	PrimitiveVariableMap variables;
	for( const auto &name : names )
	{
		ConstIndexedIOPtr ioPrimVar = ioVariables->subdirectory( name, IndexedIO::NullIfMissing );
		if ( !ioPrimVar )
		{
			continue;
		}
		int i; 
		ioPrimVar->read( g_interpolationEntry, i );

		IntVectorDataPtr indices = nullptr;
		if( ioPrimVar->hasEntry( g_indicesEntry ) )
		{
			indices = context->load<IntVectorData>( ioPrimVar.get(), g_indicesEntry );
		}

		variables.insert(
			PrimitiveVariableMap::value_type( name, PrimitiveVariable( (PrimitiveVariable::Interpolation)i, context->load<Data>( ioPrimVar.get(), g_dataEntry ), indices ) )
		);
	}

	if( v < 2 )
	{
		::convertLegacyVariables( variables );
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
		a.accumulate( it->second.data.get() );
	}
}

void Primitive::hash( MurmurHash &h ) const
{
	VisibleRenderable::hash( h );
	for( PrimitiveVariableMap::const_iterator it=variables.begin(); it!=variables.end(); it++ )
	{
		h.append( it->first );
		/// \todo: should we define PrimitiveVariable::hash()
		/// and call that here?
		h.append( it->second.interpolation );
		it->second.data->hash( h );
		if( it->second.indices )
		{
			it->second.indices->hash( h );
		}
	}
	
	topologyHash( h );
}

namespace
{

struct ReturnZeroErrorHandler
{
	typedef size_t ReturnType;

	template<typename T, typename F>
	size_t operator() ( const T *data, const F &f )
	{
		return 0;
	}
};

} // namespace

bool Primitive::isPrimitiveVariableValid( const PrimitiveVariable &pv ) const
{
	if( !pv.data || pv.interpolation==PrimitiveVariable::Invalid )
	{
		return false;
	}

	if( !pv.indices && pv.interpolation == PrimitiveVariable::Constant )
	{
		// any data is reasonable for constant interpolation
		// provided there are no indices.
		return true;
	}

	// All other interpolations require an array of data or an array of indices
	// of the correct length. It could be argued that SimpleTypedData should be
	// accepted in the rare case that variableSize==1, but we're rejecting that
	// argument on the grounds that it makes for a whole bunch of special cases
	// with no gain - the general cases require arrays. Note that variableSize
	// of 0 is a special case for empty data, which we must allow to pass here.
	size_t sz = variableSize( pv.interpolation );
	size_t dataSize = despatchTypedData<TypedDataSize, TypeTraits::IsVectorTypedData, ReturnZeroErrorHandler>( pv.data.get() );
	if( !dataSize && sz )
	{
		return false;
	}

	/// without indices, the data must match exactly
	/// \todo This is not correct in the case of CurvesPrimitives, where uniform
	/// interpolation should be treated the same as constant.
	if( !pv.indices )
	{
		return dataSize == sz;
	}

	/// if there are indices, we must ensure they're within the range of data
	/// and that they match the expected variable size.
	if( pv.indices->readable().size() != sz )
	{
		return false;
	}

	for( const auto &index : pv.indices->readable() )
	{
		if( index >= (int)dataSize )
		{
			return false;
		}
	}

	// the data doesn't need further validation since we know its
	// VectorTypedData and the indices are in the correct range.
	return true;
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
