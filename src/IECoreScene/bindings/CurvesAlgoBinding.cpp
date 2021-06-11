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

#include "boost/python.hpp"

#include "CurvesAlgoBinding.h"

#include "IECoreScene/CurvesAlgo.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECoreScene;


namespace
{

void resamplePrimitiveVariableWrapper( const CurvesPrimitive *curvesPrimitive, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation, const IECore::Canceller *canceller )
{
	IECorePython::ScopedGILRelease gilRelease;
	CurvesAlgo::resamplePrimitiveVariable( curvesPrimitive, primitiveVariable, interpolation, canceller );
}

CurvesPrimitivePtr deleteCurvesWrapper( const CurvesPrimitive *curvesPrimitive, const PrimitiveVariable &curvesToDelete, bool invert, const IECore::Canceller *canceller )
{
	IECorePython::ScopedGILRelease gilRelease;
	return CurvesAlgo::deleteCurves( curvesPrimitive, curvesToDelete, invert, canceller );
}


boost::python::list segmentWrapper(const CurvesPrimitive *curves, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues = nullptr, const Canceller *canceller = nullptr )
{
	std::vector<CurvesPrimitivePtr> segmented;
	{
		IECorePython::ScopedGILRelease gilRelease;
		segmented = CurvesAlgo::segment(curves, primitiveVariable, segmentValues);
	}

	boost::python::list returnList;
	for (auto p : segmented)
	{
		returnList.append( p );
	}
	return returnList;
}

CurvesPrimitivePtr updateEndpointMultiplicityWrapper( const CurvesPrimitive *curves, const IECore::CubicBasisf& cubicBasis, const IECore::Canceller *canceller )
{
	IECorePython::ScopedGILRelease gilRelease;
	return CurvesAlgo::updateEndpointMultiplicity( curves, cubicBasis, canceller );
}

BOOST_PYTHON_FUNCTION_OVERLOADS(segmentOverLoads, segmentWrapper, 2, 4);

} // namepsace

namespace IECoreSceneModule
{

void bindCurvesAlgo()
{
	object curveAlgoModule( borrowed( PyImport_AddModule( "IECore.CurvesAlgo" ) ) );
	scope().attr( "CurvesAlgo" ) = curveAlgoModule;

	scope meshAlgoScope( curveAlgoModule );

	def( "resamplePrimitiveVariable", &resamplePrimitiveVariableWrapper, ( arg_( "curvesPrimitive" ), arg_( "primitiveVariable" ), arg_( "interpolation" ), arg_( "canceller" ) = object() ) );
	def( "deleteCurves", &deleteCurvesWrapper, ( arg_( "curvesPrimitive" ), arg_( "curvesToDelete" ), arg_( "invert" ) = false, arg_( "canceller" ) = object() ) );
	def( "segment", ::segmentWrapper, segmentOverLoads());
	def( "updateEndpointMultiplicity", &updateEndpointMultiplicityWrapper, ( arg_( "curves" ), arg_( "cubicBasis" ), arg_( "canceller" ) = object() ) );
}

} // namespace IECoreSceneModule

