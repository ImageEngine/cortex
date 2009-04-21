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

#ifndef IE_CORE_VECTORTYPEDPARAMETER_H
#define IE_CORE_VECTORTYPEDPARAMETER_H

#include <vector>

#include "IECore/VectorTypedData.h"
#include "IECore/TypedParameter.h"

namespace IECore
{

typedef TypedParameter<std::vector<bool> > BoolVectorParameter;
typedef TypedParameter<std::vector<int> > IntVectorParameter;
typedef TypedParameter<std::vector<float> > FloatVectorParameter;
typedef TypedParameter<std::vector<double> > DoubleVectorParameter;
typedef TypedParameter<std::vector<std::string> > StringVectorParameter;
typedef TypedParameter<std::vector<Imath::V2f> > V2fVectorParameter;
typedef TypedParameter<std::vector<Imath::V3f> > V3fVectorParameter;
typedef TypedParameter<std::vector<Imath::V2d> > V2dVectorParameter;
typedef TypedParameter<std::vector<Imath::V3d> > V3dVectorParameter;
typedef TypedParameter<std::vector<Imath::Box3f> > Box3fVectorParameter;
typedef TypedParameter<std::vector<Imath::Box3d> > Box3dVectorParameter;
typedef TypedParameter<std::vector<Imath::M33f> > M33fVectorParameter;
typedef TypedParameter<std::vector<Imath::M44f> > M44fVectorParameter;
typedef TypedParameter<std::vector<Imath::M33d> > M33dVectorParameter;
typedef TypedParameter<std::vector<Imath::M44d> > M44dVectorParameter;
typedef TypedParameter<std::vector<Imath::Quatf> > QuatfVectorParameter;
typedef TypedParameter<std::vector<Imath::Quatd> > QuatdVectorParameter;
typedef TypedParameter<std::vector<Imath::Color3f> > Color3fVectorParameter;
typedef TypedParameter<std::vector<Imath::Color4f> > Color4fVectorParameter;

IE_CORE_DECLAREPTR( BoolVectorParameter );
IE_CORE_DECLAREPTR( IntVectorParameter );
IE_CORE_DECLAREPTR( FloatVectorParameter );
IE_CORE_DECLAREPTR( DoubleVectorParameter );
IE_CORE_DECLAREPTR( StringVectorParameter );
IE_CORE_DECLAREPTR( V2fVectorParameter );
IE_CORE_DECLAREPTR( V3fVectorParameter );
IE_CORE_DECLAREPTR( V2dVectorParameter );
IE_CORE_DECLAREPTR( V3dVectorParameter );
IE_CORE_DECLAREPTR( Box3fVectorParameter );
IE_CORE_DECLAREPTR( Box3dVectorParameter );
IE_CORE_DECLAREPTR( M33fVectorParameter );
IE_CORE_DECLAREPTR( M44fVectorParameter );
IE_CORE_DECLAREPTR( M33dVectorParameter );
IE_CORE_DECLAREPTR( M44dVectorParameter );
IE_CORE_DECLAREPTR( QuatfVectorParameter );
IE_CORE_DECLAREPTR( QuatdVectorParameter );
IE_CORE_DECLAREPTR( Color3fVectorParameter );
IE_CORE_DECLAREPTR( Color4fVectorParameter );

}

#endif // IE_CORE_VECTORTYPEDPARAMETER_H
