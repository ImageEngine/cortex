//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/NParticleReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/FileNameParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Timer.h"

#include "OpenEXR/ImathRandom.h"

#include <boost/algorithm/string/predicate.hpp>

#include <algorithm>
#include <fstream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( NParticleReader );

const Reader::ReaderDescription<NParticleReader> NParticleReader::m_readerDescription( "mc" );

NParticleReader::NParticleReader()
	:	ParticleReader( "Reads Maya .mc format nCaches" ), m_iffFile( nullptr ), m_frames( new IntVectorData )
{
	m_frameParameter = new IntParameter( "frameIndex", "Index of the desired frame to be loaded", 0 );
	parameters()->addParameter( m_frameParameter );
}

NParticleReader::NParticleReader( const std::string &fileName )
	:	ParticleReader( "Reads Maya .mc format nCaches" ), m_iffFile( nullptr ), m_frames( new IntVectorData )
{
	m_fileNameParameter->setTypedValue( fileName );

	m_frameParameter = new IntParameter( "frameIndex", "Index of the desired frame to be loaded", 0 );
	parameters()->addParameter( m_frameParameter );
}

NParticleReader::~NParticleReader()
{
}

bool NParticleReader::canRead( const std::string &fileName )
{
	try
	{
		IFFFile iffFile( fileName );
		IFFFile::Chunk::ChunkIterator itBegin = iffFile.root()->childrenBegin();

		if ( itBegin->isGroup() && itBegin->groupName().id() == IFFFile::Tag( "CACH" ).id() )
		{
			return true;
		}
	}
	catch( ... )
	{
	}

	return false;
}

bool NParticleReader::open()
{
	if( !m_iffFile || m_iffFileName!=fileName() )
	{
		m_iffFile = new IFFFile( fileName().c_str() );

		IFFFile::Chunk *root = m_iffFile->root();
		IFFFile::Chunk::ChunkIterator headerIt = root->childrenBegin();

		if ( !headerIt->isGroup() || headerIt->groupName().id() != kCACH )
		{
			m_header.valid = false;
			return false;
		}

		IFFFile::Chunk::ChunkIterator child = headerIt->childrenBegin();
		for ( ; child != headerIt->childrenEnd(); child++ )
		{
			if ( child->type().id() == kVRSN )
			{
				child->read( m_header.version );
			}
			else if ( child->type().id() == kSTIM )
			{
				child->read( m_header.startTime );
			}
			else if ( child->type().id() == kETIM )
			{
				child->read( m_header.endTime );
			}
		}

		m_frames->writable().clear();
		frameToRootChildren.clear();

		// single frame per file
		if ( m_header.startTime == m_header.endTime && (headerIt+1)->groupName().id() == kMYCH )
		{
			frameToRootChildren[m_header.startTime] = headerIt + 1;
			m_frames->writable().push_back( m_header.startTime );
		}
		// multiple frames per file
		else
		{
			IFFFile::Chunk::ChunkIterator bodyIt = headerIt;
			for ( ; bodyIt != root->childrenEnd(); bodyIt++ )
			{
				if ( bodyIt->groupName().id() == kMYCH )
				{
					// check children for TIME
					IFFFile::Chunk::ChunkIterator child = bodyIt->childrenBegin();
					for ( ; child != bodyIt->childrenEnd(); child++ )
					{
						if ( child->type().id() == kTIME )
						{
							int time = 0;
							child->read( time );
							frameToRootChildren[time] = bodyIt;
							m_frames->writable().push_back( time );
							break;
						}
					}
				}
			}
		}

		sort( m_frames->writable().begin(), m_frames->writable().end() );

		m_header.valid = true;
		m_iffFileName = fileName();
	}

	return m_header.valid && m_iffFileName == fileName();
}

