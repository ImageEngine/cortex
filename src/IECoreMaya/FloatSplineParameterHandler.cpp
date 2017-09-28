//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#include <cassert>
#include <algorithm>

#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/FloatSplineParameterHandler.h"
#include "IECoreMaya/MArrayIter.h"

#include "IECore/SplineParameter.h"

#include "maya/MFnCompoundAttribute.h"
#include "maya/MRampAttribute.h"
#include "maya/MFloatArray.h"
#include "maya/MIntArray.h"
#include "maya/MGlobal.h"
#include "maya/MFnDagNode.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

template<> ParameterHandler::Description< FloatSplineParameterHandler<  IECore::Splineff > >
	FloatSplineParameterHandler<  IECore::Splineff >::g_registrar( IECore::SplineffParameter::staticTypeId() );

template<> ParameterHandler::Description< FloatSplineParameterHandler<  IECore::Splinedd > >
	FloatSplineParameterHandler<  IECore::Splinedd >::g_registrar( IECore::SplineddParameter::staticTypeId() );

template<typename S>
MStatus FloatSplineParameterHandler<S>::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	assert( parameter );

	typename IECore::TypedParameter< S >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnCompoundAttribute fnCAttr( attribute );
	if( !fnCAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	MRampAttribute fnRAttr( plug );
	if( !fnRAttr.isCurveRamp() )
	{
		return MS::kFailure;
	}

	return finishUpdating( parameter, plug );
}

template<typename S>
MPlug FloatSplineParameterHandler<S>::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	assert( parameter );

	typename IECore::TypedParameter< S >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MPlug();
	}

	MRampAttribute fnRAttr;
	MObject attribute = fnRAttr.createCurveRamp( plugName, plugName );

	MPlug result = finishCreating( parameter, attribute, node );

	if( !finishUpdating( parameter, result ) )
	{
		return MPlug();
	}

	return result;
}

template<typename S>
MStatus FloatSplineParameterHandler<S>::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	assert( parameter );
	typename IECore::TypedParameter< S >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MRampAttribute fnRAttr( plug );
	if ( !fnRAttr.isCurveRamp() )
	{
		return MS::kFailure;
	}

	const S &spline = p->getTypedValue();

	MStatus s;
	MIntArray indicesToReuse;
	plug.getExistingArrayAttributeIndices( indicesToReuse, &s );
	assert( s );

	int nextNewLogicalIndex = 0;
	if( indicesToReuse.length() )
	{
		nextNewLogicalIndex = 1 + *std::max_element( MArrayIter<MIntArray>::begin( indicesToReuse ), MArrayIter<MIntArray>::end( indicesToReuse ) );
	}

	assert( indicesToReuse.length() == fnRAttr.getNumEntries() );

	size_t pointsSizeMinus2 = spline.points.size() - 2;
	unsigned pointIndex = 0;
	unsigned numExpectedPoints = 0;
	for ( typename S::PointContainer::const_iterator it = spline.points.begin(); it != spline.points.end(); ++it, ++pointIndex )
	{
		// we commonly double up the endpoints on cortex splines to force interpolation to the end.
		// maya does this implicitly, so we skip duplicated endpoints when passing the splines into maya.
		// this avoids users having to be responsible for managing the duplicates, and gives them some consistency
		// with the splines they edit elsewhere in maya.
		if( ( pointIndex==1 && *it == *spline.points.begin() ) || ( pointIndex==pointsSizeMinus2 && *it == *spline.points.rbegin() ) )
		{
			continue;
		}

		MPlug pointPlug;
		if( indicesToReuse.length() )
		{
			pointPlug = plug.elementByLogicalIndex( indicesToReuse[0] );
			indicesToReuse.remove( 0 );
		}
		else
		{
			// this creates us a new spline point for us, and avoids the bug in MRampAttribute::addEntries which
			// somehow manages to create duplicate logical indexes.
			pointPlug = plug.elementByLogicalIndex( nextNewLogicalIndex++ );
		}

		s = pointPlug.child( 0 ).setValue( it->first ); assert( s );
		s = pointPlug.child( 1 ).setValue( it->second ); assert( s );
		// hardcoding interpolation of 3 (spline) because the MRampAttribute::MInterpolation enum values don't actually
		// correspond to the necessary plug values at all.
		s = pointPlug.child( 2 ).setValue( 3 ); assert( s );

		numExpectedPoints++;
	}

	// delete any of the original indices which we didn't reuse. we can't use MRampAttrubute::deleteEntries
	// here as it's utterly unreliable.
	if( indicesToReuse.length() )
	{
		MString plugName = plug.name();
		MObject node = plug.node();
		MFnDagNode fnDAGN( node );
		if( fnDAGN.hasObj( node ) )
		{
			plugName = fnDAGN.fullPathName() + "." + plug.partialName();
		}
		for( unsigned i=0; i<indicesToReuse.length(); i++ )
		{
			// using mel because there's no equivalant api method as far as i know.
			MString command = "removeMultiInstance -b true \"" + plugName + "[" + indicesToReuse[i] + "]\"";
			s = MGlobal::executeCommand( command );
			assert( s );
			if( !s )
			{
				return s;
			}
		}
	}

