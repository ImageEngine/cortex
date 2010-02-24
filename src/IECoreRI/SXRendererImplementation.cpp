//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/private/SXRendererImplementation.h"
#include "IECoreRI/SXExecutor.h"

#include "IECore/MessageHandler.h"
#include "IECore/Shader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/SplineData.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/Transform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Group.h"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/format.hpp"

#include <iostream>

#include "ri.h"
#include "rx.h"

using namespace IECore;
using namespace IECoreRI;
using namespace Imath;
using namespace std;
using namespace boost;

////////////////////////////////////////////////////////////////////////
// IECoreRI::SXRendererImplementation implementation
////////////////////////////////////////////////////////////////////////

//const unsigned int IECoreRI::SXRendererImplementation::g_shaderCacheSize = 10 * 1024 * 1024;

IECoreRI::SXRendererImplementation::SXRendererImplementation( IECoreRI::SXRenderer *parent )
	:	m_parent( parent ), m_context( SxCreateContext() )
{
	const char *shaderSearchPath = getenv( "DL_SHADERS_PATH" );
	if( shaderSearchPath )
	{
		SxSetOption( m_context, "searchpath:shader", SxString, (SxData)&shaderSearchPath );
	}
	
}

IECoreRI::SXRendererImplementation::~SXRendererImplementation()
{
	SxDestroyContext( m_context );
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setOption", "Not implemented" );
}

IECore::ConstDataPtr IECoreRI::SXRendererImplementation::getOption( const std::string &name ) const
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::getOption", "Not implemented" );
	return 0;
}

void IECoreRI::SXRendererImplementation::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::camera", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::display", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// world
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::worldBegin()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::worldBegin", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::worldEnd()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::worldEnd", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::transformBegin()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::transformBegin", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::transformEnd()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::transformEnd", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::setTransform( const Imath::M44f &m )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setTransform", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::setTransform( const std::string &coordinateSystem )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setTransform", "Not implemented" );
}

Imath::M44f IECoreRI::SXRendererImplementation::getTransform() const
{
	return getTransform( "object" );
}

Imath::M44f IECoreRI::SXRendererImplementation::getTransform( const std::string &coordinateSystem ) const
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::getTransform", "Not implemented" );
	return M44f();
}

void IECoreRI::SXRendererImplementation::concatTransform( const Imath::M44f &m )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::concatTransform", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::coordinateSystem( const std::string &name )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::coordinateSystem", "Not implemented" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// attribute code
//////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::attributeBegin()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::attributeBegin", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::attributeEnd()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::attributeEnd", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setAttribute", "Not implemented" );
}

IECore::ConstDataPtr IECoreRI::SXRendererImplementation::getAttribute( const std::string &name ) const
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::getAttribute", "Not implemented" );
	return 0;
}

void IECoreRI::SXRendererImplementation::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	SxShaderType sxType = SxUnknown;
	if( type=="surface"	|| type=="ri:surface" )
	{
		sxType = SxSurface;
	}
	if( sxType==SxUnknown )
	{
		msg( Msg::Error, "IECoreRI::SXRendererImplementation::shader", boost::format( "Unsupported shader type \"%s\"" ) % type );
	}
	
	SxParameterList parameterList = SxCreateShaderParameterList( m_context );
	for( IECore::CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		switch( it->second->typeId() )
		{
			case FloatDataTypeId :
				SxSetParameter( parameterList, it->first.value().c_str(), SxFloat, (void *)&(static_cast<const FloatData *>( it->second.get() )->readable() ) );
				break;
			case Color3fDataTypeId :
				SxSetParameter( parameterList, it->first.value().c_str(), SxColor, (void *)&(static_cast<const Color3fData *>( it->second.get() )->readable() ) );
				break;
			case SplineffDataTypeId :
			{
				const IECore::Splineff &spline = static_cast<const SplineffData *>( it->second.get() )->readable();
				size_t size = spline.points.size();
				if( size )
				{
					vector<float> positions( size );
					vector<float> values( size );
					size_t i = 0;
					for( IECore::Splineff::PointContainer::const_iterator sIt=spline.points.begin(); sIt!=spline.points.end(); sIt++ )
					{
						positions[i] = sIt->first;
						values[i] = sIt->second;
						i++;
					}
					string positionsName = it->first.value() + "Positions";
					string valuesName = it->first.value() + "Values";
					SxSetParameter( parameterList, positionsName.c_str(), SxFloat, &(positions[0]), false, size );
					SxSetParameter( parameterList, valuesName.c_str(), SxFloat, &(values[0]), false, size );
				}
				else
				{
					msg( Msg::Warning, "IECoreRI::SXRendererImplementation::shader", boost::format( "SplinefColor3f parameter \"%s\" has no points and will be ignored" ) % it->second->typeName() );
				}
				break;	
			}
			case SplinefColor3fDataTypeId :
			{
				const IECore::SplinefColor3f &spline = static_cast<const SplinefColor3fData *>( it->second.get() )->readable();
				size_t size = spline.points.size();
				if( size )
				{
					vector<float> positions( size );
					vector<Color3f> values( size );
					size_t i = 0;
					for( IECore::SplinefColor3f::PointContainer::const_iterator sIt=spline.points.begin(); sIt!=spline.points.end(); sIt++ )
					{
						positions[i] = sIt->first;
						values[i] = sIt->second;
						i++;
					}
					string positionsName = it->first.value() + "Positions";
					string valuesName = it->first.value() + "Values";
					SxSetParameter( parameterList, positionsName.c_str(), SxFloat, &(positions[0]), false, size );
					SxSetParameter( parameterList, valuesName.c_str(), SxColor, &(values[0]), false, size );
				}
				else
				{
					msg( Msg::Warning, "IECoreRI::SXRendererImplementation::shader", boost::format( "SplinefColor3f parameter \"%s\" has no points and will be ignored" ) % it->second->typeName() );
				}
				break;	
			}
			default :
				msg( Msg::Warning, "IECoreRI::SXRendererImplementation::shader", boost::format( "Unsupported parameter type \"%s\"" ) % it->second->typeName() );
		}
	}
	
	m_shader = SxCreateShader( m_context, parameterList, name.c_str(), sxType );
	m_shaderInfo = SxCreateShaderInfo( m_context, name.c_str() );
	
}

void IECoreRI::SXRendererImplementation::light( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::light", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// motion blur
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::motionBegin( const std::set<float> &times )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::motionBegin", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::motionEnd()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::motionEnd", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::points", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::disk", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::curves( const IECore::CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::curves", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::text( const std::string &font, const std::string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::text", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::sphere", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::image", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::mesh", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::nurbs", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::patchMesh", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::geometry", format( "Unsupported geometry type \"%s\"." ) % type );
}

void IECoreRI::SXRendererImplementation::procedural( IECore::Renderer::ProceduralPtr proc )
{
	proc->render( m_parent );
}

/////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::instanceBegin", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::instanceEnd()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::instanceEnd", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::instance( const std::string &name )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::instance", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////

IECore::DataPtr IECoreRI::SXRendererImplementation::command( const std::string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::command", boost::format( "Unknown command \"%s\"" ) % name );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// shading
/////////////////////////////////////////////////////////////////////////////////////////

IECore::CompoundDataPtr IECoreRI::SXRendererImplementation::shade( const IECore::CompoundData *points ) const
{
	SXExecutor executor( m_shader, m_shaderInfo );
	return executor.execute( points );
}
