//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2017, Image Engine Design Inc. All rights reserved.
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

#include "boost/tokenizer.hpp"

#include "OpenImageIO/imageio.h"
OIIO_NAMESPACE_USING

#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"
#include "IECore/BoxOps.h"

#include "IECoreImage/ColorAlgo.h"
#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/ImageReader.h"
#include "IECoreImage/OpenImageIOAlgo.h"

using namespace std;
using namespace boost;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( ImageReader );

////////////////////////////////////////////////////////////////////////////////
// ImageReader::Implementation
////////////////////////////////////////////////////////////////////////////////

class ImageReader::Implementation : public IECore::RefCounted
{

	public :

		Implementation( const ImageReader *reader ) : m_reader( reader ), m_inputFile( nullptr ), m_inputFileName( "" )
		{
		}

		virtual ~Implementation()
		{
			ImageInput::destroy( m_inputFile );
			m_reader = nullptr;
		}

		static bool canRead( const std::string &filename )
		{
			bool result = false;

			if( ImageInput *input = ImageInput::create( filename ) )
			{
				if( !input->valid_file( filename ) )
				{
					result = false;
				}
				else
				{
					ImageSpec spec;
					input->open( filename, spec );
					result = !spec.deep;
				}

				ImageInput::destroy( input );
			}

			return result;
		}

		bool isComplete()
		{
			if( !open( /* throwOnFailure */ false ) )
			{
				return false;
			}

			try
			{
				const ImageSpec &spec = m_inputFile->spec();

				std::vector<float> data;

				if( spec.tile_width )
				{
					// if the last tile is there, its complete
					data.resize( spec.tile_width * spec.tile_height * spec.nchannels );
					return m_inputFile->read_tile(
						spec.width + spec.x - spec.tile_width,
						spec.height + spec.y - spec.tile_height,
						/* z */ 0,
						&data[0]
					);
				}
				else
				{
					// if the last scanline is there, its complete
					data.resize( spec.width * spec.nchannels );
					return m_inputFile->read_scanline( spec.height + spec.y - 1, /* z */ 0, &data[0] );
				}
			}
			catch( ... )
			{
				return false;
			}
		}

		void channelNames( std::vector<std::string> &names )
		{
			open( /* throwOnFailure */ true );

			const ImageSpec &spec = m_inputFile->spec();

			names.clear();
			names.resize( spec.nchannels );
			std::copy( spec.channelnames.begin(), spec.channelnames.end(), names.begin() );
		}

		Imath::Box2i dataWindow()
		{
			open( /* throwOnFailure */ true );

			const ImageSpec &spec = m_inputFile->spec();

			return Imath::Box2i(
				Imath::V2i( spec.x, spec.y ),
				Imath::V2i( spec.width + spec.x - 1, spec.height + spec.y - 1 )
			);
		}

		Imath::Box2i displayWindow()
		{
			open( /* throwOnFailure */ true );

			const ImageSpec &spec = m_inputFile->spec();

			return Imath::Box2i(
				Imath::V2i( spec.full_x, spec.full_y ),
				Imath::V2i( spec.full_x + spec.full_width - 1, spec.full_y + spec.full_height - 1 )
			);
		}

		void updateHeader( CompoundObject *header )
		{
			open( /* throwOnFailure */ true );

			CompoundDataPtr metadata = new CompoundData();
			updateMetadata( metadata.get() );
			auto &members = header->members();
			for( const auto &item : metadata->writable() )
			{
				members[item.first] = item.second;
			}
		}

		void updateMetadata( CompoundData *metadata )
		{
			const ImageSpec &spec = m_inputFile->spec();

			auto &members = metadata->writable();
			for ( const auto &param : spec.extra_attribs )
			{
				const OpenImageIOAlgo::DataView dataView( param );
				if( dataView.data )
				{
					addMetadata( param.name().string(), dataView.data, metadata );
				}
			}

			members["displayWindow"] = new Box2iData( displayWindow() );
			members["dataWindow"] = new Box2iData( dataWindow() );
		}

