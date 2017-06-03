//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Electric Theatre Collective Limited. All rights reserved.
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

#include "OpenEXR/ImathBoxAlgo.h"

#include "IECore/MessageHandler.h"
#include "IECore/Transform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/AttributeState.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/Writer.h"

#include "IECoreMantra/private/RendererImplementation.h"

#include "boost/format.hpp"
#include <numeric>

using namespace IECore;
using namespace IECoreMantra;
using namespace Imath;
using namespace std;
using namespace boost;

IECoreMantra::RendererImplementation::AttributeState::AttributeState()
{
	attributes = new CompoundData();
}

IECoreMantra::RendererImplementation::AttributeState::AttributeState( const AttributeState &other )
{
	attributes = other.attributes->copy();
}

///////////////////////////////////////////////////////////////////////////////////////////

IECore::ConstDataPtr IECoreMantra::RendererImplementation::getShutterOption( const std::string &name ) const
{
#if UT_MAJOR_VERSION_INT >= 16
	fpreal64 shutter[2];
#else
	fpreal shutter[2];
#endif
	if ( m_vrayproc->import("camera:shutter", shutter, 2) )
	{
		return new V2fData( V2f(shutter[0], shutter[1]) );
	}
	return 0;
}

IECore::ConstDataPtr IECoreMantra::RendererImplementation::getResolutionOption( const std::string &name ) const
{
#if UT_MAJOR_VERSION_INT >= 16
	fpreal64 res[2];
#else
	fpreal res[2];
#endif
	if ( m_vrayproc->import("image:resolution", res, 2) )
	{
		return new V2fData( V2f(res[0], res[1]) );
	}
	return 0;
}

IECore::ConstDataPtr IECoreMantra::RendererImplementation::getVelocityBlurAttribute( const std::string &name ) const
{
#if UT_MAJOR_VERSION_INT >= 16
	fpreal64 v;
#else
	int v;
#endif
	if ( m_vrayproc->import("object:velocityblur", &v, 1) )
	{
#if UT_MAJOR_VERSION_INT >= 16
		return new FloatData( v );
#else
		return new IntData( v );
#endif
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////

IECoreMantra::RendererImplementation::RendererImplementation()
{
	constructCommon( Render );
	
	if ( !(m_fpipe = popen("mantra", "w")) )
	{  
		msg( Msg::Error, "IECoreMantra::RendererImplementation:RendererImplementation", 
			 "Failed to open mantra program" );
	}
	
	fputs("# IFD created by IECoreMantra\n", m_fpipe);
	fputs("ray_time 0\n", m_fpipe);
	
	fflush( m_fpipe );
}

IECoreMantra::RendererImplementation::RendererImplementation( const std::string &ifdFileName )
{
	m_ifdFileName = ifdFileName;
	constructCommon( IfdGen );
	if ( !( m_fpipe = fopen( m_ifdFileName.c_str(), "w" )) )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation:RendererImplementation",
			"Failed to open ifd file" );
	}
	fputs("# IFD created by IECoreMantra\n", m_fpipe);
	fputs("ray_time 0\n", m_fpipe);
	fflush( m_fpipe );
}

IECoreMantra::RendererImplementation::RendererImplementation( ProceduralPrimitive *procedural )
	: m_vrayproc( (ProceduralPrimitive *)procedural )
{
	constructCommon( Procedural );
	m_vrayproc->m_renderer = this; 
	m_preWorld = false;
}

IECoreMantra::RendererImplementation::RendererImplementation(RendererImplementationPtr parent)
	: m_vrayproc( new ProceduralPrimitive() )
{
	constructCommon( Procedural );
	if ( parent ) 
	{
		m_transformStack.push( parent->m_transformStack.top() );
		m_attributeStack.push( AttributeState( parent->m_attributeStack.top() ) );
	}
	m_preWorld = false;
}

