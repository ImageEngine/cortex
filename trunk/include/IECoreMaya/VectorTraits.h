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

#ifndef IE_COREMAYA_VECTORTRAITS_H
#define IE_COREMAYA_VECTORTRAITS_H

#include "IECore/VectorTraits.h"

#include "maya/MPoint.h"
#include "maya/MFloatPoint.h"
#include "maya/MVector.h"
#include "maya/MFloatVector.h"

namespace IECore
{

template<>
struct VectorTraits<MPoint>
{
	typedef double BaseType;
	static unsigned int dimensions() { return 4; };
	static double get( const MPoint &v, unsigned int i ) { return v[i]; };
	static void set( MPoint &v, unsigned int i, double x ) { v[i] = x; };
};

template<>
struct VectorTraits<MFloatPoint>
{
	typedef double BaseType;
	static unsigned int dimensions() { return 4; };
	static float get( const MFloatPoint &v, unsigned int i ) { return v[i]; };
	static void set( MFloatPoint &v, unsigned int i, float x ) { v[i] = x; };
};

template<>
struct VectorTraits<MVector>
{
	typedef double BaseType;
	static unsigned int dimensions() { return 3; };
	static double get( const MVector &v, unsigned int i ) { return v[i]; };
	static void set( MVector &v, unsigned int i, double x ) { v[i] = x; };
};

template<>
struct VectorTraits<MFloatVector>
{
	typedef float BaseType;
	static unsigned int dimensions() { return 3; };
	static float get( const MFloatVector &v, unsigned int i ) { return v[i]; };
	static void set( MFloatVector &v, unsigned int i, float x ) { v[i] = x; };
};

} // namespace IECore

#endif // IE_COREMAYA_VECTORTRAITS_H
