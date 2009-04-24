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
	bindTypedParameter<vector<bool> >();
	bindTypedParameter<vector<int> >();
	bindTypedParameter<vector<float> >();
	bindTypedParameter<vector<double> >();
	bindTypedParameter<vector<string> >();
	bindTypedParameter<vector<V2f> >();
	bindTypedParameter<vector<V3f> >();
	bindTypedParameter<vector<V2d> >();
	bindTypedParameter<vector<V3d> >();
	bindTypedParameter<vector<Box3f> >();
	bindTypedParameter<vector<Box3d> >();
	bindTypedParameter<vector<M33f> >();
	bindTypedParameter<vector<M44f> >();
	bindTypedParameter<vector<M33d> >();
	bindTypedParameter<vector<M44d> >();
	bindTypedParameter<vector<Quatf> >();
	bindTypedParameter<vector<Quatd> >();
	bindTypedParameter<vector<Color3f> >();
	bindTypedParameter<vector<Color4f> >();
}

} // namespace IECore