void IECoreMantra::RendererImplementation::constructCommon( Mode mode )
{
	m_mode = mode;
	m_preWorld = true;
	
	m_world = new Group;
	
	m_camera = new Camera;
	m_camera->addStandardParameters();
	
	m_camera->parameters()["screenWindow"] = new Box2fData( Box2f(V2f(0,0), V2f(1,1)) ); 

	m_camera->setTransform( new MatrixTransform() );

	m_transformStack.push( M44f() );
	m_attributeStack.push( AttributeState() );
	
	m_inMotion = false;
	m_motionType = Unknown;
	m_motionSize = 0;
	
	m_getOptionHandlers["shutter"] = &IECoreMantra::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:shutter"] = &IECoreMantra::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:resolution"] = &IECoreMantra::RendererImplementation::getResolutionOption;
	m_getAttributeHandlers["vm:velocityblur"] = &IECoreMantra::RendererImplementation::getVelocityBlurAttribute;
}

IECoreMantra::RendererImplementation::~RendererImplementation()
{
	if ( m_fpipe )
	{
		if ( m_mode == Render )
		{
			if ( pclose(m_fpipe) == -1 )
			{
				msg( Msg::Error, "IECoreMantra::RendererImplementation::~RendererImplementation()", "pclose error" );
				m_fpipe = NULL;
			}
		}
		else if ( m_mode == IfdGen )
		{
			if ( fclose(m_fpipe) != 0 )
			{
				msg( Msg::Error, "IECoreMantra::RendererImplementation::~RendererImplementation()", "fclose error" );
				m_fpipe = NULL;
			}
		}
	}
}

void IECoreMantra::RendererImplementation::ifdString( IECore::ConstDataPtr value, std::string &ifd, std::string &type )
{
	switch( value->typeId() )
	{
		case FloatDataTypeId:
			{
				ConstFloatDataPtr f = runTimeCast<const FloatData>( value );
				ifd = boost::str( boost::format( "%g") % f->readable() );
				type = "float";
			}
			break;
		case BoolDataTypeId:
			{
				ConstBoolDataPtr b = runTimeCast<const BoolData>( value );
				ifd = boost::str( boost::format( "%i") % b->readable() );
				type = "bool";
			}
			break;
		case IntDataTypeId:
			{
				ConstIntDataPtr i = runTimeCast<const IntData>( value );
				ifd =  boost::str( boost::format( "%i") % i->readable() );
				type = "int";
			}
			break;
		case V2fDataTypeId:
			{
				ConstV2fDataPtr v = runTimeCast<const V2fData>( value );
				ifd = boost::str( boost::format( "%g %g") % v->readable().x % v->readable().y );
				type = "vector2";
			}
			break;
		case V3fDataTypeId:
			{
				ConstV3fDataPtr v = runTimeCast<const V3fData>( value );
				ifd = boost::str( boost::format( "%g %g %g") % v->readable().x % v->readable().y % v->readable().z );
				type = "vector3";
			}
			break;
		case Color3fDataTypeId:
			{
				ConstColor3fDataPtr v = runTimeCast<const Color3fData>( value );
				ifd = boost::str( boost::format( "%g %g %g") % v->readable().x % v->readable().y % v->readable().z );
				type = "color3";
			}
			break;
		case M33fDataTypeId:
			{
				ConstM33fDataPtr m = runTimeCast<const M33fData>( value );
				ifd = boost::str( boost::format( "%g %g %g %g %g %g %g %g %g") %
									m->readable()[0][0] % m->readable()[0][1] % m->readable()[0][2] %
									m->readable()[1][0] % m->readable()[1][1] % m->readable()[1][2] %  
									m->readable()[2][0] % m->readable()[2][1] % m->readable()[2][2] );
				type = "matrix3";
			}
			break;
		case M44fDataTypeId:
			{
				ConstM44fDataPtr m = runTimeCast<const M44fData>( value );
				ifd = boost::str( boost::format( "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g")  %
								m->readable()[0][0] % m->readable()[0][1] % m->readable()[0][2] % m->readable()[0][3] % 
								m->readable()[1][0] % m->readable()[1][1] % m->readable()[1][2] % m->readable()[1][3] % 
								m->readable()[2][0] % m->readable()[2][1] % m->readable()[2][2] % m->readable()[2][3] % 
								m->readable()[3][0] % m->readable()[3][1] % m->readable()[3][2] % m->readable()[3][3] );
				type = "matrix4";
			}
			break;
		case StringDataTypeId:
			{
				ConstStringDataPtr s = runTimeCast<const StringData>( value );
				ifd = boost::str( boost::format( "\"%s\"") % s->readable() );
				type = "string";
			}
			break;
		default:
			ifd = "0";
			type = "int";
	}	
}

