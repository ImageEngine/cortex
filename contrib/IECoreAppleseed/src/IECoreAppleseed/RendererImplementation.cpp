//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include <algorithm>

#include "boost/filesystem/convenience.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/algorithm/string.hpp"

#include "foundation/utility/searchpaths.h"

#include "renderer/api/bsdf.h"
#include "renderer/api/camera.h"
#include "renderer/api/color.h"
#include "renderer/api/display.h"
#include "renderer/api/environmentedf.h"
#include "renderer/api/environmentshader.h"
#include "renderer/api/frame.h"
#include "renderer/api/light.h"
#include "renderer/api/material.h"
#include "renderer/api/object.h"
#include "renderer/api/project.h"
#include "renderer/api/rendering.h"
#include "renderer/api/scene.h"
#include "renderer/api/surfaceshader.h"
#include "renderer/api/utility.h"

#include "IECore/MessageHandler.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/Transform.h"

#include "IECoreAppleseed/private/RendererImplementation.h"
#include "IECoreAppleseed/private/BatchPrimitiveConverter.h"
#include "IECoreAppleseed/private/InteractivePrimitiveConverter.h"
#include "IECoreAppleseed/ToAppleseedCameraConverter.h"

using namespace IECore;
using namespace IECoreAppleseed;
using namespace Imath;
using namespace std;
using namespace boost;

namespace asf = foundation;
namespace asr = renderer;

////////////////////////////////////////////////////////////////////////
// RendererImplementation implementation
////////////////////////////////////////////////////////////////////////

IECoreAppleseed::RendererImplementation::RendererImplementation()
{
	m_interactive = true;
	constructCommon();
	m_primitiveConverter.reset( new InteractivePrimitiveConverter( m_project->search_paths() ) );
}

IECoreAppleseed::RendererImplementation::RendererImplementation( const string &fileName )
{
	m_interactive = false;
	m_fileName = fileName;
	m_projectPath = filesystem::path( fileName ).parent_path();

	// create a dir to store the mesh files if it does not exist yet.
	filesystem::path geomPath = m_projectPath / "_geometry";

	if( !filesystem::exists( geomPath ) )
	{
		if( !filesystem::create_directory( geomPath ) )
		{
			msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::RendererImplementation", "Couldn't create _geometry directory." );
		}
	}

	constructCommon();
	m_project->set_path( fileName.c_str() );
	m_project->search_paths().set_root_path( m_projectPath.string().c_str() );
	m_primitiveConverter.reset( new BatchPrimitiveConverter( m_projectPath, m_project->search_paths() ) );
}

void IECoreAppleseed::RendererImplementation::constructCommon()
{
	m_mainAssembly = 0;

	m_transformStack.clear();
	m_attributeStack.push( AttributeState() );

	m_project = asr::ProjectFactory::create( "project" );
	m_project->add_default_configurations();

	// create some needed parameters that don't get created by default.
	// possible bug in appleseed?
	{
		asr::Configuration *cfg = m_project->configurations().get_by_name( "final" );
		cfg->get_parameters().insert_path( "sppm.initial_radius", "1.0" );
	}

	// create some basic project entities.
	asf::auto_release_ptr<asr::Frame> frame( asr::FrameFactory::create( "beauty", asr::ParamArray() ) );
	m_project->set_frame( frame );

	asf::auto_release_ptr<asr::Scene> scene = asr::SceneFactory::create();
	m_project->set_scene( scene );
	m_project->get_scene()->set_environment( asr::EnvironmentFactory().create( "environment", asr::ParamArray() ) );
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::setOption( const string &name, IECore::ConstDataPtr value )
{
	m_optionsMap[name] = value;

	if( 0 == name.compare( 0, 7, "as:cfg:" ) )
	{
		// appleseed render settings.

		string optName( name, 7, string::npos );
		std::replace( optName.begin(), optName.end(), ':', '.' );
		string valueStr = dataToString( value );

		if( !valueStr.empty() )
		{
			m_project->configurations().get_by_name( "final" )->get_parameters().insert_path( optName.c_str(), valueStr.c_str() );
			m_project->configurations().get_by_name( "interactive" )->get_parameters().insert_path( optName.c_str(), valueStr.c_str() );

			// if the number of passes is different than one, we need to
			// switch the shading result framebuffer in the final rendering config.
			if( optName == "generic_frame_renderer.passes" )
			{
				int numPasses = static_cast<const IntData*>( value.get() )->readable();
				m_project->configurations().get_by_name( "final" )->get_parameters().insert( "shading_result_framebuffer", numPasses == 1 ? "ephemeral" : "permanent" );
			}
		}
	}
	else if( 0 == name.compare( 0, 3, "as:" ) )
	{
		// other appleseed options.

		string optName( name, 3, string::npos );

		if( optName == "searchpath" )
		{
			if( const StringData *f = runTimeCast<const StringData>( value.get() ) )
			{
				m_project->search_paths().push_back( f->readable().c_str() );
			}
			else
			{
				msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", "as:searchpath option expects a StringData value." );
			}
		}
		else if( optName == "mesh_file_format" )
		{
			m_primitiveConverter->setOption( name, value );
		}
		else if( optName == "automatic_instancing" )
		{
			m_primitiveConverter->setOption( name, value );
		}
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// ignore options prefixed for some other renderer
	}
	else
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::setOption", format( "Unknown option \"%s\"." ) % name );
	}
}

