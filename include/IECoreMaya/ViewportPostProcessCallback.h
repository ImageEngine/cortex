//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_VIEWPORTPOSTPROCESSCALLBACK_H
#define IE_COREMAYA_VIEWPORTPOSTPROCESSCALLBACK_H

#include <map>
#include <string>

#include "maya/MString.h"
#include "maya/MMessage.h"

#include "IECore/RefCounted.h"

#include "IECoreMaya/ViewportPostProcess.h"

namespace IECoreMaya
{

/// A class which defines a mechanism for attaching ViewportPostProcess instances to a panel. Only one ViewportPostProcess can
/// be associated with any given panel, so subsequent registrations will override earlier ones.
///
/// Example:
///
///    class MyPostProcess( ImageViewportPostProcess ) :
///       def __init__( self ) :
///         ImageViewportPostProcess.__init__( self )
///
///       def postRender( self, image ) :
///
///         for i in xrange( 0, len( image["R"].data ) ):
///           image["R"].data[i] *= 0.5
///
///    ViewportPostProcessCallback.registerCallback( "modelPanel4", MyPostProcess() )
class ViewportPostProcessCallback : public IECore::RefCounted
{
	public :
	
		IE_CORE_DECLAREMEMBERPTR( ViewportPostProcessCallback );
	
		static MStatus registerCallback( const MString &panelName, ViewportPostProcessPtr postProcess );
		static MStatus deregisterCallback( const MString &panelName );
		
	protected :
	
		ViewportPostProcessCallback( const MString &panelName, ViewportPostProcessPtr postProcess );
		virtual ~ViewportPostProcessCallback();
	
	 	static void viewPreRender( const MString &panelName, void *clientData );
                static void viewPostRender( const MString &panelName, void *clientData );
		
		typedef std::map< std::string, ViewportPostProcessCallback::Ptr> Instances;
		static Instances g_instances;
                static size_t g_numInstances;
		
		MCallbackId m_viewPreRenderId;
                MCallbackId m_viewPostRenderId; 
                MCallbackId m_idleId;
		
		ViewportPostProcessPtr m_postProcess;
		
		MString m_panelName;
};

} // namespace IECoreMaya

#endif // IE_COREMAYA_VIEWPORTPOSTPROCESSCALLBACK_H
