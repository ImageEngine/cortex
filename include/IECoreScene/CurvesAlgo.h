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

#ifndef IECORESCENE_CURVESALGO_H
#define IECORESCENE_CURVESALGO_H

#include <vector>
#include <utility>

#include "IECoreScene/Export.h"
#include "IECoreScene/PrimitiveVariable.h"
#include "IECoreScene/CurvesPrimitive.h"

namespace IECoreScene
{

namespace CurvesAlgo
{

IECORESCENE_API void resamplePrimitiveVariable( const CurvesPrimitive *curvesPrimitive, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation );

/// create a new curves primitive deleting curves from the input curves primitive based on the curvesToDelete uniform (int|float|bool) PrimitiveVariable
/// When invert is set then zeros in curvesToDelete indicate which curves should be deleted
IECORESCENE_API CurvesPrimitivePtr deleteCurves( const CurvesPrimitive *curvesPrimitive, const PrimitiveVariable &curvesToDelete, bool invert = false );

} // namespace CurveAlgo
} // namespace IECoreScene


#endif // IECORESCENE_CURVESALGO_H
