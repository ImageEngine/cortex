//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/NameStateComponent.h"

#include "IECore/Exception.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( NameStateComponent );

NameStateComponent::NameMap NameStateComponent::g_nameMap;
StateComponent::Description<NameStateComponent> NameStateComponent::g_description;

NameStateComponent::NameStateComponent( const std::string &name )
{
	m_it = g_nameMap.insert( NamePair( name, g_nameMap.size() ) ).first;
}

NameStateComponent::~NameStateComponent()
{
}

const std::string &NameStateComponent::name() const
{
	return (*m_it).first.value();
}

GLuint NameStateComponent::glName() const
{
	return m_it->second;
}
			
void NameStateComponent::bind() const
{
	glLoadName( m_it->second );
}

GLbitfield NameStateComponent::mask() const
{
	return 0;
}

const std::string &NameStateComponent::nameFromGLName( GLuint glName )
{
	const NameMap::nth_index<1>::type &index = g_nameMap.get<1>();
	NameMap::nth_index<1>::type::const_iterator it = index.find( glName );
	if( it==index.end() )
	{
		throw IECore::Exception( "NameStateComponent::nameFromGLName : Invalid glName" );
	}
	return it->first.value();
}
