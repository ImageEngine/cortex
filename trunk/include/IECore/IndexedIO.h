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

#ifndef IE_CORE_INDEXEDIO_H
#define IE_CORE_INDEXEDIO_H

#include <string>
#include <list>

namespace IECore 
{
namespace IndexedIO
{
	enum OpenModeFlags
	{
		Read      = 1L << 0,
		Write     = 1L << 1,
		Append    = 1L << 2,
		
		Shared    = 1L << 3,
		Exclusive = 1L << 4,
	} ;
	
	typedef unsigned OpenMode;
	
	typedef enum
	{
		Directory=0,
		File
	} EntryType;
	
			
	typedef enum
	{
		Invalid=0,
		Float,
		FloatArray,
		Double,
		DoubleArray,
		Int,
		IntArray,
		Long,
		LongArray,
		String,
		StringArray,
		UInt,
		UIntArray,
		Char,
		CharArray,
		UChar,
		UCharArray,
		Half,
		HalfArray
	} DataType;
	
	typedef std::string EntryID;
	
	/// A representation of a single file/directory
	class Entry
	{	
		public:
			Entry();
			
			Entry( const EntryID &id, EntryType eType, DataType dType, unsigned long arrayLength);

			/// ID, or name, of the file/directory
			EntryID id() const;

			/// Returns either Directory or File.
			EntryType entryType() const;
		
			/// Should only be called on instances which represent files. Returns the type of data held by in the file. If this entry does not represent a file
			/// an IOException is thrown.
			DataType dataType() const;
		
			/// Convenience method to calculate size of array. If Entry's datatype is not an array then an IOException is thrown. 
			unsigned long arrayLength() const;
		
			EntryID m_ID;
			EntryType m_entryType;
			DataType m_dataType;
			unsigned long m_arrayLength;
	};	
	
	typedef std::list< Entry > EntryList;	
	
	// Method for establishing flattened size of a data object
	template<typename T>
	struct DataSizeTraits;
	
	// Method for flatting/unflattening data objects
	template<typename T>
	struct DataFlattenTraits;
	
	template<typename T>
	struct DataTypeTraits;
}	
}

#include "IndexedIO.inl"

#endif // IE_CORE_INDEXEDIO_H
