//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
#include <fstream>

#include "boost/format.hpp"

#include "IECore/BINMeshReader.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"
#include "IECore/ByteOrder.h"

using namespace std;
using namespace IECore;
using namespace Imath;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( BINMeshReader );

const Reader::ReaderDescription<BINMeshReader> BINMeshReader::m_readerDescription( "bin" );

template<typename T>
static void readLittleEndian( std::istream &f, T &n )
{
	f.read(( char* ) &n, sizeof( T ) );

	if ( bigEndian() )
	{
		n = reverseBytes<>( n );
	}
	else
	{
		/// Already little endian
	}
}

BINMeshReader::BINMeshReader( const string &name )
		: Reader(
		        "BINMeshReader",
		        "Realflow .bin mesh reader",
		        new ObjectParameter( "result", "the loaded 3D object", new NullObject, MeshPrimitive::staticTypeId() )
		)
{
	m_fileNameParameter->setTypedValue( name );
}

bool BINMeshReader::canRead( const string &fileName )
{
	ifstream in( fileName.c_str() );
	if ( in.fail() || !in.is_open() )
	{
		return false;
	}

	in.seekg( 0, ios_base::beg );
	uint32_t magic = 0;
	readLittleEndian( in, magic );
	return magic == 0xDADADADA && !in.fail() ;
}

ObjectPtr BINMeshReader::doOperation( ConstCompoundObjectPtr operands )
{
	const std::string &fileName = m_fileNameParameter->getTypedValue();
	ifstream f( fileName.c_str() );

	f.seekg( 0, ios_base::beg );
	uint32_t magic = 0;
	readLittleEndian( f, magic );

	uint32_t version = 0;
	readLittleEndian( f, version );
	if ( version <= 3 )
	{
		throw IOException(( boost::format( "BINMeshReader: '%s' is of an unsupported version" ) % fileName ).str() );
	}

	MeshPrimitivePtr mesh = new MeshPrimitive();

	uint32_t numVertices = 0;

	bool foundGeometryChunk = false;
	bool done = false;

	uint32_t chunkId = 0;

	while ( !done && !f.fail() )
	{
		readLittleEndian( f, chunkId );
		if ( f.fail() )
		{
			throw IOException(( boost::format( "BINMeshReader: Error encountered while reading '%s'" ) % fileName ).str() );
		}

		if ( chunkId == 0xDEDEDEDE ) /// EOF marker
		{
			if ( !foundGeometryChunk )
			{
				throw IOException(( boost::format( "BINMeshReader: No geometry chunk encountered while reading '%s'" ) % fileName ).str() );
			}
			done = true;
		}
		else if ( chunkId == 0xCCCCCCCC ) /// geometry chunk
		{
			if ( foundGeometryChunk )
			{
				throw IOException(( boost::format( "BINMeshReader: Duplicate geometry chunk encountered while reading '%s'" ) % fileName ).str() );
			}
			foundGeometryChunk = true;

			V3fVectorDataPtr pData = new V3fVectorData();

			readLittleEndian( f, numVertices );

			pData->writable().resize( numVertices );
			for ( uint32_t i = 0; i < numVertices; i ++ )
			{
				V3f p;
				readLittleEndian( f, p.x );
				readLittleEndian( f, p.y );
				readLittleEndian( f, p.z );

				pData->writable()[i] = p;
			}


			uint32_t numFaces = 0;
			readLittleEndian( f, numFaces );

			IntVectorDataPtr vertsPerFaceData = new IntVectorData();

			/// All faces are triangles
			vertsPerFaceData->writable().resize( numFaces, 3 );

			IntVectorDataPtr vertIdsData = new IntVectorData();
			vertIdsData->writable().reserve( numFaces * 3 );

			for ( uint32_t i = 0; i < numFaces; i ++ )
			{
				uint32_t v0 = 0, v1 = 0, v2 = 0;
				readLittleEndian( f, v0 );
				readLittleEndian( f, v1 );
				readLittleEndian( f, v2 );

				vertIdsData->writable().push_back( v0 );
				vertIdsData->writable().push_back( v1 );
				vertIdsData->writable().push_back( v2 );
			}

			mesh->variables[ "P" ] = PrimitiveVariable( PrimitiveVariable::Vertex, pData );
			mesh->setTopology( vertsPerFaceData, vertIdsData, "linear" );

		}
		else if ( chunkId == 0xCCCCCC00 ) /// texture chunk
		{
			if ( !foundGeometryChunk )
			{
				throw IOException(( boost::format( "BINMeshReader: No geometry chunk encountered while reading '%s'" ) % fileName ).str() );
			}

			uint32_t numFluids = 0;
			readLittleEndian( f, numFluids );

			V3fVectorDataPtr uvwData = new V3fVectorData();
			uvwData->writable().resize( numVertices );

			for ( uint32_t v = 0; v < numVertices; v ++ )
			{
				for ( uint32_t fl = 0; fl < numFluids - 1; fl ++ )
				{
					/// Just skip over there for now
					/// \todo Work out what to do with them
					float textureWeight = 0.0f;
					readLittleEndian( f, textureWeight );
				}

				V3f uvw;
				readLittleEndian( f, uvw.x );
				readLittleEndian( f, uvw.y );
				readLittleEndian( f, uvw.z );

				uvwData->writable()[v] = uvw;
			}

			mesh->variables[ "uvw" ] = PrimitiveVariable( PrimitiveVariable::Vertex, uvwData );
		}
		else if ( chunkId == 0xCCCCCC11 ) /// velocity chunk
		{
			if ( !foundGeometryChunk )
			{
				throw IOException(( boost::format( "BINMeshReader: No geometry chunk encountered while reading '%s'" ) % fileName ).str() );
			}

			V3fVectorDataPtr velocityData = new V3fVectorData();
			velocityData->writable().resize( numVertices );
			for ( uint32_t i = 0; i < numVertices; i ++ )
			{
				V3f vel;
				readLittleEndian( f, vel.x );
				readLittleEndian( f, vel.y );
				readLittleEndian( f, vel.z );

				velocityData->writable()[i] = vel;
			}
			mesh->variables[ "velocity" ] = PrimitiveVariable( PrimitiveVariable::Vertex, velocityData );
		}
		else
		{
			throw IOException(( boost::format( "BINMeshReader: Invalid chunk encountered while reading '%s'" ) % fileName ).str() );
		}
	}

	if ( chunkId != 0xDEDEDEDE )
	{
		throw IOException(( boost::format( "BINMeshReader: No end of file chunk encountered while reading '%s'" ) % fileName ).str() );
	}

	assert( mesh );
	return mesh;
}