unsigned long NParticleReader::numParticles()
{
	if( !open() )
	{
		return 0;
	}

	int numParticles = 0;
	int frameIndex = m_frameParameter->getNumericValue();
	int frame = m_frames->readable()[frameIndex];
	std::map<int, IFFFile::Chunk::ChunkIterator>::const_iterator frameIt = frameToRootChildren.find( frame );
	if( frameIt == frameToRootChildren.end() )
	{
		msg( Msg::Warning, "NParticleReader::attributeNames()", boost::format( "Frame '%d' (index '%d') does not exist in '%s'." ) % frame % frameIndex % m_iffFileName );
		return 0;
	}

	IFFFile::Chunk::ChunkIterator child = frameIt->second;
	IFFFile::Chunk::ChunkIterator it = child->childrenBegin();

	for ( ; it != child->childrenEnd(); it++ )
	{
		if ( it->type().id() == kSIZE )
		{
			it->read( numParticles );
			break;
		}
	}

	return numParticles;
}

void NParticleReader::attributeNames( std::vector<std::string> &names )
{
	names.clear();
	if( !open() )
	{
		return;
	}

	int frameIndex = m_frameParameter->getNumericValue();
	int frame = m_frames->readable()[frameIndex];
	std::map<int, IFFFile::Chunk::ChunkIterator>::const_iterator frameIt = frameToRootChildren.find( frame );
	if( frameIt == frameToRootChildren.end() )
	{
		msg( Msg::Warning, "NParticleReader::attributeNames()", boost::format( "Frame '%d' (index '%d') does not exist in '%s'." ) % frame % frameIndex % m_iffFileName );
		return;
	}

	IFFFile::Chunk::ChunkIterator child = frameIt->second;
	IFFFile::Chunk::ChunkIterator it = child->childrenBegin();

	for ( ; it != child->childrenEnd(); it++ )
	{
		if ( it->type().id() == kCHNM )
		{
			std::string channelName;
			it->read( channelName );
			names.push_back( channelName );
		}
	}
}

const IntVectorData * NParticleReader::frameTimes()
{
	if ( !open() )
	{
		msg( Msg::Error, "NParticleReader::attributeNames()", boost::format( "Failed to open '%s'." ) % m_iffFileName );
		return nullptr;
	}

	return m_frames.get();
}

template<typename T, typename F>
typename T::Ptr NParticleReader::filterAttr( const F *attr, float percentage )
{
	if( percentage < 100.0f )
	{
		// percentage filtering (and type conversion if necessary)
		typename T::Ptr result( new T );
		const typename F::ValueType &in = attr->readable();
		typename T::ValueType &out = result->writable();
		int seed = particlePercentageSeed();
		float fraction = percentage / 100.0f;
		Rand48 r;
		r.init( seed );
		for( typename F::ValueType::size_type i=0; i<in.size(); i++ )
		{
			if( r.nextf() <= fraction )
			{
				out.push_back( in[i] );
			}
		}
		return result;
	}

	if( T::staticTypeId()!=F::staticTypeId() )
	{
		// type conversion only
		typename T::Ptr result( new T );
		const typename F::ValueType &in = attr->readable();
		typename T::ValueType &out = result->writable();
		out.resize( in.size() );
		copy( in.begin(), in.end(), out.begin() );
		return result;
	}

	// no filtering of any sort needed
	return typename T::Ptr( (T *)attr );
}

