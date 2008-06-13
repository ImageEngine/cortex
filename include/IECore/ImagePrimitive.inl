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

#ifndef IECORE_IMAGEPRIMITIVE_INL
#define IECORE_IMAGEPRIMITIVE_INL

#include "IECore/Primitive.h"
#include "IECore/Exception.h"
#include "boost/format.hpp"

namespace IECore 
{

template<typename T>
typename TypedData<std::vector<T> >::Ptr ImagePrimitive::getChannel( const std::string &name )
{
	std::string reason = "";
	if( channelValid( name, &reason ) )
	{
		return runTimeCast<TypedData<std::vector<T> > >( variables.find( name )->second.data );
	}
	return 0;
}
		
template<typename T>
typename TypedData<std::vector<T> >::ConstPtr ImagePrimitive::getChannel( const std::string &name ) const
{
	std::string reason = "";
	if( channelValid( name, &reason ) )
	{
		return runTimeCast<TypedData<std::vector<T> > >( variables.find( name )->second.data );
	}
	return 0;
}
			
template<class T>
typename TypedData<std::vector<T> >::Ptr ImagePrimitive::createChannel( const std::string &name )
{	
	// create a new Variable
	PrimitiveVariable::Interpolation i = PrimitiveVariable::Vertex;
	typename TypedData<std::vector<T> >::Ptr channel = new TypedData<std::vector<T> >;
	
	int area = ( 1 + m_dataWindow.max.x - m_dataWindow.min.x ) * ( 1 + m_dataWindow.max.y - m_dataWindow.min.y );

	channel->writable().resize( area );
	
	variables.insert(PrimitiveVariableMap::value_type(name, PrimitiveVariable(i, channel)));
	
	return channel;
}

}

#endif