void IECoreMantra::RendererImplementation::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	if ( m_mode != Procedural )
	{
		if ( !m_preWorld )
		{
			msg( Msg::Warning, "IECoreMantra::RendererImplementation::camera", "Method invalid after worldBegin()" );
			return;
		}
		
		if ( !m_fpipe )
		{
			msg( Msg::Error, "IECoreMantra::RendererImplementation::setOption", "Broken pipe" );
			return;
		}
		
		std::string ifd, type;
		ifdString( value, ifd, type );
		fprintf( m_fpipe, "ray_declare global %s %s %s\n", type.c_str(), name.c_str(), ifd.c_str() );

	}
	else
	{
		SetOptionHandlerMap::iterator it = m_setOptionHandlers.find( name );
		if ( it!=m_setOptionHandlers.end() )
		{
			(this->*(it->second))( name, value );
			return;
		}
	}
}

IECore::ConstDataPtr IECoreMantra::RendererImplementation::getOption( const std::string &name ) const
{
	if ( m_mode == Procedural )
	{
		GetOptionHandlerMap::const_iterator it = m_getOptionHandlers.find( name );
		if ( it!=m_getOptionHandlers.end() )
		{
			msg (Msg::Debug, "IECoreMantra::RendererImplementation::getOption", boost::format("found: %s") % name);
			return (this->*(it->second))( name );
		}
	}
	msg ( Msg::Warning, "IECoreMantra::RendererImplementation::getOption", "Not Implemented" );
	return 0;
}

void IECoreMantra::RendererImplementation::outputCamera( IECore::CameraPtr camera )
{
	if ( m_mode == Procedural )
	{
		return;
	}
	
	if ( !m_fpipe )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::outputCamera", "Broken pipe" );
		return;
	}
	
	CompoundDataMap::const_iterator it = camera->parameters().find( "resolution" );
	ConstV2iDataPtr resolution = runTimeCast<const V2iData>( it->second );
	fprintf( m_fpipe, "ray_property image resolution %i %i\n", 
			 resolution->readable().x, resolution->readable().y );
	
	it = camera->parameters().find( "projection" );
	ConstStringDataPtr projection = runTimeCast<const StringData>( it->second );
	fprintf( m_fpipe, "ray_property camera projection  \"%s\"\n", 
			 (char *)projection->readable().c_str() ); 

	it = camera->parameters().find( "clippingPlanes" );
	ConstV2fDataPtr clip = runTimeCast<const V2fData>( it->second );
	fprintf( m_fpipe, "ray_property camera clip  %g %g\n", 
			 clip->readable().x, clip->readable().y ); 

	it = camera->parameters().find( "screenWindow" );
	ConstBox2fDataPtr window = runTimeCast<const Box2fData>( it->second );
	fprintf( m_fpipe, "ray_property image window  %g %g %g %g\n", 
			 window->readable().min.x, window->readable().max.x, 
			 window->readable().min.y, window->readable().max.y ); 
	
	it = camera->parameters().find( "cropWindow" );
	ConstBox2fDataPtr crop = runTimeCast<const Box2fData>( it->second );
	fprintf( m_fpipe, "ray_property image crop  %g %g %g %g\n", 
			 crop->readable().min.x, crop->readable().max.x, 
			 crop->readable().min.y, crop->readable().max.y ); 

	it = camera->parameters().find( "shutter" );									  
	ConstV2fDataPtr shutter = runTimeCast<const V2fData>( it->second );
	fprintf( m_fpipe, "ray_declare global vector2 camera:shutter %g %g\n",
			 shutter->readable().x, shutter->readable().y);
	
	M44f m = camera->getTransform()->transform();
	m.invert();
	fprintf( m_fpipe, "ray_transform %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n", 
		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]);
	
	fflush( m_fpipe );
}

