//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2015, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>
#include <cassert>

#include "boost/format.hpp"

#include "maya/MPxNode.h"
#include "maya/MPxLocatorNode.h"
#include "maya/MPxDeformerNode.h"
#include "maya/MPxObjectSet.h"
#include "maya/MPxFieldNode.h"
#include "maya/MPxImagePlane.h"
#include "maya/MPxSurfaceShapeUI.h"
#include "maya/MGlobal.h"
#include "maya/MDrawRegistry.h"
#undef None // must come after certain Maya includes which include X11/X.h

#include "IECore/Parameterised.h"
#include "IECore/LevelFilteredMessageHandler.h"

#include "IECoreMaya/IECoreMaya.h"
#include "IECoreMaya/CacheSet.h"
#include "IECoreMaya/ParameterisedHolder.h"
#include "IECoreMaya/TransientParameterisedHolderNode.h"
#include "IECoreMaya/OpHolder.h"
#include "IECoreMaya/PythonCmd.h"
#include "IECoreMaya/MessageHandler.h"
#include "IECoreMaya/ObjectData.h"
#include "IECoreMaya/ConverterHolder.h"
#include "IECoreMaya/ImageFile.h"
#include "IECoreMaya/ImagePlaneHolder.h"
#include "IECoreMaya/ParameterisedHolderSetValueCmd.h"
#include "IECoreMaya/ParameterisedHolderModificationCmd.h"
#include "IECoreMaya/CurveCombiner.h"
#include "IECoreMaya/Box3Manipulator.h"
#include "IECoreMaya/V3Manipulator.h"
#include "IECoreMaya/ParameterisedHolderManipContextCommand.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/DrawableHolder.h"
#include "IECoreMaya/DrawableHolderUI.h"
#include "IECoreMaya/TransformationMatrixManipulator.h"
#include "IECoreMaya/SceneShape.h"
#include "IECoreMaya/SceneShapeUI.h"
#include "IECoreMaya/SceneShapeInterface.h"
#include "IECoreMaya/SceneShapeSubSceneOverride.h"

// see ObjectParameterHandler::doUpdate() for an explanation of the necessity for dummy data
static void *dummyDataCreator()
{
	// we never want to create dummy data, we just need the type to be registered.
	// we therefore treat any creation attempt as a failure.
	std::cerr << "IECoreMaya::dummyDataCreator : unexpected attempt to make dummy data." << std::endl;
	exit( EXIT_FAILURE );
	return 0;
}

