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

#include "IECoreRI/private/SXRendererImplementation.h"
#include "IECoreRI/SXExecutor.h"
#include "IECoreRI/Convert.h"

#include "IECore/MessageHandler.h"
#include "IECore/Shader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/SplineData.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/Transform.h"
#include "IECore/Group.h"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/format.hpp"

#include "tbb/tbb_thread.h"

#include <iostream>

#include "ri.h"
#include "rx.h"

using namespace IECore;
using namespace IECoreRI;
using namespace Imath;
using namespace std;
using namespace boost;

////////////////////////////////////////////////////////////////////////
// IECoreRI::SXRendererImplementation::State implementation
////////////////////////////////////////////////////////////////////////

SXRendererImplementation::State::State()
	:	attributes( new CompoundData() ),
		context( SxContextPtr( SxCreateContext(), SxDestroyContext ) ),
		displacementShader( 0 ), surfaceShader( 0 ),
		atmosphereShader( 0 ), imagerShader( 0 )
	
{
}

SXRendererImplementation::State::State( const State &other, bool deepCopy )
	:	attributes( deepCopy ? other.attributes->copy() : other.attributes ),
		context( deepCopy ? SxContextPtr( SxCreateContext( other.context.get() ), SxDestroyContext ) : other.context ),
		displacementShader( other.displacementShader ),
		surfaceShader( other.surfaceShader ),
		atmosphereShader( other.atmosphereShader ),
		imagerShader( other.imagerShader ),
		coshaders( other.coshaders ),
		lights( other.lights )
{
}

SXRendererImplementation::State::~State()
{
}
			
////////////////////////////////////////////////////////////////////////
// IECoreRI::SXRendererImplementation implementation
////////////////////////////////////////////////////////////////////////

IECoreRI::SXRendererImplementation::SXRendererImplementation( IECoreRI::SXRenderer *parent )
	:	m_parent( parent ), m_inWorld( false )
{
	m_stateStack.push( State() );
	setAttribute( "color", new IECore::Color3fData( Color3f( 1 ) ) );
	setAttribute( "opacity", new IECore::Color3fData( Color3f( 1 ) ) );
		
	const char *shaderSearchPath = getenv( "DL_SHADERS_PATH" );
	if( shaderSearchPath )
	{
		SxSetOption( m_stateStack.top().context.get(), "searchpath:shader", SxString, (SxData)&shaderSearchPath );
	}
	
	const char *textureSearchPath = getenv( "DL_TEXTURES_PATH" );
	if( textureSearchPath )
	{
		SxSetOption( m_stateStack.top().context.get(), "searchpath:texture", SxString, (SxData)&textureSearchPath );
	}
	
	// we don't know how many threads the client will use this class on, but we have to tell
	// 3delight how many there will be or it crashes. this should be a reasonable number for most
	// use cases, and people will just have to set it themselves if they want to do something
	// out of the ordinary.
	int nThreads = tbb::tbb_thread::hardware_concurrency();
	SxSetOption( m_stateStack.top().context.get(), "render:nthreads", SxInt, &nThreads );
}

