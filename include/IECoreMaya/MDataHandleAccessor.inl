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

#ifndef IE_COREMAYA_MDATAHANDLEACCESSOR_INL
#define IE_COREMAYA_MDATAHANDLEACCESSOR_INL

#include "maya/MDataHandle.h"
#include "maya/MVector.h"
#include "maya/MFloatVector.h"
#include "maya/MMatrix.h"
#include "maya/MFloatMatrix.h"
#include "maya/MObject.h"
#include "maya/MTime.h"
#include "maya/MDistance.h"
#include "maya/MAngle.h"
#include "maya/MString.h"

/// Specialisations of MDataHandleAccessor for the various return types
namespace IECoreMaya
{

template<>
struct MDataHandleAccessor< MObject >
{
	typedef MObject ReturnType;
	
	static ReturnType get( MDataHandle &h )
	{
		return h.data();	
	}
};

template<>
struct MDataHandleAccessor< bool >
{
	typedef bool &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asBool();	
	}
};

template<>
struct MDataHandleAccessor< char >
{
	typedef char &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asChar();	
	}
};

template<>
struct MDataHandleAccessor< unsigned char >
{
	typedef unsigned char &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asUChar();	
	}
};

template<>
struct MDataHandleAccessor< short >
{
	typedef short &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asShort();	
	}
};

template<>
struct MDataHandleAccessor< int >
{
	typedef int &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asInt();	
	}
};

template<>
struct MDataHandleAccessor< void* >
{
	typedef void *&ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asAddr();	
	}
};

template<>
struct MDataHandleAccessor< float >
{
	typedef float &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asFloat();	
	}
};

template<>
struct MDataHandleAccessor< double >
{
	typedef double &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asDouble();	
	}
};

template<>
struct MDataHandleAccessor< MDistance >
{
	typedef MDistance ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asDistance();	
	}
};

template<>
struct MDataHandleAccessor< MAngle >
{
	typedef MAngle ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asAngle();	
	}
};

template<>
struct MDataHandleAccessor< MTime >
{
	typedef MTime ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asTime();	
	}
};

template<>
struct MDataHandleAccessor< short2 >
{
	typedef short2 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asShort2();	
	}
};

template<>
struct MDataHandleAccessor< int2 >
{
	typedef int2 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asInt2();	
	}
};

template<>
struct MDataHandleAccessor< float2 >
{
	typedef float2 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asFloat2();	
	}
};

template<>
struct MDataHandleAccessor< double2 >
{
	typedef double2 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asDouble2();	
	}
};

template<>
struct MDataHandleAccessor< short3 >
{
	typedef short3 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asShort3();	
	}
};

template<>
struct MDataHandleAccessor< int3 >
{
	typedef int3 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asInt3();	
	}
};

template<>
struct MDataHandleAccessor< float3 >
{
	typedef float3 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asFloat3();	
	}
};

template<>
struct MDataHandleAccessor< double3 >
{
	typedef double3 &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asDouble3();	
	}
};

template<>
struct MDataHandleAccessor< MVector >
{
	typedef MVector &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asVector();	
	}
};

template<>
struct MDataHandleAccessor< MFloatVector >
{
	typedef MFloatVector &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asFloatVector();	
	}
};

template<>
struct MDataHandleAccessor< MMatrix >
{
	typedef MMatrix &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asMatrix();	
	}
};

template<>
struct MDataHandleAccessor< MFloatMatrix >
{
	typedef MFloatMatrix &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asFloatMatrix();	
	}
};

template<>
struct MDataHandleAccessor< MString >
{
	typedef MString &ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asString();	
	}
};

template<>
struct MDataHandleAccessor< MPxData * >
{
	typedef MPxData *ReturnType;
	
	static ReturnType get( const MDataHandle &h )
	{
		return h.asPluginData();	
	}
};

} // namespace IECoreMaya


#endif // IE_COREMAYA_MDATAHANDLEACCESSOR_INL