namespace IECoreMaya
{

static unsigned long g_refCount = 0;

MStatus initialize(MFnPlugin &plugin)
{
	MStatus s = MS::kSuccess;

	if (g_refCount == 0)
	{

		// register plugin

		s = plugin.registerData( ObjectData::typeName, ObjectData::id, ObjectData::creator);
		s = plugin.registerData( "ieDummyData", DummyDataId, dummyDataCreator );

		s = plugin.registerNode( "ieCacheSet", CacheSet::id, CacheSet::creator, CacheSet::initialize,
			MPxNode::kObjectSet );

		s = plugin.registerNode( ParameterisedHolderNode::typeName, ParameterisedHolderNode::id,
			ParameterisedHolderNode::creator, ParameterisedHolderNode::initialize );
		assert( s );

		s = plugin.registerNode( ParameterisedHolderLocator::typeName, ParameterisedHolderLocator::id,
			ParameterisedHolderLocator::creator, ParameterisedHolderLocator::initialize, MPxNode::kLocatorNode );
		assert( s );

		s = plugin.registerNode( ParameterisedHolderDeformer::typeName, ParameterisedHolderDeformer::id,
			ParameterisedHolderDeformer::creator, ParameterisedHolderDeformer::initialize, MPxNode::kDeformerNode );
		assert( s );

		s = plugin.registerNode( ParameterisedHolderField::typeName, ParameterisedHolderField::id,
			ParameterisedHolderField::creator, ParameterisedHolderField::initialize, MPxNode::kFieldNode );
		assert( s );

		s = plugin.registerNode( ParameterisedHolderSet::typeName, ParameterisedHolderSet::id,
			ParameterisedHolderSet::creator, ParameterisedHolderSet::initialize, MPxNode::kObjectSet );
		assert( s );

		s = plugin.registerShape( ParameterisedHolderSurfaceShape::typeName, ParameterisedHolderSurfaceShape::id,
			ParameterisedHolderSurfaceShape::creator, ParameterisedHolderSurfaceShape::initialize, []() -> void* { return new MPxSurfaceShapeUI; } );
		assert( s );

		s = plugin.registerShape( ParameterisedHolderComponentShape::typeName, ParameterisedHolderComponentShape::id,
			ParameterisedHolderComponentShape::creator, ParameterisedHolderComponentShape::initialize, []() -> void* { return new MPxSurfaceShapeUI; } );
		assert( s );

		s = plugin.registerShape( DrawableHolder::typeName, DrawableHolder::id,
			DrawableHolder::creator, DrawableHolder::initialize, DrawableHolderUI::creator );
		assert( s );

		s = plugin.registerShape( "ieSceneShapeInterface", SceneShapeInterface::id,
			SceneShapeInterface::creator, SceneShapeInterface::initialize, SceneShapeUI::creator );
		assert( s );

		s = plugin.registerShape( "ieSceneShape", SceneShape::id,
			SceneShape::creator, SceneShape::initialize, SceneShapeUI::creator, &SceneShapeSubSceneOverride::drawDbClassification() );
		assert( s );

		s = MHWRender::MDrawRegistry::registerSubSceneOverrideCreator( SceneShapeSubSceneOverride::drawDbClassification(), SceneShapeSubSceneOverride::drawDbId(), SceneShapeSubSceneOverride::Creator );
		assert( s );

		s = plugin.registerNode( "ieOpHolderNode", OpHolderNode::id,
			OpHolderNode::creator, OpHolderNode::initialize );
		assert( s );

		s = plugin.registerNode( "ieConverterHolder", ConverterHolder::id,
			ConverterHolder::creator, ConverterHolder::initialize );
		assert( s );

		s = plugin.registerNode( TransientParameterisedHolderNode::typeName, TransientParameterisedHolderNode::id,
			TransientParameterisedHolderNode::creator, TransientParameterisedHolderNode::initialize );
		assert( s );

		s = plugin.registerNode( ParameterisedHolderImagePlane::typeName, ParameterisedHolderImagePlane::id,
			ParameterisedHolderImagePlane::creator, ParameterisedHolderImagePlane::initialize, MPxNode::kImagePlaneNode );
		assert( s );

		s = plugin.registerNode( "ieImagePlaneHolder", ImagePlaneHolder::id,
			ImagePlaneHolder::creator, ImagePlaneHolder::initialize, MPxNode::kImagePlaneNode );
		assert( s );

		s = plugin.registerNode( CurveCombiner::typeName, CurveCombiner::id,
			CurveCombiner::creator, CurveCombiner::initialize, MPxNode::kDependNode );
		assert( s );

		// This should be used to set the classification of any manipulators that are
		// compatible with the ieParameterisedHolderManipContext to permit the UI code
		// to identify if a Parameter is manipulatable.
		const MString manipClassification( "ieParameterManipulator" );
		// Convention for parameter manipulator names:
		//    ie<manipulatorTypeHint><parameterTypeName>Manipulator

		s = plugin.registerNode( "ieBox3fParameterManipulator", Box3Manipulator::id,
			Box3Manipulator::creator, Box3Manipulator::initialize, MPxNode::kManipContainer, &manipClassification );
		assert( s );

		s = plugin.registerNode( "ieTransformationMatrixfParameterManipulator", TransformationMatrixManipulator::id,
			TransformationMatrixManipulator::creator, TransformationMatrixManipulator::initialize, MPxNode::kManipContainer, &manipClassification );
		assert( s );

		s = plugin.registerNode( "ieV3fParameterManipulator", V3Manipulator::id,
			V3Manipulator::creator, V3Manipulator::initialize, MPxNode::kManipContainer, &manipClassification );
		assert( s );

		s = plugin.registerCommand( "iePython", PythonCmd::creator, PythonCmd::newSyntax );
		PythonCmd::initialize();

		s = plugin.registerCommand( "ieParameterisedHolderSetValue", ParameterisedHolderSetValueCmd::creator, ParameterisedHolderSetValueCmd::newSyntax );

		s = plugin.registerCommand( "ieParameterisedHolderModification", ParameterisedHolderModificationCmd::creator );

		s = plugin.registerContextCommand("ieParameterisedHolderManipContext", &ParameterisedHolderManipContextCommand::creator );

		MStringArray imageFileExtensions;
		imageFileExtensions.append( "exr" );

		s = plugin.registerImageFile( "ieImageFile", ImageFile::creator, imageFileExtensions);
		assert( s );

		/// \todo This may well need to change, depending on how we allow people to install
		/// the mel files.
		MString cmd = "source \"IECoreMaya/IECoreMaya.mel\";";
		s = MGlobal::executeCommand(cmd);

		if( !getenv( "IECOREMAYA_DISABLEOUTPUTREDIRECTION" ) )
		{
			IECore::MessageHandlerPtr h = new IECoreMaya::MessageHandler;
			h = new IECore::LevelFilteredMessageHandler( h, IECore::LevelFilteredMessageHandler::defaultLevel() );
			IECore::MessageHandler::setDefaultHandler( h );
		}

		if( MGlobal::mayaState() == MGlobal::kInteractive )
		{
			MGlobal::executePythonCommand( "import IECoreMaya; IECoreMaya.Menus.createCortexMenu()" );
		}

	}

	g_refCount ++;

	return MS::kSuccess;
}

MStatus uninitialize(MFnPlugin &plugin)
{
	MStatus s = MS::kSuccess;

	assert( g_refCount > 0 );

	g_refCount --;

	if (g_refCount == 0)
	{
		// unregister plugin

		s = plugin.deregisterNode( CacheSet::id );
		s = plugin.deregisterNode( ParameterisedHolderNode::id );
		s = plugin.deregisterNode( DrawableHolder::id );
		s = plugin.deregisterNode( ParameterisedHolderLocator::id );
		s = plugin.deregisterNode( ParameterisedHolderDeformer::id );
		s = plugin.deregisterNode( ParameterisedHolderField::id );
		s = plugin.deregisterNode( ParameterisedHolderSet::id );
		s = plugin.deregisterNode( ParameterisedHolderSurfaceShape::id );
		s = plugin.deregisterNode( ParameterisedHolderComponentShape::id );
		s = plugin.deregisterNode( SceneShapeInterface::id );
		s = plugin.deregisterNode( SceneShape::id );
		s = plugin.deregisterNode( OpHolderNode::id );
		s = plugin.deregisterNode( ConverterHolder::id );
		s = plugin.deregisterNode( TransientParameterisedHolderNode::id );
		s = plugin.deregisterNode( ParameterisedHolderImagePlane::id );
		s = plugin.deregisterNode( ImagePlaneHolder::id );
		s = plugin.deregisterNode( CurveCombiner::id );
		s = plugin.deregisterNode( Box3Manipulator::id );
		s = plugin.deregisterNode( TransformationMatrixManipulator::id );
		s = plugin.deregisterNode( V3Manipulator::id );

		s = plugin.deregisterCommand( "iePython" );
		PythonCmd::uninitialize();

		s = plugin.deregisterCommand( "ieParameterisedHolderModification" );
		s = plugin.deregisterCommand( "ieParameterisedHolderSetValue" );
		s = plugin.deregisterContextCommand("ieParameterisedHolderManipContext");

		s = plugin.deregisterData( DummyDataId );
		s = plugin.deregisterData( ObjectData::id );

		s = plugin.deregisterImageFile( "ieImageFile" );

		if( MGlobal::mayaState() == MGlobal::kInteractive )
		{
			MGlobal::executePythonCommand( "import IECoreMaya; IECoreMaya.Menus.removeCortexMenu()" );
		}

	}

	return s;
}

}
