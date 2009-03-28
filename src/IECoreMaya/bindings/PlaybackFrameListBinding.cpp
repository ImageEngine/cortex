//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Exception.h"

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"

#include "IECoreMaya/bindings/PlaybackFrameListBinding.h"
#include "IECoreMaya/PlaybackFrameList.h"

using namespace IECore;
using namespace boost::python;

namespace IECore
{
	template<>
	std::string repr( IECoreMaya::PlaybackFrameList &x )
	{
		std::stringstream s;

		s << "IECoreMaya.PlaybackFrameList( ";

		object item( x.getRange() );
		s << call_method< std::string >( item.ptr(), "__repr__" );
		s << " ) ";

		return s.str();
	}
}

namespace IECoreMaya
{

void bindPlaybackFrameList()
{	
	typedef class_< PlaybackFrameList, PlaybackFrameList::Ptr, bases< FrameList >, boost::noncopyable > PlaybackFrameListPyClass;
	object o = PlaybackFrameListPyClass ( "PlaybackFrameList", no_init )
		.def( init< PlaybackFrameList::Range >() )
		.add_property( "range", &PlaybackFrameList::getRange, &PlaybackFrameList::setRange )		
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(PlaybackFrameList)	
	;
	
	scope oS( o );
	
	enum_< PlaybackFrameList::Range >( "Range" )
		.value( "Animation", PlaybackFrameList::Animation )
		.value( "Playback", PlaybackFrameList::Playback )
	;		
		
	INTRUSIVE_PTR_PATCH( PlaybackFrameList, PlaybackFrameListPyClass );
	implicitly_convertible<PlaybackFrameListPtr, FrameListPtr>();	
}

}
