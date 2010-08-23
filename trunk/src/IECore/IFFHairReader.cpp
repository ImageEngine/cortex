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

#include "IECore/IFFHairReader.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/FileNameParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"
#include "IECore/Timer.h"

#include "OpenEXR/ImathRandom.h"

#include <algorithm>
#include <fstream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( IFFHairReader );

const Reader::ReaderDescription<IFFHairReader> IFFHairReader::g_readerDescription( "mchp" );

IFFHairReader::IFFHairReader( )
	:	Reader( "Reads Maya .mc format nCaches", new ObjectParameter( "result", "The loaded object.", new NullObject, CurvesPrimitive::staticTypeId() ) ),
		m_iffFile( 0 ), m_frames( new IntVectorData )
{
	m_frameParameter = new IntParameter( "frameIndex", "Index of the desired frame to be loaded", 0 );
	parameters()->addParameter( m_frameParameter );
	
	IntParameter::PresetsContainer realTypePresets;
	realTypePresets.push_back( IntParameter::Preset( "native", Native ) );
	realTypePresets.push_back( IntParameter::Preset( "float", Float ) );
	realTypePresets.push_back( IntParameter::Preset( "double", Double ) );
	m_realTypeParameter = new IntParameter(
		"realType",
		"The type of data to use to represent real values.",
		Native,
		Native,
		Double,
		realTypePresets,
		true
	);
	parameters()->addParameter( m_realTypeParameter );
}

IFFHairReader::IFFHairReader( const std::string &fileName )
	:	Reader( "Reads Maya .mc format nCaches", new ObjectParameter( "result", "The loaded object.", new NullObject, CurvesPrimitive::staticTypeId() ) ),
		m_iffFile( 0 ), m_frames( new IntVectorData )
{
	m_fileNameParameter->setTypedValue( fileName );
	
	m_frameParameter = new IntParameter( "frameIndex", "Index of the desired frame to be loaded", 0 );
	parameters()->addParameter( m_frameParameter );

	IntParameter::PresetsContainer realTypePresets;
	realTypePresets.push_back( IntParameter::Preset( "native", Native ) );
	realTypePresets.push_back( IntParameter::Preset( "float", Float ) );
	realTypePresets.push_back( IntParameter::Preset( "double", Double ) );
	m_realTypeParameter = new IntParameter(
		"realType",
		"The type of data to use to represent real values.",
		Native,
		Native,
		Double,
		realTypePresets,
		true
	);
	parameters()->addParameter( m_realTypeParameter );
}

IFFHairReader::~IFFHairReader()
{
}

bool IFFHairReader::canRead( const std::string &fileName )
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

bool IFFHairReader::open()
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
			if ( child->type().id() == kSTIM )
			{
				child->read( m_header.startTime );
			}
			else if ( child->type().id() == kETIM )
			{
				child->read( m_header.endTime );
			}
			else if ( child->type().id() == kTYPE )
			{
				child->read( m_header.type );
			}
			else if ( child->type().id() == kRATE )
			{
				child->read( m_header.rate );
			}
		}
		
		m_frames->writable().clear();
		frameToRootChildren.clear();
		
		IFFFile::Chunk::ChunkIterator bodyIt = headerIt;
		for ( ; bodyIt != root->childrenEnd(); bodyIt++ )
		{
			if ( bodyIt->groupName().id() == kHAIR )
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
		
		sort( m_frames->writable().begin(), m_frames->writable().end() );
		
		m_header.valid = true;
		m_iffFileName = fileName();
	}
	
	return m_header.valid && m_iffFileName == fileName();
}