IECore::ConstDataPtr IECoreAppleseed::RendererImplementation::getOption( const string &name ) const
{
	OptionsMap::const_iterator it( m_optionsMap.find( name ) );

	if( it != m_optionsMap.end())
	{
		return it->second;
	}

	return IECore::ConstDataPtr();
}

void IECoreAppleseed::RendererImplementation::camera( const string &name, const IECore::CompoundDataMap &parameters )
{
	const string *cameraName = getOptionAs<string>( "render:camera" );

	// ignore secondary cameras.
	if( !cameraName || name != *cameraName )
	{
		return;
	}

	CameraPtr cortexCamera = new Camera( name, 0, new CompoundData( parameters ) );
	ToAppleseedCameraConverterPtr converter = new ToAppleseedCameraConverter( cortexCamera );
	asf::auto_release_ptr<asr::Camera> appleseedCamera( static_cast<asr::Camera*>( converter->convert() ) );

	if( !appleseedCamera.get() )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::camera", "Couldn't create camera." );
		return;
	}

	appleseedCamera->transform_sequence() = m_transformStack.top();
	setCamera( cortexCamera, appleseedCamera );
}

void IECoreAppleseed::RendererImplementation::display( const string &name, const string &type, const string &data, const IECore::CompoundDataMap &parameters )
{
	// exr and png are special...
	if( type == "exr" || type == "png" )
	{
		m_project->get_frame()->get_parameters().insert( "output_filename", name.c_str() );
		m_project->get_frame()->get_parameters().insert( "output_aovs", false );

		if( type == "png" )
		{
			m_project->get_frame()->get_parameters().insert( "color_space", "srgb" );
		}
		else
		{
			m_project->get_frame()->get_parameters().insert( "color_space", "linear_rgb" );
		}
	}
	else
	{
		asr::ParamArray params = convertParams( parameters );
		params.insert( "displayName", name.c_str() );
		params.insert( "type", type.c_str() );
		params.insert( "data", data.c_str() );
		params.insert( "plugin_name", type.c_str() );
		asf::auto_release_ptr<asr::Display> dpy( asr::DisplayFactory::create( name.c_str(), params ) );
		m_project->set_display( dpy );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// world
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::worldBegin()
{
	// reset transform stack
	if( m_transformStack.size() > 1 )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::worldBegin", "Missing transformEnd() call detected." );

		m_transformStack.clear();
	}

	// create the main assembly
	asf::auto_release_ptr<asr::Assembly> assembly = asr::AssemblyFactory::create( "assembly", asr::ParamArray() );
	m_mainAssembly = assembly.get();
	m_project->get_scene()->assemblies().insert( assembly );
}

void IECoreAppleseed::RendererImplementation::worldEnd()
{
	if( m_transformStack.size() != 1 )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::worldEnd", "Missing transformEnd() call detected." );
	}

	// create a default camera if needed
	if( !m_project->get_scene()->get_camera() )
	{
		CameraPtr cortexCamera = new Camera();
		cortexCamera->parameters()["projection"] = new StringData("perspective");

		if( const V2i *res = getOptionAs<V2i>( "camera:resolution" ) )
		{
			cortexCamera->parameters()["resolution"] = new V2iData( *res );
		}

		cortexCamera->addStandardParameters();

		ToAppleseedCameraConverterPtr converter = new ToAppleseedCameraConverter( cortexCamera );
		asf::auto_release_ptr<asr::Camera> camera( static_cast<asr::Camera*>( converter->convert() ) );
		assert( camera.get() );

		setCamera( cortexCamera, camera );
	}

	// instance the main assembly
	asf::auto_release_ptr<asr::AssemblyInstance> assemblyInstance = asr::AssemblyInstanceFactory::create( "assembly_inst", asr::ParamArray(), "assembly" );
	m_project->get_scene()->assembly_instances().insert( assemblyInstance );

	if( m_fileName.empty() )
	{
		// TODO: interactive render here...
	}
	else
	{
		asr::ProjectFileWriter::write( *m_project, m_fileName.c_str(), asr::ProjectFileWriter::OmitBringingAssets | asr::ProjectFileWriter::OmitWritingGeometryFiles );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::transformBegin()
{
	m_transformStack.push( ( m_transformStack.top() ) );
}

void IECoreAppleseed::RendererImplementation::transformEnd()
{
	if( m_transformStack.size() <= 1 )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::transformEnd", "No matching transformBegin() call." );
		return;
	}

	m_transformStack.pop();
}

void IECoreAppleseed::RendererImplementation::setTransform( const Imath::M44f &m )
{
	m_transformStack.setTransform( m );
}

void IECoreAppleseed::RendererImplementation::setTransform( const string &coordinateSystem )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::setTransform", "Not implemented." );
}

