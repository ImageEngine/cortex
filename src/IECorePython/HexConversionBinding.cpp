//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/HexConversion.h"
#include "IECore/VectorTypedData.h"

#include "IECorePython/HexConversionBinding.h"

using namespace boost::python;

namespace IECorePython
{

template<typename T>
static std::string decToHexVector( typename T::ConstPtr v )
{
	const typename T::ValueType &c = v->readable();
	return IECore::decToHex( c.begin(), c.end() );
}

template<typename T>
static typename T::Ptr hexToDecVector( const char *s )
{
	typename T::Ptr result = new T;
	IECore::hexToDec<typename T::ValueType::value_type>( s, s + strlen( s ), std::back_insert_iterator<typename T::ValueType>( result->writable() ) );
	return result;
}

void bindHexConversion()
{
	def( "hexToDecChar", (char (*)( const std::string & ))IECore::hexToDec<char> );
	def( "hexToDecCharVector", hexToDecVector<IECore::CharVectorData> );
	def( "decToHexChar", (std::string (*)( char ))IECore::decToHex<char> );
	def( "decToHexCharVector", decToHexVector<IECore::CharVectorData> );
	def( "hexToDecUInt", (unsigned int (*)( const std::string & ))IECore::hexToDec<unsigned int> );
	def( "hexToDecUIntVector", hexToDecVector<IECore::UIntVectorData> );
	def( "decToHexUInt", (std::string (*)( unsigned int ))IECore::decToHex<unsigned int> );
	def( "decToHexUIntVector", decToHexVector<IECore::UIntVectorData> );
}

} // namespace IECorePython