IECoreRI::SXRendererImplementation::~SXRendererImplementation()
{
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	if( name.compare( 0, 3, "ri:" )==0 || name.compare( 0, 3, "sx:" )==0 )
	{
		switch( value->typeId() )
		{
			case IntDataTypeId :
				SxSetOption( m_stateStack.top().context.get(), name.c_str() + 3, SxInt, (void *)&(static_cast<const IntData *>( value.get() )->readable() ) );
				break;
			case FloatDataTypeId :
				SxSetOption( m_stateStack.top().context.get(), name.c_str() + 3, SxFloat, (void *)&(static_cast<const FloatData *>( value.get() )->readable() ) );
				break;
			case StringDataTypeId :
				{
					const char *s = static_cast<const StringData *>( value.get() )->readable().c_str();
					SxSetOption( m_stateStack.top().context.get(), name.c_str() + 3, SxString, &s );
					break;	
				}
			default :
				msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setOption", format( "Unsupport type \"%s\"." ) % value->typeName() );
		}
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		switch( value->typeId() )
		{
			case IntDataTypeId :
				SxSetOption( m_stateStack.top().context.get(), name.c_str(), SxInt, (void *)&(static_cast<const IntData *>( value.get() )->readable() ) );
				break;
			case FloatDataTypeId :
				SxSetOption( m_stateStack.top().context.get(), name.c_str(), SxFloat, (void *)&(static_cast<const FloatData *>( value.get() )->readable() ) );
				break;
			case StringDataTypeId :
				{
					const char *s = static_cast<const StringData *>( value.get() )->readable().c_str();
					SxSetOption( m_stateStack.top().context.get(), name.c_str(), SxString, &s );
					break;	
				}
			default :
				msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setOption", format( "Unsupport type \"%s\"." ) % value->typeName() );
		}
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// ignore options prefixed for some other renderer
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setOption", format( "Unknown option \"%s\"." ) % name );
	}
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
	if( m_inWorld )
	{
		msg( Msg::Warning, "IECoreRI::SXRendererImplementation::worldBegin", "Already in a world block" );
		return;
	}
	m_stateStack.push( State( m_stateStack.top(), true ) );
	m_inWorld = true;
}

void IECoreRI::SXRendererImplementation::worldEnd()
{
	if( !m_inWorld )
	{
		msg( Msg::Warning, "IECoreRI::SXRendererImplementation::worldEnd", "No matching worldBegin" );
		return;
	}
	m_stateStack.pop();
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::transformBegin()
{
	// New push state onto the stack: deep copy flag is false, so we don't create a new SxContext, which will swallow up any
	// coordinate systems declared before transformEnd():
	m_stateStack.push( State( m_stateStack.top(), false ) );
}

void IECoreRI::SXRendererImplementation::transformEnd()
{
	unsigned minimumStack = m_inWorld ? 2 : 1;
	if( m_stateStack.size() <= minimumStack )
	{
		IECore::msg( IECore::Msg::Error, "IECoreRI::SXRenderer::transformEnd", "No matching transformBegin." );
		return;
	}
	m_stateStack.pop();
}

void IECoreRI::SXRendererImplementation::setTransform( const Imath::M44f &m )
{
	m_stateStack.top().transform = m;
}

void IECoreRI::SXRendererImplementation::setTransform( const std::string &coordinateSystem )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::setTransform", "Not implemented" );
}

Imath::M44f IECoreRI::SXRendererImplementation::getTransform() const
{
	return m_stateStack.top().transform;
}

Imath::M44f IECoreRI::SXRendererImplementation::getTransform( const std::string &coordinateSystem ) const
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::getTransform", "Not implemented" );
	return M44f();
}

void IECoreRI::SXRendererImplementation::concatTransform( const Imath::M44f &m )
{
	m_stateStack.top().transform = m * m_stateStack.top().transform;
}

void IECoreRI::SXRendererImplementation::coordinateSystem( const std::string &name )
{
	M44f m = m_stateStack.top().transform.transposed();
	RtMatrix mm;
	convert( m, mm );
	SxDefineSpace ( m_stateStack.top().context.get(), name.c_str(), (RtFloat*)&mm[0][0] );
}

//////////////////////////////////////////////////////////////////////////////////////////
// attribute code
//////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::attributeBegin()
{
	m_stateStack.push( State( m_stateStack.top(), true ) );
}

void IECoreRI::SXRendererImplementation::attributeEnd()
{
	unsigned minimumStack = m_inWorld ? 2 : 1;
	if( m_stateStack.size() <= minimumStack )
	{
		IECore::msg( IECore::Msg::Error, "IECoreRI::SXRenderer::attributeEnd", "No matching attributeBegin." );
		return;
	}
	m_stateStack.pop();
}

/// \todo Actually pass the attributes through to the SxContext if we can get that working. Then perhaps store
/// an SxContext per State entry on the stack - this requires fixes on the 3delight side though.
void IECoreRI::SXRendererImplementation::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	m_stateStack.top().attributes->writable()[name] = value->copy();
}