Imath::M44f IECoreAppleseed::RendererImplementation::getTransform() const
{
	M44d m = m_transformStack.top().get_earliest_transform().get_local_to_parent();
	return M44f( m );
}

Imath::M44f IECoreAppleseed::RendererImplementation::getTransform( const string &coordinateSystem ) const
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::getTransform", "Not implemented." );
	return M44f();
}

void IECoreAppleseed::RendererImplementation::concatTransform( const Imath::M44f &m )
{
	m_transformStack.concatTransform( m );
}

void IECoreAppleseed::RendererImplementation::coordinateSystem( const string &name )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::coordinateSystem", "Not implemented." );
}

//////////////////////////////////////////////////////////////////////////////////////////
// attribute code
//////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::attributeBegin()
{
	transformBegin();
	m_attributeStack.push( AttributeState( m_attributeStack.top() ) );
}

void IECoreAppleseed::RendererImplementation::attributeEnd()
{
	m_attributeStack.pop();
	transformEnd();
}

void IECoreAppleseed::RendererImplementation::setAttribute( const string &name, IECore::ConstDataPtr value )
{
	m_attributeStack.top().setAttribute( name, value );
}

IECore::ConstDataPtr IECoreAppleseed::RendererImplementation::getAttribute( const string &name ) const
{
	return m_attributeStack.top().getAttribute( name );
}

void IECoreAppleseed::RendererImplementation::shader( const string &type, const string &name, const IECore::CompoundDataMap &parameters )
{
	if( type == "osl:shader" || type == "shader" )
	{
		ConstShaderPtr shader( new Shader( name, "shader", parameters ) );
		m_attributeStack.top().addOSLShader( shader );
	}
	else if( type == "osl:surface" || type == "surface" )
	{
		ConstShaderPtr shader( new Shader( name, "surface", parameters ) );
		m_attributeStack.top().setOSLSurface( shader );
	}
	else
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::shader", format( "Unknown shader type \"%s\"." ) % type );
	}
}

