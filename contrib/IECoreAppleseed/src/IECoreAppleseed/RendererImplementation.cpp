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

#include "IECoreAppleseed/private/RendererImplementation.h"

#include "IECoreAppleseed/CameraAlgo.h"
#include "IECoreAppleseed/EntityAlgo.h"
#include "IECoreAppleseed/LogTarget.h"
#include "IECoreAppleseed/ParameterAlgo.h"
#include "IECoreAppleseed/RendererController.h"
#include "IECoreAppleseed/private/BatchPrimitiveConverter.h"
#include "IECoreAppleseed/private/InteractivePrimitiveConverter.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/Transform.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/filesystem/operations.hpp"

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
#include "renderer/api/rendering.h"
#include "renderer/api/surfaceshader.h"
#include "renderer/api/utility.h"
#include "renderer/api/version.h"

#include <algorithm>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAppleseed;
using namespace Imath;
using namespace boost;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

////////////////////////////////////////////////////////////////////////
// RendererImplementation implementation
////////////////////////////////////////////////////////////////////////

IECoreAppleseed::RendererImplementation::RendererImplementation()
{
	m_logTarget.reset( new IECoreLogTarget() );
	asr::global_logger().add_target( m_logTarget.get() );

	constructCommon();

	// 16 bits float (half) is the default pixel format in appleseed.
	// For progressive rendering, force the pixel format to float
	// to avoid half -> float conversions in the display driver.
	m_project->get_frame()->get_parameters().insert( "pixel_format", "float" );

	m_primitiveConverter.reset( new InteractivePrimitiveConverter( m_project->search_paths() ) );
	m_motionHandler.reset( new MotionBlockHandler( m_transformStack, *m_primitiveConverter ) );
}

IECoreAppleseed::RendererImplementation::RendererImplementation( const string &fileName )
{
	if( fileName.empty() )
	{
		msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::RendererImplementation", "Empty project filename" );
	}

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
	m_motionHandler.reset( new MotionBlockHandler( m_transformStack, *m_primitiveConverter ) );
}

void IECoreAppleseed::RendererImplementation::constructCommon()
{
	m_mainAssembly = nullptr;

	m_transformStack.clear();
	m_attributeStack.push( AttributeState() );

	m_project = asr::ProjectFactory::create( "project" );
	m_project->add_default_configurations();

	// Insert some config params needed by the interactive renderer.
	asr::Configuration *cfg = m_project->configurations().get_by_name( "interactive" );
	asr::ParamArray *params = &cfg->get_parameters();
	params->insert( "sample_renderer", "generic" );
	params->insert( "sample_generator", "generic" );
	params->insert( "tile_renderer", "generic" );
	params->insert( "frame_renderer", "progressive" );
	params->insert( "lighting_engine", "pt" );
	params->insert( "sampling_mode", "qmc" );
	params->insert( "spectrum_mode", "rgb" );
	params->insert_path( "progressive_frame_renderer.max_fps", "5" );

	// Insert some config params needed by the final renderer.
	cfg = m_project->configurations().get_by_name( "final" );
	params = &cfg->get_parameters();
	params->insert( "sample_renderer", "generic" );
	params->insert( "sample_generator", "generic" );
	params->insert( "tile_renderer", "generic" );
	params->insert( "frame_renderer", "generic" );
	params->insert( "lighting_engine", "pt" );
	params->insert( "pixel_renderer", "uniform" );
	params->insert( "sampling_mode", "qmc" );
	params->insert( "spectrum_mode", "rgb" );
	params->insert_path( "uniform_pixel_renderer.samples", "1" );

	// create some basic project entities.
	asf::auto_release_ptr<asr::Frame> frame( asr::FrameFactory::create( "beauty", asr::ParamArray().insert( "resolution", "640 480" ) ) );
	m_project->set_frame( frame );

	asf::auto_release_ptr<asr::Scene> scene = asr::SceneFactory::create();
	m_project->set_scene( scene );
	m_project->get_scene()->set_environment( asr::EnvironmentFactory().create( "environment", asr::ParamArray() ) );
}

