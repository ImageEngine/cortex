//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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
#include "IECore/SimpleTypedParameter.h"

#include "IECoreGL/BoxPrimitive.h"

#include "IECoreMaya/TypeIds.h"
#include "IECoreMaya/TransformationMatrixManipulator.h"
#include "IECoreMaya/ParameterisedHolderInterface.h"

#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnFreePointTriadManip.h>
#include <maya/MFnRotateManip.h>
#include <maya/MFnScaleManip.h>
#include <maya/MEulerRotation.h>
#include <maya/MPxTransformationMatrix.h>

using namespace Imath;
using namespace IECore;
using namespace IECoreMaya;

MTypeId TransformationMatrixManipulator::id = TransformationMatrixManipulatorTypeId;
const MString TransformationMatrixManipulator::typeName = "ieTransformationMatrixManipulator";

TransformationMatrixManipulator::TransformationMatrixManipulator()
	: m_translatePlugName( "" ), m_rotatePlugName( "" ), m_scalePlugName( "" )
{
}

TransformationMatrixManipulator::~TransformationMatrixManipulator()
{
}

void *TransformationMatrixManipulator::creator()
{
	return new TransformationMatrixManipulator();
}

MStatus TransformationMatrixManipulator::initialize()
{
	return MPxManipContainer::initialize();
}

MStatus TransformationMatrixManipulator::createChildren()
{
	m_translateManip = addFreePointTriadManip( "Manipulates the 'translate' component of the parameter.", "translate" );
	m_rotateManip = addRotateManip( "Manipulates the 'rotate' component of the parameter", "rotate" );
	m_scaleManip = addScaleManip( "Manipulates the 'scale' component of the parameter", "scale" );

	return MStatus::kSuccess;
}

MStatus TransformationMatrixManipulator::connectToDependNode( const MObject & node )
{
	MFnDagNode dagFn( node );
	dagFn.getPath( m_nodePath );

	if( !findPlugs( dagFn ) )
	{
		return MStatus::kFailure;
	}

	MFnFreePointTriadManip translateFn( m_translateManip );
	translateFn.connectToPointPlug( m_translatePlug );

	MFnRotateManip rotateFn( m_rotateManip );
	rotateFn.setRotateMode( MFnRotateManip::kObjectSpace );
	rotateFn.connectToRotationPlug( m_rotatePlug );
	rotateFn.connectToRotationCenterPlug( m_translatePlug );
	// The Callback is used to update the orientation of the scale
	// manip as rotation changes. It dosn't actually alter the returned value.
	addManipToPlugConversionCallback(
		m_rotatePlug,
		(manipToPlugConversionCallback)&TransformationMatrixManipulator::rotationToPlugConversion
	);

	MFnScaleManip scaleFn( m_scaleManip );
	scaleFn.connectToScalePlug( m_scalePlug );

	MStatus stat = finishAddingManips();
	if( stat == MStatus::kFailure )
	{
		return MStatus::kFailure;
	}

	MPxManipContainer::connectToDependNode( node );

	// Find the matrix of the node we're manipulating so
	// we can 'parent' the manips to it.
	MDagPath transformPath = m_nodePath;
	transformPath.pop();
	m_localMatrix = transformPath.inclusiveMatrix();
	m_localMatrixInv = transformPath.inclusiveMatrixInverse();

	// Inhereit any transform to the parent
	MPxTransformationMatrix m( m_localMatrix );
	MEulerRotation r = m.eulerRotation();
	MVector t = m.translation();

	translateFn.setRotation( r );
	scaleFn.setRotation( r );
	rotateFn.setRotation( r );

	translateFn.setTranslation( t, MSpace::kTransform );
	scaleFn.setTranslation( t, MSpace::kTransform );
	rotateFn.setTranslation( t, MSpace::kTransform );

	// Update any local t/r on the scale manip
	MPoint localT = getPlugValues( m_translatePlug );
	MPoint localR = getPlugValues( m_rotatePlug );
	scaleFn.translateBy( MVector( localT.x, localT.y, localT.z), MSpace::kObject );
	scaleFn.rotateBy( MEulerRotation( localR.x, localR.y, localR.z), MSpace::kObject );

	return stat;
}

