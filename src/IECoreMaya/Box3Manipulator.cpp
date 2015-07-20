//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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

#include <maya/MFnStateManip.h>
#include <maya/MFnFreePointTriadManip.h>
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MArgList.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MEulerRotation.h>
#include <maya/MPxTransformationMatrix.h>
#undef None // must come after certain Maya includes which include X11/X.h

#include "IECore/Parameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreMaya/TypeIds.h"
#include "IECoreMaya/Box3Manipulator.h"
#include "IECoreMaya/ParameterisedHolderInterface.h"

using namespace IECore;
using namespace IECoreMaya;

MTypeId Box3Manipulator::id = Box3ManipulatorTypeId;
const MString Box3Manipulator::typeName = "ieBox3Manipulator";

Box3Manipulator::Box3Manipulator()
	: m_minPlugName( "" ), m_maxPlugName( "" ), m_worldSpace( false )
{
}

Box3Manipulator::~Box3Manipulator()
{
}

void *Box3Manipulator::creator() 
{
	return new Box3Manipulator();
}

MStatus Box3Manipulator::initialize()
{
	return MPxManipContainer::initialize();
}
    
MStatus Box3Manipulator::createChildren()
{	
	m_minManip = addFreePointTriadManip( "Manipulates the 'minimum' corner of the Box", "min" );
	m_maxManip = addFreePointTriadManip( "Manipulates the 'maximum' corner of the Box", "max" );

	m_stateManip = addStateManip( "Toggles validity checking, when on, min can never be greater than max.", "validate" );
	
	return MStatus::kSuccess;
}

MStatus Box3Manipulator::connectToDependNode( const MObject & node )
{	
	MFnDagNode dagFn( node );
	dagFn.getPath( m_nodePath );
	
	if( !findPlugs( dagFn ) )
	{
		return MStatus::kFailure;
	}
		
	MFnFreePointTriadManip minFn( m_minManip );
	MFnFreePointTriadManip maxFn( m_maxManip );
	
	minFn.connectToPointPlug( m_minPlug );
	maxFn.connectToPointPlug( m_maxPlug );
	
	addManipToPlugConversionCallback( m_minPlug, (manipToPlugConversionCallback)&Box3Manipulator::vectorManipToPlugConversion );
	addManipToPlugConversionCallback( m_maxPlug, (manipToPlugConversionCallback)&Box3Manipulator::vectorManipToPlugConversion );
	
	addPlugToManipConversionCallback( minFn.pointIndex(), (plugToManipConversionCallback)&Box3Manipulator::vectorPlugToManipConversion );
	addPlugToManipConversionCallback( maxFn.pointIndex(), (plugToManipConversionCallback)&Box3Manipulator::vectorPlugToManipConversion );		

	MFnStateManip validateFn( m_stateManip );
	validateFn.setMaxStates( 2 );

	addPlugToManipConversionCallback( validateFn.positionIndex(), (plugToManipConversionCallback)&Box3Manipulator::updateCenteredManipPosition );		

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
		// Inhereit any transform to the parent
		MDagPath transformPath = m_nodePath;
		transformPath.pop(); 
		MFnTransform transformFn( transformPath );
		m_localMatrix = transformPath.inclusiveMatrix();
		m_localMatrixInv = transformPath.inclusiveMatrixInverse();
		
		MPxTransformationMatrix m( m_localMatrix );
		MEulerRotation r = m.eulerRotation();
		MVector t = m.translation();

		minFn.setRotation( r );
		maxFn.setRotation( r );
		validateFn.setRotation( r );	
		
		minFn.setTranslation( t, MSpace::kTransform );
		maxFn.setTranslation( t, MSpace::kTransform );
		validateFn.setTranslation( t, MSpace::kTransform );
	}
	
	return stat;
}
	