IECoreAppleseed::RendererImplementation::~RendererImplementation()
{
	asr::global_logger().remove_target( m_logTarget.get() );
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::setOption( const string &name, ConstDataPtr value )
{
	m_optionsMap[name] = value;

	if( 0 == name.compare( 0, 7, "as:cfg:" ) )
	{
		// appleseed render settings.

		string optName( name, 7, string::npos );
		replace( optName.begin(), optName.end(), ':', '.' );
		string valueStr = ParameterAlgo::dataToString( value );

		if( !valueStr.empty() )
		{
			if( optName == "rendering_threads" )
			{
				if( const IntData *numThreadsData = runTimeCast<const IntData>( value.get() ) )
				{
					// if numThreads is 0, we want to use all the CPU cores.
					// We can remove any previous "rendering_threads" param and
					// let appleseed choose the number of threads to use.
					if( numThreadsData->readable() == 0 )
					{
						m_project->configurations().get_by_name( "final" )->get_parameters().remove_path( optName.c_str() );
						m_project->configurations().get_by_name( "interactive" )->get_parameters().remove_path( optName.c_str() );
						return;
					}
				}
				else
				{
					msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", "as:cfg:rendering_threads option expects an IntData value." );
				}
			}
			else if( algorithm::ends_with( optName, "max_path_length" ) )
			{
				if( const IntData *maxPathLengthData = runTimeCast<const IntData>( value.get() ) )
				{
					// if maxPathLength is 0, we want to use and unlimited number of bounces and let
					// russian roulette terminate paths when their contribution is too low.
					// We can disable max path lengths by remove any previous param.
					if( maxPathLengthData->readable() == 0 )
					{
						m_project->configurations().get_by_name( "final" )->get_parameters().remove_path( optName.c_str() );
						m_project->configurations().get_by_name( "interactive" )->get_parameters().remove_path( optName.c_str() );
						return;
					}
				}
				else
				{
					msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", format( "%s option expects an IntData value." ) % optName );
				}
			}
			else if( optName == "pt.max_ray_intensity" )
			{
				if( const FloatData *maxIntensityData = runTimeCast<const FloatData>( value.get() ) )
				{
					// if maxIntensity is 0 disable it.
					if( maxIntensityData->readable() == 0.0f )
					{
						m_project->configurations().get_by_name( "final" )->get_parameters().remove_path( optName.c_str() );
						m_project->configurations().get_by_name( "interactive" )->get_parameters().remove_path( optName.c_str() );
						return;
					}
				}
				else
				{
					msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", "as:cfg:pt:max_ray_intensity option expects a FloatData value." );
				}
			}
			else if( optName == "generic_frame_renderer.passes" )
			{
				if( const IntData *numPassesData = runTimeCast<const IntData>( value.get() ) )
				{
					int numPasses = numPassesData->readable();

					// if the number of passes is greater than one, we need to
					// switch the shading result framebuffer in the final rendering config.
					m_project->configurations().get_by_name( "final" )->get_parameters().insert( "shading_result_framebuffer", numPasses > 1 ? "permanent" : "ephemeral" );

					// enable decorrelate pixels if the number of render passes is greater than one.
					m_project->configurations().get_by_name( "final" )->get_parameters().insert_path( "uniform_pixel_renderer.decorrelate_pixels", numPasses > 1 ? "true" : "false" );
					m_project->configurations().get_by_name( "interactive" )->get_parameters().insert_path( "uniform_pixel_renderer.decorrelate_pixels", numPasses > 1 ? "true" : "false" );
				}
				else
				{
					msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", "as:cfg:generic_frame_renderer:passes option expects an IntData value." );
				}
			}
			else if( optName == "shading_engine.override_shading.mode" )
			{
				if( const StringData *overrideData = runTimeCast<const StringData>( value.get() ) )
				{
					// if no shading override is specified, remove the params.
					if( overrideData->readable() == "no_override" )
					{
						m_project->configurations().get_by_name( "final" )->get_parameters().remove_path( "shading_engine.override_shading" );
						m_project->configurations().get_by_name( "interactive" )->get_parameters().remove_path( "shading_engine.override_shading" );
						return;
					}
				}
				else
				{
					msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", "as:cfg:shading_engine:override_shading:mode option expects a StringData value." );
				}
			}

			m_project->configurations().get_by_name( "final" )->get_parameters().insert_path( optName.c_str(), valueStr.c_str() );
			m_project->configurations().get_by_name( "interactive" )->get_parameters().insert_path( optName.c_str(), valueStr.c_str() );
		}
	}
	else if( 0 == name.compare( 0, 3, "as:" ) )
	{
		// other appleseed options.

		string optName( name, 3, string::npos );

		if( optName == "searchpath" )
		{
			if( const StringData *searchPathData = runTimeCast<const StringData>( value.get() ) )
			{
				m_project->search_paths().push_back( searchPathData->readable().c_str() );
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
	else if( name == "editable" )
	{
		if( const BoolData *editableData = runTimeCast<const BoolData>( value.get() ) )
		{
			if( editableData->readable() )
			{
				m_editHandler.reset( new EditBlockHandler( *m_project ) );
			}
			else
			{
				m_editHandler.reset();
			}
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", "editable option expects an BoolData value." );
		}
	}
	else
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::setOption", format( "Unknown option \"%s\"." ) % name );
	}
}

ConstDataPtr IECoreAppleseed::RendererImplementation::getOption( const string &name ) const
{
	OptionsMap::const_iterator it( m_optionsMap.find( name ) );

	if( it != m_optionsMap.end())
	{
		return it->second;
	}

	return ConstDataPtr();
}

void IECoreAppleseed::RendererImplementation::camera( const string &name, const CompoundDataMap &parameters )
{
	// ignore other cameras if a camera has been specified
	// using the render:camera option.
	const string *cameraName = getOptionAs<string>( "render:camera" );

	if( cameraName && *cameraName != name )
	{
		return;
	}

	// ignore extra cameras if we already have one.
	if( !insideEditBlock() && !m_project->get_scene()->cameras().empty() )
	{
		return;
	}

	// ignore edits for extra cameras.
	if( insideEditBlock() )
	{
		assert( !m_project->get_scene()->cameras().empty() );

		if( name != m_project->get_scene()->cameras().get_by_index( 0 )->get_name() )
		{
			return;
		}
	}

	CompoundDataPtr params = new CompoundData( parameters );
	CameraPtr cortexCamera = new Camera( name, nullptr, params );
	cortexCamera->addStandardParameters();

	asf::auto_release_ptr<asr::Camera> appleseedCamera( CameraAlgo::convert( cortexCamera.get() ) );

	if( !appleseedCamera.get() )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::camera", "Couldn't create camera." );
		return;
	}

	if( insideEditBlock() )
	{
		// Update the camera.
		asr::Camera *camera = m_project->get_scene()->cameras().get_by_index( 0 );

		// Update the transform if needed.
		if( m_transformStack.size() )
		{
			camera->transform_sequence() = m_transformStack.top();
		}
	}
	else
	{
		// pass the shutter interval to the primitive converter.
		const V2f &shutter = params->member<V2fData>( "shutter", true )->readable();
		m_primitiveConverter->setShutterInterval( shutter.x, shutter.y );

		appleseedCamera->transform_sequence() = m_transformStack.top();
		setCamera( name, cortexCamera, appleseedCamera );
	}
}