void TransformationMatrixManipulator::draw( M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView::DisplayStatus status )
{
	MPxManipContainer::draw( view, path, style, status);

	/// \todo Is it not a bit crazy that the parameter isn't just available
	/// as a piece of member data along with m_plug? And that we instead have
	/// to jump through these hoops?

	MFnDependencyNode fnDN( m_plug.node() );
	ParameterisedHolderInterface *holder = dynamic_cast<ParameterisedHolderInterface *>( fnDN.userNode() );
	if( !holder )
	{
		return;
	}
	TransformationMatrixfParameter *parameter = runTimeCast<TransformationMatrixfParameter>( holder->plugParameter( m_plug ).get() );
	if( !parameter )
	{
		return;
	}

	const CompoundObject *uiUserData = parameter->userData()->member<CompoundObject>( "UI" );
	if( !uiUserData )
	{
		return;
	}

	const Box3fData *box = uiUserData->member<Box3fData>( "manipulatorBox" );
	if( box )
	{
		holder->setParameterisedValue( parameter );
		TransformationMatrixf t = parameter->getTypedValue();
		M44f m = t.transform();

		view.beginGL();
			glPushMatrix();
				glMultMatrixf( m.getValue() );
				glPushAttrib( GL_CURRENT_BIT );
					view.setDrawColor( 4 );
					IECoreGL::BoxPrimitive::renderWireframe( box->readable() );
				glPopAttrib();
			glPopMatrix();
		view.endGL();
	}
}

MManipData TransformationMatrixManipulator::rotationToPlugConversion( unsigned int plugIndex )
{
	MFnRotateManip rotateFn( m_rotateManip );
	MFnFreePointTriadManip translateFn( m_translateManip );
	MFnScaleManip scaleFn( m_scaleManip );

	MEulerRotation r;
	getConverterManipValue( rotateFn.rotationIndex(), r );

	MFnNumericData numericData;
	MObject returnData = numericData.create( MFnNumericData::k3Double );
	numericData.setData( r.x, r.y, r.z );

	// we need to update the position/rotation of the scale manip
	MVector t;
	getConverterManipValue( translateFn.pointIndex(), t );
	MPxTransformationMatrix m( m_localMatrix );
	scaleFn.setTranslation( m.translation(), MSpace::kTransform );
	scaleFn.setRotation( m.eulerRotation() );
	scaleFn.translateBy( t, MSpace::kObject );
	scaleFn.rotateBy( r, MSpace::kObject );

	return MManipData( returnData );
}


bool TransformationMatrixManipulator::findPlugs( MFnDagNode &dagFn )
{
	MString translatePlugName = m_plug.partialName() + "translate";
	MString rotatePlugName = m_plug.partialName() + "rotate";
	MString scalePlugName = m_plug.partialName() + "scale";

	MStatus statTranslate, statRotate, statScale;
	m_translatePlug = dagFn.findPlug( translatePlugName, &statTranslate );
	m_rotatePlug = dagFn.findPlug( rotatePlugName, &statRotate );
	m_scalePlug = dagFn.findPlug( scalePlugName, &statScale );

	if( !statTranslate || !statRotate || !statScale )
	{
		m_translatePlugName = "";
		m_rotatePlugName = "";
		m_scalePlugName = "";
		return false;
	}

	m_translatePlugName = translatePlugName;
	m_rotatePlugName = rotatePlugName;
	m_scalePlugName = scalePlugName;
	return true;
}

void TransformationMatrixManipulator::getPlugValues( MPlug &plug, double *values )
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

void TransformationMatrixManipulator::getPlugValues( MPlug &plug, MFnNumericData &data )
{
	double values[3];
	getPlugValues( plug, values );
	data.setData( values[0], values[1], values[2] );
}

MPoint TransformationMatrixManipulator::getPlugValues( MPlug &plug )
{
	double values[3];
	getPlugValues( plug, values );
	return MPoint( values[0], values[1], values[2] );
}
