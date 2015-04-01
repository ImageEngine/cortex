//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/private/EditBlockHandler.h"

#include <cassert>

#include "tbb/atomic.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/private/AppleseedUtil.h"

using namespace IECore;
using namespace boost;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

class IECoreAppleseed::EditBlockHandler::RendererController : public asr::DefaultRendererController
{
	public :

		RendererController()
		{
			m_status = ContinueRendering;
		}

		virtual void release()
		{
			delete this;
		}

		virtual Status get_status() const
		{
			return m_status;
		}

		void set_status( Status status )
		{
			m_status = status;
		}

	private :

		tbb::atomic<Status> m_status;
};

IECoreAppleseed::EditBlockHandler::EditBlockHandler( asr::Project &project )
	: m_project( project )
{
	m_editDepth = 0;
	m_rendererController.reset( new RendererController() );
}

IECoreAppleseed::EditBlockHandler::~EditBlockHandler()
{
	stopRendering();
}

void IECoreAppleseed::EditBlockHandler::renderThreadFunc( EditBlockHandler *self )
{
	try
	{
		self->m_renderer->render();
	}
	catch( ... )
	{
		msg( MessageHandler::Error, "IECoreAppleseed::Renderer", "Exception in render thread" );
	}
}

void IECoreAppleseed::EditBlockHandler::startRendering()
{
	if( m_rendererController->get_status() == asr::IRendererController::PauseRendering )
	{
		m_rendererController->set_status( asr::IRendererController::ContinueRendering );
	}
	else
	{
		asr::Configuration *cfg = m_project.configurations().get_by_name( "interactive" );
		const asr::ParamArray &params = cfg->get_parameters();

		if( !m_renderer.get() )
		{
			m_renderer.reset( new asr::MasterRenderer( m_project, params, m_rendererController.get() ) );
		}
		else
		{
			m_renderer->get_parameters() = params;
		}

		m_rendererController->set_status( asr::IRendererController::ContinueRendering );
		m_renderingThread = thread( &IECoreAppleseed::EditBlockHandler::renderThreadFunc, this );
	}
}

void IECoreAppleseed::EditBlockHandler::pauseRendering()
{
	m_rendererController->set_status( asr::IRendererController::PauseRendering );
}

void IECoreAppleseed::EditBlockHandler::stopRendering()
{
	m_rendererController->set_status( asr::IRendererController::AbortRendering );
	m_renderingThread.join();
}

bool IECoreAppleseed::EditBlockHandler::insideEditBlock() const
{
	return m_editDepth != 0;
}

const string & IECoreAppleseed::EditBlockHandler::exactScopeName() const
{
	return m_exactScopeName;
}

void IECoreAppleseed::EditBlockHandler::editBegin( const string &editType, const CompoundDataMap &parameters )
{
	if( !m_renderer.get() )
	{
		msg( Msg::Error, "IECoreAppleseed::RendererImplementation::editBegin", "editBegin called before worldEnd." );
		return;
	}

	++m_editDepth;

	if( editType == "suspendrendering" )
	{
		pauseRendering();
		return;
	}

	stopRendering();
	m_exactScopeName.clear();

	if( editType == "attribute" )
	{
		CompoundDataMap::const_iterator it = parameters.find( "exactscopename" );
		if( it != parameters.end() )
		{
			if( const StringData *f = runTimeCast<const StringData>( it->second.get() ) )
			{
				m_exactScopeName = f->readable();
			}
			else
			{
				msg( Msg::Error, "IECoreAppleseed::RendererImplementation::editBegin", "exactscopename parameter must be a StringData value." );
			}
		}
	}
}

void IECoreAppleseed::EditBlockHandler::editEnd()
{
	if( !m_renderer.get() )
	{
		msg( Msg::Error, "IECoreAppleseed::RendererImplementation::editEnd", "editEnd called before worldEnd." );
		return;
	}

	if( m_editDepth <= 0 )
	{
		msg( Msg::Error, "IECoreAppleseed::RendererImplementation::editEnd", "Bad editBegin / End block." );
		return;
	}

	if( --m_editDepth == 0)
	{
		startRendering();
	}
}