void IECoreAppleseed::RendererImplementation::display( const string &name, const string &type, const string &data, const CompoundDataMap &parameters )
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
		asr::ParamArray params;
		params.insert( "displayName", name.c_str() );
		params.insert( "type", type.c_str() );
		params.insert( "data", data.c_str() );
		params.insert( "plugin_name", type.c_str() );
		params.push( "beauty" ) = ParameterAlgo::convertParams( parameters );
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
	asf::auto_release_ptr<asr::Assembly> assembly = asr::AssemblyFactory().create( "assembly", asr::ParamArray() );
	m_mainAssembly = assembly.get();
	m_project->get_scene()->assemblies().insert( assembly );

	m_lightHandler.reset( new LightHandler( *m_project->get_scene(), m_project->search_paths() ) );
}

void IECoreAppleseed::RendererImplementation::worldEnd()
{
	if( m_transformStack.size() != 1 )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::worldEnd", "Missing transformEnd() call detected." );
	}

	// create a default camera if needed
	if( m_project->get_scene()->cameras().empty() )
	{
		CameraPtr cortexCamera = new Camera();
		cortexCamera->parameters()["projection"] = new StringData("perspective");

		if( const V2i *res = getOptionAs<V2i>( "camera:resolution" ) )
		{
			cortexCamera->parameters()["resolution"] = new V2iData( *res );
		}

		cortexCamera->addStandardParameters();

		asf::auto_release_ptr<asr::Camera> camera( CameraAlgo::convert( cortexCamera.get() ) );
		assert( camera.get() );

		setCamera( "camera", cortexCamera, camera );
	}

	// instance the main assembly
	asf::auto_release_ptr<asr::AssemblyInstance> assemblyInstance = asr::AssemblyInstanceFactory::create( "assembly_inst", asr::ParamArray(), "assembly" );
	m_project->get_scene()->assembly_instances().insert( assemblyInstance );

	// render or export the project
	if( isEditable() )
	{
		m_editHandler->startRendering();
	}
	else if( isProjectGen() )
	{
		const int writeOptions = asr::ProjectFileWriter::OmitHandlingAssetFiles | asr::ProjectFileWriter::OmitWritingGeometryFiles;
		asr::ProjectFileWriter::write( *m_project, m_fileName.c_str(), writeOptions );
	}
	else
	{
		// interactive non-editable render.
		RendererController rendererController;
		asr::Configuration *cfg = m_project->configurations().get_by_name( "final" );
		asr::MasterRenderer renderer( *m_project, cfg->get_parameters(), &rendererController);
		renderer.render();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::transformBegin()
{
	if( insideEditBlock() && m_transformStack.size() == 0 )
	{
		m_transformStack.clear();
	}

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

void IECoreAppleseed::RendererImplementation::setTransform( const M44f &m )
{
	if( insideMotionBlock() )
	{
		m_motionHandler->setTransform( m );
	}
	else
	{
		m_transformStack.setTransform( m );
	}
}

void IECoreAppleseed::RendererImplementation::setTransform( const string &coordinateSystem )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::setTransform", "Not implemented." );
}

M44f IECoreAppleseed::RendererImplementation::getTransform() const
{
	M44d m = m_transformStack.top().get_earliest_transform().get_local_to_parent();
	return M44f( m );
}

M44f IECoreAppleseed::RendererImplementation::getTransform( const string &coordinateSystem ) const
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::getTransform", "Not implemented." );
	return M44f();
}

