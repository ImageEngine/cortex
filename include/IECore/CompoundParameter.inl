//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

#ifndef IE_CORE_COMPOUNDPARAMETER_INL
#define IE_CORE_COMPOUNDPARAMETER_INL

#include "IECore/Exception.h"

namespace IECore
{

template<typename I>	
CompoundParameter::CompoundParameter( const std::string &name, const std::string &description, I membersBegin, I membersEnd, ConstCompoundObjectPtr userData )
	:	Parameter( name, description, new CompoundObject, PresetsMap(), false, userData )
{
	addParameters( membersBegin, membersEnd );
}

template<typename I>
void CompoundParameter::addParameters( I begin, I end )
{
	for( I it=begin; it!=end; it++ )
	{
		addParameter( *it );
	}
}
	
template<typename T>
typename T::Ptr CompoundParameter::parameter( const std::string &name )
{
	ParameterMap::iterator it = m_namesToParameters.find( name );
	if( it!=m_namesToParameters.end() )
	{
		return runTimeCast<T>( it->second );
	}
	return 0;
}

template<typename T>
typename T::ConstPtr CompoundParameter::parameter( const std::string &name ) const
{
	ParameterMap::const_iterator it = m_namesToParameters.find( name );
	if( it!=m_namesToParameters.end() )
	{
		return runTimeCast<const T>( it->second );
	}
	return 0;
}
		
}; // namespace IECore

#endif // IE_CORE_COMPOUNDPARAMETER_INL