void IECoreMantra::RendererImplementation::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if ( m_mode == Procedural )
	{
		return;
	}

	if ( !m_preWorld )
	{
		msg( Msg::Warning, "IECoreMantra::RendererImplementation::camera", "Method invalid after worldBegin()" );
		return;
	}
	
	if ( !m_fpipe )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::camera", "Broken pipe" );
		return;
	}

	CompoundDataPtr parameterData = (new CompoundData( parameters ))->copy();
	CameraPtr camera = new Camera( name, 0, parameterData );
	camera->addStandardParameters();
	// addStandardMantraParameters.. 
	camera->parameters()["screenWindow"] = new Box2fData( Box2f(V2f(0,0), V2f(1,1)) ); 
	
	CompoundDataMap::const_iterator transformIt=parameters.find( "transform" );
	if( transformIt!=parameters.end() )
	{
		if( M44fDataPtr m = runTimeCast<M44fData>( transformIt->second ) )										
		{
			camera->setTransform( new MatrixTransform( m->readable() ) );										 
		}																										 
		else																									  
		{
			msg( Msg::Error, "IECoreRI::RendererImplementation::camera", 
				 "\"transform\" parameter should be of type M44fData." );
		}
	}																											 
	else
	{																											 
		camera->setTransform( new MatrixTransform( getTransform() ) );											
	}
	m_camera = camera;		 	
}

void IECoreMantra::RendererImplementation::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	if ( m_mode == Procedural )
	{
		return;
	}

	if ( !m_preWorld )
	{
		msg( Msg::Warning, "IECoreMantra::RendererImplementation::camera", "Method invalid after worldBegin()" );
		return;
	}
	
	if ( !m_fpipe )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::camera", "Broken pipe" );
		return;
	}
	
	CompoundDataMap::const_iterator var_it = parameters.find( "variable" );
	CompoundDataMap::const_iterator vex_it = parameters.find( "vextype" );
	CompoundDataMap::const_iterator chn_it = parameters.find( "channel" );
	
	if ( var_it == parameters.end() || vex_it == parameters.end() || chn_it == parameters.end() )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::camera", 
			"Parameters must define 'variable', 'vextype' and 'channel' values." );
		return;
	}

	ConstStringDataPtr var = runTimeCast<const StringData>( var_it->second );
	ConstStringDataPtr vex = runTimeCast<const StringData>( vex_it->second );
	ConstStringDataPtr chn = runTimeCast<const StringData>( chn_it->second );
	
	if ( !var || !vex || !chn )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::camera", 
			"Invalid parameters." );
		return;
	}
		
	fprintf( m_fpipe, "ray_image \"%s\"\n", (char *)name.c_str() );
	
	fputs( "ray_start plane\n", m_fpipe );
	fprintf( m_fpipe, "\tray_property plane variable \"%s\"\n", (char *)var->readable().c_str() );
	fprintf( m_fpipe, "\tray_property plane vextype \"%s\"\n", (char *)vex->readable().c_str() );
	fprintf( m_fpipe, "\tray_property plane channel \"%s\"\n", (char *)chn->readable().c_str() );
	fputs( "ray_end\n", m_fpipe );
	
	fflush( m_fpipe );
}

