//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/FrameListBinding.h"

#include "IECore/ExclusionFrameList.h"
#include "IECore/Exception.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

template<>
std::string repr( ExclusionFrameList &x )
{
	std::stringstream s;

	s << "IECore.ExclusionFrameList( ";

	object item( FrameListPtr( x.getFrameList() ) );

	s << call_method< std::string >( item.ptr(), "__repr__" );

	s << ", ";

	item = object( x.getExclusionFrameList() );

	s << call_method< std::string >( item.ptr(), "__repr__" );

	s << " ) ";

	return s.str();
}

void bindExclusionFrameList()
{
	RunTimeTypedClass<ExclusionFrameList>()
		.def( init< FrameListPtr, FrameListPtr >() )
		.add_property( "frameList", &ExclusionFrameList::getFrameList, &ExclusionFrameList::setFrameList )
		.add_property( "exclusionFrameList", &ExclusionFrameList::getExclusionFrameList, &ExclusionFrameList::setExclusionFrameList )
		.def( "__repr__", repr< ExclusionFrameList > )
	;
}

}
