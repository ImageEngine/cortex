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

#ifndef IE_CORE_OBJECTREADER_H
#define IE_CORE_OBJECTEREADER_H

#include "IECore/Reader.h"
#include "IECore/Object.h"
#include "IECore/CompoundData.h"

namespace IECore
{

/// An ObjectReader reads instances of a single Object from a file with a .cob extension
class ObjectReader : public Reader
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( ObjectReader, Reader );

		ObjectReader( );
		/// Construct a new instance to read the given file
		ObjectReader( const std::string &fileName );
		virtual ~ObjectReader();

		/// Returns true if the given file is potentially readable as an Object
		static bool canRead( const std::string &fileName );
		
		/// Returns the file header in the file specified by fileName(). This is intended to 
		/// give fast access to some information about the contents of the file, without
		/// having to load the entire thing.
		virtual CompoundObjectPtr readHeader();
		
	protected:
	
		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );
	
		static IndexedIOInterfacePtr open( const std::string &fileName );
					
	private :
	
		static const ReaderDescription<ObjectReader> g_readerDescription;				
};

IE_CORE_DECLAREPTR( ObjectReader );

} // namespace IECore

#endif // IE_CORE_OBJECTREADER_H
