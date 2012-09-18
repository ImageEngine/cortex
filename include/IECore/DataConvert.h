//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_DATACONVERT_H
#define IE_CORE_DATACONVERT_H

#include "IECore/TypeTraits.h"

namespace IECore
{

/// DataConvert is a function object able to convert Simple- and VectorTypedData from
/// one type to another, using a specified Conversion. The ImageReader subclasses,
/// for example use this extensively in order to convert various signed and
/// unsigned data arrays into float arrays in a consistent manner. An example
/// usage might be:
///
/// \code
/// DataConvert< UIntVectorData, FloatVectorData, ScaledDataConversion< unsigned int, float > > converter;
/// FloatVectorPataPtr result = convert( myUIntVectorDataPtr );
/// \endcode
///
/// The "Enable" template parameter is for internal use only.
///
/// There are two variants of the function, one which constructs the conversion using its default
/// constructor, the other takes an instance of the Conversion class (to allow for external
/// initialization)
template<typename From, typename To, typename Conversion, typename Enable = void>
struct DataConvert
{
	typename To::Ptr operator()( typename From::ConstPtr from );

	typename To::Ptr operator()( typename From::ConstPtr from, Conversion &c );
};

} // namespace IECore

#include "DataConvert.inl"

#endif // IE_CORE_DATACONVERT_H
