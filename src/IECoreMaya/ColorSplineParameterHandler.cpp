//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

template<> ParameterHandler::Description< ColorSplineParameterHandler< Imath::Color3f > > 
	ColorSplineParameterHandler< Imath::Color3f >::g_registrar
		( IECore::SplinefColor3fParameter::staticTypeId() );
		
template<> ParameterHandler::Description< ColorSplineParameterHandler< Imath::Color4f > > 
	ColorSplineParameterHandler< Imath::Color4f >::g_registrar
		( IECore::SplinefColor4fParameter::staticTypeId() );		

template<typename C>
MStatus ColorSplineParameterHandler<C>::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const 
{
	assert( parameter );
	
	typename IECore::TypedParameter< IECore::Spline< float, C > >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< IECore::Spline< float, C > > >( parameter );
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

template<typename C>
MObject ColorSplineParameterHandler<C>::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	assert( parameter );

	typename IECore::TypedParameter< IECore::Spline< float, C > >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< IECore::Spline< float, C > > >( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}
	
	MRampAttribute fnRAttr;
	MObject result = fnRAttr.createColorRamp( attributeName, attributeName );
	
	update( parameter, result );
	return result;
}

template<typename C>		
MStatus ColorSplineParameterHandler<C>::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	assert( parameter );
	typename IECore::TypedParameter< IECore::Spline< float, C > >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter< IECore::Spline< float, C > > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MRampAttribute fnRAttr( plug );
	if ( !fnRAttr.isColorRamp() )
	{
		return MS::kFailure;
	}
	
	const IECore::Spline< float, C > &spline = p->getTypedValue();
	
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
	
	unsigned idx = 0;
	for ( typename IECore::Spline< float, C >::PointContainer::const_iterator it = spline.points.begin(); it != spline.points.end(); ++it, ++idx )
	{
		MColor c( IECore::convert< MColor >( it->second ) );
	
		if ( idx < indices.length() )
		{
			fnRAttr.setPositionAtIndex( it->first, indices[ idx ], &s );
			assert( s );
			fnRAttr.setColorAtIndex( c, indices[ idx ], &s );	
			assert( s );
			fnRAttr.setInterpolationAtIndex( MRampAttribute::kSpline, indices[ idx ], &s );		
			assert( s );
		}
		else
		{	
			colors.append( c );
			positions.append( it->first );	
			interps.append( MRampAttribute::kSpline );	
		}
	}
	
	fnRAttr.addEntries( positions, colors, interps, &s );
	assert( s );
	
	/// Remove all the indices we just reused
	for ( unsigned i = 0; i < idx; i ++ )
	{
		indices.remove( 0 );
	}

	/// \todo Find out why MRampAttribute::deleteEntries doesn't work	
	for ( unsigned i = 0; i < indices.length(); i ++ )
	{
		MPlug child = plug.elementByLogicalIndex( indices[i], &s );
		assert( s );
		MGlobal::executeCommand( "removeMultiInstance -break true \"" + child.name() + "\"" );				
	}
	
	return s;
}

template<typename C>
MStatus ColorSplineParameterHandler<C>::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	assert( parameter );

	typename IECore::TypedParameter< IECore::Spline< float, C > >::Ptr p = IECore::runTimeCast<IECore::TypedParameter< IECore::Spline< float, C > > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	IECore::Spline< float, C > spline;
	
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
				typename IECore::Spline< float, C >::PointContainer::value_type( 
					positions[i], IECore::convert< C >( colors[ i ] ) 
				) 
			);
		}	
	}
	
	p->setTypedValue( spline );
	
	return MS::kSuccess;
}