DataPtr NParticleReader::readAttribute( const std::string &name )
{
	if( !open() )
	{
		return nullptr;
	}

	int frameIndex = m_frameParameter->getNumericValue();
	int frame = m_frames->readable()[frameIndex];
	std::map<int, IFFFile::Chunk::ChunkIterator>::const_iterator frameIt = frameToRootChildren.find( frame );
	if( frameIt == frameToRootChildren.end() )
	{
		msg( Msg::Warning, "NParticleReader::readAttribute()", boost::format( "Frame '%d' (index '%d') does not exist in '%s'." ) % frame % frameIndex % m_iffFileName );
		return nullptr;
	}

	bool foundAttr = false;
	std::string channelName;
	IFFFile::Chunk::ChunkIterator attrIt;
	IFFFile::Chunk::ChunkIterator cache = frameIt->second;
	IFFFile::Chunk::ChunkIterator it = cache->childrenBegin();

	for ( ; it != cache->childrenEnd(); it++ )
	{
		if ( it->type().id() == kCHNM )
		{
			it->read( channelName );

			if ( name.compare( channelName ) == 0 )
			{
				attrIt = it;
				foundAttr = true;
				break;
			}
		}
	}

	if ( !foundAttr )
	{
		return nullptr;
	}

	for ( it++; it < attrIt+2 && it != cache->childrenEnd(); it++ )
	{
		int id = it->type().id();
		if ( id != kSIZE && id != kDBLA && id != kDVCA && id != kFVCA )
		{
			msg( Msg::Warning, "NParticleReader::readAttribute()", boost::format( "CHNM '%s' found, but was followed by invalid Tag '%s'." ) % name % it->type().name() );
			return nullptr;
		}
	}

	DataPtr result = nullptr;

	int numParticles = 0;
	(attrIt+1)->read( numParticles );

	switch( (attrIt+2)->type().id() )
	{
		case kDBLA :
			{
				DoubleVectorDataPtr d( new DoubleVectorData );
				d->writable().resize( numParticles );
				(attrIt+2)->read( d->writable() );
				switch( realType() )
				{
					case Native :
					case Double :
						result = filterAttr<DoubleVectorData, DoubleVectorData>( d.get(), particlePercentage() );
						break;
					case Float :
						result = filterAttr<FloatVectorData, DoubleVectorData>( d.get(), particlePercentage() );
						break;
				}
			}
			break;
		case kDVCA :
			{
				V3dVectorDataPtr d( new V3dVectorData );
				/// \todo: by all accounts the line below should be this :
				/// d->writable().resize( numParticles() );
				/// see PDCParticleReader for an explanation
				d->writable().resize( numParticles, V3d( 0 ) );
				(attrIt+2)->read( d->writable() );
				switch( realType() )
				{
					case Native :
					case Double :
						result = filterAttr<V3dVectorData, V3dVectorData>( d.get(), particlePercentage() );
						break;
					case Float :
						result = filterAttr<V3fVectorData, V3dVectorData>( d.get(), particlePercentage() );
						break;
				}
			}
			break;
		case kFVCA :
			{
				V3fVectorDataPtr d( new V3fVectorData );
				/// \todo: by all accounts the line below should be this :
				/// d->writable().resize( numParticles() );
				/// see PDCParticleReader for an explanation
				d->writable().resize( numParticles, V3f( 0 ) );
				(attrIt+2)->read( d->writable() );
				switch( realType() )
				{
					case Native :
					case Double :
						result = filterAttr<V3dVectorData, V3fVectorData>( d.get(), particlePercentage() );
						break;
					case Float :
						result = filterAttr<V3fVectorData, V3fVectorData>( d.get(), particlePercentage() );
						break;
				}
			}
			break;
		default :
			msg( Msg::Error, "NParticleReader::readAttribute()", boost::format( "CHNM '%s' found, but was followed by invalid Tag '%s'." ) % name % (attrIt+2)->type().name() );

	}
	return result;
}

std::string NParticleReader::positionPrimVarName()
{
	std::vector<std::string> names;
	attributeNames( names );

	// The position channel for an nParticle cache should be something like "nParticleShape1_position"
	for( vector<string>::const_iterator it = names.begin(); it != names.end(); it++ )
	{
		if( boost::algorithm::ends_with( *it, "_position") )
		{
			return *it ;
		}
	}

	msg( Msg::Warning, "NParticleReader::posPrimVarName()", "Cannot find name for particle position channel, using P" );
	return "P";
}



