//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Electric Theatre Collective Limited. All rights reserved.
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include <UT/UT_DSOVersion.h>

#include "IECore/MeshPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/MessageHandler.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/ToHoudiniAttribConverter.h"

#include "IECoreMantra/private/RendererImplementation.h"

#include <GA/GA_GBIterators.h>
#include <GEO/GEO_AttributeHandle.h>

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreHoudini;
using namespace IECoreMantra;
using namespace Imath;
using namespace boost;

IECoreMantra::ProceduralPrimitive::ProceduralPrimitive()
	: m_renderer(0), m_procedural(0)
{
}

IECoreMantra::ProceduralPrimitive::~ProceduralPrimitive()
{
}

#if UT_MAJOR_VERSION_INT >= 14

const char *IECoreMantra::ProceduralPrimitive::className() const
{
	return "ProceduralPrimitive";
}

#else

const char * IECoreMantra::ProceduralPrimitive::getClassName()
{
	return "ProceduralPrimitive";
}

#endif

int IECoreMantra::ProceduralPrimitive::initialize(const UT_BoundingBox* box)
{
	return 0;
}

void IECoreMantra::ProceduralPrimitive::getBoundingBox(UT_BoundingBox &box)
{
	box = convert<UT_BoundingBox>( m_bound );
}

void IECoreMantra::ProceduralPrimitive::render()
{
	m_procedural->render( m_renderer.get() );
}

void IECoreMantra::ProceduralPrimitive::addVisibleRenderable( VisibleRenderablePtr renderable )
{
	ToHoudiniGeometryConverterPtr converter = ToHoudiniGeometryConverter::create( renderable.get() );
	if( !converter )
	{
		msg( Msg::Warning, "ProceduralPrimitive::addVisibleRenderable", "converter could not be found" );
		return;
	}
	VRAY_ProceduralGeo proceduralGeo = createGeometry();
	GU_Detail *gdp = proceduralGeo.get();
#if UT_MAJOR_VERSION_INT >= 16
	GU_DetailHandle handle = proceduralGeo.handle();
#else
	GU_DetailHandle handle;
#endif
	handle.allocateAndSet( (GU_Detail*)gdp, false );
	bool converted = converter->convert( handle );
	if ( !converted )
	{
		msg( Msg::Warning, "ProceduralPrimitive::addVisibleRenderable", "converter failed" );
		return;
	}

	if ( m_renderer->m_motionType == RendererImplementation::Geometry )
	{
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:Geometry" );
		if ( !m_renderer->m_motionTimes.empty() )
		{
			VRAY_ProceduralChildPtr proceduralGeometryChild = createChild();
			proceduralGeometryChild->addGeometry(proceduralGeo);
			m_renderer->m_motionTimes.pop_front();
			if ( m_renderer->m_motionTimes.empty() )
			{
				applySettings(proceduralGeometryChild);
			}
		}
	}
	else if ( m_renderer->m_motionType == RendererImplementation::ConcatTransform ||
			  m_renderer->m_motionType == RendererImplementation::SetTransform )
	{
		// It isn't clear that this will give correct results.
		// ConcatTransform may need to interpolate transform snapshots.
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:Transform" );
		VRAY_ProceduralChildPtr proceduralGeometryChild = createChild();
		proceduralGeometryChild->addGeometry(proceduralGeo);
		while ( !m_renderer->m_motionTimes.empty() )
		{
			UT_Matrix4T<float> frontTransform =  convert< UT_Matrix4T<float> >( m_renderer->m_motionTransforms.front() );
			proceduralGeometryChild->setPreTransform( UT_Matrix4T<double>( frontTransform ),
							 m_renderer->m_motionTimes.front() );
			m_renderer->m_motionTimes.pop_front();
			m_renderer->m_motionTransforms.pop_front();
		}
		applySettings(proceduralGeometryChild);

		m_renderer->m_motionType = RendererImplementation::Unknown;
	}
	else if ( m_renderer->m_motionType == RendererImplementation::Velocity )
	{
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:Velocity" );
		import("global:fps", &m_fps, 1);
		import("camera:shutter", m_cameraShutter, 2);
		m_preBlur = -m_cameraShutter[0] / m_fps;
		m_postBlur = -m_cameraShutter[1] / m_fps;
		VRAY_ProceduralChildPtr proceduralGeometryChild = createChild();
		proceduralGeometryChild->addGeometry(proceduralGeo);
		proceduralGeo.addVelocityBlur(m_preBlur, m_postBlur);
		applySettings(proceduralGeometryChild);
		m_renderer->m_motionType = RendererImplementation::Unknown;
	}
	else
	{
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:None" );
		VRAY_ProceduralChildPtr proceduralGeometryChild = createChild();
		proceduralGeometryChild->addGeometry(proceduralGeo);
		UT_Matrix4T<float> topTransform = convert< UT_Matrix4T<float> >( m_renderer->m_transformStack.top() );
		proceduralGeometryChild->setPreTransform( UT_Matrix4T<double>( topTransform ), 0.0f);
		applySettings(proceduralGeometryChild);
	}
}

void IECoreMantra::ProceduralPrimitive::applySettings(VRAY_ProceduralChildPtr child)
{
	// Shaders are hidden in the attribute stack with a ':' prefix.
	// The renderer method Shader() stores them as a full shader invocation string.
	CompoundDataMap::const_iterator s_it = m_renderer->m_attributeStack.top().attributes->readable().find(":surface");
	if ( s_it != m_renderer->m_attributeStack.top().attributes->readable().end() )
	{
		ConstStringDataPtr s = runTimeCast<const StringData>( s_it->second );
		child->changeSetting("surface", s->readable().c_str(), "object");
	}
	CompoundDataMap::const_iterator a_it;
	for (a_it = m_renderer->m_attributeStack.top().attributes->readable().begin();
		 a_it != m_renderer->m_attributeStack.top().attributes->readable().end(); a_it++)
	{
		std::string name = a_it->first;
		if ( name.compare(0, 1, ":") == 0 ) // skip internal attributes
		{
			continue;
		}
		std::string ifd, type;
		m_renderer->ifdString( a_it->second, ifd, type);
		/// \todo there are more efficient changeSetting methods  that don't use string values
		child->changeSetting( name.c_str(), ifd.c_str() );
	}
}