ObjectPtr IFFHairReader::doOperation( const CompoundObject * operands )
{
	if( !open() )
	{
		return 0;
	}
	
	int frameIndex = m_frameParameter->getNumericValue();
	int frame = m_frames->readable()[frameIndex];
	std::map<int, IFFFile::Chunk::ChunkIterator>::const_iterator frameIt = frameToRootChildren.find( frame );
	if( frameIt == frameToRootChildren.end() )
	{
		msg( Msg::Warning, "IFFHairReader::loadFrame()", boost::format( "Frame '%d' (index '%d') does not exist in '%s'." ) % frame % frameIndex % m_iffFileName );
		return 0;
	}
	
	IFFFile::Chunk::ChunkIterator hair = frameIt->second;
	
	if ( hair->groupName().id() != kHAIR )
	{
		return 0;
	}
	
	int numHairs = 0;
	int numCVs = 0;
	int positionCVs = 0;
	bool havePosition = false;
	IntVectorDataPtr vertsPerCurve( new IntVectorData );
	V3dVectorDataPtr pData( new V3dVectorData );
	V3dVectorDataPtr vData( new V3dVectorData );
	
	IFFFile::Chunk::ChunkIterator child = hair->childrenBegin();
	for ( int i=0; child != hair->childrenEnd(); child++ )
	{
		if ( child->type().id() == kNMHA )
		{
			child->read( numHairs );
		}
		else if ( child->type().id() == kNMCV )
		{
			child->read( numCVs );			
		}
		else if ( child->type().id() == kPOSS )
		{
			loadData( child, pData, numCVs );
			
			vertsPerCurve->writable().push_back( numCVs );
			havePosition = true;
			positionCVs = numCVs;
		}
		else if ( child->type().id() == kVELS )
		{
			// skip this velocity since there was no associated position
			if ( !havePosition )
			{
				msg( Msg::Error, "IFFHairReader::loadFrame()", boost::format( "Found velocity channel with no associated position channel." ) );
				continue;
			}
			
			// CVs don't match. Use 0 velocity instead.
			if ( positionCVs != numCVs )
			{
				msg( Msg::Error, "IFFHairReader::loadFrame()", boost::format( "Found velocity channel with %d CVs following position channel with %d CVs. Inserting 0 velocity for Hair %d." ) % numCVs % positionCVs % i );
				loadData( child, vData, positionCVs, false );
			}
			else
			{
				loadData( child, vData, numCVs );
			}
			
			// increment the hair count
			i++;
			havePosition = false;
		}
	}
	
	if ( vertsPerCurve->readable().size() != (size_t)numHairs )
	{
		throw IOException( (boost::format( "IFFHairReader::loadFrame(): Found %d hairs with 0 CVs while reading %s") % ( (size_t)numHairs - vertsPerCurve->readable().size() ) % fileName() ).str() );
	}
	
	CurvesPrimitivePtr curves = new CurvesPrimitive( vertsPerCurve );
	
	DataPtr pConverted = 0;
	DataPtr vConverted = 0;
	switch( realType() )
	{
		case Native :
		case Double :
			pConverted = convertAttr<V3dVectorData, V3dVectorData>( pData );
			vConverted = convertAttr<V3dVectorData, V3dVectorData>( vData );
			break;
		case Float :
			pConverted = convertAttr<V3fVectorData, V3dVectorData>( pData );
			vConverted = convertAttr<V3fVectorData, V3dVectorData>( vData );
			break;
	}
	
	curves->variables[ "P" ] = PrimitiveVariable( PrimitiveVariable::Vertex, pConverted );
	curves->variables[ "velocity" ] = PrimitiveVariable( PrimitiveVariable::Vertex, vConverted );
	
	return curves;
}

void IFFHairReader::loadData( IFFFile::Chunk::ChunkIterator channel, V3dVectorData * channelData, int numCVs, bool fromFile )
{
	V3dVectorDataPtr data( new V3dVectorData );
	data->writable().resize( numCVs, V3d( 0 ) );
	
	if ( fromFile )
	{
		channel->read( data->writable() );
	}
	
	for ( unsigned int i=0; i < data->readable().size(); i++ )
	{
		channelData->writable().push_back( data->readable()[ i ] );
	}
}

unsigned long IFFHairReader::numHairs()
{
	if( !open() )
	{
		return 0;
	}
	
	int numHairs = 0;
	int frameIndex = m_frameParameter->getNumericValue();
	int frame = m_frames->readable()[frameIndex];
	std::map<int, IFFFile::Chunk::ChunkIterator>::const_iterator frameIt = frameToRootChildren.find( frame );
	if( frameIt == frameToRootChildren.end() )
	{
		msg( Msg::Warning, "IFFHairReader::numHairs()", boost::format( "Frame '%d' (index '%d') does not exist in '%s'." ) % frame % frameIndex % m_iffFileName );
		return 0;
	}

	IFFFile::Chunk::ChunkIterator child = frameIt->second;
	IFFFile::Chunk::ChunkIterator it = child->childrenBegin();

	for ( ; it != child->childrenEnd(); it++ )
	{
		if ( it->type().id() == kNMHA )
		{
			it->read( numHairs );
			break;
		}
	}
	
	return numHairs;
}

ConstIntVectorDataPtr IFFHairReader::frameTimes()
{
	if ( !open() )
	{
		msg( Msg::Error, "IFFHairReader::frameTimes()", boost::format( "Failed to open '%s'." ) % m_iffFileName );
		return 0;
	}
	
	return m_frames;
}

template<typename T, typename F>
IntrusivePtr<T> IFFHairReader::convertAttr( IntrusivePtr<F> attr )
{
	if( T::staticTypeId() != F::staticTypeId() )
	{
		IntrusivePtr<T> result( new T );
		const typename F::ValueType &in = attr->readable();
		typename T::ValueType &out = result->writable();
		out.resize( in.size() );
		copy( in.begin(), in.end(), out.begin() );
		return result;
	}
	
	// no type conversion necessary
	return IntrusivePtr<T>( (T *)attr.get() );
}

IFFHairReader::RealType IFFHairReader::realType() const
{
	return RealType( m_realTypeParameter->getNumericValue() );
}
