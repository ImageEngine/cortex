//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Electric Theatre Collective Limited. All rights reserved.
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

const char * IECoreMantra::ProceduralPrimitive::getClassName()
{
	return "ProceduralPrimitive";
}

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
	m_procedural->render( m_renderer );
}

void IECoreMantra::ProceduralPrimitive::addChild( ProceduralPrimitive *proc )
{
	openProceduralObject();
		addProcedural( proc );
	closeObject(); 
}

void IECoreMantra::ProceduralPrimitive::addVisibleRenderable( VisibleRenderablePtr renderable )
{
	ToHoudiniGeometryConverterPtr converter = ToHoudiniGeometryConverter::create( renderable.get() );
	if( !converter ) 
	{
		msg( Msg::Warning, "ProceduralPrimitive::addVisibleRenderable", "converter could not be found" );
		return;
	}
	GU_Detail *gdp = allocateGeometry();
	GU_DetailHandle handle;
	handle.allocateAndSet( (GU_Detail*)gdp, false );
	bool converted = converter->convert( handle );
	if ( !converted )
	{
		msg( Msg::Warning, "ProceduralPrimitive::addVisibleRenderable", "converter failed" );
		return;
	}
	/// \todo ToHoudiniGeometryConverter does not create a Houdini style uv attribute.
	/// We make one from s and t. This code should probably live in a converter or in an Op that
	/// remaps IECore conventions to common Houdini ones.
	MeshPrimitivePtr mesh = runTimeCast<MeshPrimitive> (renderable);
	if ( mesh )
	{
		gdp->addTextureAttribute( GA_ATTRIB_VERTEX );
		GEO_AttributeHandle auv = gdp->getAttribute( GA_ATTRIB_VERTEX, "uv" );
		GEO_AttributeHandle as = gdp->getAttribute( GA_ATTRIB_VERTEX, "s" );
		GEO_AttributeHandle at = gdp->getAttribute( GA_ATTRIB_VERTEX, "t" );
		if ( auv.isAttributeValid() && as.isAttributeValid() && at.isAttributeValid() )
		{
			GA_GBPrimitiveIterator it( *gdp );
			GA_Primitive *p = it.getPrimitive();
			while ( p )
			{
				for (int i = 0; i < p->getVertexCount(); ++i)
				{
					GA_Offset v = p->getVertexOffset(i);
					as.setVertex(v);
					at.setVertex(v);
					auv.setVertex(v);
					auv.setF( as.getF(0), 0 );
					auv.setF( ((at.getF(0) * -1.0f) + 1.0f), 1 ); // wat, t arrives upside down for some reason.
					auv.setF( 0.0f, 2 );
				}
				++it;
				p = it.getPrimitive();
			}
		}
	}

	if ( m_renderer->m_motionType == RendererImplementation::Geometry )
	{
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:Geometry" );
		if ( !m_renderer->m_motionTimes.empty() )
		{
			if ( (size_t)m_renderer->m_motionSize == m_renderer->m_motionTimes.size() )
			{
				openGeometryObject();
			}
			addGeometry(gdp, m_renderer->m_motionTimes.front());
			m_renderer->m_motionTimes.pop_front();
			if ( m_renderer->m_motionTimes.empty() )
			{
				applySettings();
				closeObject();
			}
		}
	}
	else if ( m_renderer->m_motionType == RendererImplementation::ConcatTransform ||
			  m_renderer->m_motionType == RendererImplementation::SetTransform )
	{
		// It isn't clear that this will give correct results. 
		// ConcatTransform may need to interpolate transform snapshots.
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:Transform" );
		openGeometryObject();
			addGeometry(gdp, 0.0f);
			while ( !m_renderer->m_motionTimes.empty() )
			{
				setPreTransform( convert< UT_Matrix4T<float> >(m_renderer->m_motionTransforms.front()),
								 m_renderer->m_motionTimes.front() );
				m_renderer->m_motionTimes.pop_front();
				m_renderer->m_motionTransforms.pop_front();
			}
			applySettings();
		closeObject();
		m_renderer->m_motionType = RendererImplementation::Unknown;
	}
	else if ( m_renderer->m_motionType == RendererImplementation::Velocity )
	{
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:Velocity" );
		import("global:fps", &m_fps, 1);
		import("camera:shutter", m_cameraShutter, 2);
		m_preBlur = -m_cameraShutter[0] / m_fps;
		m_postBlur = -m_cameraShutter[1] / m_fps;
		openGeometryObject();
			addGeometry(gdp, 0.0f);
			addVelocityBlurGeometry(gdp, m_preBlur, m_postBlur);
			applySettings();
		closeObject();
		m_renderer->m_motionType = RendererImplementation::Unknown;
	}
	else
	{
		msg(Msg::Debug, "IECoreMantra::ProceduralPrimitive::addVisibleRenderable", "MotionBlur:None" );
		openGeometryObject();
			addGeometry( gdp, 0.0f );
			setPreTransform( convert< UT_Matrix4T<float> >(m_renderer->m_transformStack.top()), 0.0f);
			applySettings();
		closeObject();
	}
}

void IECoreMantra::ProceduralPrimitive::applySettings()
{
	// Shaders are hidden in the attribute stack with a ':' prefix.
	// The renderer method Shader() stores them as a full shader invocation string.
	CompoundDataMap::const_iterator s_it = m_renderer->m_attributeStack.top().attributes->readable().find(":surface");
	if ( s_it != m_renderer->m_attributeStack.top().attributes->readable().end() )
	{
		ConstStringDataPtr s = runTimeCast<const StringData>( s_it->second );
		changeSetting("surface", s->readable().c_str(), "object");
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
		changeSetting( name.c_str(), ifd.c_str() );
	}
}

