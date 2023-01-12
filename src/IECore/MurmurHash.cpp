//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MurmurHash.h"
#include "IECore/Exception.h"

#include <boost/format.hpp>

#include <iomanip>
#include <sstream>

using namespace IECore;

namespace
{

std::string internalToString( uint64_t const h1, uint64_t const h2 )
{
	std::stringstream s;
	s << std::hex << std::setfill( '0' ) << std::setw( 16 ) << h1 << std::setw( 16 ) << h2;
	return s.str();
}

void internalFromString( const std::string &repr, uint64_t &h1, uint64_t &h2 )
{
	if( repr.length() != static_cast<std::string::size_type>( 32 ) )
	{
		throw Exception(
			boost::str(
				boost::format(
					"Invalid IECore::MurmurHash string representation \"%s\", must have 32 characters" )
				% repr
		) );
	}

	std::stringstream s;
	s.str( repr.substr( 0, 16 ) );
	s >> std::hex >> h1;
	s.clear();
	s.str( repr.substr( 16, 16 ) );
	s >> std::hex >> h2;
}

} // namespace

MurmurHash::MurmurHash( const std::string &repr )
	:	m_h1( 0 ), m_h2( 0 )
{
	internalFromString( repr, m_h1, m_h2 );
}

std::string MurmurHash::toString() const
{
	return internalToString( m_h1, m_h2 );
}

MurmurHash MurmurHash::fromString( const std::string &repr )
{
	return MurmurHash( repr );
}

std::ostream &IECore::operator << ( std::ostream &o, const MurmurHash &hash )
{
	o << hash.toString();
	return o;
}