void IECoreMantra::RendererImplementation::worldBegin()
{
	if ( m_mode == Procedural )
	{
		return;
	}
	
	if ( m_mode == Render )
	{
		// if rendering put it somewhere temporary
		m_worldFileName = boost::str( boost::format( "%s/ieworld_%d.cob" ) % P_tmpdir % getpid() );
		// mark it for removal
		fprintf( m_fpipe, "ray_declare global string ieworldremove %i\n", 1 );
	}
	else if ( m_mode == IfdGen )
	{
		// if generating an ifd for some other process to render put it next to the ifd
		m_worldFileName = m_ifdFileName + std::string(".ieworld.cob");
		// keep the cob file as the scene may be rendered multiple times
		fprintf( m_fpipe, "ray_declare global string ieworldremove %i\n", 0 );
	}

	fprintf( m_fpipe, "ray_declare global string ieworldfile %s\n", m_worldFileName.c_str() );

	outputCamera( m_camera );
	m_transformStack.top() = M44f();
	m_preWorld = false;
	
	// add world procedural
	CompoundDataMap dummyTopology;
	PrimitiveVariableMap dummyPrimVars;
	geometry( "ieworld", dummyTopology, dummyPrimVars);
}

void IECoreMantra::RendererImplementation::worldEnd()
{
	if ( m_mode == Procedural )
	{
		return;
	}
	
	if ( m_preWorld )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::worldEnd", "Invalid world block nesting" );
		return;
	}
	
	if ( !m_fpipe )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::worldEnd", "Broken pipe" );
		return;
	}
	
	// write world to disk
	try
	{
		WriterPtr writer = Writer::create( m_world, m_worldFileName.c_str() );
		writer->write();
	}
	catch( IECore::Exception e )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::worldEnd", "World cache write failed" );
	}
	
	fputs( "ray_raytrace\n\n", m_fpipe );
	fputs( "ray_quit\n", m_fpipe );
	
	fflush( m_fpipe );
	
	// close mantra
	if ( pclose(m_fpipe) == -1 )
	{	
		msg( Msg::Error, "IECoreMantra::RendererImplementation::worldEnd", "pclose error" );
	}
	m_fpipe = NULL;
}

void IECoreMantra::RendererImplementation::transformBegin()
{
	m_transformStack.push( m_transformStack.top() );
}

void IECoreMantra::RendererImplementation::transformEnd()
{
	if( m_transformStack.size() <= 1 )
	{
		msg( Msg::Warning, "IECoreMantra::RendererImplementation::transformEnd", 
			 "No matching transformBegin() call." );
		return;
	}
	m_transformStack.pop(); 
}

void IECoreMantra::RendererImplementation::setTransform( const Imath::M44f &m )
{
	if ( m_inMotion )
	{
		if ( m_mode != Procedural )
		{
			msg( Msg::Warning, "IECoreMantra::RendererImplementation::setTransform", 
				 "Motion blur not currently supported in Render or IfdGen mode" );
		}
		else
		{
			if ( m_motionType == Unknown ) // first call stores motion type
			{
				m_motionType = SetTransform;
			}
			if ( m_motionType != SetTransform ) // subsequent calls must comply
			{
				msg( Msg::Warning, "IECoreMantra::RendererImplementation:setTransform", 
					 "Render methods inside a motion block must be consistent.");
				return;
			}
			if ( m_motionTransforms.size() == m_motionTimes.size() )
			{
				msg( Msg::Warning, "IECoreMantra::RendererImplementation:setTransform",
					 "More calls were made than times specified with motionBegin()");
				return;
			}
			m_motionTransforms.push_back( m );
		}
	}
	else
	{
		m_transformStack.top() = m;
		if ( m_mode != Procedural )
		{
			m_world->setTransform( new MatrixTransform( m_transformStack.top() ) );
		}
	}
}

void IECoreMantra::RendererImplementation::setTransform( const std::string &coordinateSystem )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::setTransform", "Not implemented" );
}

Imath::M44f IECoreMantra::RendererImplementation::getTransform() const
{
	return m_transformStack.top();
}

Imath::M44f IECoreMantra::RendererImplementation::getTransform( const std::string &coordinateSystem ) const
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::getTransform", "Not implemented" );
	M44f result;
	return result;
}

