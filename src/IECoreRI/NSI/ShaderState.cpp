//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

#include "nsi.h"
#include "nsi_ri.h"

#include "boost/make_shared.hpp"
#include "boost/algorithm/string/predicate.hpp"

#include "IECore/SimpleTypedData.h"

#include "IECoreRI/NSI/AttributeAlgo.h"
#include "IECoreRI/NSI/private/ShaderState.h"

using namespace std;
using namespace boost;
using namespace IECore;
using namespace IECoreRI::NSI;

ShaderState::ShaderState( HandleGeneratorPtr handleGenerator )
	:	m_handleMap( boost::make_shared<HandleMap>() ), m_handleGenerator( handleGenerator )
{
}

ShaderState::ShaderState( const ShaderState &other )
	:	m_handleMap( other.m_handleMap ), m_handleGenerator( other.m_handleGenerator )
{
}

bool ShaderState::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if( !starts_with( type, "osl:" ) )
	{
		return false;
	}

	const StringData *handleData = NULL;
	CompoundDataMap::const_iterator handleIt = parameters.find( "__handle" );
	if( handleIt != parameters.end() )
	{
		handleData = runTimeCast<const StringData>( handleIt->second.get() );
	}

	std::string nsiHandle = m_handleGenerator->generate( handleData ? handleData->readable() : name );

	NSIContext_t nsiContext = RiToNSIContext( RiGetContext() );
	NSICreate(
		nsiContext,
		nsiHandle.c_str(),
		"shader", // type
		0, // nparams
		NULL // params
	);

	const char *fileName = name.c_str();
	NSIParam_t fileNameParam = { "shaderfilename", &fileName, NSITypeString, 0, 1 };
	NSISetAttribute(
		nsiContext,
		nsiHandle.c_str(),
		1,
		&fileNameParam
	);

	if( handleData )
	{
		// We need to store the NSI handle so we can look it up later
		// when linking shaders. Since we do lazy shallow copies in our
		// copy constructor, we must now do a full deep copy if we are
		// to modify the handle map.
		if( !m_handleMap.unique() )
		{
			m_handleMap = boost::make_shared<HandleMap>( *m_handleMap );
		}
		(*m_handleMap)[handleData->readable()] = nsiHandle;
	}

	for( CompoundDataMap::const_iterator it = parameters.begin(), eIt = parameters.end(); it != eIt; ++it )
	{
		if( it == handleIt )
		{
			continue;
		}

		if( const StringData *valueData = runTimeCast<const StringData>( it->second.get() ) )
		{
			const string &value = valueData->readable();
			if( starts_with( value, "link:" ) )
			{
				const size_t i = value.find_first_of( "." );
				const string fromHandle( value.begin() + 5, value.begin() + i );
				const char *fromAttr = value.c_str() + i + 1;

				HandleMap::const_iterator fromNSIHandleIt = m_handleMap->find( fromHandle );
				if( fromNSIHandleIt != m_handleMap->end() )
				{
					NSIConnect(
						nsiContext,
						fromNSIHandleIt->second.c_str(),
						fromAttr,
						nsiHandle.c_str(),
						it->first.c_str()
					);
				}
				continue;
			}
		}

		setAttribute( nsiContext, nsiHandle.c_str(), it->first.c_str(), it->second.get() );
	}

	if( type == "osl:surface" )
	{
		const char *h = nsiHandle.c_str();
		RiAttribute( "nsi", "string oslsurface", &h, RI_NULL );
	}
	else if( type == "osl:displacement" )
	{
		const char *h = nsiHandle.c_str();
		RiAttribute( "nsi", "string osldisplacement", &h, RI_NULL );
	}


	return true;
}
