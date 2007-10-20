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

#include "IECore/DirNameParameter.h"
#include "IECore/CompoundObject.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include <algorithm>

using namespace boost;
using namespace std;
using namespace IECore;

DirNameParameter::DirNameParameter( const std::string &name, const std::string &description,
			const std::string &defaultValue, bool allowEmptyString,  PathParameter::CheckType check,
			const StringParameter::PresetsMap &presets, bool presetsOnly, ConstCompoundObjectPtr userData )
	:	PathParameter( name, description, defaultValue, allowEmptyString, check, presets, presetsOnly, userData )
{
}

bool DirNameParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	if (!PathParameter::valueValid( value, reason ) )
	{
		return false;
	}
	
	ConstStringDataPtr s = static_pointer_cast<const StringData>( value );

	// empty check
	if( s->readable()=="" )
	{
		// if empty strings are allowed and we have one then we should
		// skip the other checks
		return true;
	}
	
	// dir type check
	if( boost::filesystem::exists(boost::filesystem::path(s->readable(), boost::filesystem::native)) &&
		 !boost::filesystem::is_directory(boost::filesystem::path(s->readable(), boost::filesystem::native)))
	{
		if( reason )
		{
			*reason = "\"" + s->readable() + "\" is not a directory!";
		}
		return false;
	}
	return true;
}