void IECoreMantra::RendererImplementation::concatTransform( const Imath::M44f &m )
{
	if ( m_inMotion )
	{
		if ( m_mode != Procedural )
		{
			msg( Msg::Warning, "IECoreMantra::RendererImplementation::concatTransform", 
				 "Motion blur not currently supported in Render or IfdGen mode" );
		}
		else
		{
			if ( m_motionType == Unknown ) // first call stores motion type
			{
				m_motionType = ConcatTransform;
			}
			if ( m_motionType != ConcatTransform ) // subsequent calls must comply
			{
				msg( Msg::Warning, "IECoreMantra::RendererImplementation:concatTransform", 
					 "Render methods inside a motion block must be consistent.");
				return;
			}
			if ( m_motionTransforms.size() == m_motionTimes.size() )
			{
				msg( Msg::Warning, "IECoreMantra::RendererImplementation:concatTransform",
					 "More calls were made than times specified with motionBegin()");
				return;
			}
			m_motionTransforms.push_back( m );
		}
	}
	else
	{
		m_transformStack.top() = m * m_transformStack.top();
		if ( m_mode != Procedural )
		{
			m_world->setTransform( new MatrixTransform( m_transformStack.top() ) );
		}
	}
}

void IECoreMantra::RendererImplementation::coordinateSystem( const std::string &name )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::coordinateSystem", "Not implemented" );
}

void IECoreMantra::RendererImplementation::attributeBegin()
{
	transformBegin();
	m_attributeStack.push( AttributeState( m_attributeStack.top() ) );
}

void IECoreMantra::RendererImplementation::attributeEnd()
{
	m_attributeStack.pop();
	transformEnd();
}

void IECoreMantra::RendererImplementation::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	m_attributeStack.top().attributes->writable()[name] = value->copy();
}

IECore::ConstDataPtr IECoreMantra::RendererImplementation::getAttribute( const std::string &name ) const
{
	GetAttributeHandlerMap::const_iterator it = m_getAttributeHandlers.find( name );
	if ( it!=m_getAttributeHandlers.end() )
	{
		msg (Msg::Debug, "IECoreMantra::RendererImplementation::getAttribute", boost::format("found: %s") % name);
		return (this->*(it->second))( name );
	}
	return m_attributeStack.top().attributes->member<Data>( name );
}

void IECoreMantra::RendererImplementation::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if ( type == "surface" || type == "displacement" )
	{
		// convert parameters to a shader invocation string
		std::vector<std::string> parms;
		std::string tmptype;
		CompoundDataMap::const_iterator it;
		for ( it = parameters.begin(); it != parameters.end(); it++ )
		{
			std::string ifd;
			ifdString( it->second, ifd, tmptype );
			parms.push_back( it->first );
			parms.push_back( string(" ") );
			parms.push_back( ifd );
			parms.push_back( string(" ") );
		}
		std::string parmstring  = accumulate( parms.begin(), parms.end(), string("") );
		
		StringDataPtr shader = 0; 
		if ( parmstring.empty() ) 
		{
			shader = new StringData( name );
		}
		else
		{
			shader = new StringData( boost::str( boost::format("%s %s") % name % parmstring ) );
		}

		if( type == "surface" )
		{
			m_attributeStack.top().attributes->writable()[":surface"] = shader;
		}
		else if ( type == "displacement" )
		{
			m_attributeStack.top().attributes->writable()[":displacement"] = shader;
		}
		
		if ( m_mode != Procedural )
		{
			IECore::AttributeStatePtr state = new IECore::AttributeState( m_attributeStack.top().attributes );
			m_world->addState( state );
		}
	}
	else
	{
		msg (Msg::Warning, "IECoreMantra::RendererImplementation::shader", boost::format("type not supported: %s") % type);
	}
}

void IECoreMantra::RendererImplementation::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::light", "Not implemented" );
}

void IECoreMantra::RendererImplementation::illuminate( const std::string &lightHandle, bool on )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::text", "Not implemented" );
}