void IECoreAppleseed::RendererImplementation::concatTransform( const M44f &m )
{
	if( insideMotionBlock() )
	{
		m_motionHandler->concatTransform( m );
	}
	else
	{
		m_transformStack.concatTransform( m );
	}
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

void IECoreAppleseed::RendererImplementation::setAttribute( const string &name, ConstDataPtr value )
{
	m_attributeStack.top().setAttribute( name, value );
}

ConstDataPtr IECoreAppleseed::RendererImplementation::getAttribute( const string &name ) const
{
	return m_attributeStack.top().getAttribute( name );
}

void IECoreAppleseed::RendererImplementation::shader( const string &type, const string &name, const CompoundDataMap &parameters )
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

		if( insideEditBlock() )
		{
			m_attributeStack.top().editShaderGroup( *m_mainAssembly, m_editHandler->exactScopeName() );
		}
	}
	else
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::shader", format( "Unknown shader type \"%s\"." ) % type );
	}
}

void IECoreAppleseed::RendererImplementation::light( const string &name, const string &handle, const CompoundDataMap &parameters )
{
	if( !m_lightHandler.get() )
	{
		msg( MessageHandler::Error ,"IECoreAppleseed::RendererImplementation::light", "Light specified before worldBegin." );
		return;
	}

	const char *unprefixedName = name.c_str();
	if( name.find( ':' ) != string::npos )
	{
		if( boost::starts_with( name, "as:" ) )
		{
			unprefixedName += 3;
		}
		else
		{
			return;
		}
	}

	// check if the light is an environment light.
	if( algorithm::ends_with( unprefixedName, "_environment_edf" ) )
	{
		const string &lightName = m_attributeStack.top().name();
		const string *environmentEDFName = getOptionAs<string>( "as:environment_edf" );

		// ignore other environment lights if one has been specified
		// using the as:environment_edf option.
		if( environmentEDFName && *environmentEDFName != lightName )
		{
			return;
		}

		// ignore extra environment lights if we already have one.
		if( !insideEditBlock() && !m_project->get_scene()->environment_edfs().empty() )
		{
			return;
		}

		// ignore edits for extra environment lights.
		if( insideEditBlock() )
		{
			if( !m_project->get_scene()->environment_edfs().empty() )
			{
				asr::EnvironmentEDF *envLight = m_project->get_scene()->environment_edfs().get_by_index( 0 );

				if( lightName != envLight->get_name() )
				{
					return;
				}
			}
		}

		const bool *envEDFVisible = getOptionAs<bool>( "as:environment_edf_background" );
		m_lightHandler->environment( unprefixedName, handle, envEDFVisible && *envEDFVisible, parameters );
	}
	else
	{
		m_lightHandler->light( unprefixedName, handle,
			m_transformStack.top().get_earliest_transform(), parameters );
	}
}

