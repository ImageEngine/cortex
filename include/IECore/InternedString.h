//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_INTERNEDSTRING_H
#define IECORE_INTERNEDSTRING_H

#include <string>
#include <stdint.h>

namespace IECore
{

/// The InternedString class provides a means of efficiently storing
/// multiple different objects with the same string value. It does this
/// by keeping a static table with the actual values in it, with
/// the object instances just referencing the values in the table.
/// \ingroup utilityGroup
class InternedString
{
	public :

		inline InternedString();
		inline InternedString( const std::string &value );
		inline InternedString( const InternedString &other );
		inline InternedString( const char *value );
		inline InternedString( ::int64_t number );

		inline ~InternedString();

		// The equality operators are extremely fast because they
		// need only compare the address of the internal string,
		// because it was made unique upon construction.
		inline bool operator != ( const InternedString &other ) const;
		inline bool operator == ( const InternedString &other ) const;
		/// Note that this compares the addresses of the internal
		/// unique strings, rather than performing an actual string
		/// comparison.
		inline bool operator < ( const InternedString &other ) const;

		inline operator const std::string & () const;

		inline const std::string &value() const;
		inline const std::string &string() const;
		inline const char *c_str() const;

		static size_t numUniqueStrings();

	private :

		static const std::string *internedString( const char *value );

		const std::string *m_value;

		static const InternedString &emptyString();
		static const InternedString &numberString( int64_t number );

};

std::ostream &operator << ( std::ostream &o, const InternedString &str );

} // namespace IECore

#include "IECore/InternedString.inl"

#endif // IECORE_INTERNEDSTRING_H
