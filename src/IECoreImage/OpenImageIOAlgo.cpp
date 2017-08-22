//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, John Haddon. All rights reserved.
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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
#include "OpenImageIO/ustring.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/TimeCodeData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreImage/OpenImageIOAlgo.h"

using namespace OIIO;
using namespace IECore;

namespace IECoreImage
{

namespace OpenImageIOAlgo
{

OIIO::TypeDesc::VECSEMANTICS vecSemantics( IECore::GeometricData::Interpretation interpretation )
{
	switch( interpretation )
	{
		case GeometricData::Point :
			return TypeDesc::POINT;
		case GeometricData::Normal :
			return TypeDesc::NORMAL;
		case GeometricData::Vector :
			return TypeDesc::VECTOR;
		case GeometricData::Color :
			return TypeDesc::COLOR;
		default :
			return TypeDesc::NOXFORM;
	}
}

IECore::GeometricData::Interpretation geometricInterpretation( OIIO::TypeDesc::VECSEMANTICS semantics )
{
	switch( semantics )
	{
		case TypeDesc::NOXFORM :
			return GeometricData::Numeric;
		case TypeDesc::COLOR :
			return GeometricData::Color;
		case TypeDesc::POINT :
			return GeometricData::Point;
		case TypeDesc::VECTOR :
			return GeometricData::Vector;
		case TypeDesc::NORMAL :
			return GeometricData::Normal;
		default :
			return GeometricData::Numeric;
	}
}

std::string extensions()
{
	static std::string g_extensions;
	if( !g_extensions.empty() )
	{
		return g_extensions;
	}

	std::string attr;
	getattribute( "extension_list", attr );

	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	Tokenizer formats( attr, boost::char_separator<char>( ";" ) );
	for( Tokenizer::const_iterator fIt = formats.begin(), eFIt = formats.end(); fIt != eFIt; ++fIt )
	{
		size_t colonPos = fIt->find( ':' );
		if( colonPos != std::string::npos )
		{
			std::string formatExtensions = fIt->substr( colonPos + 1 );
			Tokenizer extTok( formatExtensions, boost::char_separator<char>( "," ) );
			for( Tokenizer::iterator token = extTok.begin(), tokEnd = extTok.end(); token != tokEnd; ++token )
			{
				g_extensions += " " + *token;
			}
		}
	}

	return g_extensions;
}

ColorConfig *colorConfig()
{
	static ColorConfig *g_colorConfig;
	if( !g_colorConfig )
	{
		g_colorConfig = new ColorConfig();
	}

	return g_colorConfig;
}

std::string colorSpace( const std::string &fileFormat, const OIIO::ImageSpec &spec )
{
	const char *linear = colorConfig()->getColorSpaceNameByRole( "scene_linear" );
	if( linear == nullptr )
	{
		linear = "Linear";
	}

	const char *log = colorConfig()->getColorSpaceNameByRole( "compositing_log" );
	if( log == nullptr )
	{
		log = "Cineon";
	}

	const char *display = colorConfig()->getColorSpaceNameByRole( "color_picking" );
	if( display == nullptr )
	{
		display = "sRGB";
	}

	if( fileFormat == "bmp" )
	{
		return display;
	}
	else if( fileFormat == "cineon" )
	{
		return log;
	}
	else if( fileFormat == "dpx" )
	{
		if( spec.format.basetype == TypeDesc::UINT16 )
		{
			int bps = spec.get_int_attribute( "oiio:BitsPerSample", 0 );
			if( bps == 10 || bps == 12 )
			{
				return log;
			}
		}

		return display;
	}
	else if( fileFormat == "jpeg" || fileFormat == "jpeg2000" )
	{
		// greyscale jpegs shouldn't get color conversion
		if( spec.nchannels == 1 && spec.channelnames[0] == "Y" )
		{
			return linear;
		}

		return display;
	}
	else if( fileFormat == "png" )
	{
		return display;
	}
	else if( fileFormat == "tiff" )
	{
		if( spec.format.basetype == TypeDesc::UINT8 || spec.format.basetype == TypeDesc::UINT16 )
		{
			return display;
		}

		return linear;
	}

	return linear;
}

DataView::DataView()
	:	data( nullptr ), rawData( nullptr ), m_charPointer( nullptr )
{
}

DataView::DataView( const IECore::Data *d, bool createUStrings, bool copyData )
	:	data( copyData ? data->copy() : nullptr ), rawData( nullptr ), m_charPointer( nullptr )
{
	switch( d ? d->typeId() : IECore::InvalidTypeId )
	{

		// Simple data

		case CharDataTypeId :
			type = TypeDesc::CHAR;
			rawData = static_cast<const CharData *>( d )->baseReadable();
			break;
		case UCharDataTypeId :
			type = TypeDesc::UCHAR;
			rawData = static_cast<const UCharData *>( d )->baseReadable();
			break;
		case StringDataTypeId :
			type = TypeDesc::TypeString;
			m_charPointer = static_cast<const StringData *>( d )->readable().c_str();
			if( createUStrings )
			{
				m_charPointer = ustring( m_charPointer ).c_str();
			}
			rawData = &m_charPointer;
			break;
		case UShortDataTypeId :
			type = TypeDesc::USHORT;
			rawData = static_cast<const UShortData *>( d )->baseReadable();
			break;
		case ShortDataTypeId :
			type = TypeDesc::SHORT;
			rawData = static_cast<const ShortData *>( d )->baseReadable();
			break;
		case UIntDataTypeId :
			type = TypeDesc::UINT;
			rawData = static_cast<const UIntData *>( d )->baseReadable();
			break;
		case HalfDataTypeId :
			type = TypeDesc::HALF;
			rawData = static_cast<const HalfData *>( d )->baseReadable();
			break;
		case IntDataTypeId :
			type = TypeDesc::TypeInt;
			rawData = static_cast<const IntData *>( d )->baseReadable();
			break;
		case FloatDataTypeId :
			type = TypeDesc::TypeFloat;
			rawData = static_cast<const FloatData *>( d )->baseReadable();
			break;
		case DoubleDataTypeId :
			type = TypeDesc::DOUBLE;
			rawData = static_cast<const DoubleData *>( d )->baseReadable();
			break;
		case V2iDataTypeId :
			type = TypeDesc( TypeDesc::INT, TypeDesc::VEC2 );
			rawData = static_cast<const V2iData *>( d )->baseReadable();
			break;
		case V3iDataTypeId :
			type = TypeDesc( TypeDesc::INT, TypeDesc::VEC3 );
			rawData = static_cast<const V3iData *>( d )->baseReadable();
			break;
		case Box2iDataTypeId :
			type = TypeDesc( TypeDesc::INT, TypeDesc::VEC2, TypeDesc::NOXFORM, 2 );
			rawData = static_cast<const Box2iData *>( d )->baseReadable();
			break;
		case V2fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::VEC2, vecSemantics( static_cast<const V2fData *>( d )->getInterpretation() ) );
			rawData = static_cast<const V2fData *>( d )->baseReadable();
			break;
		case V3fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::VEC3, vecSemantics( static_cast<const V3fData *>( d )->getInterpretation() ) );
			rawData = static_cast<const V3fData *>( d )->baseReadable();
			break;
		case Box2fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::VEC2, TypeDesc::NOXFORM, 2 );
			rawData = static_cast<const Box2fData *>( d )->baseReadable();
			break;
		case M33fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::MATRIX33 );
			rawData = static_cast<const M33fData *>( d )->baseReadable();
			break;
		case M44fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::MATRIX44 );
			rawData = static_cast<const M44fData *>( d )->baseReadable();
			break;
		case V2dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::VEC2, vecSemantics( static_cast<const V2dData *>( d )->getInterpretation() ) );
			rawData = static_cast<const V2dData *>( d )->baseReadable();
			break;
		case V3dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::VEC3, vecSemantics( static_cast<const V3dData *>( d )->getInterpretation() ) );
			rawData = static_cast<const V3dData *>( d )->baseReadable();
			break;
		case Box2dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::VEC2, TypeDesc::NOXFORM, 2 );
			rawData = static_cast<const Box2dData *>( d )->baseReadable();
			break;
		case M33dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::MATRIX33 );
			rawData = static_cast<const M33dData *>( d )->baseReadable();
			break;
		case M44dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::MATRIX44 );
			rawData = static_cast<const M44dData *>( d )->baseReadable();
			break;
		case Color3fDataTypeId :
			type = TypeDesc::TypeColor;
			rawData = static_cast<const Color3fData *>( d )->baseReadable();
			break;
		case TimeCodeDataTypeId :
			type = TypeDesc::TypeTimeCode;
			rawData = static_cast<const TimeCodeData *>( d )->baseReadable();
			break;

		// Vector data

		case DoubleVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::DOUBLE,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const DoubleVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const DoubleVectorData *>( d )->baseReadable();
			break;
		case FloatVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const FloatVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const FloatVectorData *>( d )->baseReadable();
			break;
		case HalfVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::HALF,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const HalfVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const HalfVectorData *>( d )->baseReadable();
			break;
		case IntVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::INT,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const IntVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const IntVectorData *>( d )->baseReadable();
			break;
		case UIntVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::UINT,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const UIntVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const UIntVectorData *>( d )->baseReadable();
			break;
		case CharVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::CHAR,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const CharVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const CharVectorData *>( d )->baseReadable();
			break;
		case UCharVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::UCHAR,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const UCharVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const UCharVectorData *>( d )->baseReadable();
			break;
		case V3fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::VEC3,
				vecSemantics( static_cast<const V3fVectorData *>( d )->getInterpretation() ),
				static_cast<const V3fVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const V3fVectorData *>( d )->baseReadable();
			break;
		case Color3fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::VEC3,
				TypeDesc::COLOR,
				static_cast<const Color3fVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const Color3fVectorData *>( d )->baseReadable();
			break;
		case M44fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::MATRIX44,
				TypeDesc::NOSEMANTICS,
				static_cast<const M44fVectorData *>( d )->readable().size()
			);
			rawData = static_cast<const M44fVectorData *>( d )->baseReadable();
			break;

		default :
			// Default initialisers above did what we need already
			break;

	}
}

