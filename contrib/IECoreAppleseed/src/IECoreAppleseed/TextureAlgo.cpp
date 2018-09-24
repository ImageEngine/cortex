//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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


#include "IECoreAppleseed/TextureAlgo.h"

#include "IECoreAppleseed/EntityAlgo.h"

#include "renderer/api/scene.h"

using namespace IECoreAppleseed;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

string doCreateTextureEntity( asr::TextureContainer &textureContainer, asr::TextureInstanceContainer &textureInstanceContainer, const asf::SearchPaths &searchPaths, const string &textureName, const string &fileName, const asr::ParamArray &txInstanceParams )
{
	asr::ParamArray params;
	params.insert( "filename", fileName.c_str() );
	params.insert( "color_space", "linear_rgb" );

	asf::auto_release_ptr<asr::Texture> texture( asr::DiskTexture2dFactory().create( textureName.c_str(), params, searchPaths ) );
	string txName = EntityAlgo::insertEntityWithUniqueName( textureContainer, texture, textureName );

	string textureInstanceName = txName + "_instance";
	asf::auto_release_ptr<asr::TextureInstance> textureInstance( asr::TextureInstanceFactory().create( textureInstanceName.c_str(), txInstanceParams, txName.c_str() ) );
	return EntityAlgo::insertEntityWithUniqueName( textureInstanceContainer, textureInstance, textureInstanceName.c_str() );
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementation of public API.
//////////////////////////////////////////////////////////////////////////

namespace IECoreAppleseed
{

namespace TextureAlgo
{

string createTextureEntity( asr::TextureContainer &textureContainer, asr::TextureInstanceContainer &textureInstanceContainer, const asf::SearchPaths &searchPaths, const string &textureName, const string &fileName )
{
	return doCreateTextureEntity( textureContainer, textureInstanceContainer, searchPaths, textureName, fileName, asr::ParamArray() );
}

string createAlphaMapTextureEntity( asr::TextureContainer &textureContainer, asr::TextureInstanceContainer &textureInstanceContainer, const asf::SearchPaths &searchPaths, const string &textureName, const string &fileName )
{
	asr::ParamArray params;
	params.insert( "alpha_mode", "detect" );
	return doCreateTextureEntity( textureContainer, textureInstanceContainer, searchPaths, textureName, fileName, params );
}

} // namespace TextureAlgo

} // namespace IECoreAppleseed
