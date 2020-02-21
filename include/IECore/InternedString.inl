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

#ifndef IECORE_INTERNEDSTRING_INL
#define IECORE_INTERNEDSTRING_INL

namespace IECore
{

inline InternedString::InternedString()
	:	m_value( InternedString::emptyString().m_value )
{
}

inline InternedString::InternedString( const std::string &value )
	:	m_value( internedString( value.c_str() ) )
{
}

inline InternedString::InternedString( const InternedString &other )
	:	m_value( other.m_value )
{
}

inline InternedString::InternedString( const char *value )
	:	m_value( internedString( value ) )
{
}

inline InternedString::InternedString( const char *value, size_t length )
	:	m_value( internedString( value, length ) )
{
}

#if BOOST_VERSION > 105500

inline InternedString::InternedString( const boost::string_view &value )
	:	InternedString( value.data(), value.size() )
{
}

#endif

inline InternedString::InternedString( int64_t number )
	: m_value( numberString( number ).m_value )
{
}

inline InternedString::~InternedString()
{
}

inline bool InternedString::operator != ( const InternedString &other ) const
{
	return m_value!=other.m_value;
}

inline bool InternedString::operator == ( const InternedString &other ) const
{
	return m_value==other.m_value;
}

inline bool InternedString::operator < ( const InternedString &other ) const
{
	return m_value < other.m_value;
}

inline InternedString::operator const std::string & () const
{
	return *m_value;
}

inline const std::string &InternedString::value() const
{
	return *m_value;
}

inline const std::string &InternedString::string() const
{
	return *m_value;
}

inline const char *InternedString::c_str() const
{
	return m_value->c_str();
}

} // namespace IECore

namespace std
{

template <>
struct hash<IECore::InternedString>
{

	size_t operator()( const IECore::InternedString &s ) const
	{
		return std::hash<std::string>()( s.string() );
	}

};

} // namespace std

#endif // IECORE_INTERNEDSTRING_INL
