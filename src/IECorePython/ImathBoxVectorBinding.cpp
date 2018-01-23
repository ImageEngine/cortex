//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/VectorTypedData.h"
#include "IECorePython/VectorTypedDataBinding.inl"

using namespace Imath;

namespace IECorePython
{

IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( Box2i )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( Box2f )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( Box2d )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( Box3i )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( Box3f )
IECOREPYTHON_DEFINEVECTORDATASTRSPECIALISATION( Box3d )

void bindImathBoxVectorTypedData()
{
	BIND_VECTOR_TYPEDDATA ( Box< V2i >, "Box2i")
	BIND_VECTOR_TYPEDDATA ( Box< V2f >, "Box2f")
	BIND_VECTOR_TYPEDDATA ( Box< V2d >, "Box2d")
	BIND_VECTOR_TYPEDDATA ( Box< V3i >, "Box3i")
	BIND_VECTOR_TYPEDDATA ( Box< V3f >, "Box3f")
	BIND_VECTOR_TYPEDDATA ( Box< V3d >, "Box3d")
}

} // namespace IECorePython