void IECoreMantra::RendererImplementation::motionBegin( const std::set<float> &times )
{
	if ( m_mode != Procedural )
	{
		msg( Msg::Warning, "IECoreMantra::RendererImplementation::motionBegin", "Method only supported in Procedural mode" );
		return;
	}
	m_motionTimes.clear();
	m_motionTimes.assign(times.begin(), times.end());
	m_motionSize = m_motionTimes.size();
	m_inMotion = true;
}

void IECoreMantra::RendererImplementation::motionEnd()
{
	m_inMotion = false;
}

void IECoreMantra::RendererImplementation::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	if ( m_mode != Procedural )
	{
		return;
	}

	PrimitiveVariableMap::const_iterator it = primVars.find( "P" );
	if ( it != primVars.end() )
	{
		PrimitiveVariableMap::const_iterator v = primVars.find( "v" );
		if ( v != primVars.end() )
		{
			m_motionType = Velocity; // not strictly correct, only when attribute object:velocityblur == 1
		}
		PointsPrimitivePtr points = new IECore::PointsPrimitive( numPoints );
		points->variables = primVars;
		VisibleRenderablePtr renderable = runTimeCast<VisibleRenderable>( points );
		m_vrayproc->addVisibleRenderable( renderable );
	}
}

void IECoreMantra::RendererImplementation::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::disk", "Not implemented" );
}

void IECoreMantra::RendererImplementation::curves( const IECore::CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::curves", "Not implemented" );
}

void IECoreMantra::RendererImplementation::text( const std::string &font, const std::string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::text", "Not implemented" );
}

void IECoreMantra::RendererImplementation::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::sphere", "Not implemented" );
}

void IECoreMantra::RendererImplementation::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::image", "Not implemented" );
}

void IECoreMantra::RendererImplementation::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{

	if ( m_inMotion )
	{
		if ( m_motionType == Unknown )
		{
			m_motionType = Geometry;
		}
		if ( m_motionType != Geometry )
		{
			msg( Msg::Warning, "IECoreMantra::RendererImplementation:mesh", 
				 "Render methods inside a motion block must be consistent.");
			return;
		}
		if ( m_motionTransforms.size() == m_motionTimes.size() )
		{
			msg( Msg::Warning, "IECoreMantra::RendererImplementation:mesh",
				 "More calls were made than times specified with motionBegin()");
			return;
		}
	}

	MeshPrimitivePtr mesh = new IECore::MeshPrimitive( vertsPerFace, vertIds, interpolation );
	mesh->variables = primVars;
	VisibleRenderablePtr renderable = runTimeCast<VisibleRenderable>( mesh );
	
	if ( m_mode != Procedural )
	{
		m_world->addChild( renderable );
	}
	else 
	{
		m_vrayproc->addVisibleRenderable( renderable );
	}
}

void IECoreMantra::RendererImplementation::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::nurbs", "Not implemented" );
}

void IECoreMantra::RendererImplementation::patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::patchMesh", "Not implemented" );
}

