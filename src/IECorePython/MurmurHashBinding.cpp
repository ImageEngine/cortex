//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"
#include "boost/python.hpp"

#include "IECorePython/MurmurHashBinding.h"

#include "IECore/DataAlgo.h"
#include "IECore/MurmurHash.h"
#include "IECore/VectorTypedData.h"

using namespace boost::python;
using namespace IECore;

namespace
{

static std::string repr( const MurmurHash &hash )
{
	return "IECore.MurmurHash( \"" + hash.toString() + "\" )";
}

template<typename T>
static void appendArray( MurmurHash &hash, typename TypedData<std::vector<T> >::ConstPtr data )
{
	hash.append( data->readable() );
}

static void appendInt( MurmurHash &hash, int64_t v )
{
	// Function that keeps backward compatibility for int types.
	// \todo Consider removing this function in Cortex 8
	if ( v == int64_t(int(v)) )
	{
		hash.append( int(v) );
	}
	else
	{
		hash.append( v );
	}
}

static long hash( MurmurHash *h )
{
	return h->h1();
}

#define IECORETEST_ASSERT( x ) \
	if( !( x ) ) \
	{ \
		throw IECore::Exception( boost::str( \
			boost::format( "Failed assertion \"%s\" : %s line %d" ) % #x % __FILE__ % __LINE__ \
		) ); \
	}

struct HashDispatchFunctor
{
	template< class T>
	IECore::MurmurHash operator()( const T *data )
	{
		IECore::MurmurHash result;
		result.append( data->readable() );
		return result;
	}

	IECore::MurmurHash operator()( const IECore::Data *data )
	{
		throw IECore::Exception( "Unsupported" );
	}
};

void testMurmurHashDispatch()
{

	// If the dispatch can even compile, that means we have hash functions defined for all the types
	// that dispatch uses.  We should also check a few types to make sure the Data dispatch actually works
	HashDispatchFunctor hashFunctor;
	IECore::MurmurHash h;
	h.append( 42.37f );
	IECORETEST_ASSERT( h == IECore::dispatch( IECore::FloatDataPtr( new IECore::FloatData( 42.37 ) ).get(), hashFunctor ) );

	h = IECore::MurmurHash();
	h.append( std::string("foo") );
	IECORETEST_ASSERT( h == IECore::dispatch( IECore::StringDataPtr( new IECore::StringData( "foo" ) ).get(), hashFunctor ) );

	h = IECore::MurmurHash();
	h.append( std::vector<float>{ 1, 3, 37.03 } );
	IECORETEST_ASSERT( h == IECore::dispatch( IECore::FloatVectorDataPtr( new IECore::FloatVectorData( std::vector<float>{ 1, 3, 37.03 } ) ).get(), hashFunctor ) );
}

} // namespace

void IECorePython::bindMurmurHash()
{

	class_<MurmurHash>( "MurmurHash" )
		.def( init<>() )
		.def( init<const MurmurHash &>() )
		.def( init<const std::string &>() )
		.def( init<uint64_t, uint64_t>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const float & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const double & ))&MurmurHash::append, return_self<>() )
		.def( "append", &appendInt, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const std::string & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const InternedString & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::V2i & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::V2f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::V2d & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::V3i & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::V3f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::V3d & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Color3f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Color4f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::M33f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::M33d & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::M44f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::M44d & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Box2i & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Box2f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Box2d & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Box3i & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Box3f & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Box3d & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Quatf & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const Imath::Quatd & ))&MurmurHash::append, return_self<>() )
		.def( "append", (MurmurHash &(MurmurHash::*)( const MurmurHash & ))&MurmurHash::append, return_self<>() )
		// array append functions
		.def( "append", &appendArray<char>, return_self<>() )
		.def( "append", &appendArray<unsigned char>, return_self<>() )
		.def( "append", &appendArray<short>, return_self<>() )
		.def( "append", &appendArray<unsigned short>, return_self<>() )
		.def( "append", &appendArray<int>, return_self<>() )
		.def( "append", &appendArray<unsigned int>, return_self<>() )
		.def( "append", &appendArray<int64_t>, return_self<>() )
		.def( "append", &appendArray<uint64_t>, return_self<>() )
		.def( "append", &appendArray<half>, return_self<>() )
		.def( "append", &appendArray<float>, return_self<>() )
		.def( "append", &appendArray<double>, return_self<>() )
		.def( "append", &appendArray<std::string>, return_self<>() )
		.def( "append", &appendArray<InternedString>, return_self<>() )
		.def( "append", &appendArray<Imath::V2i>, return_self<>() )
		.def( "append", &appendArray<Imath::V2f>, return_self<>() )
		.def( "append", &appendArray<Imath::V2d>, return_self<>() )
		.def( "append", &appendArray<Imath::V3i>, return_self<>() )
		.def( "append", &appendArray<Imath::V3f>, return_self<>() )
		.def( "append", &appendArray<Imath::V3d>, return_self<>() )
		.def( "append", &appendArray<Imath::Color3f>, return_self<>() )
		.def( "append", &appendArray<Imath::Color4f>, return_self<>() )
		.def( "append", &appendArray<Imath::M33f>, return_self<>() )
		.def( "append", &appendArray<Imath::M33d>, return_self<>() )
		.def( "append", &appendArray<Imath::M44f>, return_self<>() )
		.def( "append", &appendArray<Imath::M44d>, return_self<>() )
		.def( "append", &appendArray<Imath::Box2i>, return_self<>() )
		.def( "append", &appendArray<Imath::Box2f>, return_self<>() )
		.def( "append", &appendArray<Imath::Box2d>, return_self<>() )
		.def( "append", &appendArray<Imath::Box3i>, return_self<>() )
		.def( "append", &appendArray<Imath::Box3f>, return_self<>() )
		.def( "append", &appendArray<Imath::Box3d>, return_self<>() )
		.def( "append", &appendArray<Imath::Quatf>, return_self<>() )
		.def( "append", &appendArray<Imath::Quatd>, return_self<>() )
		.def( "append", &appendArray<bool>, return_self<>() )
		.def( self == self )
		.def( self != self )
		.def( self < self )
		.def( "copyFrom", &MurmurHash::operator=, return_self<>() )
		.def( "__repr__", &repr )
		.def( "__str__", &MurmurHash::toString )
		.def( "__hash__", &hash )
		.def( "toString", &MurmurHash::toString )
		.def( "fromString", (MurmurHash (*)( const std::string & ))&MurmurHash::fromString )
		.staticmethod( "fromString" )
		.def( "h1", &MurmurHash::h1 )
		.def( "h2", &MurmurHash::h2 )
	;

	def( "testMurmurHashDispatch", &testMurmurHashDispatch );
}
