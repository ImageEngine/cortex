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

#include <cassert>

#include "IECore/Exception.h"
#include "IECore/IndexedIO.h"

using namespace IECore::IndexedIO;

Entry::Entry() : m_ID(""), m_entryType( IndexedIO::Directory), m_dataType( IndexedIO::Invalid), m_arrayLength(0)
{
}

Entry::Entry( const EntryID &id, EntryType eType, DataType dType, unsigned long arrayLength)
: m_ID(id), m_entryType(eType), m_dataType(dType), m_arrayLength(arrayLength)
{
}

const EntryID &Entry::id() const
{
	return m_ID;
}

EntryType Entry::entryType() const
{
	return m_entryType;
}

DataType Entry::dataType() const
{
	if (m_entryType == Directory)
	{
		throw IOException(m_ID);
	}
	
	return m_dataType;
}

bool Entry::isArray() const
{
	return isArray( m_dataType );
}

bool Entry::isArray( DataType dType )
{
	switch( dType )
	{
		case FloatArray:
		case DoubleArray:
		case HalfArray:
		case IntArray:
		case LongArray:
		case StringArray:
		case UIntArray:
		case CharArray:
		case UCharArray:
		case ShortArray:
		case UShortArray:
		case Int64Array:
		case UInt64Array:
			return true;
		default:
			return false;
	}
}

unsigned long Entry::arrayLength() const
{
	if ( !isArray() )
	{
		throw IOException(m_ID);
	}
	
	return m_arrayLength;	
}
