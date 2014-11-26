//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include "boost/lexical_cast.hpp"

#include "renderer/api/color.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/private/AppleseedUtil.h"

using namespace IECore;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

string IECoreAppleseed::dataToString( ConstDataPtr value )
{
	stringstream ss;

	switch( value->typeId() )
	{
		case IntDataTypeId :
			{
				int x = static_cast<const IntData*>( value.get() )->readable();
				ss << x;
			}
			break;

		case FloatDataTypeId :
			{
				float x = static_cast<const FloatData*>( value.get() )->readable();
				ss << x;
			}
			break;

		case StringDataTypeId :
			{
				const string &x = static_cast<const StringData*>( value.get() )->readable();
				ss << x;
			}
			break;

		case V2iDataTypeId :
			{
				const Imath::V2i &x = static_cast<const V2iData*>( value.get() )->readable();
				ss << x.x << ", " << x.y;
			}
			break;

		case Color3fDataTypeId :
			{
				const Imath::Color3f &x = static_cast<const Color3fData*>( value.get() )->readable();
				ss << x.x << ", " << x.y << ", " << x.z;
			}
			break;
		case BoolDataTypeId :
			{
				bool x = static_cast<const BoolData*>( value.get() )->readable();
				ss << x;
			}
			break;

		default:
			break;
	}

	return ss.str();
}

void IECoreAppleseed::setParam( const string &name, const Data *value, asr::ParamArray &params )
{
	switch( value->typeId() )
	{
		case IntDataTypeId :
			{
				int x = static_cast<const IntData*>( value)->readable();
				params.insert( name, x );
			}
			break;

		case FloatDataTypeId :
			{
				float x = static_cast<const FloatData*>( value)->readable();
				params.insert( name, x );
			}
			break;

		case StringDataTypeId :
			{
				const string &x = static_cast<const StringData*>( value)->readable();
				params.insert( name, x.c_str() );
			}
			break;

		case BoolDataTypeId :
			{
				bool x = static_cast<const BoolData*>( value)->readable();
				params.insert( name, x);
			}
			break;

		default:
			// some kind of warning would be nice here...
			break;
	}
}

asr::ParamArray IECoreAppleseed::convertParams( const CompoundDataMap &parameters )
{
	asr::ParamArray result;

	for( CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); ++it )
		setParam( it->first.value(), it->second.get(), result );

	return result;
}

string IECoreAppleseed::createColorEntity( asr::ColorContainer &colorContainer, const Imath::C3f &color, const string &name )
{
	// for monochrome colors, we don't need to create a color entity at all.
	if( color.x == color.y && color.x == color.z )
	{
		return boost::lexical_cast<string>( color.x );
	}

	asr::ColorValueArray values( 3, &color.x );
	asr::ParamArray params;
	params.insert( "color_space", "linear_rgb" );

	asf::auto_release_ptr<asr::ColorEntity> c = asr::ColorEntityFactory::create( name.c_str(), params, values );
	return insertEntityWithUniqueName( colorContainer, c, name.c_str() );
}

string IECoreAppleseed::createTextureEntity( asr::TextureContainer &textureContainer, asr::TextureInstanceContainer &textureInstanceContainer, const asf::SearchPaths &searchPaths, const string &textureName, const string &fileName )
{
	asr::ParamArray params;
	params.insert( "filename", fileName.c_str() );
	params.insert( "color_space", "linear_rgb" );

	asf::auto_release_ptr<asr::Texture> texture( asr::DiskTexture2dFactory().create( textureName.c_str(), params, searchPaths ) );
	string txName = insertEntityWithUniqueName( textureContainer, texture, textureName );

	string textureInstanceName = txName + "_instance";
	asf::auto_release_ptr<asr::TextureInstance> textureInstance( asr::TextureInstanceFactory().create( textureInstanceName.c_str(), asr::ParamArray(), txName.c_str() ) );
	return insertEntityWithUniqueName( textureInstanceContainer, textureInstance, textureInstanceName.c_str() );
}
