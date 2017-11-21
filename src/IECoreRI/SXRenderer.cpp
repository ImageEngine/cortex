//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/SXRenderer.h"
#include "IECoreRI/private/SXRendererImplementation.h"

using namespace IECoreRI;

IE_CORE_DEFINERUNTIMETYPED( SXRenderer );

SXRenderer::SXRenderer()
	:	m_implementation( new SXRendererImplementation( this ) )
{
}

SXRenderer::~SXRenderer()
{
}

void SXRenderer::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	m_implementation->setOption( name, value );
}

IECore::ConstDataPtr SXRenderer::getOption( const std::string &name ) const
{
	return m_implementation->getOption( name );
}

void SXRenderer::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	m_implementation->camera( name, parameters );
}

void SXRenderer::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	m_implementation->display( name, type, data, parameters );
}

void SXRenderer::worldBegin()
{
	m_implementation->worldBegin();
}

void SXRenderer::worldEnd()
{
	m_implementation->worldEnd();
}

void SXRenderer::transformBegin()
{
	m_implementation->transformBegin();
}

void SXRenderer::transformEnd()
{
	m_implementation->transformEnd();
}

void SXRenderer::setTransform( const Imath::M44f &m )
{
	m_implementation->setTransform( m );
}

void SXRenderer::setTransform( const std::string &coordinateSystem )
{
	m_implementation->setTransform( coordinateSystem );
}

Imath::M44f SXRenderer::getTransform() const
{
	return m_implementation->getTransform();
}

Imath::M44f SXRenderer::getTransform( const std::string &coordinateSystem ) const
{
	return m_implementation->getTransform( coordinateSystem );
}

void SXRenderer::concatTransform( const Imath::M44f &m )
{
	m_implementation->concatTransform( m );
}

void SXRenderer::coordinateSystem( const std::string &name )
{
	m_implementation->coordinateSystem( name );
}

void SXRenderer::attributeBegin()
{
	m_implementation->attributeBegin();
}

void SXRenderer::attributeEnd()
{
	m_implementation->attributeEnd();
}

void SXRenderer::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	m_implementation->setAttribute( name, value );
}

IECore::ConstDataPtr SXRenderer::getAttribute( const std::string &name ) const
{
	return m_implementation->getAttribute( name );
}

void SXRenderer::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	m_implementation->shader( type, name, parameters );
}

void SXRenderer::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	m_implementation->light( name, handle, parameters );
}

void SXRenderer::illuminate( const std::string &lightHandle, bool on )
{
	m_implementation->illuminate( lightHandle, on );
}

void SXRenderer::motionBegin( const std::set<float> &times )
{
	m_implementation->motionBegin( times );
}

void SXRenderer::motionEnd()
{
	m_implementation->motionEnd();
}

void SXRenderer::points( size_t numPoints, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->points( numPoints, primVars );
}

void SXRenderer::disk( float radius, float z, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->disk( radius, z, thetaMax, primVars );
}

void SXRenderer::curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->curves( basis, periodic, numVertices, primVars );
}

void SXRenderer::text( const std::string &font, const std::string &text, float kerning, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->text( font, text, kerning, primVars );
}

void SXRenderer::sphere( float radius, float zMin, float zMax, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->sphere( radius, zMin, zMax, thetaMax, primVars );
}

void SXRenderer::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->image( dataWindow, displayWindow, primVars );
}

void SXRenderer::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->mesh( vertsPerFace, vertIds, interpolation, primVars );
}

void SXRenderer::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->nurbs( uOrder, uKnot, uMin, uMax, vOrder, vKnot, vMin, vMax, primVars );
}

void SXRenderer::patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->patchMesh( uBasis, vBasis, nu, uPeriodic, nv, vPeriodic, primVars );
}

void SXRenderer::geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECoreScene::PrimitiveVariableMap &primVars )
{
	m_implementation->geometry( type, topology, primVars );
}

void SXRenderer::procedural( IECoreScene::Renderer::ProceduralPtr proc )
{
	m_implementation->procedural( proc );
}

void SXRenderer::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	m_implementation->instanceBegin( name, parameters );
}

void SXRenderer::instanceEnd()
{
	m_implementation->instanceEnd();
}

void SXRenderer::instance( const std::string &name )
{
	m_implementation->instance( name );
}

IECore::DataPtr SXRenderer::command( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	return m_implementation->command( name, parameters );
}

void SXRenderer::editBegin( const std::string &editType, const IECore::CompoundDataMap &parameters )
{
	m_implementation->editBegin( editType, parameters );
}

void SXRenderer::editEnd()
{
	m_implementation->editEnd();
}

IECore::CompoundDataPtr SXRenderer::shade( const IECore::CompoundData *points ) const
{
	return m_implementation->shade( points );
}

IECore::CompoundDataPtr SXRenderer::shade( const IECore::CompoundData *points, const Imath::V2i &gridSize ) const
{
	return m_implementation->shade( points, gridSize );
}

IECore::CompoundDataPtr SXRenderer::shadePlane( const Imath::V2i &resolution ) const
{
	return m_implementation->shadePlane( resolution );
}

IECoreImage::ImagePrimitivePtr SXRenderer::shadePlaneToImage( const Imath::V2i &resolution ) const
{
	return m_implementation->shadePlaneToImage( resolution );
}
