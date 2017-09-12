//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MESHALGO_H
#define IECORE_MESHALGO_H

#include <utility>

#include "IECore/PrimitiveVariable.h"
#include "IECore/MeshPrimitive.h"

namespace IECore
{

namespace MeshAlgo
{

/// Calculate the surface tangent vectors of a mesh primitive.
std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangents( const MeshPrimitive *mesh, const std::string &uvSet = "uv", bool orthoTangents = true, const std::string &position = "P" );

/// Calculate the face area of a mesh primitive.
PrimitiveVariable calculateFaceArea( const MeshPrimitive *mesh, const std::string &position = "P" );

/// Calculate the face texture area of a mesh primitive based on the specified UV set.
PrimitiveVariable calculateFaceTextureArea( const MeshPrimitive *mesh, const std::string &uvSet = "uv", const std::string &position = "P" );

void resamplePrimitiveVariable( const MeshPrimitive *mesh, PrimitiveVariable& primitiveVariable, PrimitiveVariable::Interpolation interpolation );

/// create a new MeshPrimitive deleting faces from the input MeshPrimitive based on the facesToDelete uniform (int|float|bool) PrimitiveVariable
MeshPrimitivePtr deleteFaces( const MeshPrimitive *meshPrimitive, const PrimitiveVariable &facesToDelete );

/// Reverses the winding order of each face by adjusting the vertex ids and updating all FaceVarying
/// primitive variables to match.
void reverseWinding( MeshPrimitive *meshPrimitive );

} // namespace MeshAlgo

} // namespace IECore

#endif // IECORE_MESHALGO_H
