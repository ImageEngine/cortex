//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_DATETIMEDATA_H
#define IECORE_DATETIMEDATA_H

#include "IECore/Export.h"
#include "IECore/TypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/date_time/posix_time/ptime.hpp"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECore
{

/// DateTimeData provides a good example for the implementation of a TypedData class
/// wrapping a custom data type. Here we declare a new class named DateTimeData, which
/// wraps the boost ptime class, has no base type (hence the void argument) and will use
/// a SimpleDataHolder to store the value internally. Were ptime to be a type requiring
/// much more memory then SharedDataHolder would have been a more appropriate choice for
/// the latter. See DateTimeData.cpp for further details.
IECORE_DECLARE_TYPEDDATA( DateTimeData, boost::posix_time::ptime, void, SimpleDataHolder )

IECORE_API void murmurHashAppend( IECore::MurmurHash &h, const boost::posix_time::ptime &time );

} // namespace IECore

#endif // IECORE_DATETIMEDATA_H
