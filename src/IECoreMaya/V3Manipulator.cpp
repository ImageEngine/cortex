//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundObject.h"
#include "IECore/SimpleTypedData.h"
#include "IECoreMaya/TypeIds.h"
#include "IECoreMaya/V3Manipulator.h"
#include "IECoreMaya/ParameterisedHolderInterface.h"

#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MGlobal.h>
#include <maya/MVector.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnFreePointTriadManip.h>
#include <maya/MFnDirectionManip.h>
#include <maya/MPxTransformationMatrix.h>
#include <maya/MEulerRotation.h>

using namespace IECore;
using namespace IECoreMaya;

MTypeId V3Manipulator::id = V3ManipulatorTypeId;

V3Manipulator::V3Manipulator()
	: m_worldSpace( false )
{
}

V3Manipulator::~V3Manipulator()
{
}

void *V3Manipulator::creator()
{
	return new V3Manipulator();
}

MStatus V3Manipulator::initialize()
{
	return MPxManipContainer::initialize();
}

MStatus V3Manipulator::createChildren()
{
	m_translateManip = addFreePointTriadManip( "Manipulates the vector in space.", "translate" );
	return MStatus::kSuccess;
}

MStatus V3Manipulator::connectToDependNode( const MObject & node )
{
	MFnDagNode dagFn( node );
	MDagPath nodePath;
	dagFn.getPath( nodePath );

	MStatus status;
	m_translatePlug = dagFn.findPlug( m_plug.partialName(), true, &status );
	if( !status )
	{
		return MStatus::kFailure;
	}

	MFnFreePointTriadManip translateFn( m_translateManip );
	translateFn.connectToPointPlug( m_translatePlug );

	addManipToPlugConversionCallback( m_translatePlug, (manipToPlugConversionCallback)&V3Manipulator::vectorManipToPlugConversion );
	addPlugToManipConversionCallback( translateFn.pointIndex(), (plugToManipConversionCallback)&V3Manipulator::vectorPlugToManipConversion );

	MStatus stat = finishAddingManips();
	if( stat == MStatus::kFailure )
	{
		return MStatus::kFailure;
	}

	MPxManipContainer::connectToDependNode( node );

	readParameterOptions( dagFn );

	if( m_worldSpace)
	{
		m_localMatrix.setToIdentity();
		m_localMatrixInv.setToIdentity();
	}
	else
	{
		// Inherit any transform to the parent
		MDagPath transformPath = nodePath;
		transformPath.pop();
		MFnTransform transformFn( transformPath );
		m_localMatrix = transformPath.inclusiveMatrix();
		m_localMatrixInv = transformPath.inclusiveMatrixInverse();
	}
	return stat;
}

void V3Manipulator::draw( M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView::DisplayStatus status )
{
	MPxManipContainer::draw( view, path, style, status);
}

MManipData V3Manipulator::vectorPlugToManipConversion( unsigned int manipIndex )
{
	MFnFreePointTriadManip translateFn( m_translateManip );

	MFnNumericData numericData;
    MObject returnData = numericData.create( MFnNumericData::k3Double );
    numericData.setData( 0.0, 0.0, 0.0 );
	MPoint p = getPlugValues( m_translatePlug ) * m_localMatrix;
	numericData.setData( p.x, p.y, p.z );
	return MManipData( returnData );
}

MManipData V3Manipulator::vectorManipToPlugConversion( unsigned int plugIndex )
{
	MFnFreePointTriadManip translateFn( m_translateManip );
	MPoint t;
	getConverterManipValue( translateFn.pointIndex(), t );
	t = t * m_localMatrixInv;

	MFnNumericData numericData;
    MObject returnData;

	// We have to check what type of data to generate so Maya
	// will be able to set it back into the attribute correctly.
	MFnNumericAttribute attr( m_translatePlug.attribute() );
	if( attr.unitType() == MFnNumericData::k3Float )
	{
		returnData = numericData.create( MFnNumericData::k3Float );
		numericData.setData( float(t.x), float(t.y), float(t.z) );
	}
	else
	{
		returnData = numericData.create( MFnNumericData::k3Double );
		numericData.setData( t.x, t.y, t.z );
	}
	return MManipData( returnData );
}

void V3Manipulator::getPlugValues( MPlug &plug, double *values )
{
	if( plug.numChildren() == 3 )
	{
		for( unsigned i = 0; i<3; i++ )
		{
			MPlug child = plug.child( i );
			*values = child.asDouble();
			values++;
		}
	}
	else
	{
		for( unsigned i = 0; i<3; i++ )
		{
			MPlug element = plug.elementByLogicalIndex( i );
			*values = element.asDouble();
			values++;
		}
	}
}

void V3Manipulator::getPlugValues( MPlug &plug, MFnNumericData &data )
{
	double values[3];
	getPlugValues( plug, values );
	data.setData( values[0], values[1], values[2] );
}

MPoint V3Manipulator::getPlugValues( MPlug &plug )
{
	double values[3];
	getPlugValues( plug, values );
	return MPoint( values[0], values[1], values[2] );
}

void V3Manipulator::readParameterOptions( MFnDagNode &nodeFn )
{
	ParameterisedHolderInterface *pHolder = dynamic_cast<ParameterisedHolderInterface *>( nodeFn.userNode() );
	if( !pHolder )
	{
		return;
	}

	ParameterPtr parameter = pHolder->plugParameter( m_plug );
	CompoundObjectPtr userData = parameter->userData();

	if( CompoundObjectPtr uiData = userData->member<CompoundObject>( "UI" ) )
	{
		// World space parameter values
		if( StringDataPtr wsData = uiData->member<StringData>( "manipSpace" ) )
		{
			if( wsData->readable() == "world" )
			{
				m_worldSpace = true;
			}
			else if( wsData->readable() == "object" )
			{
				m_worldSpace = false;
			}
			else
			{
				MGlobal::displayWarning( "V3Manipulator: Ignoring invalid v3ManipSpace '"
										 + MString( wsData->readable().c_str() )
										 + "' for parameter '"
										 + MString( parameter->name().c_str() )
										 + "', using 'object'." );
			}
		}
	}
}

