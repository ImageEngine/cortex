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

#ifndef IE_MAYACORE_MFNDATATYPETRAITS_H
#define IE_MAYACORE_MFNDATATYPETRAITS_H

#include "maya/MFnData.h"

#include "maya/MUintArray.h"
#include "maya/MUint64Array.h"
#include "maya/MStringArray.h"
#include "maya/MTimeArray.h"
#include "maya/MPlugArray.h"
#include "maya/MObjectArray.h"
#include "maya/MIntArray.h"
#include "maya/MFloatArray.h"
#include "maya/MDoubleArray.h"
#include "maya/MDagPathArray.h"
#include "maya/MColorArray.h"
#include "maya/MVectorArray.h"
#include "maya/MFloatVectorArray.h"
#include "maya/MPointArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFnUInt64ArrayData.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MFnIntArrayData.h"
#include "maya/MFnDoubleArrayData.h"
#include "maya/MFnVectorArrayData.h"
#include "maya/MFnPointArrayData.h"

namespace IECoreMaya
{

/// This traits class is useful for determining various bits
/// of information about the simple MFnData types, for use in template
/// code.
template<MFnData::Type T>
struct MFnDataTypeTraits
{
	/// The Maya value type
	typedef void ValueType;

	/// The Dependecny Graph data type
	static MFn::Type dataType()
	{
		return MFn::kInvalid;
	}
};

template<>
struct MFnDataTypeTraits<MFnData::kMatrix>
{
	typedef MMatrix ValueType;

	static MFn::Type dataType()
	{
		return MFn::kMatrixData;
	}
};

template<>
struct MFnDataTypeTraits<MFnData::kString>
{
	typedef MString ValueType;

	static MFn::Type dataType()
	{
		return MFn::kStringData;
	}
};

template<>
struct MFnDataTypeTraits<MFnData::kStringArray>
{
	typedef MStringArray ValueType;

	static MFn::Type dataType()
	{
		return MFn::kStringArrayData;
	}
};

template<>
struct MFnDataTypeTraits<MFnData::kVectorArray>
{
	typedef MVectorArray ValueType;

	static MFn::Type dataType()
	{
		return MFn::kVectorArrayData;
	}
};

template<>
struct MFnDataTypeTraits<MFnData::kPointArray>
{
	typedef MPointArray ValueType;

	static MFn::Type dataType()
	{
		return MFn::kPointArrayData;
	}
};

template<>
struct MFnDataTypeTraits<MFnData::kIntArray>
{
	typedef MIntArray ValueType;

	static MFn::Type dataType()
	{
		return MFn::kIntArrayData;
	}
};

template<>
struct MFnDataTypeTraits<MFnData::kDoubleArray>
{
	typedef MDoubleArray ValueType;

	static MFn::Type dataType()
	{
		return MFn::kDoubleArrayData;
	}
};

} // namespace IECoreMaya

#endif // IE_MAYACORE_MFNDATATYPETRAITS_H
