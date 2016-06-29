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

#include "IECoreAppleseed/ColorAlgo.h"
#include "IECoreAppleseed/EntityAlgo.h"

#include "boost/lexical_cast.hpp"

#include "OpenEXR/ImathColor.h"

using namespace Imath;
using namespace boost;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

namespace IECoreAppleseed
{

namespace ColorAlgo
{

pair<string, asr::ColorEntity*> createColorEntity( asr::ColorContainer &colorContainer, const C3f &color, const string &name )
{
	// for monochrome colors, we don't need to create a color entity at all.
	if( color.x == color.y && color.x == color.z )
	{
		return make_pair( lexical_cast<string>( color.x ), static_cast<asr::ColorEntity *>( 0 ) );
	}

	asr::ColorValueArray values( 3, &color.x );
	asr::ParamArray params;
	params.insert( "color_space", "linear_rgb" );

	asf::auto_release_ptr<asr::ColorEntity> c = asr::ColorEntityFactory::create( name.c_str(), params, values );
	asr::ColorEntity *colorEntity = c.get();
	return make_pair( EntityAlgo::insertEntityWithUniqueName( colorContainer, c, name.c_str() ), colorEntity );
}

} // namespace ColorAlgo

} // namespace IECoreAppleseed
