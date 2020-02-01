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

#include "IECoreImage/OpenImageIOAlgo.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/TimeCodeData.h"
#include "IECore/VectorTypedData.h"

#include "OpenImageIO/imageio.h"
#include "OpenImageIO/ustring.h"

#include "boost/tokenizer.hpp"

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

#if OIIO_VERSION > 10805

		case GeometricData::Rational:
			return TypeDesc::RATIONAL;

#endif

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

#if OIIO_VERSION > 10805

		case TypeDesc::RATIONAL:
			return GeometricData::Rational;

#endif

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
	:	data( nullptr )
{
}

DataView::DataView( const DataView &other )
	:	type( other.type ), data( other.data ), m_charPointers( other.m_charPointers )
{
	if( m_charPointers.size() )
	{
		data = &m_charPointers[0];
	}
}

DataView &DataView::operator=( const DataView &rhs )
{
	type = rhs.type;
	data = rhs.data;
	m_charPointers = rhs.m_charPointers;
	if( m_charPointers.size() )
	{
		data = &m_charPointers[0];
	}
	return *this;
}

DataView::DataView( const IECore::Data *d, bool createUStrings )
	:	data( nullptr )
{
	switch( d ? d->typeId() : IECore::InvalidTypeId )
	{

		// Simple data

		case CharDataTypeId :
			type = TypeDesc::CHAR;
			data = static_cast<const CharData *>( d )->baseReadable();
			break;
		case UCharDataTypeId :
			type = TypeDesc::UCHAR;
			data = static_cast<const UCharData *>( d )->baseReadable();
			break;
		case StringDataTypeId :
			type = TypeDesc::TypeString;
			m_charPointers.resize(1);
			m_charPointers[0] = static_cast<const StringData *>( d )->readable().c_str();
			if( createUStrings )
			{
				m_charPointers[0] = ustring( m_charPointers[0] ).c_str();
			}
			data = &m_charPointers[0];
			break;
		case UShortDataTypeId :
			type = TypeDesc::USHORT;
			data = static_cast<const UShortData *>( d )->baseReadable();
			break;
		case ShortDataTypeId :
			type = TypeDesc::SHORT;
			data = static_cast<const ShortData *>( d )->baseReadable();
			break;
		case UIntDataTypeId :
			type = TypeDesc::UINT;
			data = static_cast<const UIntData *>( d )->baseReadable();
			break;
		case HalfDataTypeId :
			type = TypeDesc::HALF;
			data = static_cast<const HalfData *>( d )->baseReadable();
			break;
		case IntDataTypeId :
			type = TypeDesc::TypeInt;
			data = static_cast<const IntData *>( d )->baseReadable();
			break;
		case FloatDataTypeId :
			type = TypeDesc::TypeFloat;
			data = static_cast<const FloatData *>( d )->baseReadable();
			break;
		case DoubleDataTypeId :
			type = TypeDesc::DOUBLE;
			data = static_cast<const DoubleData *>( d )->baseReadable();
			break;
		case V2iDataTypeId :
			type = TypeDesc( TypeDesc::INT, TypeDesc::VEC2, vecSemantics( static_cast<const V2iData *>( d )->getInterpretation() ) );
			data = static_cast<const V2iData *>( d )->baseReadable();
			break;
		case V3iDataTypeId :
			type = TypeDesc( TypeDesc::INT, TypeDesc::VEC3 );
			data = static_cast<const V3iData *>( d )->baseReadable();
			break;
		case Box2iDataTypeId :
			type = TypeDesc( TypeDesc::INT, TypeDesc::VEC2, TypeDesc::NOXFORM, 2 );
			data = static_cast<const Box2iData *>( d )->baseReadable();
			break;
		case V2fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::VEC2, vecSemantics( static_cast<const V2fData *>( d )->getInterpretation() ) );
			data = static_cast<const V2fData *>( d )->baseReadable();
			break;
		case V3fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::VEC3, vecSemantics( static_cast<const V3fData *>( d )->getInterpretation() ) );
			data = static_cast<const V3fData *>( d )->baseReadable();
			break;
		case Box2fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::VEC2, TypeDesc::NOXFORM, 2 );
			data = static_cast<const Box2fData *>( d )->baseReadable();
			break;
		case M33fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::MATRIX33 );
			data = static_cast<const M33fData *>( d )->baseReadable();
			break;
		case M44fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::MATRIX44 );
			data = static_cast<const M44fData *>( d )->baseReadable();
			break;
		case V2dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::VEC2, vecSemantics( static_cast<const V2dData *>( d )->getInterpretation() ) );
			data = static_cast<const V2dData *>( d )->baseReadable();
			break;
		case V3dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::VEC3, vecSemantics( static_cast<const V3dData *>( d )->getInterpretation() ) );
			data = static_cast<const V3dData *>( d )->baseReadable();
			break;
		case Box2dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::VEC2, TypeDesc::NOXFORM, 2 );
			data = static_cast<const Box2dData *>( d )->baseReadable();
			break;
		case M33dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::MATRIX33 );
			data = static_cast<const M33dData *>( d )->baseReadable();
			break;
		case M44dDataTypeId :
			type = TypeDesc( TypeDesc::DOUBLE, TypeDesc::MATRIX44 );
			data = static_cast<const M44dData *>( d )->baseReadable();
			break;
		case Color3fDataTypeId :
			type = TypeDesc::TypeColor;
			data = static_cast<const Color3fData *>( d )->baseReadable();
			break;
		case Color4fDataTypeId :
			type = TypeDesc( TypeDesc::FLOAT, TypeDesc::VEC4, TypeDesc::COLOR );
			data = static_cast<const Color4fData *>( d )->baseReadable();
			break;
		case TimeCodeDataTypeId :
			type = TypeDesc::TypeTimeCode;
			data = static_cast<const TimeCodeData *>( d )->baseReadable();
			break;

		// Vector data

		case DoubleVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::DOUBLE,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const DoubleVectorData *>( d )->readable().size()
			);
			data = static_cast<const DoubleVectorData *>( d )->baseReadable();
			break;
		case FloatVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const FloatVectorData *>( d )->readable().size()
			);
			data = static_cast<const FloatVectorData *>( d )->baseReadable();
			break;
		case HalfVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::HALF,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const HalfVectorData *>( d )->readable().size()
			);
			data = static_cast<const HalfVectorData *>( d )->baseReadable();
			break;
		case IntVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::INT,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const IntVectorData *>( d )->readable().size()
			);
			data = static_cast<const IntVectorData *>( d )->baseReadable();
			break;
		case UIntVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::UINT,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const UIntVectorData *>( d )->readable().size()
			);
			data = static_cast<const UIntVectorData *>( d )->baseReadable();
			break;
		case CharVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::CHAR,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const CharVectorData *>( d )->readable().size()
			);
			data = static_cast<const CharVectorData *>( d )->baseReadable();
			break;
		case UCharVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::UCHAR,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				static_cast<const UCharVectorData *>( d )->readable().size()
			);
			data = static_cast<const UCharVectorData *>( d )->baseReadable();
			break;
		case V2fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::VEC2,
				vecSemantics( static_cast<const V2fVectorData *>( d )->getInterpretation() ),
				static_cast<const V2fVectorData *>( d )->readable().size()
			);
			data = static_cast<const V2fVectorData *>( d )->baseReadable();
			break;
		case V2iVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::INT,
				TypeDesc::VEC2,
				vecSemantics( static_cast<const V2iVectorData *>( d )->getInterpretation() ),
				static_cast<const V2iVectorData *>( d )->readable().size()
			);
			data = static_cast<const V2iVectorData *>( d )->baseReadable();
			break;
		case V3fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::VEC3,
				vecSemantics( static_cast<const V3fVectorData *>( d )->getInterpretation() ),
				static_cast<const V3fVectorData *>( d )->readable().size()
			);
			data = static_cast<const V3fVectorData *>( d )->baseReadable();
			break;
		case V3iVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::INT,
				TypeDesc::VEC3,
				vecSemantics( static_cast<const V3iVectorData *>( d )->getInterpretation() ),
				static_cast<const V3iVectorData *>( d )->readable().size()
			);
			data = static_cast<const V3iVectorData *> ( d )->baseReadable();
			break;
		case Color3fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::VEC3,
				TypeDesc::COLOR,
				static_cast<const Color3fVectorData *>( d )->readable().size()
			);
			data = static_cast<const Color3fVectorData *>( d )->baseReadable();
			break;
		case Color4fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::VEC4,
				TypeDesc::COLOR,
				static_cast<const Color4fVectorData *>( d )->readable().size()
			);
			data = static_cast<const Color4fVectorData *>( d )->baseReadable();
			break;
		case M44fVectorDataTypeId :
			type = TypeDesc(
				TypeDesc::FLOAT,
				TypeDesc::MATRIX44,
				TypeDesc::NOSEMANTICS,
				static_cast<const M44fVectorData *>( d )->readable().size()
			);
			data = static_cast<const M44fVectorData *>( d )->baseReadable();
			break;
		case StringVectorDataTypeId:
		{
			const auto &readableStrings = static_cast<const StringVectorData *>( d )->readable();
			size_t numStrings = readableStrings.size();
			type = TypeDesc(
				TypeDesc::STRING,
				TypeDesc::SCALAR,
				TypeDesc::NOSEMANTICS,
				numStrings
			);

			m_charPointers.resize( numStrings );

			if ( createUStrings )
			{
				for(size_t i = 0; i < numStrings; ++i)
				{
					m_charPointers[i] = ustring( readableStrings[i].c_str() ).c_str();
				}
			}
			else
			{
				for(size_t i = 0; i < numStrings; ++i)
				{
					m_charPointers[i] = readableStrings[i].c_str();
				}
			}

			data = &m_charPointers[0];
		}
			break;
		default :
			// Default initialisers above did what we need already
			break;

	}
}

