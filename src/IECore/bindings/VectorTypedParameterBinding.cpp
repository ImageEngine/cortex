//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECore/bindings/TypedParameterBinding.h"
#include "IECore/VectorTypedParameter.h"

using namespace std;
using namespace Imath;

namespace IECore
{

void bindVectorTypedParameter()
{
	bindTypedParameter<vector<bool> >( "BoolVectorParameter" );
	bindTypedParameter<vector<int> >( "IntVectorParameter" );
	bindTypedParameter<vector<float> >( "FloatVectorParameter" );
	bindTypedParameter<vector<double> >( "DoubleVectorParameter" );
	bindTypedParameter<vector<string> >( "StringVectorParameter" );
	bindTypedParameter<vector<V2f> >( "V2fVectorParameter" );
	bindTypedParameter<vector<V3f> >( "V3fVectorParameter" );
	bindTypedParameter<vector<V2d> >( "V2dVectorParameter" );
	bindTypedParameter<vector<V3d> >( "V3dVectorParameter" );
	bindTypedParameter<vector<Box3f> >( "Box3fVectorParameter" );
	bindTypedParameter<vector<Box3d> >( "Box3dVectorParameter" );
	bindTypedParameter<vector<M33f> >( "M33fVectorParameter" );
	bindTypedParameter<vector<M44f> >( "M44fVectorParameter" );
	bindTypedParameter<vector<M33d> >( "M33dVectorParameter" );
	bindTypedParameter<vector<M44d> >( "M44dVectorParameter" );
	bindTypedParameter<vector<Quatf> >( "QuatfVectorParameter" );
	bindTypedParameter<vector<Quatd> >( "QuatdVectorParameter" );
	bindTypedParameter<vector<Color3f> >( "Color3fVectorParameter" );
	bindTypedParameter<vector<Color4f> >( "Color4fVectorParameter" );
}

} // namespace IECore
