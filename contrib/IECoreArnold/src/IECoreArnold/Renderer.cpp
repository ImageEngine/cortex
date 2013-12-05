//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

// This must come before the Cortex includes, because on OSX headers included
// by TBB define macros which conflict with the inline functions in ai_types.h.
#include "ai.h"

#include "IECoreArnold/Renderer.h"
#include "IECoreArnold/private/RendererImplementation.h"

using namespace IECoreArnold;

IE_CORE_DEFINERUNTIMETYPED( Renderer );

Renderer::Renderer()
	:	m_implementation( new RendererImplementation() )
{
}

Renderer::Renderer( const std::string &assFileName )
	:	m_implementation( new RendererImplementation( assFileName ) )
{
}

Renderer::Renderer( RendererImplementationPtr implementation )
	:	m_implementation( implementation )
{
}

Renderer::Renderer( const AtNode *proceduralNode )
	:	m_implementation( new RendererImplementation( (const AtNode *)proceduralNode ) )
{
}

Renderer::~Renderer()
{
}

void Renderer::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	m_implementation->setOption( name, value );
}

IECore::ConstDataPtr Renderer::getOption( const std::string &name ) const
{
	return m_implementation->getOption( name );
}

void Renderer::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	m_implementation->camera( name, parameters );
}

void Renderer::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	m_implementation->display( name, type, data, parameters );
}

void Renderer::worldBegin()
{
	m_implementation->worldBegin();
}

void Renderer::worldEnd()
{
	m_implementation->worldEnd();
}

void Renderer::transformBegin()
{
	m_implementation->transformBegin();
}

void Renderer::transformEnd()
{
	m_implementation->transformEnd();
}

void Renderer::setTransform( const Imath::M44f &m )
{
	m_implementation->setTransform( m );
}

void Renderer::setTransform( const std::string &coordinateSystem )
{
	m_implementation->setTransform( coordinateSystem );
}

Imath::M44f Renderer::getTransform() const
{
	return m_implementation->getTransform();
}

Imath::M44f Renderer::getTransform( const std::string &coordinateSystem ) const
{
	return m_implementation->getTransform( coordinateSystem );
}

void Renderer::concatTransform( const Imath::M44f &m )
{
	m_implementation->concatTransform( m );
}

void Renderer::coordinateSystem( const std::string &name )
{
	m_implementation->coordinateSystem( name );
}

void Renderer::attributeBegin()
{
	m_implementation->attributeBegin();
}

void Renderer::attributeEnd()
{
	m_implementation->attributeEnd();
}

void Renderer::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	m_implementation->setAttribute( name, value );
}

IECore::ConstDataPtr Renderer::getAttribute( const std::string &name ) const
{
	return m_implementation->getAttribute( name );
}

void Renderer::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	m_implementation->shader( type, name, parameters );
}

void Renderer::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	m_implementation->light( name, handle, parameters );
}

void Renderer::illuminate( const std::string &lightHandle, bool on )
{
	m_implementation->illuminate( lightHandle, on );
}

void Renderer::motionBegin( const std::set<float> &times )
{
	m_implementation->motionBegin( times );
}

void Renderer::motionEnd()
{
	m_implementation->motionEnd();
}

void Renderer::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->points( numPoints, primVars );
}

void Renderer::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->disk( radius, z, thetaMax, primVars );
}

void Renderer::curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->curves( basis, periodic, numVertices, primVars );
}

void Renderer::text( const std::string &font, const std::string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->text( font, text, kerning, primVars );
}

void Renderer::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->sphere( radius, zMin, zMax, thetaMax, primVars );
}

void Renderer::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->image( dataWindow, displayWindow, primVars );
}

void Renderer::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->mesh( vertsPerFace, vertIds, interpolation, primVars );
}

void Renderer::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->nurbs( uOrder, uKnot, uMin, uMax, vOrder, vKnot, vMin, vMax, primVars );
}

void Renderer::patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->patchMesh( uBasis, vBasis, nu, uPeriodic, nv, vPeriodic, primVars );
}

void Renderer::geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->geometry( type, topology, primVars );
}

void Renderer::procedural( IECore::Renderer::ProceduralPtr proc )
{
	m_implementation->procedural( proc );
}

void Renderer::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	m_implementation->instanceBegin( name, parameters );
}

void Renderer::instanceEnd()
{
	m_implementation->instanceEnd();
}

void Renderer::instance( const std::string &name )
{
	m_implementation->instance( name );
}

IECore::DataPtr Renderer::command( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	return m_implementation->command( name, parameters );
}

void Renderer::editBegin( const std::string &editType, const IECore::CompoundDataMap &parameters )
{
	m_implementation->editBegin( editType, parameters );
}

void Renderer::editEnd()
{
	m_implementation->editEnd();
}
	
size_t Renderer::numProceduralNodes() const
{
	return m_implementation->m_nodes.size();
}
		
void *Renderer::proceduralNode( size_t index )
{
	return m_implementation->m_nodes[index];
}