void IECoreAppleseed::RendererImplementation::light( const string &name, const string &handle, const IECore::CompoundDataMap &parameters )
{
	bool isEnvironment = boost::algorithm::ends_with( name, "_environment_edf" );
	const string *environmentEDFName = 0;

	// ignore enviromnment EDFs not selected in the appleseed options node.
	if( isEnvironment )
	{
		environmentEDFName = getOptionAs<string>( "as:environment_edf" );

		if( !environmentEDFName || m_attributeStack.top().name() != *environmentEDFName )
			return;
	}

	asr::ParamArray params;
	for( CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		string paramName = it->first.value();
		ConstDataPtr paramValue = it->second;

		if( isEnvironment && paramName == "radiance_map" )
		{
			if( paramValue->typeId() != StringDataTypeId )
			{
				msg( MessageHandler::Warning, "IECoreAppleseed::RendererImplementation::light", "Expected radianceMap parameter to be a string" );
				continue;
			}

			const string &fileName = static_cast<const StringData*>( paramValue.get() )->readable();
			string textureName = m_attributeStack.top().name() + "." + paramName;
			string textureInstanceName = createTextureEntity( m_project->get_scene()->textures(), m_project->get_scene()->texture_instances(), m_project->search_paths(), textureName, fileName );
			params.insert( "radiance", textureInstanceName.c_str() );
		}
		else
		{
			if( paramValue->typeId() == Color3fDataTypeId )
			{
				string colorName = m_attributeStack.top().name() + "." + paramName;
				const Imath::Color3f &col = static_cast<const Color3fData*>( paramValue.get() )->readable();
				colorName = createColorEntity( m_project->get_scene()->colors(), col, colorName.c_str() );
				params.insert( paramName.c_str(), colorName.c_str() );
			}
			else
			{
				params.insert( paramName.c_str(), dataToString( paramValue ) );
			}
		}
	}

	if( isEnvironment )
	{
		asr::EnvironmentEDFFactoryRegistrar factoryRegistrar;
		const asr::IEnvironmentEDFFactory *factory = factoryRegistrar.lookup( name.c_str() );

		if( !factory )
		{
			msg( MessageHandler::Warning, "IECoreAppleseed::RendererImplementation::light", format( "Unknown light model \"%s\"." ) % name );
			return;
		}

		asf::auto_release_ptr<asr::EnvironmentEDF> light( factory->create( m_attributeStack.top().name().c_str(), params ) );
		insertEntityWithUniqueName( m_project->get_scene()->environment_edfs(), light, m_attributeStack.top().name() );
		m_project->get_scene()->get_environment()->get_parameters().insert( "environment_edf", environmentEDFName->c_str() );

		const bool *envEDFVisible = getOptionAs<bool>( "as:environment_edf_background" );
		if( envEDFVisible && *envEDFVisible == true )
		{
			asr::ParamArray params;
			params.insert( "environment_edf", environmentEDFName->c_str() );
			asf::auto_release_ptr<asr::EnvironmentShader> envShader( asr::EnvironmentShaderFactoryRegistrar().lookup( "edf_environment_shader" )->create( "environment_shader", params ) );
			m_project->get_scene()->environment_shaders().insert( envShader );
			m_project->get_scene()->get_environment()->get_parameters().insert( "environment_shader", "environment_shader" );
		}
	}
	else // normal, singular light
	{
		asr::LightFactoryRegistrar factoryRegistrar;
		const asr::ILightFactory *factory = factoryRegistrar.lookup( name.c_str() );

		if( !factory )
		{
			msg( MessageHandler::Warning, "IECoreAppleseed::RendererImplementation::light", format( "Unknown light model \"%s\"." ) % name );
			return;
		}

		asf::auto_release_ptr<asr::Light> light( factory->create( m_attributeStack.top().name().c_str(), params ) );
		light->set_transform( m_transformStack.top().get_earliest_transform() );
		insertEntityWithUniqueName( m_mainAssembly->lights(), light, m_attributeStack.top().name() );
	}
}

void IECoreAppleseed::RendererImplementation::illuminate( const string &lightHandle, bool on )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::illuminate", "Not implemented." );
}

/////////////////////////////////////////////////////////////////////////////////////////
// motion blur
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::motionBegin( const std::set<float> &times )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::motionBegin", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::motionEnd()
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::motionEnd", "Not implemented." );
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::points", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::disk", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::curves( const IECore::CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::curves", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::text( const string &font, const string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::text", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::sphere", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::image", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	if( !m_mainAssembly )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation", "Geometry not inside world block, ignoring." );
		return;
	}

	MeshPrimitivePtr mesh = new IECore::MeshPrimitive( vertsPerFace, vertIds, interpolation );
	mesh->variables = primVars;

	string materialName = currentMaterialName();

	if( const asr::Assembly *assembly = m_primitiveConverter->convertPrimitive( mesh, m_attributeStack.top(), materialName, *m_mainAssembly ) )
	{
		createAssemblyInstance( assembly->get_name() );
	}
}

