//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_OBJECTWRITER_H
#define IE_CORE_OBJECTWRITER_H

#include "IECore/Writer.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// An ObjectWriter writes instances of a single Object to a file with a .cob extension
class ObjectWriter : public Writer
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( ObjectWriter, Writer )

		ObjectWriter( );

		/// Construct a new instance which can write the given object to the specified filename
		ObjectWriter( ObjectPtr object, const std::string &fileName );
		
		static bool canWrite( ConstObjectPtr object, const std::string &fileName );
		
	protected :

		virtual void doWrite();
		
		ObjectParameterPtr m_headerParameter;

	private :
	
		void constructParameters();
	
		static const WriterDescription<ObjectWriter> g_writerDescription;
		
};

IE_CORE_DECLAREPTR( ObjectWriter );

} // namespace IECore

#endif // IE_CORE_OBJECTWRITER_H
