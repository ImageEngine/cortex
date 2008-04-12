//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/IECoreGL.h"

#include "maya/MPxNode.h"
#include "maya/MPxLocatorNode.h"
#include "maya/MPxDeformerNode.h"
#include "maya/MPxObjectSet.h"
#include "maya/MPxFieldNode.h"
#include "maya/MGlobal.h"

#include "IECore/Parameterised.h"
#include "IECore/LevelFilteredMessageHandler.h"

#include "IECoreMaya/IECoreMaya.h"
#include "IECoreMaya/CacheSet.h"
#include "IECoreMaya/ParameterisedHolder.h"
#include "IECoreMaya/OpHolder.h"
#include "IECoreMaya/PythonCmd.h"
#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/ProceduralHolderUI.h"
#include "IECoreMaya/SystemExitCmd.h"
#include "IECoreMaya/MessageHandler.h"
#include "IECoreMaya/ObjectData.h"
#include "IECoreMaya/ConverterHolder.h"
#include "IECoreMaya/ImageFile.h"

namespace IECoreMaya
{

static unsigned long g_refCount = 0;

MStatus initialize(MFnPlugin &plugin)
{
	MStatus s = MS::kSuccess;
	
	if (g_refCount == 0)
	{

		if( MGlobal::mayaState()==MGlobal::kInteractive )
		{
			IECoreGL::init( true );
		}
	
		// register plugin
		
		s = plugin.registerNode( "ieCacheSet", CacheSet::id, CacheSet::creator, CacheSet::initialize,
			MPxNode::kObjectSet );
					
		s = plugin.registerNode( ParameterisedHolderNode::typeName, ParameterisedHolderNode::id, 
			ParameterisedHolderNode::creator, ParameterisedHolderNode::initialize );
		assert( s );	
			
		s = plugin.registerNode( ParameterisedHolderLocator::typeName, ParameterisedHolderLocator::id, 
			ParameterisedHolderLocator::creator, ParameterisedHolderLocator::initialize );
		assert( s );
		
		s = plugin.registerNode( ParameterisedHolderDeformer::typeName, ParameterisedHolderDeformer::id, 
			ParameterisedHolderDeformer::creator, ParameterisedHolderDeformer::initialize );
		assert( s );
		
		s = plugin.registerNode( ParameterisedHolderField::typeName, ParameterisedHolderField::id, 
			ParameterisedHolderField::creator, ParameterisedHolderField::initialize );
		assert( s );
		
		s = plugin.registerNode( ParameterisedHolderSet::typeName, ParameterisedHolderSet::id, 
			ParameterisedHolderSet::creator, ParameterisedHolderSet::initialize );
		assert( s );
		
		s = plugin.registerShape( ParameterisedHolderSurfaceShape::typeName, ParameterisedHolderSurfaceShape::id, 
			ParameterisedHolderSurfaceShape::creator, ParameterisedHolderSurfaceShape::initialize, ProceduralHolderUI::creator );
		assert( s );
		
		s = plugin.registerShape( ParameterisedHolderComponentShape::typeName, ParameterisedHolderComponentShape::id, 
			ParameterisedHolderComponentShape::creator, ParameterisedHolderComponentShape::initialize, ProceduralHolderUI::creator );
		assert( s );
		
		s = plugin.registerShape( "ieProceduralHolder", ProceduralHolder::id, 
			ProceduralHolder::creator, ProceduralHolder::initialize, ProceduralHolderUI::creator );	
		assert( s );
		
		s = plugin.registerNode( "ieOpHolderNode", OpHolderNode::id, 
			OpHolderNode::creator, OpHolderNode::initialize );
		assert( s );
			
		s = plugin.registerNode( "ieConverterHolder", ConverterHolder::id, 
			ConverterHolder::creator, ConverterHolder::initialize );			
		assert( s );
		
		s = plugin.registerCommand( "iePython", PythonCmd::creator, PythonCmd::newSyntax );
		PythonCmd::initialize();
		
		s = plugin.registerCommand( "ieSystemExit", SystemExitCmd::creator );
		
		s = plugin.registerData( ObjectData::typeName, ObjectData::id, ObjectData::creator);
		
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
			IECore::MessageHandler::pushHandler( h );
		}
	}
	
	g_refCount ++;
	
	return s;
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
		s = plugin.deregisterNode( ProceduralHolder::id );
		s = plugin.deregisterNode( OpHolderNode::id );
		s = plugin.deregisterNode( ConverterHolder::id );
		
		s = plugin.deregisterCommand( "iePython" );
		s = plugin.deregisterCommand( "ieSystemExit" );
		PythonCmd::uninitialize();
		
		s = plugin.deregisterData( ObjectData::id );
		
		s = plugin.deregisterImageFile( "ieImageFile" );
		
		// \todo Should we also pop our message handler here if we pushed one in initialize?
		// We're not doing that for now as IECore.Log.setLogLevel messes with the balancing of the
		// stack so i'm not sure popping is a good idea. Also even if we fix that then there's no real
		// guarantee that we're popping the one we pushed (other people could have pushed their own
		// handlers since ours, and not popped 'em). Not sure - maybe we need some guidelines as to
		// the nesting of handlers?
	}
	
	return s;
}

}
