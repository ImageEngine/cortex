//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_SXTYPETRAITS_H
#define IECORERI_SXTYPETRAITS_H

#include "sx.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

namespace IECoreRI
{

/// A traits class to help map from the SxType enum values to actual types
/// we would use to represent those values in cortex.
template<SxType>
struct SXTypeTraits
{
	typedef void Type;
};

template<>
struct SXTypeTraits<SxFloat>
{
	typedef float Type;
	typedef IECore::FloatData DataType;
	typedef IECore::FloatVectorData VectorDataType;
};

template<>
struct SXTypeTraits<SxPoint>
{
	typedef Imath::V3f Type;
	typedef IECore::V3fData DataType;
	typedef IECore::V3fVectorData VectorDataType;
};

template<>
struct SXTypeTraits<SxVector>
{
	typedef Imath::V3f Type;
	typedef IECore::V3fData DataType;
	typedef IECore::V3fVectorData VectorDataType;
};

template<>
struct SXTypeTraits<SxNormal>
{
	typedef Imath::V3f Type;
	typedef IECore::V3fData DataType;
	typedef IECore::V3fVectorData VectorDataType;
};

template<>
struct SXTypeTraits<SxColor>
{
	typedef Imath::Color3f Type;
	typedef IECore::Color3fData DataType;
	typedef IECore::Color3fVectorData VectorDataType;
};


} // namespace IECoreRI

#endif // IECORERI_SXTYPETRAITS_H
