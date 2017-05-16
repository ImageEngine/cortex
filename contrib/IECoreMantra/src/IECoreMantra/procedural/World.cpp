//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Electric Theatre Collective Limited. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the following disclaimer.
//
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the following disclaimer in the
//	   documentation and/or other materials provided with the distribution.
//
//	 * Neither the name of Image Engine Design nor the names of any
//	   other contributors to this software may be used to endorse or
//	   promote products derived from this software without specific prior
//	   written permission.
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

#include <unistd.h>

#include <iostream>

#include <UT/UT_WorkArgs.h>
#include <UT/UT_String.h>
#include <GU/GU_Detail.h>

#include "IECore/MessageHandler.h"
#include "IECore/Group.h"
#include "IECore/Reader.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreMantra/Renderer.h"
#include "IECoreMantra/ProceduralPrimitive.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreMantra;
using namespace std;
using namespace boost;

class VRAY_ieWorld : public ProceduralPrimitive {
public:
	VRAY_ieWorld();
	virtual ~VRAY_ieWorld();

#if UT_MAJOR_VERSION_INT >= 14

	virtual const char *className() const;

#else

	virtual const char  *getClassName();

#endif

	virtual int	  initialize(const UT_BoundingBox *);
	virtual void	 getBoundingBox(UT_BoundingBox &box);
	virtual void	 render();
	UT_String m_worldFileName;
	int m_remove;
};

static VRAY_ProceduralArg theArgs[] = {
};

VRAY_Procedural *
allocProcedural(const char *)
{
   return new VRAY_ieWorld();
}

const VRAY_ProceduralArg *
getProceduralArgs(const char *)
{
	return theArgs;
}

VRAY_ieWorld::VRAY_ieWorld()
{
	m_remove = 0;
}

VRAY_ieWorld::~VRAY_ieWorld()
{
	if ( m_remove == 1 )
	{
		if( access( m_worldFileName.buffer(), R_OK|F_OK ) != -1 )
		{
			if ( remove( m_worldFileName.buffer() ) != 0 )
			{
				msg( Msg::Warning, "VRAY_ieWorld", boost::format("Failed to remove ieworld cache file: %s") % m_worldFileName.buffer() );
			}
		}
	}
}

#if UT_MAJOR_VERSION_INT >= 14

const char *VRAY_ieWorld::className() const
{
	return "VRAY_ieWorld";
}

#else

const char *VRAY_ieWorld::getClassName()
{
	return "VRAY_ieWorld";
}

#endif

// The initialize method is called when the procedural is created. 
// Returning zero (failure) will abort the rendering of this procedural.
// The bounding box passed in is the user defined bounding box. 
// If the user didn't specify a bounding box, then the box will be NULL
int
VRAY_ieWorld::initialize(const UT_BoundingBox *box)
{
	if ( box )
    {
		m_bound = convert<Imath::Box3f> ( *box );
	}
	import( "ieworldfile", m_worldFileName);
	import( "ieworldremove", &m_remove, 1);
	if( access( m_worldFileName.buffer(), R_OK|F_OK ) == -1 )
	{
		msg( Msg::Warning, "VRAY_ieWorld", boost::format("Failed to find ieworld cache file: %s") % m_worldFileName.buffer() );
		return 0;
	}
	return 1;	
}

void
VRAY_ieWorld::getBoundingBox(UT_BoundingBox &box)
{
	box = convert<UT_BoundingBox>( m_bound );
}

// When mantra determines that the bounding box needs to be rendered, the 
// render method is called. At this point, the procedural can either 
// generate geometry (VRAY_Procedural::openGeometryObject()) or it can 
// generate further procedurals (VRAY_Procedural::openProceduralObject()).
void
VRAY_ieWorld::render()
{
	ConstVisibleRenderablePtr renderable = 0;
	try
	{
		ReaderPtr reader = Reader::create( m_worldFileName.buffer() );
		renderable = runTimeCast<VisibleRenderable>( reader->read() );
	}
	catch ( IECore::Exception e )
	{
		msg( Msg::Warning, "VRAY_ieWorld", boost::format("Failed to load ieworld cache file: %s") % m_worldFileName.buffer() );
	}
	if ( !renderable )
	{
		msg( Msg::Warning, "VRAY_ieWorld", boost::format("Failed to read ieworld cache file: %s") % m_worldFileName.buffer() );
	}
	IECoreMantra::RendererPtr renderer = new IECoreMantra::Renderer( this );
	renderable->render( renderer.get() );
}