IECore::ConstDataPtr IECoreRI::SXRendererImplementation::getAttribute( const std::string &name ) const
{
	return m_stateStack.top().attributes->member<Data>( name, false );
}

void IECoreRI::SXRendererImplementation::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{	
	
	if( type=="displacement" || type=="ri:displacement" )
	{
		m_stateStack.top().displacementShader = createShader( name.c_str(), 0, parameters );
	}
	else if( type=="surface" || type=="ri:surface" )
	{
		m_stateStack.top().surfaceShader = createShader( name.c_str(), 0, parameters );
	}
	else if( type=="atmosphere"	|| type=="ri:atmosphere" )
	{
		m_stateStack.top().atmosphereShader = createShader( name.c_str(), 0, parameters );
	}
	else if( type=="imager"	|| type=="ri:imager" )
	{
		m_stateStack.top().imagerShader = createShader( name.c_str(), 0, parameters );
	}
	else if( type=="shader" || type=="ri:shader" )
	{
		const StringData *handleData = 0;
		CompoundDataMap::const_iterator it = parameters.find( "__handle" );
		if( it!=parameters.end() )
		{
			handleData = runTimeCast<const StringData>( it->second );
		}
		if( !handleData )
		{
			msg( Msg::Error, "IECoreRI::SXRendererImplementation::shader", "Must specify StringData \"__handle\" parameter for coshaders." );
		}
		else
		{
			SxShader s = createShader( name.c_str(), handleData->readable().c_str(), parameters );
			if( s )
			{
				m_stateStack.top().coshaders.push_back( s );
			}
		}
	}
	else
	{
		msg( Msg::Error, "IECoreRI::SXRendererImplementation::shader", boost::format( "Unsupported shader type \"%s\"" ) % type );
	}
}

void IECoreRI::SXRendererImplementation::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	SxShader s = createShader( name.c_str(), 0, parameters );
	m_stateStack.top().lights.push_back( s );
}

void IECoreRI::SXRendererImplementation::illuminate( const std::string &lightHandle, bool on )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::illuminate", "Not implemented" );
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
// rerendering
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::SXRendererImplementation::editBegin( const std::string &editType, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::editBegin", "Not implemented" );
}

void IECoreRI::SXRendererImplementation::editEnd()
{
	msg( Msg::Warning, "IECoreRI::SXRendererImplementation::editEnd", "Not implemented" );
}
		
/////////////////////////////////////////////////////////////////////////////////////////
// shading
/////////////////////////////////////////////////////////////////////////////////////////

