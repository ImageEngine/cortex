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

#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/ColorSplineParameterHandler.h"
#include "IECoreMaya/MArrayIter.h"

#include "IECore/SplineParameter.h"

#include "IECoreMaya/Convert.h"

#include "maya/MFnCompoundAttribute.h"
#include "maya/MRampAttribute.h"
#include "maya/MFloatArray.h"
#include "maya/MIntArray.h"
#include "maya/MColor.h"
#include "maya/MColorArray.h"
#include "maya/MGlobal.h"
#include "maya/MFnDagNode.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

template<> ParameterHandler::Description< ColorSplineParameterHandler< IECore::SplinefColor3f > >
	ColorSplineParameterHandler< IECore::SplinefColor3f >::g_registrar
		( IECore::SplinefColor3fParameter::staticTypeId() );

template<> ParameterHandler::Description< ColorSplineParameterHandler< IECore::SplinefColor4f > >
	ColorSplineParameterHandler< IECore::SplinefColor4f >::g_registrar
		( IECore::SplinefColor4fParameter::staticTypeId() );

template<typename S>
MStatus ColorSplineParameterHandler<S>::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
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
	if( !fnRAttr.isColorRamp() )
	{
		return MS::kFailure;
	}

	return finishUpdating( parameter, plug );
}

template<typename S>
MPlug ColorSplineParameterHandler<S>::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	assert( parameter );

	typename IECore::TypedParameter< S >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MPlug();
	}

	MRampAttribute fnRAttr;
	MObject attribute = fnRAttr.createColorRamp( plugName, plugName );

	MPlug result = finishCreating( parameter, attribute, node );
	doUpdate( parameter, result );

	return result;
}

template<typename S>
MStatus ColorSplineParameterHandler<S>::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	assert( parameter );
	typename IECore::TypedParameter< S >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MRampAttribute fnRAttr( plug );
	if ( !fnRAttr.isColorRamp() )
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
			pointPlug = plug.elementByLogicalIndex( nextNewLogicalIndex++ );
		}

		s = pointPlug.child( 0 ).setValue( it->first ); assert( s );
		MPlug colorPlug = pointPlug.child( 1 );
		colorPlug.child( 0 ).setValue( it->second[0] );
		colorPlug.child( 1 ).setValue( it->second[1] );
		colorPlug.child( 2 ).setValue( it->second[2] );
		// hardcoding interpolation of 3 (spline) because the MRampAttribute::MInterpolation enum values don't actually
		// correspond to the necessary plug values at all.
		s = pointPlug.child( 2 ).setValue( 3 ); assert( s );

		numExpectedPoints++;
	}

	// delete any of the original indices which we didn't reuse. we can't use MRampAttribute::deleteEntries
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
		assert( fnRAttr.getNumEntries() == numExpectedPoints );

		MIntArray indices;
		MFloatArray positions;
		MColorArray colors;
		MIntArray interps;
		fnRAttr.getEntries( indices, positions, colors, interps, &s );
		assert( s );
		assert( numExpectedPoints == positions.length() );
		assert( numExpectedPoints == colors.length() );
		assert( numExpectedPoints == interps.length() );
		assert( numExpectedPoints == indices.length() );

		for ( unsigned i = 0; i < positions.length(); i++ )
		{
			float position = positions[ i ];
			const MVector color( colors[ i ][ 0 ], colors[ i ][ 1 ], colors[ i ][ 2 ] );

			bool found = false;

			for ( typename S::PointContainer::const_iterator it = spline.points.begin(); it != spline.points.end() && !found; ++it )
			{
				MVector color2( it->second[0], it->second[1], it->second[2] );
				if ( fabs( it->first - position ) < 1.e-3f && ( color2 - color ).length() < 1.e-3f )
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
MStatus ColorSplineParameterHandler<S>::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
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

	if ( !fnRAttr.isColorRamp() )
	{
		return MS::kFailure;
	}

	MIntArray indices;
	(const_cast<MPlug &>( plug )).getExistingArrayAttributeIndices( indices, &s );

	for( unsigned i = 0; i < indices.length(); i++ )
	{
		MPlug pointPlug = plug.elementByLogicalIndex( indices[i] );
		MPlug colorPlug = pointPlug.child( 1 );

		typename S::YType y( 1 );
		y[0] = colorPlug.child( 0 ).asDouble();
		y[1] = colorPlug.child( 1 ).asDouble();
		y[2] = colorPlug.child( 2 ).asDouble();

		spline.points.insert(
			typename S::PointContainer::value_type( pointPlug.child( 0 ).asDouble(), y )
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
