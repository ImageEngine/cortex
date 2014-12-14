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

#ifndef IE_CORE_FRAMERANGE_H
#define IE_CORE_FRAMERANGE_H

#include "IECore/Export.h"
#include "IECore/FrameList.h"

namespace IECore
{

/// The FrameRange class simply stores an integer range consisting of a start, end and step.
/// Exceptions are thrown if these members are modified in such a way as to create an invalid range.
/// \ingroup fileSequenceGroup
class IECORE_API FrameRange : public FrameList
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( FrameRange, FrameList );

		FrameRange( Frame start, Frame end, Frame step = 1 );
		virtual ~FrameRange();

		virtual void asList( std::vector<Frame> &frames ) const ;
		virtual std::string asString() const ;
		virtual bool isEqualTo( ConstFrameListPtr other ) const;
		virtual FrameListPtr copy() const;

		static FrameListPtr parse( const std::string &frameList );

		Frame getStart();
		void setStart( Frame start );

		Frame getEnd();
		void setEnd( Frame end );

		Frame getStep();
		void setStep( Frame step );

	protected :

		Frame m_start;
		Frame m_end;
		Frame m_step;

	private :

		static FrameList::Parser< FrameRange > g_parserRegistrar;
};

IE_CORE_DECLAREPTR( FrameRange );

} // namespace IECore


#endif // IE_CORE_FRAMERANGE_H
