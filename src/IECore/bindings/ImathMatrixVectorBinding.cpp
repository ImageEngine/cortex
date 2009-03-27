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
#include "boost/python/make_constructor.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"
#include "boost/numeric/conversion/cast.hpp"
#include "boost/python/implicit.hpp"

#include "OpenEXR/ImathMatrix.h"

#include "IECore/VectorTypedData.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/VectorTypedDataBinding.inl"

using namespace std;
using std::string;
using namespace boost;
using namespace boost::python;
using namespace Imath;

namespace IECore {

IE_COREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( M33f )
IE_COREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( M33d )
IE_COREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( M44f )
IE_COREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( M44d )

void bindImathMatrixVectorTypedData()
{
	BIND_SIMPLE_OPERATED_VECTOR_TYPEDDATA( Matrix33< float >,  "M33fVectorData", "M33f")
	BIND_SIMPLE_OPERATED_VECTOR_TYPEDDATA( Matrix33< double >, "M33dVectorData", "M33d")
	BIND_SIMPLE_OPERATED_VECTOR_TYPEDDATA( Matrix44< float >,  "M44fVectorData", "M44f")
	BIND_SIMPLE_OPERATED_VECTOR_TYPEDDATA( Matrix44< double >, "M44dVectorData", "M44d")
}

} // namespace IECore