void Box3Manipulator::draw( M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView::DisplayStatus status )
{
	
	MPxManipContainer::draw( view, path, style, status);
	
	MFnStateManip validateFn( m_stateManip );
	MFnFreePointTriadManip minFn( m_minManip );
	MFnFreePointTriadManip maxFn( m_maxManip );
	
	MPoint min, max, minOffset, maxOffset, center, notUsed;
	getConverterManipValue( minFn.pointIndex(), min );
	getConverterManipValue( maxFn.pointIndex(), max );
	getConverterManipValue( validateFn.positionIndex(), center );
	
	// We want to offset the labels by a constant amount in view space,
	// so as the user changes the view, the lable doesnt separate/overlap
	// the control.
	short x, y;
	
	bool drawLabel = view.worldToView( center * m_localMatrix, x, y );
	if( drawLabel ) 
	{
		y -= 18;
		view.viewToWorld( x, y, center, notUsed );
	}
	
	bool drawMin = view.worldToView( min * m_localMatrix, x, y );
	if( drawMin ) 
	{
		y -= 18;
		view.viewToWorld( x, y, minOffset, notUsed );
	}
	
	bool drawMax = view.worldToView( max * m_localMatrix, x, y );
	if( drawMax ) 
	{
		y -= 18;
		view.viewToWorld( x, y, maxOffset, notUsed );
	}
	
	view.beginGL(); 
	
		if( drawMin )
			view.drawText( MString("min"), minOffset, M3dView::kCenter );
	
		if( drawMax )
			view.drawText( MString("max"), maxOffset, M3dView::kCenter );	
		
		if( m_label != "" && drawLabel )
		{
			view.drawText( m_label, center, M3dView::kCenter );	
		}
		
	view.endGL();

	MTransformationMatrix m( m_localMatrix );
	
	MVector t = m.getTranslation( MSpace::kWorld );
	
	double r[3];
	MTransformationMatrix::RotationOrder ro;
	m.getRotation( r, ro );
	
	double s[3];
	m.getScale( s, MSpace::kWorld );

	view.beginGL(); 

		glPushMatrix();
		
		/// \todo Support other rotation orders
		glTranslated( t.x, t.y, t.z );
		glRotated( r[2] * 57.29577, 0.0, 0.0, 1.0 );
		glRotated( r[1] * 57.29577, 0.0, 1.0, 0.0 );
		glRotated( r[0] * 57.29577, 1.0, 0.0, 0.0 );
		glScaled( s[0], s[1], s[2] );

		// If the bbox is invalid, then set the color to something
		// more, er, SCARY.
		if ( min[0] > max[0] || min[1] > max[1] || min[2] > max[2] ) 
		{
			if (status == M3dView::kActive) {
				view.setDrawColor(12, M3dView::kActiveColors);
			} else {
				view.setDrawColor(12, M3dView::kDormantColors);
			}  
		}
		else
		{
			if (status == M3dView::kActive) {
				view.setDrawColor(14, M3dView::kActiveColors);
			} else {
				view.setDrawColor(14, M3dView::kDormantColors);
			}  
		}
	
		glLineStipple( 2, 0xAAAA );
		glEnable( GL_LINE_STIPPLE ); 
	
		glBegin( GL_LINE_LOOP );

			glVertex3f( min[0], min[1], min[2] );
			glVertex3f( min[0], max[1], min[2] );
			glVertex3f( max[0], max[1], min[2] );
			glVertex3f( max[0], min[1], min[2] );

		glEnd();
		
		glBegin( GL_LINE_LOOP );
		
			glVertex3f( min[0], min[1], max[2] );
			glVertex3f( min[0], max[1], max[2] );
			glVertex3f( max[0], max[1], max[2] );
			glVertex3f( max[0], min[1], max[2] );
		
		
		glEnd();
		
		glBegin( GL_LINES );
		
			glVertex3f( min[0], min[1], min[2] );
			glVertex3f( min[0], min[1], max[2] );
			
			glVertex3f( min[0], max[1], min[2] );
			glVertex3f( min[0], max[1], max[2] );

			glVertex3f( max[0], min[1], min[2] );
			glVertex3f( max[0], min[1], max[2] );
			
			glVertex3f( max[0], max[1], min[2] );
			glVertex3f( max[0], max[1], max[2] );
		
		glEnd();
		
		glDisable( GL_LINE_STIPPLE ); 
		
		glPopMatrix();
		
	view.endGL();

}


MManipData Box3Manipulator::vectorPlugToManipConversion( unsigned int manipIndex )
{
	MFnFreePointTriadManip minFn( m_minManip );
	MFnFreePointTriadManip maxFn( m_maxManip );
	
	MFnNumericData numericData;
    MObject returnData = numericData.create( MFnNumericData::k3Double );
    numericData.setData( 0.0, 0.0, 0.0 );
	
	MPlug sourcePlug;
		
	if( manipIndex == minFn.pointIndex() )
	{
		sourcePlug = m_minPlug;
	}
	else if( manipIndex == maxFn.pointIndex() )
	{
		sourcePlug = m_maxPlug;
	}
	else
	{
		return returnData;
	}
		
	MPoint p = getPlugValues( sourcePlug );
	
	numericData.setData( p.x, p.y, p.z );	
		
	return MManipData( returnData );	
}