void IECoreAppleseed::RendererImplementation::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::nurbs", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::patchMesh", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::geometry( const string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::geometry", "Not implemented." );
}

/////////////////////////////////////////////////////////////////////////////////////////
// procedurals
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::procedural( IECore::Renderer::ProceduralPtr proc )
{
	// appleseed does not support procedurals yet, so we expand them immediately.
	proc->render( this );
}

/////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::instanceBegin( const string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::instanceBegin", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::instanceEnd()
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::instanceEnd", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::instance( const string &name )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::instance", "Not implemented." );
}

/////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////

IECore::DataPtr IECoreAppleseed::RendererImplementation::command( const string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::command", "Not implemented." );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// rerendering
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::editBegin( const string &editType, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::editBegin", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::editEnd()
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::editEnd", "Not implemented." );
}

/////////////////////////////////////////////////////////////////////////////////////////
// private
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::setCamera( CameraPtr cortexCamera, asf::auto_release_ptr<asr::Camera> &appleseedCamera )
{
	m_project->get_scene()->set_camera( appleseedCamera );

	// resolution
	m_project->get_frame()->get_parameters().insert( "camera", "camera" );
	const V2iData *resolution = cortexCamera->parametersData()->member<V2iData>( "resolution" );
	asf::Vector2i res( resolution->readable().x, resolution->readable().y );
	m_project->get_frame()->get_parameters().insert( "resolution", res );

	// crop window
	const Box2fData *cropWindow = cortexCamera->parametersData()->member<Box2fData>( "cropWindow" );
	asf::AABB2u crop;
	crop.min[0] = cropWindow->readable().min.x * ( res[0] - 1 );
	crop.min[1] = cropWindow->readable().min.y * ( res[1] - 1 );
	crop.max[0] = cropWindow->readable().max.x * ( res[0] - 1 );
	crop.max[1] = cropWindow->readable().max.y * ( res[1] - 1 );
	m_project->get_frame()->set_crop_window( crop );
}

string IECoreAppleseed::RendererImplementation::currentShaderGroupName()
{
	string shaderGroupName;

	if( m_attributeStack.top().shadingStateValid() )
	{
		MurmurHash shaderGroupHash = m_attributeStack.top().shaderGroupHash();

		ShaderGroupMap::const_iterator it( m_shaderGroupNames.find( shaderGroupHash ) );

		if( it == m_shaderGroupNames.end() )
		{
			shaderGroupName = m_attributeStack.top().createShaderGroup( *m_mainAssembly );
			m_shaderGroupNames[shaderGroupHash] = shaderGroupName;
		}
		else
		{
			shaderGroupName = it->second;
		}
	}

	return shaderGroupName;
}

string IECoreAppleseed::RendererImplementation::currentMaterialName()
{
	string materialName;

	if( m_attributeStack.top().shadingStateValid() )
	{
		MurmurHash materialHash = m_attributeStack.top().materialHash();

		MaterialMap::const_iterator it( m_materialNames.find( materialHash ) );

		if( it == m_materialNames.end() )
		{
			string ShaderGroupName = currentShaderGroupName();
			materialName = m_attributeStack.top().createMaterial( *m_mainAssembly, ShaderGroupName );
			m_materialNames[materialHash] = materialName;
		}
		else
		{
			materialName = it->second;
		}
	}

	return materialName;
}

void IECoreAppleseed::RendererImplementation::createAssemblyInstance( const string &assemblyName )
{
	string assemblyInstanceName = m_attributeStack.top().name() + "_instance";

	asr::ParamArray params;
	params.insert( "visibility", m_attributeStack.top().visibilityDictionary() );
	asf::auto_release_ptr<asr::AssemblyInstance> assemblyInstance = asr::AssemblyInstanceFactory::create( assemblyInstanceName.c_str(), params, assemblyName.c_str() );

	assemblyInstance->transform_sequence() = m_transformStack.top();
	insertEntityWithUniqueName( m_mainAssembly->assembly_instances(), assemblyInstance, assemblyInstanceName );
}

asr::Project *IECoreAppleseed::RendererImplementation::appleseedProject() const
{
	return m_project.get();
}
