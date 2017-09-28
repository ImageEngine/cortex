//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnCompoundAttribute.h"

#include "IECore/SimpleTypedParameter.h"

#include "IECoreMaya/LineSegmentParameterHandler.h"
#include "IECoreMaya/NumericTraits.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< LineSegmentParameterHandler<LineSegment3fParameter> > g_ineSegment2dRegistrar( IECore::LineSegment3fParameter::staticTypeId() );
static ParameterHandler::Description< LineSegmentParameterHandler<LineSegment3dParameter> > g_ineSegment3dRegistrar( IECore::LineSegment3dParameter::staticTypeId() );

template<typename T>
MPlug LineSegmentParameterHandler<T>::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	typename T::ConstPtr p = runTimeCast<const T>( parameter );
	if( !p )
	{
		return MPlug();
	}

	MFnNumericAttribute fnNAttr;
	MFnCompoundAttribute fnCAttr;
	MObject oStart, oEnd;
	switch( Point::dimensions() )
	{
		case 2 :
			{
				MObject oStartX = fnNAttr.create( plugName + "StartX", plugName + "StartX", NumericTraits<Point>::baseDataType() );
				MObject oStartY = fnNAttr.create( plugName + "StartY", plugName + "StartY", NumericTraits<Point>::baseDataType() );
				oStart = fnNAttr.create( plugName + "Start", plugName + "Start", oStartX, oStartY );

				MObject oEndX = fnNAttr.create( plugName + "EndX", plugName + "EndX", NumericTraits<Point>::baseDataType() );
				MObject oEndY = fnNAttr.create( plugName + "EndY", plugName + "EndY", NumericTraits<Point>::baseDataType() );
				oEnd = fnNAttr.create( plugName + "End", plugName + "End", oEndX, oEndY );
			}
			break;
		case 3 :
			oStart = fnNAttr.createPoint( plugName + "Start", plugName + "Start" );
			oEnd = fnNAttr.createPoint( plugName + "End", plugName + "End" );
			break;
		default :
			return MPlug();
	}

	MObject attribute = fnCAttr.create( plugName, plugName );
	fnCAttr.addChild( oStart );
	fnCAttr.addChild( oEnd );

	MPlug result = finishCreating( parameter, attribute, node );
	doUpdate( parameter, result );

	return result;
}

template<typename T>
MStatus LineSegmentParameterHandler<T>::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename T::ConstPtr p = runTimeCast<const T>( parameter );
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

	if( fnCAttr.numChildren()!=2 )
	{
		return MS::kFailure;
	}

	MFnNumericAttribute fnStartAttr( fnCAttr.child( 0 ) );
	if( fnStartAttr.unitType()!=NumericTraits<Point>::dataType() )
	{
		return MS::kFailure;
	}

	MFnNumericAttribute fnEndAttr( fnCAttr.child( 1 ) );
	if( fnEndAttr.unitType()!=NumericTraits<Point>::dataType() )
	{
		return MS::kFailure;
	}

	// Set the default value for the leaf attributes individually. Calling
	// the variants of setDefault that set several components at a time
	// seems to exercise a maya bug. See similar comment in CompoundNumericParameterHandler.
	LineSegment defValue = p->typedDefaultValue();
	MStatus s;
	for( unsigned i=0; i<Point::dimensions(); i++ )
	{
		MObject startChildAttr = fnStartAttr.child( i, &s );
		if( !s )
		{
			return s;
		}

		MObject endChildAttr = fnEndAttr.child( i, &s );
		if( !s )
		{
			return s;
		}

		MFnNumericAttribute fnStartChildAttr( startChildAttr, &s );
		if( !s )
		{
			return s;
		}

		MFnNumericAttribute fnEndChildAttr( endChildAttr, &s );
		if( !s )
		{
			return s;
		}

		s = fnStartChildAttr.setDefault( defValue.p0[i] );
		if( !s )
		{
			return s;
		}

		s = fnEndChildAttr.setDefault( defValue.p1[i] );
		if( !s )
		{
			return s;
		}

	}

	return finishUpdating( parameter, plug );
}


template<typename T>
MStatus LineSegmentParameterHandler<T>::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename T::ConstPtr p = runTimeCast<const T>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	if( plug.numChildren() != 2 )
	{
		return MS::kFailure;
	}

	MPlug startPlug = plug.child( 0 );
	MPlug endPlug = plug.child( 1 );

	if( startPlug.numChildren()!=Point::dimensions() || endPlug.numChildren()!=Point::dimensions() )
	{
		return MS::kFailure;
	}

	LineSegment v = p->getTypedValue();
	for( unsigned i=0; i<startPlug.numChildren(); i++ )
	{
		MStatus s = startPlug.child( i ).setValue( v.p0[i] );
		if( !s )
		{
			return s;
		}
		s = endPlug.child( i ).setValue( v.p1[i] );
		if( !s )
		{
			return s;
		}
	}

	return MS::kSuccess;
}

template<typename T>
MStatus LineSegmentParameterHandler<T>::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	typename T::Ptr p = runTimeCast<T>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	if( plug.numChildren() != 2 )
	{
		return MS::kFailure;
	}

	MPlug startPlug = plug.child( 0 );
	MPlug endPlug = plug.child( 1 );

	if( startPlug.numChildren()!=Point::dimensions() || endPlug.numChildren()!=Point::dimensions() )
	{
		return MS::kFailure;
	}

	LineSegment v;
	for( unsigned i=0; i<startPlug.numChildren(); i++ )
	{
		MStatus s = startPlug.child( i ).getValue( v.p0[i] );
		if( !s )
		{
			return s;
		}
		s = endPlug.child( i ).getValue( v.p1[i] );
		if( !s )
		{
			return s;
		}
	}

	p->setTypedValue( v );
	return MS::kSuccess;
}