void IECoreAppleseed::RendererImplementation::illuminate( const string &lightHandle, bool on )
{
	if( !m_lightHandler.get() )
	{
		msg( MessageHandler::Error ,"IECoreAppleseed::RendererImplementation::light", "illuminate called before worldBegin." );
		return;
	}

	m_lightHandler->illuminate( lightHandle, on );
}

/////////////////////////////////////////////////////////////////////////////////////////
// motion blur
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::motionBegin( const set<float> &times )
{
	if (insideMotionBlock() )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::motionBegin", "No matching motionEnd() call." );
		return;
	}

	m_motionHandler->motionBegin( times );
}

void IECoreAppleseed::RendererImplementation::motionEnd()
{
	if (!insideMotionBlock() )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::motionEnd", "No matching motionBegin() call." );
		return;
	}

	m_motionHandler->motionEnd( m_attributeStack.top(), m_mainAssembly );
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::points( size_t numPoints, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::points", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::disk( float radius, float z, float thetaMax, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::disk", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::curves( const CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::curves", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::text( const string &font, const string &text, float kerning, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::text", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::sphere( float radius, float zMin, float zMax, float thetaMax, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::sphere", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::image( const Box2i &dataWindow, const Box2i &displayWindow, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::image", "Not implemented." );
}

void IECoreAppleseed::RendererImplementation::mesh( ConstIntVectorDataPtr vertsPerFace, ConstIntVectorDataPtr vertIds, const string &interpolation, const PrimitiveVariableMap &primVars )
{
	if( !m_mainAssembly )
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation", "Geometry not inside world block, ignoring." );
		return;
	}

	MeshPrimitivePtr mesh = new MeshPrimitive( vertsPerFace, vertIds, interpolation );
	mesh->variables = primVars;

	string materialName = currentMaterialName();

	if( insideMotionBlock() )
	{
		m_motionHandler->primitive( mesh, materialName );
	}
	else
	{
		if( const asr::Assembly *assembly = m_primitiveConverter->convertPrimitive( mesh, m_attributeStack.top(), materialName, *m_mainAssembly ) )
		{
			createAssemblyInstance( assembly->get_name() );
		}
	}
}

void IECoreAppleseed::RendererImplementation::nurbs( int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const PrimitiveVariableMap &primVars )
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

void IECoreAppleseed::RendererImplementation::procedural( Renderer::ProceduralPtr proc )
{
	// appleseed does not support procedurals yet, so we expand them immediately.
	proc->render( this );
}

/////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::instanceBegin( const string &name, const CompoundDataMap &parameters )
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

