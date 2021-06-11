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

#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/PrimitiveVariable.h"

#include <utility>
#include <vector>

namespace IECoreScene
{

namespace CurvesAlgo
{

/// NOTE: Most of these functions may optionally take a Canceller.
/// If provided, this will be periodically checked, and cancel the calculation with an exception
/// if the canceller has been triggered ( indicating the result is no longer needed )

IECORESCENE_API void resamplePrimitiveVariable( const CurvesPrimitive *curvesPrimitive, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation, const IECore::Canceller *canceller = nullptr  );

/// create a new curves primitive deleting curves from the input curves primitive based on the curvesToDelete uniform (int|float|bool) PrimitiveVariable
/// When invert is set then zeros in curvesToDelete indicate which curves should be deleted
IECORESCENE_API CurvesPrimitivePtr deleteCurves( const CurvesPrimitive *curvesPrimitive, const PrimitiveVariable &curvesToDelete, bool invert = false, const IECore::Canceller *canceller = nullptr  );

/// Segment a CurvesPrimitve in to N CurvesPrimitives based on the N unique values contained in the segmentValues argument.
/// If segmentValues isn't supplied then primitive is split into the unique values contained in the primitiveVariable.
/// The primitiveVariable must have 'Uniform' iterpolation and match the base type of the VectorTypedData in the segmentValues.
/// Specifying the two parameters segmentValues & primitiveVariable allows for a subset of curves to be created, rather than
/// completely segmententing the curves based on the unique values in a primitive variable.
IECORESCENE_API std::vector<CurvesPrimitivePtr> segment( const CurvesPrimitive *curves, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues = nullptr, const IECore::Canceller *canceller = nullptr  );

/// Update the number of replicated end points based on the basis.
IECORESCENE_API CurvesPrimitivePtr updateEndpointMultiplicity( const CurvesPrimitive *curves, const IECore::CubicBasisf& cubicBasis, const IECore::Canceller *canceller = nullptr  );

} // namespace CurveAlgo

} // namespace IECoreScene


#endif // IECORESCENE_CURVESALGO_H
