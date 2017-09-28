//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Export.h"
#include "IECore/TimeCodeData.h"
#include "IECore/TypedData.inl"
#include "IECore/MurmurHash.h"

namespace IECore
{

static IndexedIO::EntryID g_valueEntry("value");

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TimeCodeData, TimeCodeDataTypeId )

template<>
void TimeCodeData::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIO *container = context->rawContainer();

	const Imf::TimeCode &timeCode = readable();
	/// \todo: should we be using FILM24_PACKING rather than the default?
	unsigned data[2] = { timeCode.timeAndFlags(), timeCode.userData() };
	container->write( g_valueEntry, data, 2 );
}

template<>
void TimeCodeData::load( LoadContextPtr context )
{
	Data::load( context );
	const IndexedIO *container = context->rawContainer();

	unsigned data[2];
	unsigned *dataPtr = &data[0];
	container->read( g_valueEntry, dataPtr, 2 );

	Imf::TimeCode &timeCode = writable();
	timeCode.setTimeAndFlags( data[0] );
	timeCode.setUserData( data[1] );
}

template<>
bool TimeCodeData::isEqualTo( const Object *other ) const
{
	if( !Data::isEqualTo( other ) )
	{
		return false;
	}

	const Imf::TimeCode &thisCode = readable();
	const TimeCodeData *tOther = static_cast<const TimeCodeData *>( other );
	const Imf::TimeCode &thatCode = tOther->readable();

	return ( thisCode.timeAndFlags() == thatCode.timeAndFlags() && thisCode.userData() == thatCode.userData() );
}

template<>
void SimpleDataHolder<Imf::TimeCode>::hash( MurmurHash &h ) const
{
	const Imf::TimeCode &timeCode = readable();
	h.append( Imath::V2i( timeCode.timeAndFlags(), timeCode.userData() ) );
}

template class IECORE_API TypedData<Imf::TimeCode>;

} // namespace IECore