void IECoreMantra::RendererImplementation::geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars )
{
	if ( m_mode == Procedural )
	{
		return;
	}
	
	if ( !m_fpipe )
	{
		msg( Msg::Error, "IECoreMantra::RendererImplementation::geometry", "Broken pipe" );
		return;
	}

	if ( type == "ieworld" )
	{
		Box3f b = Box3f( V3f( limits<float>::min() ), V3f( limits<float>::max() ) );
		fputs( "ray_start object\n", m_fpipe );
		fputs( "\tray_transform 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1\n", m_fpipe );
		fprintf( m_fpipe, "\tray_procedural -m %g %g %g -M %g %g %g ieworld\n",
				 b.min.x, b.min.y, b.min.z, b.max.x, b.max.y, b.max.z );
		fputs( "ray_end\n", m_fpipe );
		fflush( m_fpipe );
		return;
	}

	if ( type == "ieprocedural" )
	{
		CompoundDataMap::const_iterator class_it = topology.find( "className");
		CompoundDataMap::const_iterator version_it = topology.find( "classVersion");
		CompoundDataMap::const_iterator parameters_it = topology.find( "parameterString");
		CompoundDataMap::const_iterator bound_it = topology.find( "bound" );
		
		ConstStringDataPtr className = 0;
		ConstIntDataPtr classVersion = new IntData(1);
		ConstStringDataPtr parameterString = new StringData("");

		if ( class_it == topology.end() )
		{
			msg( Msg::Error, "IECoreMantra::RendererImplementation::geometry", "Invalid className parameter." );
			return;
		}
		else
		{
			className = runTimeCast<const StringData>( class_it->second );
			if ( !className )
			{
				msg( Msg::Error, "IECoreMantra::RendererImplementation::geometry", "Invalid className parameter." );
				return;
			}
		}
		
		if ( version_it != topology.end() )
		{
			ConstIntDataPtr argClassVersion = runTimeCast<const IntData>( version_it->second );
			if ( argClassVersion )
			{
				classVersion = argClassVersion;
			}
		}
		
		if ( parameters_it != topology.end() )
		{
			ConstStringDataPtr argParameters = runTimeCast<const StringData>( parameters_it->second );
			if ( argParameters )
			{
				parameterString = argParameters;
			}
		}

		Box3f b = Box3f( V3f( limits<float>::min() ), V3f( limits<float>::max() ) );
		if ( bound_it != topology.end() )
		{
			ConstBox3fDataPtr argBound = runTimeCast<const Box3fData>( bound_it->second );
			b = argBound->readable();
		}
		
		fputs( "ray_start object\n", m_fpipe );
		
		M44f m = getTransform();
		
		fprintf( m_fpipe, "\tray_transform %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n", 
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
		
		fprintf( m_fpipe, "\tray_procedural -m %g %g %g -M %g %g %g ieprocedural"
				 " className \"%s\" classVersion %i parameterString \"%s\"\n", 
				 b.min.x, b.min.y, b.min.z, b.max.x, b.max.y, b.max.z,
				 (char *)className->readable().c_str(), 
				 classVersion->readable(), 
				 (char *)parameterString->readable().c_str() );
		
		fputs( "ray_end\n", m_fpipe );

		fflush( m_fpipe );
	}
}

void IECoreMantra::RendererImplementation::procedural( IECore::Renderer::ProceduralPtr proc )
{
	if ( m_mode != Procedural )
	{
		msg( Msg::Warning, "IECoreMantra::RendererImplementation::procedural", "Not implemented in this mode, use geometry(\"ieprocedural\", ..)" );
		return;
	}
	// make a new Renderer, copy data members from parent Renderer
	RendererImplementationPtr renderer = new RendererImplementation( this ); // construct from parent pointer
	// connect the IECore procedural to the VRAY procedural
	renderer->m_vrayproc->m_procedural = proc;
	// set the back pointer on the VRAY procedural to this new renderer
	renderer->m_vrayproc->m_renderer = renderer;
	// set the bound on the VRAY procedural
	Box3f bound = proc->bound();
	bound = transform(bound, m_transformStack.top() );
	renderer->m_vrayproc->m_bound = bound;
	// add the new VRAY procedural to it's parent
	VRAY_ProceduralChildPtr child = m_vrayproc->createChild();
	child->addProcedural( renderer->m_vrayproc );
}

void IECoreMantra::RendererImplementation::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::instanceBegin", "Not implemented" );
}

void IECoreMantra::RendererImplementation::instanceEnd()
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::instanceEnd", "Not implemented" );
}

void IECoreMantra::RendererImplementation::instance( const std::string &name )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::instance", "Not implemented" );
}

IECore::DataPtr 
IECoreMantra::RendererImplementation::command( const std::string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::command", "Not implemented" );
	return 0;
}

void IECoreMantra::RendererImplementation::editBegin( const std::string &editType, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::editBegin", "Not implemented" );
}

void IECoreMantra::RendererImplementation::editEnd()
{
	msg( Msg::Warning, "IECoreMantra::RendererImplementation::editEnd", "Not implemented" );
}
