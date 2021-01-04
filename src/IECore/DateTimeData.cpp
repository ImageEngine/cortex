//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DateTimeData.h"

#include "IECore/Export.h"
#include "IECore/MurmurHash.h"
#include "IECore/TypedData.inl"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/date_time/posix_time/posix_time.hpp"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECore
{

static IndexedIO::EntryID g_valueEntry("value");

/// DateTimeData provides a good example for the implementation of a TypedData class
/// wrapping a custom data type. Here we use a macro to quickly implement the required
/// methods of the RunTimeTyped base class that our new class is a descendant of. See
/// further comments for other important details.
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( DateTimeData, DateTimeDataTypeId )

/// Here we must make a specialisation of the TypedData::save() method, capable of storing
/// our custom data type into an IndexedIO container. You may store your data
/// in whatever form is most appropriate, using any of the features of the IndexedIO.
template<>
void DateTimeData::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIO *container = context->rawContainer();

	/// This is cross-platform and handles special values cleanly. It's also going to be smaller than
	/// creating a proper container, and storing the day/month/year/time_of_day components individually.
	/// Boost doesn't make this any easier for us as many of the time functions deal with "long" integer types,
	/// meaning that on 32-bit platforms can't just store the number of nanoseconds since midnight (there are
	/// ~10^14 nanoseconds in a day)
	container->write( g_valueEntry, boost::posix_time::to_iso_string( readable() ) );
}

/// Here we specialise the TypedData::load() method to correctly load the data produced by save().
template<>
void DateTimeData::load( LoadContextPtr context )
{
	Data::load( context );
	const IndexedIO *container = context->rawContainer();

	std::string t;
	container->read( g_valueEntry, t );

	try
	{
		writable() = boost::posix_time::from_iso_string( t );
	}
	catch( const boost::bad_lexical_cast & )
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

/// Here we specialise the SimpleDataHolder::hash() method to appropriately add our internal data to the hash.
template<>
void SimpleDataHolder<boost::posix_time::ptime>::hash( MurmurHash &h ) const
{
	h.append( boost::posix_time::to_iso_string( readable() ) );
}

template class TypedData< boost::posix_time::ptime >;

} // namespace IECore
