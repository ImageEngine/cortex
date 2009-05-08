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

#ifndef IE_CORE_REORDEREDFRAMELIST_H
#define IE_CORE_REORDEREDFRAMELIST_H

#include "IECore/FrameList.h"

namespace IECore
{

/// The ReorderedFrameList is a base class for any FrameList classes which
/// hold a child FrameList and return some reordering of that list in the
/// asList() method.
class ReorderedFrameList : public FrameList
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( ReorderedFrameList, FrameList );

		ReorderedFrameList( FrameListPtr frameList );

		FrameListPtr getFrameList();
		void setFrameList( FrameListPtr frameList );

		virtual ~ReorderedFrameList();

		virtual bool isEqualTo( ConstFrameListPtr other ) const;

	protected :

		FrameListPtr m_frameList;

		/// This utility can be used by the parse functions in derived classes.
		/// It matches strings like "(...)s" or "...s", where s is the suffix. It
		/// then returns a FrameList parsed from the "..." section.
		template< typename T >
		static FrameListPtr parseForChildList( const std::string &frameList );
};

IE_CORE_DECLAREPTR( ReorderedFrameList );

} // namespace IECore

#include "IECore/ReorderedFrameList.inl"

#endif // IE_CORE_REORDEREDFRAMELIST_H
