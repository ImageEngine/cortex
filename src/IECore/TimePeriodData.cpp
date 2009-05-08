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

#include "IECore/TimePeriodData.h"
#include "IECore/TypedData.inl"
#include "IECore/DateTimeData.h"

namespace IECore
{

IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( TimePeriodData, TimePeriodDataTypeId )
IE_CORE_DEFINETYPEDDATANOBASESIZE( TimePeriodData )

template<>
void TypedData< TimePeriod >::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );

	container->write( "begin", boost::posix_time::to_iso_string( readable().begin() ) );
	container->write( "end", boost::posix_time::to_iso_string( readable().end() ) );
}

template<>
void TypedData< TimePeriod >::load( LoadContextPtr context )
{
	Data::load( context );

	unsigned int v = 0;

	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );

	std::string beginStr;
	container->read( "begin", beginStr );
	boost::posix_time::ptime begin;
	try
	{
		begin = boost::posix_time::from_iso_string( beginStr );
	}
	catch ( boost::bad_lexical_cast )
	{
		/// Do these checks here instead of first as they're likely to be the least-used cases.
		if ( beginStr == "not-a-date-time" )
		{
			begin = boost::posix_time::not_a_date_time;
		}
		else if ( beginStr == "+infinity" )
		{
			begin = boost::posix_time::pos_infin;
		}
		else if ( beginStr == "-infinity" )
		{
			begin = boost::posix_time::neg_infin;
		}
		else
		{
			throw;
		}
	}

	std::string endStr;
	container->read( "end", endStr );
	boost::posix_time::ptime end;
	try
	{
		end = boost::posix_time::from_iso_string( endStr );
	}
	catch ( boost::bad_lexical_cast )
	{
		/// Do these checks here instead of first as they're likely to be the least-used cases.
		if ( endStr == "not-a-date-time" )
		{
			end = boost::posix_time::not_a_date_time;
		}
		else if ( endStr == "+infinity" )
		{
			end = boost::posix_time::pos_infin;
		}
		else if ( endStr == "-infinity" )
		{
			end = boost::posix_time::neg_infin;
		}
		else
		{
			throw;
		}
	}

	writable() = boost::posix_time::time_period( begin, end );
}

template class TypedData< TimePeriod >;

} // namespace IECore
