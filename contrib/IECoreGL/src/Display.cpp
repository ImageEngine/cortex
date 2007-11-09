//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/private/Display.h"
#include "IECoreGL/FrameBuffer.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/DepthTexture.h"

#include "IECore/Writer.h"
#include "IECore/MessageHandler.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/CompoundParameter.h"

using namespace IECoreGL;

Display::Display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
	:	m_name( name ), m_type( type ), m_data( data )
{
	for( IECore::CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		m_parameters[it->first] = it->second->copy();
	}
}

void Display::display( ConstFrameBufferPtr frameBuffer ) const
{
	IECore::ImagePrimitivePtr image = 0;
	if( m_data=="rgba" )
	{
		image = frameBuffer->getColor()->imagePrimitive();
	}
	else if( m_data=="rgb" )
	{
		image = frameBuffer->getColor()->imagePrimitive();
		image->variables.erase( "A" );
	}
	else if( m_data=="z" )
	{
		image = frameBuffer->getDepth()->imagePrimitive();
	}
	else
	{
		IECore::msg( IECore::Msg::Warning, "Display::display", boost::format( "Unsupported data format \"%s\"." ) % m_data );
		return;
	}
	
	IECore::WriterPtr writer = IECore::Writer::create( image, "tmp." + m_type );
	if( !writer )
	{
		IECore::msg( IECore::Msg::Warning, "Display::display", boost::format( "Unsupported display type \"%s\"." ) % m_type );
		return;
	}
	
	writer->parameters()->parameter<IECore::FileNameParameter>( "fileName" )->setTypedValue( m_name );
	writer->write();
}
