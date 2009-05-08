//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/date_time/posix_time/posix_time.hpp"

#include "IECore/DateTimeData.h"
#include "IECore/TypedData.inl"

namespace IECore
{

IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( DateTimeData, DateTimeDataTypeId )
IE_CORE_DEFINETYPEDDATANOBASESIZE( DateTimeData )

template<>
void DateTimeData::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIOInterfacePtr container = context->rawContainer();

	/// This is cross-platform and handles special values cleanly. It's also going to be smaller than
	/// creating a proper container, and storing the day/month/year/time_of_day components individually.
	/// Boost doesn't make this any easier for us as many of the time functions deal with "long" integer types,
	/// meaning that on 32-bit platforms can't just store the number of nanoseconds since midnight (there are
	/// ~10^14 nanoseconds in a day)
	container->write( "value", boost::posix_time::to_iso_string( readable() ) );
}

template<>
void DateTimeData::load( LoadContextPtr context )
{
	Data::load( context );
	IndexedIOInterfacePtr container = context->rawContainer();

	std::string t;
	container->read( "value", t );

	try
	{
		writable() = boost::posix_time::from_iso_string( t );
	}
	catch ( boost::bad_lexical_cast )
	{
		/// Do these checks here instead of first as they're likely to be the least-used cases.
		if ( t == "not-a-date-time" )
		{
			writable() = boost::posix_time::not_a_date_time;
		}
		else if ( t == "+infinity" )
		{
			writable() = boost::posix_time::pos_infin;
		}
		else if ( t == "-infinity" )
		{
			writable() = boost::posix_time::neg_infin;
		}
		else
		{
			throw;
		}
	}
}

template class TypedData< boost::posix_time::ptime >;

} // namespace IECore
