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

#include "maya/M3dView.h"
#include "maya/MImage.h"
#include "maya/MUiMessage.h"

#include "IECore/ImagePrimitive.h"
#include "IECore/Writer.h"
#include "IECore/ImagePremultiplyOp.h"

#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/ViewportPostProcessCallback.h"

using namespace IECore;
using namespace IECoreMaya;

ViewportPostProcessCallback::Instances ViewportPostProcessCallback::g_instances;
size_t ViewportPostProcessCallback::g_numInstances = 0;

MStatus ViewportPostProcessCallback::registerCallback( const MString &panelName, ViewportPostProcessPtr postProcess )
{
	assert( postProcess );

        std::string key = panelName.asChar();

        Instances::iterator it = g_instances.find( key );
        if ( it == g_instances.end() )
        {
                g_instances[key] = new ViewportPostProcessCallback( panelName, postProcess );
                assert( g_numInstances == g_instances.size() );
        }
        else
        {
                it->second->m_postProcess = postProcess;
        }

        return MS::kSuccess;

}

MStatus ViewportPostProcessCallback::deregisterCallback( const MString &panelName )
{
        std::string key = panelName.asChar();

        Instances::iterator it = g_instances.find( key );
        if ( it == g_instances.end() )
        {
                return MS::kFailure;
        }
        else
        {
                g_instances.erase( it );
                return MS::kSuccess;
        }

}

ViewportPostProcessCallback::ViewportPostProcessCallback( const MString &panelName, ViewportPostProcessPtr postProcess ) :  m_postProcess( postProcess ), m_panelName( panelName )
{
	g_numInstances ++;

	MStatus s;

	m_viewPreRenderId = MUiMessage::add3dViewPreRenderMsgCallback( panelName, &ViewportPostProcessCallback::viewPreRender, this, &s );
        IECoreMaya::StatusException::throwIfError( s );

        m_viewPostRenderId = MUiMessage::add3dViewPostRenderMsgCallback( panelName, &ViewportPostProcessCallback::viewPostRender, this, &s );
        IECoreMaya::StatusException::throwIfError( s );

	/// \todo deregister when panel is destroyed?
}


ViewportPostProcessCallback::~ViewportPostProcessCallback()
{
	MMessage::removeCallback( m_viewPreRenderId );
	MMessage::removeCallback( m_viewPostRenderId );

	-- g_numInstances;
}

void ViewportPostProcessCallback::viewPreRender( const MString &panelName, void *clientData )
{
        assert( clientData );
        ViewportPostProcessCallback::Ptr cb = static_cast< ViewportPostProcessCallback * >( clientData );
        assert( cb->m_panelName == panelName );
        assert( g_instances.find( cb->m_panelName.asChar() ) != g_instances.end() );
        cb->m_postProcess->preRender( panelName.asChar() );

}

void ViewportPostProcessCallback::viewPostRender( const MString &panelName, void *clientData )
{
	assert( clientData );
        ViewportPostProcessCallback::Ptr cb = static_cast< ViewportPostProcessCallback * >( clientData );
        assert( cb->m_panelName == panelName );
        assert( g_instances.find( cb->m_panelName.asChar() ) != g_instances.end() );
	assert( cb->m_postProcess );

	M3dView view;
	MStatus s = M3dView::getM3dViewFromModelPanel( cb->m_panelName, view );
	assert( s );

	MImage viewportImage;
	s = view.readColorBuffer( viewportImage, true );
	if ( !s )
	{
		return;
	}

	bool needsDepth = cb->m_postProcess->needsDepth();

	if ( needsDepth )
	{
		unsigned int xOffset=0, yOffset=0, width=0, height=0;
		view.viewport( xOffset, yOffset, width, height );

		std::vector<float> depthBuffer;
		depthBuffer.resize( width * height );

		s = view.readDepthMap( 0, 0, width, height, (unsigned char*)( &depthBuffer[0] ), M3dView::kDepth_Float );
		if ( s )
		{
			viewportImage.setDepthMap( &depthBuffer[0], width, height );
		}
	}

#ifndef NDEBUG
	unsigned oldWidth, oldHeight;
	s = viewportImage.getSize( oldWidth, oldHeight );
	assert( s ) ;
#endif

	cb->m_postProcess->postRender( panelName.asChar(), viewportImage );

#ifndef NDEBUG
	unsigned newWidth, newHeight;
	s = viewportImage.getSize( newWidth, newHeight );
	assert( s ) ;
	assert( newWidth == oldWidth );
	assert( newHeight == oldHeight );
#endif

	s = view.writeColorBuffer( viewportImage );

	assert( s );
}
