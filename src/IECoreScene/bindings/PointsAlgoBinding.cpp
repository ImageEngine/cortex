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

#include "PointsAlgoBinding.h"

#include "IECoreScene/PointsAlgo.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost;
using namespace boost::python;
using namespace IECorePython;
using namespace IECoreScene;

namespace
{

void resamplePrimitiveVariableWrapper( const PointsPrimitive *points, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation, const IECore::Canceller *canceller )
{
	IECorePython::ScopedGILRelease gilRelease;
	PointsAlgo::resamplePrimitiveVariable( points, primitiveVariable, interpolation, canceller );
}

PointsPrimitivePtr deletePointsWrapper( const PointsPrimitive *meshPrimitive, const PrimitiveVariable &pointsToDelete, bool invert, const IECore::Canceller *canceller )
{
	IECorePython::ScopedGILRelease gilRelease;
	return PointsAlgo::deletePoints( meshPrimitive, pointsToDelete, invert, canceller );
}

PointsPrimitivePtr mergePointsWrapper( boost::python::list &pointsPrimitiveList, const IECore::Canceller *canceller )
{
	int numPointsPrimitives = boost::python::len( pointsPrimitiveList );
	std::vector<const PointsPrimitive *> pointsPrimitiveVec( numPointsPrimitives );

	for( int i = 0; i < numPointsPrimitives; ++i )
	{
		PointsPrimitivePtr ptr = boost::python::extract<PointsPrimitivePtr>( pointsPrimitiveList[i] );
		pointsPrimitiveVec[i] = ptr.get();
	}

	IECorePython::ScopedGILRelease gilRelease;
	return PointsAlgo::mergePoints( pointsPrimitiveVec );
}

boost::python::list segmentWrapper(const PointsPrimitive *points, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues = nullptr, const IECore::Canceller *canceller = nullptr )
{
	std::vector<PointsPrimitivePtr> segmentedPoints;
	{
		IECorePython::ScopedGILRelease gilRelease;
		segmentedPoints = PointsAlgo::segment(points, primitiveVariable, segmentValues);
	}

	boost::python::list returnList;
	for (auto p : segmentedPoints)
	{
		returnList.append( p );
	}
	return returnList;

}

BOOST_PYTHON_FUNCTION_OVERLOADS(segmentOverLoads, segmentWrapper, 2, 4);

} // namepsace

namespace IECoreSceneModule
{

void bindPointsAlgo()
{
	object pointsAlgoModule( borrowed( PyImport_AddModule( "IECore.PointsAlgo" ) ) );
	scope().attr( "PointsAlgo" ) = pointsAlgoModule;

	scope pointsAlgoScope( pointsAlgoModule );

	def( "resamplePrimitiveVariable", &resamplePrimitiveVariableWrapper, ( arg_( "points"), arg_( "primitiveVaraiable" ), arg_( "interpolation" ), arg_( "canceller" ) = object() ) );
	def( "deletePoints", &deletePointsWrapper, ( arg_( "meshPrimitive" ), arg_( "pointsToDelete" ), arg_( "invert" ) = false, arg_( "canceller" ) = object() ) );
	def( "mergePoints", &::mergePointsWrapper, ( arg_("pointsPrimitives"), arg_("canceller") = object() ) );
	def( "segment", &::segmentWrapper, segmentOverLoads());
}

} // namespace IECoreSceneModule