DataView::DataView( const OIIO::ParamValue &param )
	:	type( param.type() ), data( nullptr ), rawData( param.data() ), m_charPointer( nullptr )
{
	switch ( type.basetype )
	{
		case TypeDesc::CHAR :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				data = new CharData( *static_cast<const char *>( rawData ) );
			}
			break;
		}
		case TypeDesc::UCHAR :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				data = new UCharData( *static_cast<const unsigned char *>( rawData ) );
			}
			break;
		}
		case TypeDesc::STRING :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				data = new StringData( static_cast<const ustring *>( rawData )->c_str() );
			}
			break;
		}
		case TypeDesc::USHORT :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				data = new UShortData( *static_cast<const unsigned short *>( rawData ) );
			}
			break;
		}
		case TypeDesc::SHORT :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				data = new ShortData( *static_cast<const short *>( rawData ) );
			}
			break;
		}
		case TypeDesc::UINT :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				const unsigned *typedData = static_cast<const unsigned *>( rawData );
				if ( type.vecsemantics == TypeDesc::TIMECODE )
				{
					data = new TimeCodeData( Imf::TimeCode( typedData[0], typedData[1] ) );
				}
				else
				{
					data = new UIntData( *typedData );
				}
			}
			break;
		}
		case TypeDesc::INT :
		{
			const int *typedData = static_cast<const int *>( rawData );
			switch ( type.aggregate )
			{
				case TypeDesc::SCALAR :
				{
					if( !type.arraylen )
					{
						data = new IntData( *typedData );
					}
					else
					{
						IntVectorDataPtr vectorData = new IntVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], vectorData->writable().begin() );
						data = vectorData;
					}
					break;
				}
				case TypeDesc::VEC2 :
				{
					if( !type.arraylen )
					{
						data = new V2iData( Imath::V2i( typedData[0], typedData[1] ) );
					}
					else if( type.arraylen  == 2 )
					{
						data = new Box2iData( Imath::Box2i( Imath::V2i( typedData[0], typedData[1] ), Imath::V2i( typedData[2], typedData[3] ) ) );
					}
					else
					{
						V2iVectorDataPtr vectorData = new V2iVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], &vectorData->baseWritable()[0] );
						data = vectorData;
					}

					break;
				}
				case TypeDesc::VEC3 :
				{
					data = new V3iData( Imath::V3i( typedData[0], typedData[1], typedData[2] ) );
					break;
				}
				default :
				{
					break;
				}
			}
			break;
		}
		case TypeDesc::HALF :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				data = new HalfData( *static_cast<const half *>( rawData ) );
			}
			break;
		}
		case TypeDesc::FLOAT :
		{
			const float *typedData = static_cast<const float *>( rawData );
			switch ( type.aggregate )
			{
				case TypeDesc::SCALAR :
				{
					if( !type.arraylen )
					{
						data = new FloatData( *typedData );
					}
					else
					{
						FloatVectorDataPtr vectorData = new FloatVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], vectorData->writable().begin() );
						data = vectorData;
					}
					break;
				}
				case TypeDesc::VEC2 :
				{
					if( !type.arraylen )
					{
						data = new V2fData( Imath::V2f( typedData[0], typedData[1] ) );
					}
					else if( type.arraylen  == 2 )
					{
						data = new Box2fData( Imath::Box2f( Imath::V2f( typedData[0], typedData[1] ), Imath::V2f( typedData[2], typedData[3] ) ) );
					}
					else
					{
						V2fVectorDataPtr vectorData = new V2fVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], &vectorData->baseWritable()[0] );
						data = vectorData;
					}
					break;
				}
				case TypeDesc::VEC3 :
				{
					data = new V3fData( Imath::V3f( typedData[0], typedData[1], typedData[2] ) );
					break;
				}
				case TypeDesc::MATRIX33 :
				{
					data = new M33fData( Imath::M33f( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8] ) );
					break;
				}
				case TypeDesc::MATRIX44 :
				{
					data = new M44fData( Imath::M44f( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8], typedData[9], typedData[10], typedData[11], typedData[12], typedData[13], typedData[14], typedData[15] ) );
					break;
				}
				default :
				{
					break;
				}
			}
			break;
		}
		case TypeDesc::DOUBLE :
		{
			const double *typedData = static_cast<const double *>( rawData );
			switch ( type.aggregate )
			{
				case TypeDesc::SCALAR :
				{
					if( !type.arraylen )
					{
						data = new DoubleData( *typedData );
					}
					else
					{
						DoubleVectorDataPtr vectorData = new DoubleVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], vectorData->writable().begin() );
						data = vectorData;
					}
					break;
				}
				case TypeDesc::VEC2 :
				{
					if( !type.arraylen )
					{
						data = new V2dData( Imath::V2d( typedData[0], typedData[1] ) );
					}
					else if( type.arraylen  == 2 )
					{
						data = new Box2dData( Imath::Box2d( Imath::V2d( typedData[0], typedData[1] ), Imath::V2d( typedData[2], typedData[3] ) ) );
					}
					else
					{
						V2dVectorDataPtr vectorData = new V2dVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], &vectorData->baseWritable()[0] );
						data = vectorData;
					}
					break;
				}
				case TypeDesc::VEC3 :
				{
					data = new V3dData( Imath::V3d( typedData[0], typedData[1], typedData[2] ) );
					break;
				}
				case TypeDesc::MATRIX33 :
				{
					data = new M33dData( Imath::M33d( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8] ) );
					break;
				}
				case TypeDesc::MATRIX44 :
				{
					data = new M44dData( Imath::M44d( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8], typedData[9], typedData[10], typedData[11], typedData[12], typedData[13], typedData[14], typedData[15] ) );
					break;
				}
				default :
				{
					break;
				}
			}
			break;
		}
		default :
		{
			break;
		}
	}
}

} // namespace OpenImageIOAlgo

} // namespace IECoreImage
