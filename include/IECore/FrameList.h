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

#ifndef IE_CORE_FRAMELIST_H
#define IE_CORE_FRAMELIST_H

#include <vector>
#include <string>

#include "IECore/RunTimeTyped.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( FrameList );

/// The FrameList class defines an abstract base class for classes which are
/// able to generate a list of frame numbers.
class FrameList : public RunTimeTyped
{
	public:
	
		typedef int64_t Frame;
	
		IE_CORE_DECLARERUNTIMETYPED( FrameList, RunTimeTyped );
	
		FrameList();
		virtual ~FrameList();
		
		/// This method returns a simple list of frames. They are not guaranteed to
		/// be in ascending order, but they are guaranteed to be non repeating - make
		/// sure you honour these requirements when creating subclasses.
		virtual void asList( std::vector<Frame> &frames ) const = 0;		
		virtual std::string asString() const = 0;
		virtual bool isEqualTo( ConstFrameListPtr other ) const;		
		virtual FrameListPtr copy() const = 0;
		
		/// This method takes the list of frames returned by asList() and returns a list
		/// of lists of frames, where each sublist contains no more than clumpSize frames.
		void asClumpedList( std::vector< std::vector<Frame> > &clumpedFrames, unsigned int clumpSize ) const;
		
		/// Parses a string and returns the FrameList object that it represents.
		/// Strings may be in any of the forms returned by str( SomeFrameListSubclass ).
		/// Subclasses must register a suitable parser for the form that they return
		/// by instantiating a static FrameList::Parser instance. Throws an Exception if the string 
		/// is in an unrecognised form.	
		static FrameListPtr parse( const std::string &frameList );
		
		bool operator ==( const FrameList &other ) const;
		
	protected :
	
		typedef FrameListPtr (*ParserFn)( const std::string &frameList );
		
		static void registerParser( ParserFn fn );
		
		template<class T>
		class Parser
		{
			public :
				Parser();
		};	
		
	private :
	
		typedef std::vector< ParserFn > ParserList;
		
		static ParserList *parserList();
		
};
	
} // namespace IECore

#include "IECore/FrameList.inl"

#endif // IE_CORE_FRAMELIST_H