		DataPtr readChannel( const std::string &name, bool raw )
		{
			open( /* throwOnFailure */ true );

			const ImageSpec &spec = m_inputFile->spec();

			const auto channelIt = find( spec.channelnames.begin(), spec.channelnames.end(), name );
			if( channelIt == spec.channelnames.end() )
			{
				throw InvalidArgumentException( "Image Reader : Non-existent image channel \"" + name + "\" requested." );
			}

			size_t channelIndex = channelIt - spec.channelnames.begin();

			if( raw )
			{
				switch( spec.format.basetype )
				{
					case TypeDesc::UCHAR :
					{
						return readTypedChannel<unsigned char>( channelIndex, spec.format );
					}
					case TypeDesc::CHAR :
					{
						return readTypedChannel<char>( channelIndex, spec.format );
					}
					case TypeDesc::USHORT :
					{
						return readTypedChannel<unsigned short>( channelIndex, spec.format );
					}
					case TypeDesc::SHORT :
					{
						return readTypedChannel<short>( channelIndex, spec.format );
					}
					case TypeDesc::UINT :
					{
						return readTypedChannel<unsigned int>( channelIndex, spec.format );
					}
					case TypeDesc::INT :
					{
						return readTypedChannel<int>( channelIndex, spec.format );
					}
					case TypeDesc::HALF :
					{
						return readTypedChannel<half>( channelIndex, spec.format );
					}
					case TypeDesc::FLOAT :
					{
						return readTypedChannel<float>( channelIndex, spec.format );
					}
					case TypeDesc::DOUBLE :
					{
						return readTypedChannel<double>( channelIndex, spec.format );
					}
					default :
					{
						throw IECore::IOException( ( boost::format( "ImageReader : Unsupported data type \"%d\"" ) % spec.format ).str() );
					}
				}
			}
			else
			{
				DataPtr data = readTypedChannel<float>( channelIndex, TypeDesc::FLOAT );
				if( (int)channelIndex != spec.alpha_channel && (int)channelIndex != spec.z_channel )
				{
					std::string linearColorSpace = OpenImageIOAlgo::colorSpace( "", spec );
					std::string currentColorSpace = OpenImageIOAlgo::colorSpace( m_inputFile->format_name(), spec );
					ColorAlgo::transformChannel( data.get(), currentColorSpace, linearColorSpace );
				}

				return data;
			}
		}

	private :

		template<class T>
		DataPtr readTypedChannel( size_t channelIndex, TypeDesc dataType )
		{
			typedef TypedData<vector<T> > DataType;
			typename DataType::Ptr data = new DataType;

			bool status;

			const ImageSpec &spec = m_inputFile->spec();

			data->writable().resize( spec.width * spec.height );

			if( spec.tile_width )
			{
				status = m_inputFile->read_tiles(
					/* xbegin */ spec.x, /* xend */ spec.width + spec.x,
					/* ybegin */ spec.y, /* yend */ spec.height + spec.y,
					/* zbegin */ 0, /* zend */ 1,
					/* chbegin */ channelIndex, /* chend */ channelIndex + 1,
					/* format */ dataType,
					/* data */ &( data->writable()[0] )
				);
			}
			else
			{
				status = m_inputFile->read_scanlines(
					/* ybegin */ spec.y, /* yend */spec.height + spec.y,
					/* z */ 0,
					/* chbegin */ channelIndex, /* chend */ channelIndex + 1,
					/* format */ dataType,
					/* data */ &( data->writable()[0] )
				);
			}

			if( !status )
			{
				throw IOException( string( "ImageReader : Failed to read channel \"" ) + m_inputFile->spec().channelnames[channelIndex] + "\". " + m_inputFile->geterror() );
			}

			return data;
		}

		void addMetadata( const std::string &name, DataPtr data, CompoundData *metadata )
		{
			// search for '.'
			std::string::size_type pos = name.find_first_of( "." );

			auto &members = metadata->writable();

			if ( std::string::npos != pos )
			{
				std::string thisName = name.substr( 0, pos );
				std::string newName = name.substr( pos+1, name.size() );

				CompoundData *newMetaData = metadata->member<CompoundData>( thisName, /* throwExceptions = */ true, /* createIfMissing = */ true );
				addMetadata( newName, data, newMetaData );
				return;
			}

			members[name] = data;
		}

		// Tries to open the file, returning true on success and false on failure. On success,
		// m_inputFile will be valid. If throwOnFailure is true then a descriptive
		// Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false )
		{
			if( m_inputFile && m_reader->fileName() == m_inputFileName )
			{
				// we already opened the right file successfully
				return true;
			}

			ImageInput::destroy( m_inputFile );
			m_inputFileName = "";

			m_inputFile = ImageInput::open( m_reader->fileName() );
			if( !m_inputFile )
			{
				ImageInput::destroy( m_inputFile );
				if( !throwOnFailure )
				{
					return false;
				}
				else
				{
					throw IOException( string( "Failed to open file \"" ) + m_reader->fileName() + "\". " + geterror() );
				}
			}

			m_inputFileName = m_reader->fileName();

			return true;
		}

