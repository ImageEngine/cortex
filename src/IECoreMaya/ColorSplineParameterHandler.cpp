//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/Parameter.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/ColorSplineParameterHandler.h"

#include "IECore/TypedParameter.h"

#include "IECoreMaya/Convert.h"

#include "maya/MFnCompoundAttribute.h"
#include "maya/MRampAttribute.h"
#include "maya/MFloatArray.h"
#include "maya/MIntArray.h"
#include "maya/MColor.h"
#include "maya/MColorArray.h"
#include "maya/MGlobal.h"

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
MStatus ColorSplineParameterHandler<S>::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const 
{
	assert( parameter );
	
	typename IECore::TypedParameter< S >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MFnCompoundAttribute fnCAttr( attribute );
	if( !fnCAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}
	
	/// \todo See if the attribute is of type ColorRamp - can't do this yet as we can't construct
	/// an MRampAttribute from just the MObject. We need either the node, too, or an MPlug
			
	return MS::kSuccess;
}

template<typename S>
MObject ColorSplineParameterHandler<S>::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	assert( parameter );

	typename IECore::TypedParameter< S >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< S > >( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}
	
	MRampAttribute fnRAttr;
	MObject result = fnRAttr.createColorRamp( attributeName, attributeName );
	
	update( parameter, result );
	return result;
}

template<typename S>		
MStatus ColorSplineParameterHandler<S>::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
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
	MColorArray colors;
	MFloatArray positions;
	MIntArray interps;
	MIntArray indices;	
	fnRAttr.getEntries( indices, positions, colors, interps, &s );
	assert( s );
	positions.clear();
	colors.clear();
	interps.clear();
	
	assert( indices.length() == fnRAttr.getNumEntries() );
	
	size_t pointsSizeMinus2 = spline.points.size() - 2;
	unsigned idx = 0;
	unsigned expectedPoints = 0;
	unsigned reusedIndices = 0;	
	for ( typename S::PointContainer::const_iterator it = spline.points.begin(); it != spline.points.end(); ++it, ++idx )
	{
		// we commonly double up the endpoints on cortex splines to force interpolation to the end.
		// maya does this implicitly, so we skip duplicated endpoints when passing the splines into maya.
		// this avoids users having to be responsible for managing the duplicates, and gives them some consistency
		// with the splines they edit elsewhere in maya.
		if( idx==1 && *it == *spline.points.begin() || idx==pointsSizeMinus2 && *it == *spline.points.rbegin()  )
		{
			continue;
		}
		
		expectedPoints ++;
		
		if ( idx < std::min( 2u, indices.length() ) )
		{
			reusedIndices ++;
			fnRAttr.setPositionAtIndex( it->first, indices[ idx ], &s );
			assert( s );
			MColor c = IECore::convert< MColor >( it->second );
			fnRAttr.setColorAtIndex( c, indices[ idx ], &s );	
			assert( s );
			fnRAttr.setInterpolationAtIndex( MRampAttribute::kSpline, indices[ idx ], &s );		
			assert( s );
		}
		else
		{	
			colors.append( IECore::convert< MColor >( it->second ) );
			positions.append( it->first );	
			interps.append( MRampAttribute::kSpline );	
		}
	}
	
	assert( positions.length() == colors.length() );
	assert( positions.length() == interps.length() );
	assert( expectedPoints == reusedIndices + positions.length() );	
		
#ifndef NDEBUG
	unsigned int oldNumEntries = fnRAttr.getNumEntries();
#endif		
	
	fnRAttr.addEntries( positions, colors, interps, &s );
	assert( s );
	
	assert( fnRAttr.getNumEntries() == oldNumEntries + positions.length() );
	
	/// Remove all the indices we just reused
	for ( unsigned i = 0; i < reusedIndices ; i ++ )
	{
		assert( indices.length() > 0 );
		indices.remove( 0 );
	}
	
	/// Delete any ununsed indices
	if ( indices.length() )
	{
		fnRAttr.deleteEntries( indices, &s );
		assert( s );
	}		
	
#ifndef NDEBUG	
	{
		assert( fnRAttr.getNumEntries() == expectedPoints );
		
		MIntArray indices;
		MFloatArray positions;
		MColorArray colors;
		MIntArray interps;
		fnRAttr.getEntries( indices, positions, colors, interps, &s );				
		assert( s );
		assert( expectedPoints == positions.length() );		
		assert( expectedPoints == colors.length() );
		assert( expectedPoints == interps.length() );
		assert( expectedPoints == indices.length() );
		
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
MStatus ColorSplineParameterHandler<S>::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
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
	
	if ( fnRAttr.getNumEntries( &s ) > 0 )
	{
		assert( s );
		
		MIntArray indices;
		MFloatArray positions;
		MColorArray colors;
		MIntArray interps;
		fnRAttr.getEntries( indices, positions, colors, interps, &s );
		assert( s );

		if ( positions.length() != colors.length() || positions.length() != interps.length() )
		{
			return MS::kFailure;
		}

		for ( unsigned i = 0; i < positions.length(); i ++)
		{	
			spline.points.insert( 
				typename S::PointContainer::value_type( 
					static_cast< typename S::XType >( positions[i] ), IECore::convert< typename S::YType >( colors[ i ] ) 
				) 
			);
		}	
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