SxShader IECoreRI::SXRendererImplementation::createShader( const char *name, const char *handle, const IECore::CompoundDataMap &parameters ) const
{
	// create a shader which we'll use just for getting information from. we have to do this
	// in a temporary context created just for the purpose, so that we don't end up making two shaders
	// in the context we actually care about.
	
	boost::shared_ptr<void> tmpContext( SxCreateContext( m_stateStack.top().context.get() ), SxDestroyContext );
	
	SxShader shaderInfo = SxCreateShader( tmpContext.get(), 0, name, 0 );
	if( !shaderInfo )
	{
		// 3delight will have printed a warning already.
		return 0;
	}
	
	// convert the parameter list for the shader
		
	SxParameterList parameterList = SxCreateParameterList( m_stateStack.top().context.get(), 1, "shader" );
	for( IECore::CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		if( it->first=="__handle" )
		{
			// skip the special handle parameter intended for use as the coshader handle
			continue;
		}
		switch( it->second->typeId() )
		{
			case FloatDataTypeId :
				SxSetParameter( parameterList, it->first.value().c_str(), SxFloat, (void *)&(static_cast<const FloatData *>( it->second.get() )->readable() ) );
				break;
			case IntDataTypeId :
			{
				float value = static_cast<const IntData *>( it->second.get() )->readable();
				SxSetParameter( parameterList, it->first.value().c_str(), SxFloat, &value );
				break;	
			}
			case BoolDataTypeId :
			{
				float value = static_cast<const BoolData *>( it->second.get() )->readable() ? 1.0f : 0.0f;
				SxSetParameter( parameterList, it->first.value().c_str(), SxFloat, &value );
				break;		
			}
			case V3fDataTypeId :
			{
				unsigned numParameters = SxGetNumParameters( shaderInfo );
				SxType type = SxInvalid;
				unsigned arraySize;
				for( unsigned i=0; i<numParameters; i++ )
				{
					bool varying;
					SxData defaultValue;
					const char *name = SxGetParameterInfo( shaderInfo, i, &type, &varying, &defaultValue, &arraySize );
					if( 0==strcmp( name, it->first.value().c_str() ) )
					{
						break;
					}
					else
					{
						type = SxInvalid;
					}
				}
				if( type==SxPoint || type==SxVector || type==SxNormal )
				{
					SxSetParameter( parameterList, it->first.value().c_str(), type, (void *)&(static_cast<const V3fData *>( it->second.get() )->readable() ) );
				}
				else if( type==SxFloat && arraySize==3 )
				{
					SxSetParameter( parameterList, it->first.value().c_str(), type, (void *)&(static_cast<const V3fData *>( it->second.get() )->readable() ), false, arraySize );
				}
				else
				{
					msg( Msg::Warning, "IECoreRI::SXRendererImplementation::createShader", boost::format( "Parameter \"%s\" is not a point, vector, normal or float[3] and will be ignored" ) % it->second->typeName() );
				}
				break;
			}
			case Color3fDataTypeId :
				SxSetParameter( parameterList, it->first.value().c_str(), SxColor, (void *)&(static_cast<const Color3fData *>( it->second.get() )->readable() ) );
				break;
			case M33fDataTypeId :
				SxSetParameter( parameterList, it->first.value().c_str(), SxMatrix, (void *)&(static_cast<const M33fData *>( it->second.get() )->readable() ) );
				break;
			case StringDataTypeId :
			{
				const char *s = static_cast<const StringData *>( it->second.get() )->readable().c_str();
				SxSetParameter( parameterList, it->first.value().c_str(), SxString, &s );
				break;
			}
			case StringVectorDataTypeId :
			{
				const std::vector<std::string> &strings = static_cast<const StringVectorData *>( it->second.get() )->readable();
				std::vector<const char *> charPtrs; charPtrs.resize( strings.size() );
				for( unsigned i=0; i<strings.size(); i++ )
				{
					charPtrs[i] = strings[i].c_str();
				}
				SxSetParameter( parameterList, it->first.value().c_str(), SxString, &(charPtrs[0]), false, strings.size() );
				break;
			}
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
					msg( Msg::Warning, "IECoreRI::SXRendererImplementation::createShader", boost::format( "SplinefColor3f parameter \"%s\" has no points and will be ignored" ) % it->second->typeName() );
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
					msg( Msg::Warning, "IECoreRI::SXRendererImplementation::createShader", boost::format( "SplinefColor3f parameter \"%s\" has no points and will be ignored" ) % it->second->typeName() );
				}
				break;	
			}
			default :
				msg( Msg::Warning, "IECoreRI::SXRendererImplementation::createShader", boost::format( "Unsupported parameter type \"%s\"" ) % it->second->typeName() );
		}
	}
		
	return SxCreateShader( m_stateStack.top().context.get(), parameterList, name, handle );
}

IECore::CompoundDataPtr IECoreRI::SXRendererImplementation::shade( const IECore::CompoundData *points ) const
{
	Imath::V2i gridSize( 0 );
	return shade( points, gridSize );
}

IECore::CompoundDataPtr IECoreRI::SXRendererImplementation::shade( const IECore::CompoundData *points, const Imath::V2i &gridSize ) const
{
	SXExecutor::ShaderVector shaders;
	const State &state = m_stateStack.top();
	if( state.displacementShader )
	{
		shaders.push_back( state.displacementShader );
	}
	if( state.surfaceShader )
	{
		shaders.push_back( state.surfaceShader );
	}
	if( state.atmosphereShader )
	{
		shaders.push_back( state.atmosphereShader );
	}
	if( state.imagerShader )
	{
		shaders.push_back( state.imagerShader );
	}
	
	if( !shaders.size() )
	{
		throw Exception( "No shaders specified" );
	}
	
	SXExecutor executor( shaders, m_stateStack.top().context.get(), m_stateStack.top().coshaders, m_stateStack.top().lights );
	return executor.execute( points, gridSize );
}

