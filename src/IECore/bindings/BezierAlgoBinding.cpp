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

#include <boost/python.hpp>

#include "IECore/bindings/BezierAlgoBinding.h"
#include "IECore/BezierAlgo.h"

using namespace boost::python;

namespace IECore
{

template<typename V>
struct BezierCallback
{
	object c;
	void operator()( const V &v )
	{
		c( v );
	}

};

template<typename Vec>
void bezierSubdivideBinding( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &v3, typename Vec::BaseType tolerance, object f )
{
	BezierCallback<Vec> c; c.c = f;
	bezierSubdivide( v0, v1, v2, v3, tolerance, c );
}

void bindBezierAlgo()
{
	def( "bezierSubdivide", &bezierSubdivideBinding<Imath::V2f> );
	def( "bezierSubdivide", &bezierSubdivideBinding<Imath::V2d> );
	def( "bezierSubdivide", &bezierSubdivideBinding<Imath::V3f> );
	def( "bezierSubdivide", &bezierSubdivideBinding<Imath::V3d> );
}

} // namespace IECore
