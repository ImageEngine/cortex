//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/filesystem/convenience.hpp"

#include "IECore/CompoundParameter.h"
#include "IECore/DeepImageWriter.h"
#include "IECore/FileNameParameter.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( DeepImageWriter )

DeepImageWriter::DeepImageWriter( const std::string &description ) : Parameterised( description )
{
	m_fileNameParameter = new FileNameParameter( "fileName", "The filename to be written to.", "", "", false );
	
	StringVectorDataPtr defaultChannels = new StringVectorData();
	std::vector<std::string> &channels = defaultChannels->writable();
	channels.push_back( "R" );
	channels.push_back( "G" );
	channels.push_back( "B" );
	channels.push_back( "A" );
	m_channelsParameter = new StringVectorParameter( "channelNames", "The list of channels to write.", defaultChannels );
	
	m_resolutionParameter = new V2iParameter( "resolution", "The resolution of the image to write.", new V2iData( Imath::V2i( 2048, 1556 ) ) );
	
	parameters()->addParameter( m_fileNameParameter );
	parameters()->addParameter( m_channelsParameter );
	parameters()->addParameter( m_resolutionParameter );
	parameters()->addParameter( new M44fParameter( "worldToCameraMatrix", "world to camera space transformation matrix", new M44fData() ) );
	parameters()->addParameter( new M44fParameter( "worldToNDCMatrix", "world to screen space projection matrix", new M44fData() ) );
}

const std::string &DeepImageWriter::fileName() const
{
	return m_fileNameParameter->getTypedValue();
}

StringVectorParameter *DeepImageWriter::channelNamesParameter()
{
	return m_channelsParameter;
}

const StringVectorParameter *DeepImageWriter::channelNamesParameter() const
{
	return m_channelsParameter;
}

V2iParameter *DeepImageWriter::resolutionParameter()
{
	return m_resolutionParameter;
}

const V2iParameter *DeepImageWriter::resolutionParameter() const
{
	return m_resolutionParameter;
}

M44fParameter *DeepImageWriter::worldToCameraParameter()
{
	return parameters()->parameter<M44fParameter>( "worldToCameraMatrix" );
}

const M44fParameter *DeepImageWriter::worldToCameraParameter() const
{
	return parameters()->parameter<M44fParameter>( "worldToCameraMatrix" );
}

M44fParameter *DeepImageWriter::worldToNDCParameter()
{
	return parameters()->parameter<M44fParameter>( "worldToNDCMatrix" );
}

const M44fParameter *DeepImageWriter::worldToNDCParameter() const
{
	return parameters()->parameter<M44fParameter>( "worldToNDCMatrix" );
}

void DeepImageWriter::writePixel( int x, int y, const DeepPixel *pixel )
{
	if ( !pixel || !pixel->numSamples() )
	{
		return;
	}
	
	if ( pixel->numChannels() != m_channelsParameter->getTypedValue().size() )
	{
		throw InvalidArgumentException( std::string( "DeepPixel does not have the correct channels." ) );
	}
	
	doWritePixel( x, y, pixel );
}

void DeepImageWriter::registerDeepImageWriter( const std::string &extensions, CanWriteFn canWrite, CreatorFn creator, TypeId typeId )
{
	assert( canWrite );
	assert( creator );
	assert( typeId != InvalidTypeId );

	ExtensionsToFnsMap *m = extensionsToFns();
	std::vector<std::string> splitExt;
	boost::split( splitExt, extensions, boost::is_any_of( " " ) );
	DeepImageWriterFns w;
	w.creator = creator;
	w.canWrite = canWrite;
	w.typeId = typeId;
	for ( std::vector<std::string>::const_iterator it=splitExt.begin(); it != splitExt.end(); it++ )
	{
		m->insert( ExtensionsToFnsMap::value_type( "." + *it, w ) );
	}
}

DeepImageWriterPtr DeepImageWriter::create( const std::string &fileName )
{
	/// \todo We can stop using this deprecated form when we no longer need to be compatible
	/// with boost 1.43. At that point we can use path.extension().string() and compile with
	/// BOOST_FILESYSTEM_NO_DEPRECATED.
	std::string ext = boost::filesystem::extension( boost::filesystem::path( fileName ) );

	ExtensionsToFnsMap *m = extensionsToFns();
	ExtensionsToFnsMap::const_iterator it = m->find( ext );

	if ( it == m->end() )
	{
		throw Exception( std::string( "Unrecognized output file format '") + ext + "'!" );
	}

	for ( it=m->begin(); it != m->end(); it++ )
	{
		if ( it->first == ext )
		{
			if ( it->second.canWrite( fileName ) )
			{
				return it->second.creator( fileName );
			}
		}
	}

	throw Exception( std::string( "Unable to find DeepImageWriter able to write to file of type '") + ext + "'!" );
}

void DeepImageWriter::supportedExtensions( std::vector<std::string> &extensions )
{
	extensions.clear();
	ExtensionsToFnsMap *m = extensionsToFns();
	for ( ExtensionsToFnsMap::const_iterator it=m->begin(); it != m->end(); it++ )
	{
		extensions.push_back( it->first.substr( 1 ) );
	}
}

void DeepImageWriter::supportedExtensions( TypeId typeId, std::vector<std::string> &extensions )
{
	extensions.clear();
	ExtensionsToFnsMap *m = extensionsToFns();

	const std::set< TypeId > &derivedTypes = RunTimeTyped::derivedTypeIds( typeId );

	for ( ExtensionsToFnsMap::const_iterator it=m->begin(); it != m->end(); it++ )
	{
		if ( it->second.typeId == typeId || std::find( derivedTypes.begin(), derivedTypes.end(), it->second.typeId ) != derivedTypes.end() )
		{
			extensions.push_back( it->first.substr( 1 ) );
		}
	}
}

DeepImageWriter::ExtensionsToFnsMap *DeepImageWriter::extensionsToFns()
{
	static ExtensionsToFnsMap *m = new ExtensionsToFnsMap;
	return m;
}