IECore::CompoundDataPtr IECoreRI::SXRendererImplementation::shadePlane( const V2i &resolution ) const
{
	IECore::CompoundDataPtr points = new IECore::CompoundData();
	
	V3fVectorDataPtr pData = new IECore::V3fVectorData();
	V3fVectorDataPtr nData = new IECore::V3fVectorData();
	FloatVectorDataPtr sData = new IECore::FloatVectorData();
	FloatVectorDataPtr tData = new IECore::FloatVectorData();

	std::vector<V3f> &p = pData->writable();
	std::vector<V3f> &n = nData->writable();
	std::vector<float> &s = sData->writable();
	std::vector<float> &t = tData->writable();
	
	unsigned numPoints = resolution[0] * resolution[1];
	
	p.resize( numPoints );
	n.resize( numPoints );
	s.resize( numPoints );
	t.resize( numPoints );
	
	unsigned xResMinus1 = resolution[0] - 1;
	unsigned yResMinus1 = resolution[1] - 1;
	
	unsigned i = 0;
	for( int y = 0; y < resolution[1]; y++ )
	{
		for( int x = 0; x < resolution[0]; x++ )
		{
			p[i] = V3f( float(x) / xResMinus1 , float(y) / yResMinus1, 0.0 );	
			s[i] = p[i][0];
			t[i] = p[i][1];
			n[i] = V3f( 0.0f, 0.0f, 1.0f );
			i++;
		}
	}	
	
	points->writable()[ "P" ] = pData;
	points->writable()[ "N" ] = nData;
	points->writable()[ "s" ] = sData;
	points->writable()[ "t" ] = tData;
	
	return shade( points, resolution );
}

IECore::ImagePrimitivePtr IECoreRI::SXRendererImplementation::shadePlaneToImage( const V2i &resolution ) const
{
	IECore::CompoundDataPtr result = shadePlane( resolution );
	
	Box2i window =  Box2i( V2i( 0, 0 ), V2i( resolution[0] - 1, resolution[1] - 1 ) );
	
	IECore::ImagePrimitivePtr img = new IECore::ImagePrimitive( window, window );
	IECore::FloatVectorDataPtr rData = img->createChannel<float>( "R" );
	IECore::FloatVectorDataPtr gData = img->createChannel<float>( "G" );
	IECore::FloatVectorDataPtr bData = img->createChannel<float>( "B" );
	IECore::FloatVectorDataPtr aData = img->createChannel<float>( "A" );

	std::vector<float> &r = rData->writable();
	std::vector<float> &g = gData->writable();
	std::vector<float> &b = bData->writable();
	std::vector<float> &a = aData->writable();

	unsigned numPoints = resolution[0] * resolution[1];

	r.resize( numPoints );
	g.resize( numPoints );
	b.resize( numPoints );
	a.resize( numPoints );
	
	IECore::Color3fVectorDataPtr cData = result->member<Color3fVectorData>( "Ci", false );
	IECore::Color3fVectorDataPtr oData = result->member<Color3fVectorData>( "Oi", false );
	if( !cData || !oData )
	{
		throw( Exception( "The renderer didn't return Ci/Oi when shading the points." ) );
	}
	
	const std::vector<Color3f> &c = cData->readable();
	const std::vector<Color3f> &o = oData->readable();

	if( c.size() != numPoints )
	{
		throw( Exception( boost::str( 
			boost::format( "The renderer didn't return the right number of shaded points. (%d but should be %d)." )
		 	% c.size() % numPoints
		) ) );
	}

	for( std::vector<V3f>::size_type i=0; i<c.size(); i++ )
	{
		r[i] = c[i][0];
		g[i] = c[i][1];
		b[i] = c[i][2];
		a[i] = ( o[i][0] + o[i][1] + o[i][2] ) / 3.0f;
	}
	
	return img;
}


