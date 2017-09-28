//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreImage/DisplayDriver.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( DisplayDriver );

DisplayDriver::DisplayDriver( const Box2i &displayWindow, const Box2i &dataWindow, const vector<string> &channelNames, ConstCompoundDataPtr parameters ) :
	m_displayWindow( displayWindow ), m_dataWindow( dataWindow ), m_channelNames( channelNames )
{
}

DisplayDriver::~DisplayDriver()
{
}

Imath::Box2i DisplayDriver::displayWindow() const
{
	return m_displayWindow;
}

Imath::Box2i DisplayDriver::dataWindow() const
{
	return m_dataWindow;
}

const std::vector<std::string> &DisplayDriver::channelNames() const
{
	return m_channelNames;
}

DisplayDriverPtr DisplayDriver::create( const std::string &typeName, const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters )
{
	DisplayDriverPtr res;
	const TypeNamesToCreators &creators = typeNamesToCreators();
	TypeNamesToCreators::const_iterator it = creators.find( typeName );
	if( it != creators.end() )
	{
		return it->second( displayWindow, dataWindow, channelNames, parameters );
	}

	throw Exception( boost::str( boost::format( "Display driver \"%s\" not registered" ) % typeName ) );
}

void DisplayDriver::registerType( const std::string &typeName, CreatorFn creator )
{
	typeNamesToCreators()[typeName] = creator;
}

DisplayDriver::TypeNamesToCreators &DisplayDriver::typeNamesToCreators()
{
	static TypeNamesToCreators creators;
	return creators;
}