MManipData Box3Manipulator::vectorManipToPlugConversion( unsigned int plugIndex )
{	
	
	unsigned int validateState = 0;
	MFnStateManip validateFn( m_stateManip );
	getConverterManipValue( validateFn.stateIndex(), validateState );
	
	MFnFreePointTriadManip minFn( m_minManip );
	MFnFreePointTriadManip maxFn( m_maxManip );
	
	MPoint min, max;
	getConverterManipValue( minFn.pointIndex(), min );
	getConverterManipValue( maxFn.pointIndex(), max );
	
	MPoint out;
	MPlug sourcePlug;
		
	if( validateState == 0 )
	{
		if( plugIndex == 0 )
		{
			out.x = std::min( min[0], max[0] );
			out.y = std::min( min[1], max[1] );
			out.z = std::min( min[2], max[2] );	
		}
		else
		{
			out.x = std::max( min[0], max[0] );
			out.y = std::max( min[1], max[1] );
			out.z = std::max( min[2], max[2] );	
		}
	}
	else
	{
		if( plugIndex == 0 )
		{
			out = min;
			sourcePlug = m_minPlug;
		}
		else
		{
			out = max;
			sourcePlug = m_maxPlug;
		}
	}
	
	MFnNumericData numericData;
    MObject returnData;
	
	// We have to check what type of data to generate so Maya
	// will be able to set it back into the attribute correctly.
	MFnNumericAttribute attr( sourcePlug.attribute() );
	if( attr.unitType() == MFnNumericData::k3Float )
	{
		returnData = numericData.create( MFnNumericData::k3Float );
		numericData.setData( float(out.x), float(out.y), float(out.z) );
	}
	else
	{	
		returnData = numericData.create( MFnNumericData::k3Double );
		numericData.setData( out.x, out.y, out.z );
	}	

	return MManipData( returnData );
}

MManipData Box3Manipulator::updateCenteredManipPosition( unsigned int manipIndex )
{
	MFnNumericData numericData;
    MObject returnData = numericData.create( MFnNumericData::k3Double );
	
	MPoint min = getPlugValues( m_minPlug );
	MPoint max = getPlugValues( m_maxPlug );
	
	MPoint average = ( ( min + max ) / 2.0 );
	
	numericData.setData( average.x, average.y, average.z );
	
	return MManipData( returnData );	
}

bool Box3Manipulator::findPlugs( MFnDagNode &dagFn )
{	
	MString minPlugName = m_plug.partialName() + "Min";
	MString maxPlugName = m_plug.partialName() + "Max";

	MStatus statMin, statMax;
	m_minPlug = dagFn.findPlug( minPlugName, &statMin );
	m_maxPlug = dagFn.findPlug( maxPlugName, &statMax );

	if( statMin == MStatus::kFailure || statMax == MStatus::kFailure )
	{
		m_minPlugName = "";
		m_maxPlugName = "";
		return false;
	}
	
	m_minPlugName = minPlugName;
	m_maxPlugName = maxPlugName;
	
	return true;
}

void Box3Manipulator::getPlugValues( MPlug &plug, double *values )
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

void Box3Manipulator::getPlugValues( MPlug &plug, MFnNumericData &data )
{
	double values[3];
	getPlugValues( plug, values ); 
	data.setData( values[0], values[1], values[2] );
}

MPoint Box3Manipulator::getPlugValues( MPlug &plug )
{
	double values[3];
	getPlugValues( plug, values );
	return MPoint( values[0], values[1], values[2] );
}

void Box3Manipulator::readParameterOptions( MFnDagNode &nodeFn )
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
		if( StringDataPtr wsData = uiData->member<StringData>( "box3ManipSpace" ) )
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
				MGlobal::displayWarning( "Box3Manipulator: Ignoring invalid box3ManipSpace '"
										 + MString( wsData->readable().c_str() )
										 + "' for parameter '" 
										 + MString( parameter->name().c_str() )
										 + "', using 'object'." );
			}
		}				
	}
}
