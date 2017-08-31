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

#ifndef IECORE_POINTSALGO_H
#define IECORE_POINTSALGO_H


#include "IECore/PrimitiveVariable.h"
#include "IECore/PointsPrimitive.h"

namespace IECore
{

namespace PointsAlgo
{
void resamplePrimitiveVariable( const PointsPrimitive *points, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation );

/// create a new PointsPrimitive deleting points from the input PointsPrimitive based on the pointsToDelete vertex (int|float|bool) PrimitiveVariable
PointsPrimitivePtr deletePoints( const PointsPrimitive *meshPrimitive, const PrimitiveVariable &pointsToDelete);

/// merge points primitives - when conflicting primitive variables are encountered earlier elements in the input vector take priority.
/// constant interpolated primitive variables: first occurance of the primitive variable is used and others ignored.
/// vertex interpolated primitive variables: type conversion is attempted where later primitives variables in the list are cast to earlier ones.
PointsPrimitivePtr mergePoints( const std::vector<const PointsPrimitive *> &pointsPrimitives );

} // namespace PointsAlgo
} // namespace IECore


#endif // IECORE_POINTSALGO_H
