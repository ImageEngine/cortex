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

#ifndef IECOREAPPLESEED_APPLESEEDUTIL_H
#define IECOREAPPLESEED_APPLESEEDUTIL_H

#include <string>

#include "boost/format.hpp"

#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathMatrix.h"

#include "renderer/api/scene.h"
#include "renderer/api/texture.h"
#include "renderer/api/utility.h"

#include "IECore/CompoundData.h"

namespace IECoreAppleseed
{

std::string dataToString( IECore::ConstDataPtr value );

void setParam( const std::string &name, const IECore::Data *value, renderer::ParamArray& params );

renderer::ParamArray convertParams( const IECore::CompoundDataMap &parameters );

template<class Container, class T>
std::string insertEntityWithUniqueName( Container &container, foundation::auto_release_ptr<T> entity, const std::string &name )
{
    if( container.get_by_name( name.c_str() ) == 0 )
    {
        entity->set_name( name.c_str() );
        container.insert( entity );
        return name;
    }

    boost::format fmt( name + "_%1%" );

    int i = 2;
    while( true )
    {
        std::string new_name = ( fmt % i++ ).str();
        if( container.get_by_name( new_name.c_str() ) == 0 )
        {
            entity->set_name( new_name.c_str() );
            container.insert( entity );
            return new_name;
        }
    }
}

std::string createColorEntity( renderer::ColorContainer &colorContainer, const Imath::C3f &color, const std::string &name );
std::string createTextureEntity( renderer::TextureContainer &textureContainer, renderer::TextureInstanceContainer &textureInstanceContainer, const foundation::SearchPaths &searchPaths, const std::string &textureName, const std::string &fileName );

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_APPLESEEDUTIL_H
