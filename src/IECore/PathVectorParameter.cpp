//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PathVectorParameter.h"
#include "IECore/CompoundObject.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include <algorithm>

using namespace boost;
using namespace std;
using namespace IECore;

PathVectorParameter::PathVectorParameter( const std::string &name, const std::string &description,
                              const std::vector<std::string> &defaultValue, bool allowEmptyList, CheckType check,
                              const StringVectorParameter::PresetsMap &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
		:	StringVectorParameter( name, description, defaultValue, presets, presetsOnly, userData ), m_allowEmptyList( allowEmptyList ), m_check( check )
{
}

bool PathVectorParameter::allowEmptyList() const
{
	return m_allowEmptyList;
}

bool PathVectorParameter::mustExist() const
{
	return ( m_check == PathVectorParameter::MustExist );
}

bool PathVectorParameter::mustNotExist() const
{
	return ( m_check == PathVectorParameter::MustNotExist );
}

bool PathVectorParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	/// \todo Validate the individual path names using utilities found in boost::filesystem

	if ( !StringVectorParameter::valueValid( value, reason ) )
	{
		return false;
	}
	// if the above passed we know we have a string vector
	ConstStringVectorDataPtr s = static_pointer_cast<const StringVectorData>( value );

	// empty check
	if ( !allowEmptyList() && !s->readable().size() )
	{
		if ( reason )
		{
			*reason = "Empty filename not allowed";
		}
		return false;
	}
	else
	{
		if ( !s->readable().size() )
		{
			// if empty lists are allowed and we have one then we should
			// skip the other checks
			return true;
		}
	}

	// valid path check
	try
	{
		for ( StringVectorData::ValueType::const_iterator it = s->readable().begin(); it != s->readable().end(); ++it )
		{		
			boost::filesystem::path p( *it );
		}
	}
	catch ( ... )
	{
		if ( reason )
		{
			*reason = "Path has invalid form";
		}
		return false;
	}

	// existence check
	for ( StringVectorData::ValueType::const_iterator it = s->readable().begin(); it != s->readable().end(); ++it )
	{
		if ( boost::filesystem::exists(boost::filesystem::path( *it )))
		{
			if ( mustNotExist() )
			{
				if ( reason )
				{
					*reason = "\"" + *it + "\" already exists.";
				}
				return false;
			}
		}
		else
		{
			if ( mustExist() )
			{
				if ( reason )
				{
					*reason = "\"" + *it + "\" does not exist.";
				}
				return false;
			}
		}
	}

	return true;
}