IECore::DataPtr data( const OIIO::ParamValue &value )
{
	OIIO::TypeDesc type = value.type();
	switch( type.basetype )
	{
		case TypeDesc::CHAR :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				return new CharData( *static_cast<const char *>( value.data() ) );
			}
			return nullptr;
		}
		case TypeDesc::UCHAR :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				return new UCharData( *static_cast<const unsigned char *>( value.data() ) );
			}
			return nullptr;
		}
		case TypeDesc::STRING :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				if ( type.arraylen == 0 )
				{
					return new StringData( static_cast<const ustring *>( value.data() )->c_str() );
				}
				else
				{
					const ustring *typedData = static_cast<const ustring *>( value.data() );

					StringVectorDataPtr vectorData = new StringVectorData();
					auto &writable = vectorData->writable();
					writable.resize( type.arraylen );
					for (int i = 0; i <  type.arraylen; ++i)
					{
						writable[i] = typedData[i].string();
					}
					return vectorData;
				}
			}
			return nullptr;
		}
		case TypeDesc::USHORT :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				return new UShortData( *static_cast<const unsigned short *>( value.data() ) );
			}
			return nullptr;
		}
		case TypeDesc::SHORT :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				return new ShortData( *static_cast<const short *>( value.data() ) );
			}
			return nullptr;
		}
		case TypeDesc::UINT :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				const unsigned *typedData = static_cast<const unsigned *>( value.data() );
				if ( type.vecsemantics == TypeDesc::TIMECODE )
				{
					return new TimeCodeData( Imf::TimeCode( typedData[0], typedData[1] ) );
				}
				else
				{
					return new UIntData( *typedData );
				}
			}
			return nullptr;
		}
		case TypeDesc::INT :
		{
			const int *typedData = static_cast<const int *>( value.data() );
			switch ( type.aggregate )
			{
				case TypeDesc::SCALAR :
				{
					if( !type.arraylen )
					{
						return new IntData( *typedData );
					}
					else
					{
						IntVectorDataPtr vectorData = new IntVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], vectorData->writable().begin() );
						return vectorData;
					}
				}
				case TypeDesc::VEC2 :
				{
					if( !type.arraylen )
					{

#if OIIO_VERSION > 10805

						GeometricData::Interpretation interpretation = ( type.vecsemantics == TypeDesc::RATIONAL ) ? GeometricData::Interpretation::Rational : GeometricData::Interpretation::None;

#else

						GeometricData::Interpretation interpretation = GeometricData::Interpretation::None;

#endif

						return new V2iData( Imath::V2i( typedData[0], typedData[1] ), interpretation );
					}
					else if( type.arraylen  == 2 )
					{
						return new Box2iData( Imath::Box2i( Imath::V2i( typedData[0], typedData[1] ), Imath::V2i( typedData[2], typedData[3] ) ) );
					}
					else
					{
						V2iVectorDataPtr vectorData = new V2iVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], &vectorData->baseWritable()[0] );
						return vectorData;
					}
				}
				case TypeDesc::VEC3 :
				{
					return new V3iData( Imath::V3i( typedData[0], typedData[1], typedData[2] ) );
				}
				default :
				{
					return nullptr;
				}
			}
		}
		case TypeDesc::HALF :
		{
			if ( type.aggregate == TypeDesc::SCALAR )
			{
				return new HalfData( *static_cast<const half *>( value.data() ) );
			}
			return nullptr;
		}
		case TypeDesc::FLOAT :
		{
			const float *typedData = static_cast<const float *>( value.data() );
			switch ( type.aggregate )
			{
				case TypeDesc::SCALAR :
				{
					if( !type.arraylen )
					{
						return new FloatData( *typedData );
					}
					else
					{
						FloatVectorDataPtr vectorData = new FloatVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], vectorData->writable().begin() );
						return vectorData;
					}
				}
				case TypeDesc::VEC2 :
				{
					if( !type.arraylen )
					{
						return new V2fData( Imath::V2f( typedData[0], typedData[1] ) );
					}
					else if( type.arraylen  == 2 )
					{
						return new Box2fData( Imath::Box2f( Imath::V2f( typedData[0], typedData[1] ), Imath::V2f( typedData[2], typedData[3] ) ) );
					}
					else
					{
						V2fVectorDataPtr vectorData = new V2fVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], &vectorData->baseWritable()[0] );
						return vectorData;
					}
				}
				case TypeDesc::VEC3 :
				{
					return new V3fData( Imath::V3f( typedData[0], typedData[1], typedData[2] ) );
				}
				case TypeDesc::MATRIX33 :
				{
					return new M33fData( Imath::M33f( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8] ) );
				}
				case TypeDesc::MATRIX44 :
				{
					return new M44fData( Imath::M44f( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8], typedData[9], typedData[10], typedData[11], typedData[12], typedData[13], typedData[14], typedData[15] ) );
				}
				default :
				{
					return nullptr;
				}
			}
		}
		case TypeDesc::DOUBLE :
		{
			const double *typedData = static_cast<const double *>( value.data() );
			switch ( type.aggregate )
			{
				case TypeDesc::SCALAR :
				{
					if( !type.arraylen )
					{
						return new DoubleData( *typedData );
					}
					else
					{
						DoubleVectorDataPtr vectorData = new DoubleVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], vectorData->writable().begin() );
						return vectorData;
					}
				}
				case TypeDesc::VEC2 :
				{
					if( !type.arraylen )
					{
						return new V2dData( Imath::V2d( typedData[0], typedData[1] ) );
					}
					else if( type.arraylen  == 2 )
					{
						return new Box2dData( Imath::Box2d( Imath::V2d( typedData[0], typedData[1] ), Imath::V2d( typedData[2], typedData[3] ) ) );
					}
					else
					{
						V2dVectorDataPtr vectorData = new V2dVectorData();
						vectorData->writable().resize( type.arraylen );
						std::copy( &typedData[0], &typedData[type.arraylen], &vectorData->baseWritable()[0] );
						return vectorData;
					}
				}
				case TypeDesc::VEC3 :
				{
					return new V3dData( Imath::V3d( typedData[0], typedData[1], typedData[2] ) );
				}
				case TypeDesc::MATRIX33 :
				{
					return new M33dData( Imath::M33d( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8] ) );
				}
				case TypeDesc::MATRIX44 :
				{
					return new M44dData( Imath::M44d( typedData[0], typedData[1], typedData[2], typedData[3], typedData[4], typedData[5], typedData[6], typedData[7], typedData[8], typedData[9], typedData[10], typedData[11], typedData[12], typedData[13], typedData[14], typedData[15] ) );
				}
				default :
				{
					return nullptr;
				}
			}
		}
		default :
		{
			return nullptr;
		}
	}
}

} // namespace OpenImageIOAlgo

} // namespace IECoreImage
