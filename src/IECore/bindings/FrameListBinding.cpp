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

#include "IECore/FrameList.h"
#include "IECore/Exception.h"

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/FrameListBinding.h"

using namespace boost::python;

namespace IECore 
{

struct FrameListHelper
{	
	static list asList( FrameListPtr l )
	{
		std::vector<FrameList::Frame> frames;
		l->asList( frames );

		list result;

		for ( std::vector<FrameList::Frame>::const_iterator it = frames.begin(); it != frames.end(); ++it )
		{
			result.append( *it );
		}

		return result;
	}

	static list asClumpedList( FrameListPtr l, unsigned int clumpSize )
	{
		std::vector< std::vector<FrameList::Frame> > clumpList;
		l->asClumpedList( clumpList, clumpSize );

		list result;

		for ( std::vector< std::vector<FrameList::Frame> >::const_iterator clumpIt = clumpList.begin(); clumpIt != clumpList.end(); ++clumpIt )
		{
			list clump;

			for ( std::vector<FrameList::Frame>::const_iterator frameIt = clumpIt->begin(); frameIt != clumpIt->end(); ++ frameIt )
			{
				clump.append( *frameIt );
			}

			result.append( clump );
		}

		return result;
	}	
};

void bindFrameList()
{	
	RunTimeTypedClass<FrameList>()
		.def( "asList", FrameListHelper::asList )
		.def( "isEqualTo", &FrameList::isEqualTo )
		.def( "copy", &FrameList::copy )
		.def( "asClumpedList", &FrameListHelper::asClumpedList )
		.def( "parse", &FrameList::parse ).staticmethod( "parse" )
		.def( "__str__", &FrameList::asString )
		.def( self == self )
	;	
}

}
