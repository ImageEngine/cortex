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

#ifndef IE_CORE_REVERSEDFRAMELIST_H
#define IE_CORE_REVERSEDFRAMELIST_H

#include "IECore/ReorderedFrameList.h"

namespace IECore
{

/// The ReversedFrameList class is a ReorderedFrameList which does a sort
/// of binary refinement thing on the child frame list. This is useful
/// when rendering a sequence of images, as you get a slow refinement of
/// the whole sequence, providing earlier information about stuff going on
/// in the middle and end of the sequence.
class ReversedFrameList : public ReorderedFrameList
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( ReversedFrameList, ReorderedFrameList );

		ReversedFrameList( FrameListPtr frameList = 0 );

		virtual ~ReversedFrameList();

		virtual void asList( std::vector<Frame> &frames ) const ;
		virtual std::string asString() const;
		virtual bool isEqualTo( ConstFrameListPtr other ) const ;
		virtual FrameListPtr copy() const ;

		static std::string suffix();

		static FrameListPtr parse( const std::string &frameList );

	private :

		static FrameList::Parser< ReversedFrameList > g_parserRegistrar;
};

IE_CORE_DECLAREPTR( ReversedFrameList );

} // namespace IECore

#endif // IE_CORE_REVERSEDFRAMELIST_H