DataPtr IECoreAppleseed::RendererImplementation::command( const string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::command", "Not implemented." );
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// rerendering
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreAppleseed::RendererImplementation::editBegin( const string &editType, const CompoundDataMap &parameters )
{
	if( isEditable() )
	{
		m_transformStack.clear();

		if( editType == "option" )
		{
			// Option edits begin with no transform in place.
			m_transformStack.pop();
		}

		// Clear attribute stack.
		while ( m_attributeStack.size() )
			m_attributeStack.pop();

		m_attributeStack.push( AttributeState() );

		m_editHandler->editBegin( editType, parameters );
	}
	else
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::editBegin", "Non editable render." );
	}
}

void IECoreAppleseed::RendererImplementation::editEnd()
{
	if( isEditable() )
	{
		m_editHandler->editEnd();
	}
	else
	{
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::editEnd", "Non editable render." );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// private
/////////////////////////////////////////////////////////////////////////////////////////

bool IECoreAppleseed::RendererImplementation::isProjectGen() const
{
	return !m_fileName.empty();
}

bool IECoreAppleseed::RendererImplementation::isEditable() const
{
	return m_editHandler.get();
}

void IECoreAppleseed::RendererImplementation::setCamera( const string &name, CameraPtr cortexCamera,
	asf::auto_release_ptr<asr::Camera> &appleseedCamera )
{
	appleseedCamera->set_name( name.c_str() );
	m_project->get_scene()->cameras().clear();
	m_project->get_scene()->cameras().insert( appleseedCamera );
	m_project->get_frame()->get_parameters().insert( "camera", name.c_str() );

	// resolution
	const V2iData *resolution = cortexCamera->parametersData()->member<V2iData>( "resolution" );
	asf::Vector2i res( resolution->readable().x, resolution->readable().y );
	m_project->get_frame()->get_parameters().insert( "resolution", res );

	// replace the frame by an updated one.
	// this is needed when doing interactive rendering
	asr::ParamArray params = m_project->get_frame()->get_parameters();
	m_project->set_frame( asr::FrameFactory().create( "beauty", params ) );

	// crop window
	const Box2fData *cropWindow = cortexCamera->parametersData()->member<Box2fData>( "cropWindow" );
	asf::AABB2u crop;
	crop.min[0] = (int)(cropWindow->readable().min.x * ( res[0] - 1 ));
	crop.min[1] = (int)(cropWindow->readable().min.y * ( res[1] - 1 ));
	crop.max[0] = (int)(cropWindow->readable().max.x * ( res[0] - 1 ));
	crop.max[1] = (int)(cropWindow->readable().max.y * ( res[1] - 1 ));
	m_project->get_frame()->set_crop_window( crop );
}

string IECoreAppleseed::RendererImplementation::currentShaderGroupName()
{
	if( m_attributeStack.top().shadingStateValid() )
	{
		return m_attributeStack.top().createShaderGroup( *m_mainAssembly );
	}

	return string();
}

string IECoreAppleseed::RendererImplementation::currentMaterialName()
{
	if( m_attributeStack.top().shadingStateValid() )
	{
		string ShaderGroupName = currentShaderGroupName();
		return m_attributeStack.top().createMaterial( *m_mainAssembly, ShaderGroupName );
	}

	return string();
}

void IECoreAppleseed::RendererImplementation::createAssemblyInstance( const string &assemblyName )
{
	string assemblyInstanceName = m_attributeStack.top().name() + "_assembly_instance";

	asr::ParamArray params;

	if( !m_attributeStack.top().visibilityDictionary().empty() )
	{
		params.insert( "visibility", m_attributeStack.top().visibilityDictionary() );
	}

	asf::auto_release_ptr<asr::AssemblyInstance> assemblyInstance = asr::AssemblyInstanceFactory::create( assemblyInstanceName.c_str(), params, assemblyName.c_str() );

	assemblyInstance->transform_sequence() = m_transformStack.top();
	EntityAlgo::insertEntityWithUniqueName( m_mainAssembly->assembly_instances(), assemblyInstance, assemblyInstanceName );
}

bool RendererImplementation::insideMotionBlock() const
{
	return m_motionHandler->insideMotionBlock();
}

bool RendererImplementation::insideEditBlock() const
{
	return m_editHandler.get() && m_editHandler->insideEditBlock();
}

asr::Project *IECoreAppleseed::RendererImplementation::appleseedProject() const
{
	return m_project.get();
}
