//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/PrimitiveVariable.h"

using namespace IECore;

PrimitiveVariable::PrimitiveVariable()
	: interpolation( Invalid ), data( nullptr ), indices( nullptr )
{
}

PrimitiveVariable::PrimitiveVariable( Interpolation i, DataPtr d )
	: interpolation( i ), data( d ), indices( nullptr )
{
}

PrimitiveVariable::PrimitiveVariable( Interpolation i, DataPtr d, IntVectorDataPtr indices )
	: interpolation( i ), data( d ), indices( indices )
{
}

PrimitiveVariable::PrimitiveVariable( const PrimitiveVariable &other )
{
	interpolation = other.interpolation;
	data = other.data;
	indices = other.indices;
}

PrimitiveVariable::PrimitiveVariable( const PrimitiveVariable &other, bool deepCopy )
{
	interpolation = other.interpolation;
	if( deepCopy )
	{
		data = other.data ? other.data->copy() : nullptr;
		indices = other.indices ? other.indices->copy() : nullptr;
	}
	else
	{
		data = other.data;
		indices = other.indices;
	}
}

bool PrimitiveVariable::operator==( const PrimitiveVariable &other ) const
{
	if( interpolation != other.interpolation )
	{
		return false;
	}

	// if they're both null then they're both equal
	bool dataEqual = !data && !other.data;
	bool indicesEqual = !indices && !other.indices;

	if( data && other.data )
	{
		dataEqual = data->isEqualTo( other.data.get() );
	}

	if( indices && other.indices )
	{
		indicesEqual = indices->isEqualTo( other.indices.get() );
	}

	return dataEqual && indicesEqual;
}

bool PrimitiveVariable::operator!=( const PrimitiveVariable &other ) const
{
	return !(*this == other);
}