		const ImageReader *m_reader;
		ImageInput *m_inputFile;
		std::string m_inputFileName;

};


////////////////////////////////////////////////////////////////////////////////
// ImageReader
////////////////////////////////////////////////////////////////////////////////

const Reader::ReaderDescription<ImageReader> ImageReader::g_readerDescription( OpenImageIOAlgo::extensions() );

ImageReader::ImageReader() :
	Reader( "Reads image files using OpenImageIO.", new ObjectParameter( "result", "The loaded object", new NullObject, ImagePrimitive::staticTypeId() ) ),
	m_implementation( nullptr )
{
	constructCommon();
}

ImageReader::ImageReader( const string &fileName ) :
	Reader( "Reads image files using OpenImageIO.", new ObjectParameter( "result", "The loaded object", new NullObject, ImagePrimitive::staticTypeId() ) ),
	m_implementation( nullptr )
{
	constructCommon();

	m_fileNameParameter->setTypedValue( fileName );
}

void ImageReader::constructCommon()
{
	m_implementation = new ImageReader::Implementation( this );

	m_channelNamesParameter = new StringVectorParameter(
		"channels",
		"The names of all channels to load from the file. If the list is empty (the default value) "
		"then all channels are loaded."
	);

	m_rawChannelsParameter = new BoolParameter(
		"rawChannels",
		"Specifies if the returned data channels should be what's stored in the file. That's not possible when "
		"the image pixels are not byte aligned.",
		false
	);

	parameters()->addParameter( m_channelNamesParameter );
	parameters()->addParameter( m_rawChannelsParameter );
}

ImageReader::~ImageReader()
{
}

bool ImageReader::canRead( const std::string &filename )
{
	return Implementation::canRead( filename );
}

void ImageReader::channelNames( std::vector<std::string> &names )
{
	m_implementation->channelNames( names );
}

bool ImageReader::isComplete()
{
	return m_implementation->isComplete();
}

Imath::Box2i ImageReader::dataWindow()
{
	return m_implementation->dataWindow();
}

Imath::Box2i ImageReader::displayWindow()
{
	return m_implementation->displayWindow();
}

ObjectPtr ImageReader::doOperation( const CompoundObject *operands )
{
	bool rawChannels = operands->member< BoolData >( "rawChannels" )->readable();

	ImagePrimitivePtr image = new ImagePrimitive( dataWindow(), displayWindow() );

	vector<string> channelNames;
	channelsToRead( channelNames );

	for( size_t ci = 0, cend = channelNames.size(); ci != cend; ++ci )
	{
		DataPtr d = m_implementation->readChannel( channelNames[ci], rawChannels );
		assert( d  );
		assert( rawChannels || d->typeId()==FloatVectorDataTypeId );

		assert( image->channelValid( d.get() ) );

		image->channels[ channelNames[ci] ] = d;
	}

	m_implementation->updateMetadata( image->blindData() );

	return image;
}

DataPtr ImageReader::readChannel( const std::string &name, bool raw )
{
	return m_implementation->readChannel( name, raw );
}

void ImageReader::channelsToRead( vector<string> &names )
{
	vector<string> allNames;
	channelNames( allNames );

	ConstStringVectorParameterPtr p = channelNamesParameter();
	const StringVectorData *d = static_cast<const StringVectorData *>( p->getValue() );

	// give all channels when no list is provided
	if (!d->readable().size())
	{
		names = allNames;
		return;
	}

	// otherwise, copy in the requested names from the parameter set.
	// this is intersection(A, D)
	names.clear();
	for (vector<string>::const_iterator it = d->readable().begin(); it != d->readable().end(); it++)
	{
		if (find(allNames.begin(), allNames.end(), *it) != allNames.end())
		{
			names.push_back(*it);
		}
	}
}

StringVectorParameter *ImageReader::channelNamesParameter()
{
	return m_channelNamesParameter.get();
}

const StringVectorParameter *ImageReader::channelNamesParameter() const
{
	return m_channelNamesParameter.get();
}

BoolParameter *ImageReader::rawChannelsParameter()
{
	return m_rawChannelsParameter.get();
}

const BoolParameter *ImageReader::rawChannelsParameter() const
{
	return m_rawChannelsParameter.get();
}

CompoundObjectPtr ImageReader::readHeader()
{
	std::vector<std::string> cn;
	channelNames( cn );

	CompoundObjectPtr header = Reader::readHeader();

	m_implementation->updateHeader( header.get() );

	header->members()["channelNames"] = new StringVectorData( cn );

	return header;
}