#ifndef NDEBUG
	{
		MIntArray allLogicalIndices; plug.getExistingArrayAttributeIndices( allLogicalIndices );
		assert( fnRAttr.getNumEntries() == numExpectedPoints );
		assert( fnRAttr.getNumEntries() == allLogicalIndices.length() );

		// the MRampAttribute has the wonderful "feature" that addEntries() is somehow capable
		// of creating duplicate logical array indices, which causes no end of trouble
		// down the line. check that we've managed to avoid this pitfall.
		std::set<int> uniqueIndices;
		std::copy(
			MArrayIter<MIntArray>::begin( allLogicalIndices ),
			MArrayIter<MIntArray>::end( allLogicalIndices ),
			std::insert_iterator<std::set<int> >( uniqueIndices, uniqueIndices.begin() )
		);
		assert( uniqueIndices.size()==allLogicalIndices.length() );

		// then check that every element of the ramp has a suitable equivalent in
		// the original spline
		MIntArray indices;
		MFloatArray positions;
		MFloatArray values;
		MIntArray interps;
		fnRAttr.getEntries( indices, positions, values, interps, &s );
		assert( s );
		assert( numExpectedPoints == positions.length() );
		assert( numExpectedPoints == values.length() );
		assert( numExpectedPoints == interps.length() );
		assert( numExpectedPoints == indices.length() );

		for ( unsigned i = 0; i < positions.length(); i++ )
		{
			float position = positions[ i ];
			float value = values[ i ];

			bool found = false;

			for ( typename S::PointContainer::const_iterator it = spline.points.begin(); it != spline.points.end() && !found; ++it )
			{
				if ( fabs( it->first - position ) < 1.e-3f && fabs( it->second - value ) < 1.e-3f )
				{
					found = true;
				}
			}
			assert( found );
		}
	}
#endif

	return MS::kSuccess;
}

template<typename S>
MStatus FloatSplineParameterHandler<S>::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	assert( parameter );

	typename IECore::TypedParameter< S >::Ptr p = IECore::runTimeCast<IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	S spline;

	MStatus s;
	MRampAttribute fnRAttr( plug, &s );
	assert( s );

	if ( !fnRAttr.isCurveRamp() )
	{
		return MS::kFailure;
	}

	MIntArray indices;
	(const_cast<MPlug &>( plug )).getExistingArrayAttributeIndices( indices, &s );

	for( unsigned i = 0; i < indices.length(); i++ )
	{
		MPlug pointPlug = plug.elementByLogicalIndex( indices[i] );
		spline.points.insert(
			typename S::PointContainer::value_type( pointPlug.child( 0 ).asDouble(), pointPlug.child( 1 ).asDouble() )
		);
	}

	// maya seems to do an implicit doubling up of the end points to cause interpolation to the ends.
	// our spline has no such implicit behaviour so we explicitly double up.
	if( spline.points.size() )
	{
#ifndef NDEBUG
		size_t oldSplineSize = spline.points.size();
#endif

		assert( spline.points.begin()->first <= spline.points.rbegin()->first );
		spline.points.insert( *spline.points.begin() );
		spline.points.insert( *spline.points.rbegin() );
		assert( spline.points.size() == oldSplineSize + 2 );
	}

	p->setTypedValue( spline );

	if( spline.points.size() )
	{
		assert( spline.points.size() >= 2 );
		assert( spline.points.size() == fnRAttr.getNumEntries() + 2 );
	}

	return MS::kSuccess;
}
