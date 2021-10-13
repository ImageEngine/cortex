//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

#include "IECoreGL/Renderer.h"

#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/CurvesPrimitive.h"
#include "IECoreGL/DiskPrimitive.h"
#include "IECoreGL/Font.h"
#include "IECoreGL/FontLoader.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/LuminanceTexture.h"
#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/PointsPrimitive.h"
#include "IECoreGL/QuadPrimitive.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/SpherePrimitive.h"
#include "IECoreGL/State.h"
#include "IECoreGL/TextPrimitive.h"
#include "IECoreGL/TextureLoader.h"
#include "IECoreGL/ToGLCameraConverter.h"
#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/private/DeferredRendererImplementation.h"
#include "IECoreGL/private/Display.h"
#include "IECoreGL/private/ImmediateRendererImplementation.h"

#include "IECoreScene/Camera.h"
#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/MeshNormalsOp.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/Transform.h"

#include "IECore/BoxOps.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/SplineData.h"

#include "OpenEXR/ImathBoxAlgo.h"

#include <mutex>
#include <stack>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreGL;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( IECoreGL::Renderer );

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// static utility functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
typename T::ConstPtr castWithWarning( ConstDataPtr data, const std::string &name, const std::string &context )
{
	typename T::ConstPtr c = runTimeCast<const T>( data );
	if( !c )
	{
		msg( Msg::Warning, context, boost::format( "Expected \"%s\" to be of type \"%s\"." ) % name % T::staticTypeName() );
	}
	return c;
}

template<typename T>
T parameterValue( const char *name, const CompoundDataMap &parameters, T defaultValue )
{
	CompoundDataMap::const_iterator it = parameters.find( name );
	if( it!=parameters.end() )
	{
		typename TypedData<T>::ConstPtr p = runTimeCast<const TypedData<T> >( it->second );
		if( p )
		{
			return p->readable();
		}
	}
	return defaultValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// member data held in a single structure
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/// \todo Now we're adding methods to this, we should probably rename it to Impl
/// or Implementation. We should perhaps also rename the RendererImplementation
/// classes to RendererBackend or something to avoid confusion.
struct IECoreGL::Renderer::MemberData
{
	enum Mode
	{
		Immediate,
		Deferred
	};

	struct
	{
		Mode mode;
		V2f shutter;
		IECore::CompoundDataMap user;
		string fontSearchPath;
		string fontSearchPathDefault;
		string shaderSearchPath;
		string shaderSearchPathDefault;
		string shaderIncludePath;
		string shaderIncludePathDefault;
		string textureSearchPath;
		string textureSearchPathDefault;
		vector<CameraPtr> cameras;
		vector<DisplayPtr> displays;
		bool drawCoordinateSystems;
	} options;

	/// This is used only before worldBegin, so we can correctly get the transforms for cameras.
	/// After worldBegin the transform stack is taken care of by the backend implementations.
	std::stack<Imath::M44f> transformStack;

	bool inWorld;
	bool inEdit;
	RendererImplementationPtr implementation;
	ShaderLoaderPtr shaderLoader;
	TextureLoaderPtr textureLoader;
#ifdef IECORE_WITH_FREETYPE
	FontLoaderPtr fontLoader;
#endif // IECORE_WITH_FREETYPE

	typedef std::map<std::string, GroupPtr> InstanceMap;
	InstanceMap instances;
	Group *currentInstance;

	CachedConverterPtr cachedConverter;

	// we don't want to just destroy objects in the removeObject command, as we could be in
	// any thread at the time, and we can only destroy gl resources on the thread which has
	// the gl context. so we stash them in here until the editEnd command, and then destroy
	// them. the implication is therefore that editEnd must be called on the main gl thread,
	// but that procedurals are free to call removeObject regardless of which thread they're
	// being called from.
	std::set<RenderablePtr> removedObjects;
	std::mutex removedObjectsMutex;

	void addPrimitive( const IECoreScene::Primitive *corePrimitive )
	{
		ConstPrimitivePtr glPrimitive;
		if( implementation->getState<AutomaticInstancingStateComponent>()->value() )
		{
			glPrimitive = IECore::runTimeCast<const Primitive>( cachedConverter->convert( corePrimitive ) );
		}
		else
		{
			ToGLConverterPtr converter = ToGLConverter::create( corePrimitive, IECoreGL::Primitive::staticTypeId() );
			glPrimitive = IECore::runTimeCast<const Primitive>( converter->convert() );
		}

		addPrimitive( glPrimitive );
	}

	void addPrimitive( IECoreGL::ConstPrimitivePtr glPrimitive )
	{
		if( currentInstance )
		{
			addCurrentInstanceChild( glPrimitive );
		}
		else if( checkCulling( glPrimitive->bound() ) )
		{
			implementation->addPrimitive( glPrimitive );
		}
	}

	void addCurrentInstanceChild( IECoreGL::ConstRenderablePtr child )
	{
		IECoreGL::GroupPtr childGroup = new IECoreGL::Group();
		childGroup->setTransform( transformStack.top() );
		/// \todo See todo in DeferredRendererImplementation::addPrimitive().
		childGroup->addChild( boost::const_pointer_cast<Renderable>( child ) );
		currentInstance->addChild( childGroup );
	}

	bool checkCulling( const Imath::Box3f &bound )
	{
		const Imath::Box3f &cullBox = implementation->getState<CullingBoxStateComponent>()->value();
		if( cullBox.isEmpty() )
		{
			// culling is disabled... p should be rendered.
			return true;
		}

		if( bound == Procedural::noBound )
		{
			return true;
		}

		Imath::Box3f b = bound;
		switch( implementation->getState<CullingSpaceStateComponent>()->value() )
		{
			case ObjectSpace :
				// if in local space we don't have to transform bounding box of p.
				break;
			case WorldSpace :
				// transform procedural bounding box to world space to match culling box space.
				b = Imath::transform( b, implementation->getTransform() );
				break;
			default :
				msg( Msg::Warning, "Renderer::checkCulling", "Unnexpected culling space!" );
				return true;
		}
		return cullBox.intersects( b );
	}

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// structors
/////////////////////////////////////////////////////////////////////////////////////////////////////////

IECoreGL::Renderer::Renderer()
{
	m_data = new MemberData;

	m_data->options.mode = MemberData::Immediate;
	m_data->options.shutter = V2f( 0 );

	const char *fontPath = getenv( "IECORE_FONT_PATHS" );
	m_data->options.fontSearchPath = m_data->options.fontSearchPathDefault = fontPath ? fontPath : "";
	const char *shaderPath = getenv( "IECOREGL_SHADER_PATHS" );
	m_data->options.shaderSearchPath = m_data->options.shaderSearchPathDefault = shaderPath ? shaderPath : "";
	const char *shaderIncludePath = getenv( "IECOREGL_SHADER_INCLUDE_PATHS" );
	m_data->options.shaderIncludePath = m_data->options.shaderIncludePathDefault = shaderIncludePath ? shaderIncludePath : "";
	const char *texturePath = getenv( "IECOREGL_TEXTURE_PATHS" );
	m_data->options.textureSearchPath = m_data->options.textureSearchPathDefault = texturePath ? texturePath : "";
	m_data->options.drawCoordinateSystems = false;

	m_data->transformStack.push( M44f() );

	m_data->inWorld = false;
	m_data->inEdit = false;
	m_data->currentInstance = nullptr;
	m_data->implementation = nullptr;
	m_data->shaderLoader = nullptr;

	m_data->cachedConverter = CachedConverter::defaultCachedConverter();
}

IECoreGL::Renderer::~Renderer()
{
	delete m_data;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// options etc
/////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*OptionSetter)( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData );
typedef std::map<string, OptionSetter> OptionSetterMap;

typedef IECore::DataPtr (*OptionGetter)( const std::string &name, IECoreGL::Renderer::MemberData *memberData );
typedef std::map<string, OptionGetter> OptionGetterMap;

static void modeOptionSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	if( ConstStringDataPtr s = castWithWarning<StringData>( value, name, "Renderer::setOption" ) )
	{
		if( s->readable()=="immediate" )
		{
			memberData->options.mode = IECoreGL::Renderer::MemberData::Immediate;
		}
		else if( s->readable()=="deferred" )
		{
			memberData->options.mode = IECoreGL::Renderer::MemberData::Deferred;
		}
		else
		{
			msg( Msg::Warning, "Renderer::setOption", boost::format( "Unsuppported mode value \"%s\"." ) % s->readable() );
		}
	}
	return;
}

static IECore::DataPtr modeOptionGetter( const std::string &name, IECoreGL::Renderer::MemberData *memberData )
{
	switch( memberData->options.mode )
	{
		case IECoreGL::Renderer::MemberData::Immediate :
			return new StringData( "immediate" );
		default :
			return new StringData( "deferred" );
	}
}

static void shutterOptionSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	if( ConstV2fDataPtr s = castWithWarning<V2fData>( value, name, "Renderer::setOption" ) )
	{
		memberData->options.shutter = s->readable();
	}
}

static IECore::DataPtr shutterOptionGetter( const std::string &name, IECoreGL::Renderer::MemberData *memberData )
{
	return new V2fData( memberData->options.shutter );
}

static void fontSearchPathOptionSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	if( ConstStringDataPtr s = castWithWarning<StringData>( value, name, "Renderer::setOption" ) )
	{
		memberData->options.fontSearchPath = s->readable();
	}
}

static IECore::DataPtr fontSearchPathOptionGetter( const std::string &name, IECoreGL::Renderer::MemberData *memberData )
{
	return new StringData( memberData->options.fontSearchPath );
}

static void shaderSearchPathOptionSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	if( ConstStringDataPtr s = castWithWarning<StringData>( value, name, "Renderer::setOption" ) )
	{
		memberData->options.shaderSearchPath = s->readable();
	}
}

static IECore::DataPtr shaderSearchPathOptionGetter( const std::string &name, IECoreGL::Renderer::MemberData *memberData )
{
	return new StringData( memberData->options.shaderSearchPath );
}

static void shaderIncludePathOptionSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	if( ConstStringDataPtr s = castWithWarning<StringData>( value, name, "Renderer::setOption" ) )
	{
		memberData->options.shaderIncludePath = s->readable();
	}
}

static IECore::DataPtr shaderIncludePathOptionGetter( const std::string &name, IECoreGL::Renderer::MemberData *memberData )
{
	return new StringData( memberData->options.shaderIncludePath );
}

static void textureSearchPathOptionSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	if( ConstStringDataPtr s = castWithWarning<StringData>( value, name, "Renderer::setOption" ) )
	{
		memberData->options.textureSearchPath = s->readable();
	}
}

static IECore::DataPtr textureSearchPathOptionGetter( const std::string &name, IECoreGL::Renderer::MemberData *memberData )
{
	return new StringData( memberData->options.textureSearchPath );
}

static void drawCoordinateSystemsOptionSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	if( ConstBoolDataPtr b = castWithWarning<BoolData>( value, name, "Renderer::setOption" ) )
	{
		memberData->options.drawCoordinateSystems = b->readable();
	}
}

static IECore::DataPtr drawCoordinateSystemsOptionGetter( const std::string &name, IECoreGL::Renderer::MemberData *memberData )
{
	return new BoolData( memberData->options.drawCoordinateSystems );
}

static const OptionSetterMap *optionSetters()
{
	static OptionSetterMap *o = new OptionSetterMap;
	if( !o->size() )
	{
		(*o)["gl:mode"] = modeOptionSetter;
		(*o)["shutter"] = shutterOptionSetter;
		(*o)["searchPath:font"] = fontSearchPathOptionSetter;
		(*o)["gl:searchPath:shader"] = shaderSearchPathOptionSetter;
		(*o)["searchPath:shader"] = shaderSearchPathOptionSetter;
		(*o)["gl:searchPath:shaderInclude"] = shaderIncludePathOptionSetter;
		(*o)["searchPath:shaderInclude"] = shaderIncludePathOptionSetter;
		(*o)["gl:searchPath:texture"] = textureSearchPathOptionSetter;
		(*o)["searchPath:texture"] = textureSearchPathOptionSetter;
		(*o)["gl:drawCoordinateSystems"] = drawCoordinateSystemsOptionSetter;
	}
	return o;
}

static const OptionGetterMap *optionGetters()
{
	static OptionGetterMap *o = new OptionGetterMap;
	if( !o->size() )
	{
		(*o)["gl:mode"] = modeOptionGetter;
		(*o)["shutter"] = shutterOptionGetter;
		(*o)["searchPath:font"] = fontSearchPathOptionGetter;
		(*o)["gl:searchPath:shader"] = shaderSearchPathOptionGetter;
		(*o)["searchPath:shader"] = shaderSearchPathOptionGetter;
		(*o)["gl:searchPath:shaderInclude"] = shaderIncludePathOptionGetter;
		(*o)["searchPath:shaderInclude"] = shaderIncludePathOptionGetter;
		(*o)["gl:searchPath:texture"] = textureSearchPathOptionGetter;
		(*o)["searchPath:texture"] = textureSearchPathOptionGetter;
		(*o)["gl:drawCoordinateSystems"] = drawCoordinateSystemsOptionGetter;
	}
	return o;
}

void IECoreGL::Renderer::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	if( m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::setOption", "Cannot call setOption after worldBegin()." );
		return;
	}

	const OptionSetterMap *o = optionSetters();
	OptionSetterMap::const_iterator it = o->find( name );
	if( it!=o->end() )
	{
		it->second( name, value, m_data );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		m_data->options.user[name] = value->copy();
	}
	else if( name.compare( 0, 3, "gl:" )==0 || name.find( ':' )==string::npos )
	{
		msg( Msg::Warning, "Renderer::setOption", boost::format( "Unsuppported option \"%s\"." ) % name );
		return;
	}
}

IECore::ConstDataPtr IECoreGL::Renderer::getOption( const std::string &name ) const
{
	const OptionGetterMap *o = optionGetters();
	OptionGetterMap::const_iterator it = o->find( name );
	if( it!=o->end() )
	{
		return it->second( name, m_data );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		IECore::CompoundDataMap::const_iterator it = m_data->options.user.find( name );
		if( it!=m_data->options.user.end() )
		{
			return it->second;
		}
		else
		{
			return nullptr;
		}
	}
	else if( name.compare( 0, 3, "gl:" )==0 || name.find( ':' )==string::npos )
	{
		msg( Msg::Warning, "Renderer::getOption", boost::format( "Unsuppported option \"%s\"." ) % name );
		return nullptr;
	}

	return nullptr;
}


void IECoreGL::Renderer::camera( const std::string &unusedName, const IECore::CompoundDataMap &parameters )
{
	if( m_data->inWorld )
	{
		msg( Msg::Warning, "IECoreGL::Renderer::camera", "Cameras can not be specified after worldBegin." );
		return;
	}
	if ( m_data->currentInstance )
	{
		msg( Msg::Warning, "IECoreGL::Renderer::camera", "Cameras can not be specified during instance definition." );
		return;
	}

	try
	{
		IECoreScene::CameraPtr coreCamera = new IECoreScene::Camera( new CompoundData( parameters ) );

		IECoreGL::CameraPtr camera = IECore::runTimeCast<IECoreGL::Camera>( ToGLCameraConverter( coreCamera ).convert() );

		// TODO delete this
		// Cortex cameras are now driven by physical parameters, not a screenWindow
		// But for compatibility with the deprecated GL stuff, if someone tries to override the computed
		// frustum, then stomp that straight onto the GL camera
		/*auto screenWindowParm = parameters.find( "screenWindow" );
		if( screenWindowParm != parameters.end() )
		{
			Box2fData *screenWindowData = runTimeCast< Box2fData >( screenWindowParm->second.get() );
			if( screenWindowData )
			{
				camera->setNormalizedScreenWindow( screenWindowData->readable() );
			}
		}*/

		// we have to store these till worldBegin, as only then are we sure what sort of renderer backend we have
		if( camera )
		{
			camera->setTransform( m_data->transformStack.top() );
			m_data->options.cameras.push_back( camera );
		}
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "IECoreGL::Renderer::camera", e.what() );
		return;
	}
}


void IECoreGL::Renderer::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	// we store displays till worldbegin, as until that point we don't have a renderer implementation to pass
	// them to
	if( m_data->inWorld )
	{
		msg( Msg::Warning, "IECoreGL::Renderer::display", "Displays can not be specified after worldBegin." );
		return;
	}
	if ( m_data->currentInstance )
	{
		msg( Msg::Warning, "IECoreGL::Renderer::display", "Displays can not be specified during instance definition." );
		return;
	}
	m_data->options.displays.push_back( new Display( name, type, data, parameters ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// world begin/end
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void IECoreGL::Renderer::worldBegin()
{
	if( m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::worldBegin", "Cannot call worldBegin() again before worldEnd()." );
		return;
	}
	if ( m_data->currentInstance )
	{
		msg( Msg::Warning, "IECoreGL::Renderer::worldBegin", "worldBegin can not be called during instance definition." );
		return;
	}

	m_data->inWorld = true;

	if( m_data->options.mode==MemberData::Deferred )
	{
		m_data->implementation = new DeferredRendererImplementation;
	}
	else
	{
		m_data->implementation = new ImmediateRendererImplementation;
	}

	if( m_data->options.shaderSearchPath==m_data->options.shaderSearchPathDefault && m_data->options.shaderIncludePath==m_data->options.shaderIncludePathDefault )
	{
		// use the shared default cache if we can
		m_data->shaderLoader = ShaderLoader::defaultShaderLoader();
	}
	else
	{
		IECore::SearchPath includePaths( m_data->options.shaderIncludePath );
		m_data->shaderLoader = new ShaderLoader( IECore::SearchPath( m_data->options.shaderSearchPath ), &includePaths );
	}

	if( m_data->options.textureSearchPath==m_data->options.textureSearchPathDefault )
	{
		// use the shared default cache if we can
		m_data->textureLoader = TextureLoader::defaultTextureLoader();
	}
	else
	{
		m_data->textureLoader = new TextureLoader( IECore::SearchPath( m_data->options.textureSearchPath ) );
	}

#ifdef IECORE_WITH_FREETYPE
	if( m_data->options.fontSearchPath==m_data->options.fontSearchPathDefault )
	{
		// use the shared default cache if we can
		m_data->fontLoader = FontLoader::defaultFontLoader();
	}
	else
	{
		m_data->fontLoader = new FontLoader( IECore::SearchPath( m_data->options.fontSearchPath ) );
	}
#endif // IECORE_WITH_FREETYPE

	if( m_data->options.cameras.size() )
	{
		for( unsigned int i=0; i<m_data->options.cameras.size(); i++ )
		{
			m_data->implementation->addCamera( m_data->options.cameras[i] );
		}
	}
	else
	{
		// specify the default camera
		IECoreScene::ConstCameraPtr defaultCamera = new IECoreScene::Camera();
		IECoreGL::CameraPtr camera = IECore::runTimeCast<IECoreGL::Camera>( ToGLCameraConverter( defaultCamera ).convert() );
		m_data->implementation->addCamera( camera );
	}

	for( unsigned int i=0; i<m_data->options.displays.size(); i++ )
	{
		m_data->implementation->addDisplay( m_data->options.displays[i] );
	}
	m_data->implementation->worldBegin();

	ShaderStateComponentPtr defaultShaderState = new ShaderStateComponent( m_data->shaderLoader, m_data->textureLoader, "", "", "", new CompoundObject );
	m_data->implementation->addState( defaultShaderState );
}

void IECoreGL::Renderer::worldEnd()
{
	if( !m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::worldEnd", "Cannot call worldEnd() before worldBegin()." );
		return;
	}
	if ( m_data->currentInstance )
	{
		msg( Msg::Warning, "IECoreGL::Renderer::worldEnd", "worldEnd can not be called during instance definition." );
		return;
	}
	m_data->implementation->worldEnd();
	m_data->inWorld = false;
	m_data->cachedConverter->clearUnused();
}

ScenePtr IECoreGL::Renderer::scene()
{
	DeferredRendererImplementationPtr r = runTimeCast<DeferredRendererImplementation>( m_data->implementation );
	if( r )
	{
		return r->scene();
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void IECoreGL::Renderer::transformBegin()
{
	if( m_data->inWorld )
	{
		m_data->implementation->transformBegin();
	}
	else
	{
		m_data->transformStack.push( m_data->transformStack.top() );
	}
}

void IECoreGL::Renderer::transformEnd()
{
	if( m_data->inWorld )
	{
		bool wasRight = ( determinant( m_data->implementation->getTransform() ) >= 0 );
		m_data->implementation->transformEnd();
		bool isRight = ( determinant( m_data->implementation->getTransform() ) >= 0 );

		if ( wasRight != isRight )
		{
			bool l = m_data->implementation->getState<RightHandedOrientationStateComponent>()->value();
			m_data->implementation->addState( new RightHandedOrientationStateComponent( !l ) );
		}
	}
	else
	{
		if( m_data->transformStack.size() )
		{
			m_data->transformStack.pop();
		}
		else
		{
			msg( Msg::Error, "IECoreGL::Renderer::transformEnd", "Bad nesting detected." );
		}
	}
}

void IECoreGL::Renderer::setTransform( const Imath::M44f &m )
{
	if( m_data->inWorld )
	{
		m_data->implementation->setTransform( m );

		if( determinant( m ) < 0.0f )
		{
			bool l = m_data->implementation->getState<RightHandedOrientationStateComponent>()->value();
			m_data->implementation->addState( new RightHandedOrientationStateComponent( !l ) );
		}
	}
	else
	{
		m_data->transformStack.top() = m;
	}
}

void IECoreGL::Renderer::setTransform( const std::string &coordinateSystem )
{
	msg( Msg::Warning, "Renderer::setTransform", "Not implemented" );
}

Imath::M44f IECoreGL::Renderer::getTransform() const
{
	if( m_data->inWorld )
	{
		return m_data->implementation->getTransform();
	}
	else
	{
		return m_data->transformStack.top();
	}
}

Imath::M44f IECoreGL::Renderer::getTransform( const std::string &coordinateSystem ) const
{
	msg( Msg::Warning, "Renderer::getTransform", "Not implemented" );
	return M44f();
}

void IECoreGL::Renderer::concatTransform( const Imath::M44f &m )
{
	if( m_data->inWorld )
	{
		m_data->implementation->concatTransform( m );
		if( determinant( m ) < 0.0f )
		{
			bool l = m_data->implementation->getState<RightHandedOrientationStateComponent>()->value();
			m_data->implementation->addState( new RightHandedOrientationStateComponent( !l ) );
		}
	}
	else
	{
		m_data->transformStack.top() = m * m_data->transformStack.top();
	}
}

void IECoreGL::Renderer::coordinateSystem( const std::string &name )
{
	if( m_data->options.drawCoordinateSystems )
	{
		IntVectorDataPtr numVerticesData = new IntVectorData;
		std::vector<int> &numVertices = numVerticesData->writable();
		numVertices.push_back( 2 );
		numVertices.push_back( 2 );
		numVertices.push_back( 2 );

		V3fVectorDataPtr pointsData = new V3fVectorData();
		std::vector<V3f> &points = pointsData->writable();
		points.push_back( V3f( 0 ) );
		points.push_back( V3f( 1, 0, 0 ) );
		points.push_back( V3f( 0 ) );
		points.push_back( V3f( 0, 1, 0 ) );
		points.push_back( V3f( 0 ) );
		points.push_back( V3f( 0, 0, 1 ) );

		PrimitiveVariableMap primVars;
		primVars["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, pointsData );

		attributeBegin();
			setAttribute( "name", new StringData( "coordinateSystem:" + name ) );
			setAttribute( "gl:curvesPrimitive:useGLLines", new BoolData( true ) );
			setAttribute( "gl:curvesPrimitive:glLineWidth", new FloatData( 2 ) );
			curves( CubicBasisf::linear(), false, numVerticesData, primVars );
		attributeEnd();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// attribute state
/////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*AttributeSetter)( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData );
typedef std::map<string, AttributeSetter> AttributeSetterMap;
typedef IECore::ConstDataPtr (*AttributeGetter)( const std::string &name, const IECoreGL::Renderer::MemberData *memberData );
typedef std::map<string, AttributeGetter> AttributeGetterMap;

template<class T>
static void typedAttributeSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	typedef IECore::TypedData<typename T::ValueType> DataType;
	typename DataType::ConstPtr d = runTimeCast<const DataType>( value );
	if( !d )
	{
		msg( Msg::Warning, "Renderer::setAttribute", boost::format( "Expected data of type \"%s\" for attribute \"%s\"." ) % DataType::staticTypeName() % name );
		return;
	}
	memberData->implementation->addState( new T( d->readable() ) );
}

template<class T>
static IECore::ConstDataPtr typedAttributeGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	typedef IECore::TypedData<typename T::ValueType> DataType;
	const T *a = memberData->implementation->template getState<T>();
	return new DataType( a->value() );
}

static void colorAttributeSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstColor3fDataPtr d = castWithWarning<const Color3fData>( value, name, "Renderer::setAttribute" );
	if( d )
	{
		Color::ConstPtr c = memberData->implementation->getState<Color>();
		Color4f cc = c->value();
		cc[0] = d->readable()[0];
		cc[1] = d->readable()[1];
		cc[2] = d->readable()[2];
		memberData->implementation->addState( new Color( cc ) );
	}
}

static IECore::ConstDataPtr colorAttributeGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	const IECoreGL::Color *a = memberData->implementation->getState<Color>();
	Color4f c = a->value();
	return new Color3fData( Color3f( c[0], c[1], c[2] ) );
}

static IECore::ConstDataPtr opacityAttributeGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	const IECoreGL::Color *a = memberData->implementation->getState<Color>();
	Color4f c = a->value();
	return new Color3fData( Color3f( c[3] ) );
}

static void opacityAttributeSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstColor3fDataPtr d = castWithWarning<const Color3fData>( value, name, "Renderer::setAttribute" );
	if( d )
	{
		const Color *c = memberData->implementation->getState<Color>();
		Color4f cc = c->value();
		cc[3] = (d->readable()[0] + d->readable()[1] + d->readable()[2]) / 3.0f;
		memberData->implementation->addState( new Color( cc ) );
	}
}

static IECore::ConstDataPtr blendFactorGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	const BlendFuncStateComponent *b = memberData->implementation->getState<BlendFuncStateComponent>();
	GLenum f = name=="gl:blend:srcFactor" ? b->value().src : b->value().dst;
	switch( f )
	{
		case GL_ZERO :
			return new StringData( "zero" );
		case GL_ONE :
			return new StringData( "one" );
		case GL_SRC_COLOR :
			return new StringData( "srcColor" );
		case GL_ONE_MINUS_SRC_COLOR :
			return new StringData( "oneMinusSrcColor" );
		case GL_DST_COLOR :
			return new StringData( "dstColor" );
		case GL_ONE_MINUS_DST_COLOR :
			return new StringData( "oneMinusDstColor" );
		case GL_SRC_ALPHA :
			return new StringData( "srcAlpha" );
		case GL_ONE_MINUS_SRC_ALPHA :
			return new StringData( "oneMinusSrcAlpha" );
		case GL_DST_ALPHA :
			return new StringData( "dstAlpha" );
		case GL_ONE_MINUS_DST_ALPHA :
			return new StringData( "oneMinusDstAlpha" );
		case GL_CONSTANT_COLOR :
			return new StringData( "constantColor" );
		case GL_ONE_MINUS_CONSTANT_COLOR :
			return new StringData( "oneMinusConstantColor" );
		case GL_CONSTANT_ALPHA :
			return new StringData( "constantAlpha" );
		case GL_ONE_MINUS_CONSTANT_ALPHA :
			return new StringData( "oneMinusConstantAlpha" );
		default :
			msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Invalid state for \"%s\"." ) % name );
			return new StringData( "invalid" );
	}
}

static void blendFactorSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstStringDataPtr d = castWithWarning<const StringData>( value, name, "Renderer::setAttribute" );
	if( !d )
	{
		return;
	}

	GLenum f;
	const std::string &v = d->readable();
	if( v=="zero" )
	{
		f = GL_ZERO;
	}
	else if( v=="one" )
	{
		f = GL_ONE;
	}
	else if( v=="srcColor" )
	{
		f = GL_SRC_COLOR;
	}
	else if( v=="oneMinusSrcColor" )
	{
		f = GL_ONE_MINUS_SRC_COLOR;
	}
	else if( v=="dstColor" )
	{
		f = GL_DST_COLOR;
	}
	else if( v=="oneMinusDstColor" )
	{
		f = GL_ONE_MINUS_DST_COLOR;
	}
	else if( v=="srcAlpha" )
	{
		f = GL_SRC_ALPHA;
	}
	else if( v=="oneMinusSrcAlpha" )
	{
		f = GL_ONE_MINUS_SRC_ALPHA;
	}
	else if( v=="dstAlpha" )
	{
		f = GL_DST_ALPHA;
	}
	else if( v=="oneMinusDstAlpha" )
	{
		f = GL_ONE_MINUS_DST_ALPHA;
	}
	else if( v=="constantColor" )
	{
		f = GL_CONSTANT_COLOR;
	}
	else if( v=="oneMinusConstantColor" )
	{
		f = GL_ONE_MINUS_CONSTANT_COLOR;
	}
	else if( v=="constantAlpha" )
	{
		f = GL_CONSTANT_ALPHA;
	}
	else if( v=="oneMinusConstantAlpha" )
	{
		f = GL_ONE_MINUS_CONSTANT_ALPHA;
	}
	else
	{
		msg( Msg::Error, "Renderer::setAttribute", boost::format( "Unsupported value \"%s\" for attribute \"%s\"." ) % v % name );
		return;
	}
	const BlendFuncStateComponent *b = memberData->implementation->getState<BlendFuncStateComponent>();
	BlendFactors bf = b->value();
	if( name=="gl:blend:srcFactor" )
	{
		bf.src = f;
	}
	else
	{
		bf.dst = f;
	}
	memberData->implementation->addState( new BlendFuncStateComponent( bf ) );
}

static void alphaFuncSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{

	const AlphaFuncStateComponent *a = memberData->implementation->getState<AlphaFuncStateComponent>();
	AlphaFunc af = a->value();

	if( name == "gl:alphaTest:mode" )
	{
		ConstStringDataPtr d = castWithWarning<const StringData>( value, name, "Renderer::setAttribute" );
		if( !d )
		{
			return;
		}

		GLenum m;
		const std::string &v = d->readable();
		if( v=="never" )
		{
			m = GL_NEVER;
		}
		else if( v=="less" )
		{
			m = GL_LESS;
		}
		else if( v=="equal" )
		{
			m = GL_EQUAL;
		}
		else if( v=="lequal" )
		{
			m = GL_LEQUAL;
		}
		else if( v=="greater" )
		{
			m = GL_GREATER;
		}
		else if( v=="notequal" )
		{
			m = GL_NOTEQUAL;
		}
		else if( v=="gequal" )
		{
			m = GL_GEQUAL;
		}
		else if( v=="always" )
		{
			m = GL_ALWAYS;
		}
		else
		{
			msg( Msg::Error, "Renderer::setAttribute", boost::format( "Unsupported value \"%s\" for attribute \"%s\"." ) % v % name );
			return;
		}
		af.mode = m;
	}
	else if( name == "gl:alphaTest:value" )
	{
		ConstFloatDataPtr d = castWithWarning<const FloatData>( value, name, "Renderer::setAttribute" );
		if( !d )
		{
			return;
		}
		af.value = d->readable();
	}
	else
	{
		return;
	}

	memberData->implementation->addState( new AlphaFuncStateComponent( af ) );
}

static IECore::ConstDataPtr alphaFuncGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	const AlphaFuncStateComponent *b = memberData->implementation->getState<AlphaFuncStateComponent>();

	if( name == "gl:alphaTest:mode" )
	{
		GLenum m = b->value().mode;
		switch( m )
		{
			case GL_NEVER:
				return new StringData( "never" );
			case GL_LESS:
				return new StringData( "less" );
			case GL_EQUAL:
				return new StringData( "equal" );
			case GL_LEQUAL:
				return new StringData( "lequal" );
			case GL_GREATER:
				return new StringData( "greater" );
			case GL_NOTEQUAL:
				return new StringData( "notequal" );
			case GL_GEQUAL:
				return new StringData( "gequal" );
			case GL_ALWAYS:
				return new StringData( "always" );
			default :
				msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Invalid state for \"%s\"." ) % name );
				return new StringData( "invalid" );
		}
	}
	else if( name == "gl:alphaTest:value" )
	{
		return new FloatData( b->value().value );
	}

	msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Invalid state for \"%s\"." ) % name );
	return nullptr;
}

static IECore::ConstDataPtr blendEquationGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	const BlendEquationStateComponent *b = memberData->implementation->getState<BlendEquationStateComponent>();
	switch( b->value() )
	{
		case GL_FUNC_ADD :
			return new StringData( "add" );
		case GL_FUNC_SUBTRACT :
			return new StringData( "subtract" );
		case GL_FUNC_REVERSE_SUBTRACT :
			return new StringData( "reverseSubtract" );
		case GL_MIN :
			return new StringData( "min" );
		case GL_MAX :
			return new StringData( "max" );
		default :
			msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Invalid state for \"%s\"." ) % name );
			return new StringData( "invalid" );
	}
}

static void blendEquationSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstStringDataPtr d = castWithWarning<const StringData>( value, name, "Renderer::setAttribute" );
	if( !d )
	{
		return;
	}

	GLenum f;
	const std::string &v = d->readable();
	if( v=="add" )
	{
		f = GL_FUNC_ADD;
	}
	else if( v=="subtract" )
	{
		f = GL_FUNC_SUBTRACT;
	}
	else if( v=="reverseSubtract" )
	{
		f = GL_FUNC_REVERSE_SUBTRACT;
	}
	else if( v=="min" )
	{
		f = GL_MIN;
	}
	else if( v=="max" )
	{
		f = GL_MAX;
	}
	else
	{
		msg( Msg::Error, "Renderer::setAttribute", boost::format( "Unsupported value \"%s\" for attribute \"%s\"." ) % v % name );
		return;
	}

	memberData->implementation->addState( new BlendEquationStateComponent( f ) );
}

static IECore::ConstDataPtr pointsPrimitiveUseGLPointsGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	const IECoreGL::PointsPrimitive::UseGLPoints *b = memberData->implementation->getState<IECoreGL::PointsPrimitive::UseGLPoints>();
	switch( b->value() )
	{
		case ForPointsOnly :
			return new StringData( "forGLPoints" );
		case ForPointsAndDisks :
			return new StringData( "forParticlesAndDisks" );
		case ForAll :
			return new StringData( "forAll" );
		default :
			msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Invalid state for \"%s\"." ) % name );
			return new StringData( "invalid" );
	}

}

static void pointsPrimitiveUseGLPointsSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstStringDataPtr d = castWithWarning<const StringData>( value, name, "Renderer::setAttribute" );
	if( !d )
	{
		return;
	}
	GLPointsUsage u;
	const std::string &v = d->readable();
	if( v=="forGLPoints" )
	{
		u = ForPointsOnly;
	}
	else if( v=="forParticlesAndDisks" )
	{
		u = ForPointsAndDisks;
	}
	else if( v=="forAll" )
	{
		u = ForAll;
	}
	else
	{
		msg( Msg::Error, "Renderer::setAttribute", boost::format( "Unsupported value \"%s\" for attribute \"%s\"." ) % v % name );
		return;
	}
	memberData->implementation->addState( new IECoreGL::PointsPrimitive::UseGLPoints( u ) );
}

static IECore::ConstDataPtr nameGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	const NameStateComponent *n = memberData->implementation->getState<NameStateComponent>();
	return new StringData( n->name() );
}

static void nameSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstStringDataPtr d = castWithWarning<const StringData>( value, name, "Renderer::setAttribute" );
	if( !d )
	{
		return;
	}
	memberData->implementation->addState( new NameStateComponent( d->readable() ) );
}

static IECore::ConstDataPtr textPrimitiveTypeGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{

#ifdef IECORE_WITH_FREETYPE

	const TextPrimitive::Type *b = memberData->implementation->getState<TextPrimitive::Type>();
	switch( b->value() )
	{
		case TextPrimitive::Mesh :
			return new StringData( "mesh" );
		case TextPrimitive::Sprite :
			return new StringData( "sprite" );
		default :
			msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Invalid state for \"%s\"." ) % name );
			return new StringData( "invalid" );
	}

#else

	IECore::msg( IECore::Msg::Warning, "Renderer::getAttribute", "IECore was not built with FreeType support." );
	return 0;

#endif // IECORE_WITH_FREETYPE

}

static void textPrimitiveTypeSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{

#ifdef IECORE_WITH_FREETYPE

	ConstStringDataPtr d = castWithWarning<const StringData>( value, name, "Renderer::setAttribute" );
	if( !d )
	{
		return;
	}
	TextPrimitive::RenderType t;
	const std::string &v = d->readable();
	if( v=="mesh" )
	{
		t = TextPrimitive::Mesh;
	}
	else if( v=="sprite" )
	{
		t = TextPrimitive::Sprite;
	}
	else
	{
		msg( Msg::Error, "Renderer::setAttribute", boost::format( "Unsupported value \"%s\" for attribute \"%s\"." ) % v % name );
		return;
	}
	memberData->implementation->addState( new TextPrimitive::Type( t ) );

#else

	IECore::msg( IECore::Msg::Warning, "Renderer::setAttribute", "IECore was not built with FreeType support." );

#endif // IECORE_WITH_FREETYPE

}

template<class T>
static IECore::ConstDataPtr rendererSpaceGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	typename T::ConstPtr b = memberData->implementation->getState< T >();
	switch( b->value() )
	{
		case ObjectSpace :
			return new StringData( "object" );
		case WorldSpace :
			return new StringData( "world" );
		default :
			msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Invalid state for \"%s\"." ) % name );
			return new StringData( "invalid" );
	}
}

template<class T>
static void rendererSpaceSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstStringDataPtr d = castWithWarning<const StringData>( value, name, "Renderer::setAttribute" );
	if( !d )
	{
		return;
	}
	RendererSpace s;
	const std::string &v = d->readable();
	if( v=="object" )
	{
		s = ObjectSpace;
	}
	else if ( v == "world" )
	{
		s = WorldSpace;
	}
	else
	{
		msg( Msg::Error, "Renderer::setAttribute", boost::format( "Unsupported value \"%s\" for attribute \"%s\"." ) % v % name );
		return;
	}
	memberData->implementation->addState( new T( s ) );
}

static const AttributeSetterMap *attributeSetters()
{
	static AttributeSetterMap *a = new AttributeSetterMap;
	if( !a->size() )
	{
		(*a)["gl:primitive:wireframe"] = typedAttributeSetter<IECoreGL::Primitive::DrawWireframe>;
		(*a)["gl:primitive:wireframeWidth"] = typedAttributeSetter<IECoreGL::Primitive::WireframeWidth>;
		(*a)["gl:primitive:bound"] = typedAttributeSetter<IECoreGL::Primitive::DrawBound>;
		(*a)["gl:primitive:solid"] = typedAttributeSetter<IECoreGL::Primitive::DrawSolid>;
		(*a)["gl:primitive:outline"] = typedAttributeSetter<IECoreGL::Primitive::DrawOutline>;
		(*a)["gl:primitive:outlineWidth"] = typedAttributeSetter<IECoreGL::Primitive::OutlineWidth>;
		(*a)["gl:primitive:points"] = typedAttributeSetter<IECoreGL::Primitive::DrawPoints>;
		(*a)["gl:primitive:pointWidth"] = typedAttributeSetter<IECoreGL::Primitive::PointWidth>;
		(*a)["gl:primitive:sortForTransparency"] = typedAttributeSetter<IECoreGL::Primitive::TransparencySort>;
		(*a)["gl:primitive:wireframeColor"] = typedAttributeSetter<WireframeColorStateComponent>;
		(*a)["gl:primitive:boundColor"] = typedAttributeSetter<BoundColorStateComponent>;
		(*a)["gl:primitive:outlineColor"] = typedAttributeSetter<OutlineColorStateComponent>;
		(*a)["gl:primitive:pointColor"] = typedAttributeSetter<PointColorStateComponent>;
		(*a)["gl:primitive:selectable"] = typedAttributeSetter<IECoreGL::Primitive::Selectable>;
		(*a)["gl:color"] = typedAttributeSetter<Color>;
		(*a)["color"] = colorAttributeSetter;
		(*a)["opacity"] = opacityAttributeSetter;
		(*a)["gl:blend:color"] = typedAttributeSetter<BlendColorStateComponent>;
		(*a)["gl:blend:srcFactor"] = blendFactorSetter;
		(*a)["gl:blend:dstFactor"] = blendFactorSetter;
		(*a)["gl:blend:equation"] = blendEquationSetter;
		(*a)["gl:shade:transparent"] = typedAttributeSetter<TransparentShadingStateComponent>;
		(*a)["gl:pointsPrimitive:useGLPoints"] = pointsPrimitiveUseGLPointsSetter;
		(*a)["gl:pointsPrimitive:glPointWidth"] = typedAttributeSetter<IECoreGL::PointsPrimitive::GLPointWidth>;
		(*a)["name"] = nameSetter;
		(*a)["doubleSided"] = typedAttributeSetter<DoubleSidedStateComponent>;
		(*a)["rightHandedOrientation"] = typedAttributeSetter<RightHandedOrientationStateComponent>;
		(*a)["gl:curvesPrimitive:useGLLines"] = typedAttributeSetter<IECoreGL::CurvesPrimitive::UseGLLines>;
		(*a)["gl:curvesPrimitive:glLineWidth"] = typedAttributeSetter<IECoreGL::CurvesPrimitive::GLLineWidth>;
		(*a)["gl:curvesPrimitive:ignoreBasis"] = typedAttributeSetter<IECoreGL::CurvesPrimitive::IgnoreBasis>;
		(*a)["gl:smoothing:points"] = typedAttributeSetter<PointSmoothingStateComponent>;
		(*a)["gl:smoothing:lines"] = typedAttributeSetter<LineSmoothingStateComponent>;
		(*a)["gl:smoothing:polygons"] = typedAttributeSetter<PolygonSmoothingStateComponent>;
		(*a)["gl:textPrimitive:type"] = textPrimitiveTypeSetter;
		(*a)["gl:cullingSpace"] = rendererSpaceSetter<CullingSpaceStateComponent>;
		(*a)["gl:cullingBox"] = typedAttributeSetter<CullingBoxStateComponent>;
		(*a)["gl:procedural:reentrant"] = typedAttributeSetter<ProceduralThreadingStateComponent>;
		(*a)["gl:visibility:camera"] = typedAttributeSetter<CameraVisibilityStateComponent>;
		(*a)["gl:depthTest"] = typedAttributeSetter<DepthTestStateComponent>;
		(*a)["gl:depthMask"] = typedAttributeSetter<DepthMaskStateComponent>;
		(*a)["gl:alphaTest"] = typedAttributeSetter<AlphaTestStateComponent>;
		(*a)["gl:alphaTest:mode"] = alphaFuncSetter;
		(*a)["gl:alphaTest:value"] = alphaFuncSetter;
		(*a)["automaticInstancing"] = typedAttributeSetter<AutomaticInstancingStateComponent>;
		(*a)["gl:automaticInstancing"] = typedAttributeSetter<AutomaticInstancingStateComponent>;
	}
	return a;
}

static const AttributeGetterMap *attributeGetters()
{
	static AttributeGetterMap *a = new AttributeGetterMap;
	if( !a->size() )
	{
		(*a)["gl:primitive:wireframe"] = typedAttributeGetter<IECoreGL::Primitive::DrawWireframe>;
		(*a)["gl:primitive:wireframeWidth"] = typedAttributeGetter<IECoreGL::Primitive::WireframeWidth>;
		(*a)["gl:primitive:bound"] = typedAttributeGetter<IECoreGL::Primitive::DrawBound>;
		(*a)["gl:primitive:solid"] = typedAttributeGetter<IECoreGL::Primitive::DrawSolid>;
		(*a)["gl:primitive:outline"] = typedAttributeGetter<IECoreGL::Primitive::DrawOutline>;
		(*a)["gl:primitive:outlineWidth"] = typedAttributeGetter<IECoreGL::Primitive::OutlineWidth>;
		(*a)["gl:primitive:points"] = typedAttributeGetter<IECoreGL::Primitive::DrawPoints>;
		(*a)["gl:primitive:pointWidth"] = typedAttributeGetter<IECoreGL::Primitive::PointWidth>;
		(*a)["gl:primitive:sortForTransparency"] = typedAttributeGetter<IECoreGL::Primitive::TransparencySort>;
		(*a)["gl:primitive:wireframeColor"] = typedAttributeGetter<WireframeColorStateComponent>;
		(*a)["gl:primitive:boundColor"] = typedAttributeGetter<BoundColorStateComponent>;
		(*a)["gl:primitive:outlineColor"] = typedAttributeGetter<OutlineColorStateComponent>;
		(*a)["gl:primitive:pointColor"] = typedAttributeGetter<PointColorStateComponent>;
		(*a)["gl:primitive:selectable"] = typedAttributeGetter<IECoreGL::Primitive::Selectable>;
		(*a)["gl:color"] = typedAttributeGetter<Color>;
		(*a)["color"] = colorAttributeGetter;
		(*a)["opacity"] = opacityAttributeGetter;
		(*a)["gl:blend:color"] = typedAttributeGetter<BlendColorStateComponent>;
		(*a)["gl:blend:srcFactor"] = blendFactorGetter;
		(*a)["gl:blend:dstFactor"] = blendFactorGetter;
		(*a)["gl:blend:equation"] = blendEquationGetter;
		(*a)["gl:shade:transparent"] = typedAttributeGetter<TransparentShadingStateComponent>;
		(*a)["gl:pointsPrimitive:useGLPoints"] = pointsPrimitiveUseGLPointsGetter;
		(*a)["gl:pointsPrimitive:glPointWidth"] = typedAttributeGetter<IECoreGL::PointsPrimitive::GLPointWidth>;
		(*a)["name"] = nameGetter;
		(*a)["doubleSided"] = typedAttributeGetter<DoubleSidedStateComponent>;
		(*a)["rightHandedOrientation"] = typedAttributeGetter<RightHandedOrientationStateComponent>;
		(*a)["gl:curvesPrimitive:useGLLines"] = typedAttributeGetter<IECoreGL::CurvesPrimitive::UseGLLines>;
		(*a)["gl:curvesPrimitive:glLineWidth"] = typedAttributeGetter<IECoreGL::CurvesPrimitive::GLLineWidth>;
		(*a)["gl:curvesPrimitive:ignoreBasis"] = typedAttributeGetter<IECoreGL::CurvesPrimitive::IgnoreBasis>;
		(*a)["gl:smoothing:points"] = typedAttributeGetter<PointSmoothingStateComponent>;
		(*a)["gl:smoothing:lines"] = typedAttributeGetter<LineSmoothingStateComponent>;
		(*a)["gl:smoothing:polygons"] = typedAttributeGetter<PolygonSmoothingStateComponent>;
		(*a)["gl:textPrimitive:type"] = textPrimitiveTypeGetter;
		(*a)["gl:cullingSpace"] = rendererSpaceGetter<CullingSpaceStateComponent>;
		(*a)["gl:cullingBox"] = typedAttributeGetter<CullingBoxStateComponent>;
		(*a)["gl:procedural:reentrant"] = typedAttributeGetter<ProceduralThreadingStateComponent>;
		(*a)["gl:visibility:camera"] = typedAttributeGetter<CameraVisibilityStateComponent>;
		(*a)["gl:depthTest"] = typedAttributeGetter<DepthTestStateComponent>;
		(*a)["gl:depthMask"] = typedAttributeGetter<DepthMaskStateComponent>;
		(*a)["gl:alphaTest"] = typedAttributeGetter<AlphaTestStateComponent>;
		(*a)["gl:alphaTest:mode"] = alphaFuncGetter;
		(*a)["gl:alphaTest:value"] = alphaFuncGetter;
		(*a)["automaticInstancing"] = typedAttributeGetter<AutomaticInstancingStateComponent>;
		(*a)["gl:automaticInstancing"] = typedAttributeGetter<AutomaticInstancingStateComponent>;
	}
	return a;
}

void IECoreGL::Renderer::attributeBegin()
{
	if ( !m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::attributeBegin", "Unsupported attributeBegin outside world begin/end blocks." );
		return;
	}
	m_data->implementation->attributeBegin();
}

void IECoreGL::Renderer::attributeEnd()
{
	if ( !m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::attributeBegin", "Unsupported attributeBegin outside world begin/end blocks." );
		return;
	}
	m_data->implementation->attributeEnd();
}

void IECoreGL::Renderer::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	if ( !m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::setAttribute", "Unsupported setAttribute outside world begin/end blocks." );
		return;
	}
	const AttributeSetterMap *s = attributeSetters();
	AttributeSetterMap::const_iterator it = s->find( name );
	if( it!=s->end() )
	{
		it->second( name, value, m_data );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		m_data->implementation->addUserAttribute( name, value->copy() );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// prefixed for some other renderer, so we can ignore it
	}
	else
	{
		msg( Msg::Warning, "Renderer::setAttribute", boost::format( "Unsupported attribute \"%s\"." ) % name );
	}
}

IECore::ConstDataPtr IECoreGL::Renderer::getAttribute( const std::string &name ) const
{
	if ( !m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::getAttribute", "Unsupported getAttribute outside world begin/end blocks." );
		return nullptr;
	}

	const AttributeGetterMap *g = attributeGetters();
	AttributeGetterMap::const_iterator it = g->find( name );
	if( it!=g->end() )
	{
		return it->second( name, m_data );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		return m_data->implementation->getUserAttribute( name );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// prefixed for some other renderer, so we can ignore it
		return nullptr;
	}
	else
	{
		msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Unsupported attribute \"%s\"." ) % name );
	}
	return nullptr;
}

void IECoreGL::Renderer::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if ( !m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::shader", "Unsupported shader call outside world begin/end blocks." );
		return;
	}

	if( type=="surface" || type=="gl:surface" )
	{
		string vertexSource = parameterValue<string>( "gl:vertexSource", parameters, "" );
		string geometrySource = parameterValue<string>( "gl:geometrySource", parameters, "" );
		string fragmentSource = parameterValue<string>( "gl:fragmentSource", parameters, "" );

		if( vertexSource == "" && geometrySource == "" && fragmentSource == "" )
		{
			m_data->shaderLoader->loadSource( name, vertexSource, geometrySource, fragmentSource );
		}

		CompoundObjectPtr parametersData = new CompoundObject;
		for( CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
		{
			if( it->first!="gl:fragmentSource" && it->first!="gl:geometrySource" && it->first!="gl:vertexSource" )
			{
				parametersData->members()[it->first] = it->second;
			}
		}

		ShaderStateComponentPtr shaderState = new ShaderStateComponent( m_data->shaderLoader, m_data->textureLoader, vertexSource, geometrySource, fragmentSource, parametersData );
		m_data->implementation->addState( shaderState );
	}
	else if( type.find( "gl:" ) == 0 || type.find_first_of( ":" ) == string::npos )
	{
		msg( Msg::Warning, "Renderer::shader", boost::format( "Unsupported shader type \"%s\"." ) % type );
	}
}

void IECoreGL::Renderer::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "Renderer::light", "Not implemented" );
}

void IECoreGL::Renderer::illuminate( const std::string &lightHandle, bool on )
{
	msg( Msg::Warning, "Renderer::illuminate", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// motion
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void IECoreGL::Renderer::motionBegin( const std::set<float> &times )
{
	msg( Msg::Warning, "Renderer::motionBegin", "Not implemented" );
}

void IECoreGL::Renderer::motionEnd()
{
	msg( Msg::Warning, "Renderer::motionEnd", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addPrimVarsToPrimitive( IECoreGL::PrimitivePtr primitive, const IECoreScene::PrimitiveVariableMap &primVars )
{
	// add primVars to the gl primitive
	for( IECoreScene::PrimitiveVariableMap::const_iterator it=primVars.begin(); it!=primVars.end(); it++ )
	{
		try
		{
			primitive->addPrimitiveVariable( it->first, it->second );
		}
		catch( const std::exception &e )
		{
			IECore::msg( IECore::Msg::Error, "Renderer::addPrimitive", boost::format( "Failed to add primitive variable %s (%s)." ) % it->first % e.what() );
		}
	}
}

void IECoreGL::Renderer::points( size_t numPoints, const IECoreScene::PrimitiveVariableMap &primVars )
{
	try
	{
		IECoreScene::PointsPrimitivePtr p = new IECoreScene::PointsPrimitive( numPoints );
		p->variables = primVars;
		m_data->addPrimitive( p.get() );
	}
	catch( const std::exception &e )
	{
		msg( Msg::Warning, "Renderer::points", e.what() );
		return;
	}
}

void IECoreGL::Renderer::disk( float radius, float z, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars )
{
	DiskPrimitivePtr prim = new DiskPrimitive( radius, z, thetaMax );
	addPrimVarsToPrimitive( prim, primVars );
	m_data->addPrimitive( prim );
}

void IECoreGL::Renderer::curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECoreScene::PrimitiveVariableMap &primVars )
{
	try
	{
		IECoreScene::CurvesPrimitivePtr c = new IECoreScene::CurvesPrimitive( numVertices, basis, periodic );
		c->variables = primVars;
		m_data->addPrimitive( c.get() );
	}
	catch( const std::exception &e )
	{
		msg( Msg::Warning, "Renderer::curves", e.what() );
		return;
	}
}

void IECoreGL::Renderer::text( const std::string &font, const std::string &text, float kerning, const IECoreScene::PrimitiveVariableMap &primVars )
{

#ifdef IECORE_WITH_FREETYPE
	FontPtr f = m_data->fontLoader->load( font );

	if( !f )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::text", boost::format( "Font \"%s\" not found." ) % font );
		return;
	}

	f->coreFont()->setKerning( kerning );

	TextPrimitivePtr prim = new TextPrimitive( text, f );
	addPrimVarsToPrimitive( prim, primVars );
	m_data->addPrimitive( prim );
#else
	IECore::msg( IECore::Msg::Warning, "Renderer::text", "IECore was not built with FreeType support." );
#endif // IECORE_WITH_FREETYPE
}

void IECoreGL::Renderer::sphere( float radius, float zMin, float zMax, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars )
{
	SpherePrimitivePtr prim = new SpherePrimitive( radius, zMin, zMax, thetaMax );
	addPrimVarsToPrimitive( prim, primVars );
	m_data->addPrimitive( prim );
}

static const std::string &imageFragmentShader()
{
	// fragment shader
	static const std::string shaderCode =
		"uniform sampler2D texture;"
		""
		"varying vec2 fragmentuv;"
		""
		"void main()"
		"{"
		"	gl_FragColor = texture2D( texture, fragmentuv );"
		"}";
	return shaderCode;
}

/// \todo This positions images incorrectly when dataWindow!=displayWindow. This is because the texture
/// contains only the dataWindow contents, but we've positioned the card as if it will contain the whole
/// displayWindow.
void IECoreGL::Renderer::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECoreScene::PrimitiveVariableMap &primVars )
{
	if ( m_data->currentInstance )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::image", "Images currently not supported inside instances." );
		return;
	}

	IECoreImage::ImagePrimitivePtr image = new IECoreImage::ImagePrimitive( dataWindow, displayWindow );
	Imath::V3f boxMin( displayWindow.min.x, displayWindow.min.y, 0.0 );
	Imath::V3f boxMax( 1.0f + displayWindow.max.x, 1.0f + displayWindow.max.y, 0.0 );
	Imath::V3f center = (boxMin + boxMax) / 2.0;
	Imath::Box3f bound( boxMin - center, boxMax - center );

	if( !m_data->checkCulling( bound ) )
	{
		return;
	}

	for( const auto &primVar : primVars )
	{
		if(
			primVar.second.interpolation == PrimitiveVariable::Vertex ||
			primVar.second.interpolation == PrimitiveVariable::Varying ||
			primVar.second.interpolation == PrimitiveVariable::FaceVarying
		)
		{
			image->channels[primVar.first] = primVar.second.data;
		}
	}

	IECore::CompoundObjectPtr params = new IECore::CompoundObject();
	params->members()[ "texture" ] = image;

	ShaderStateComponentPtr shaderState = new ShaderStateComponent( m_data->shaderLoader, m_data->textureLoader, "", "", imageFragmentShader(), params );

	m_data->implementation->transformBegin();

		M44f xform;
		xform[3][0] = center.x;
		xform[3][1] = center.y;
		xform[3][2] = center.z;

		xform[0][0] = boxSize( bound ).x ;
		xform[1][1] = boxSize( bound ).y ;
		xform[2][2] = 1.0;

		m_data->implementation->concatTransform( xform );
		m_data->implementation->attributeBegin();
		m_data->implementation->addState( shaderState );
		QuadPrimitivePtr quad = new QuadPrimitive( 1.0, 1.0 );
		m_data->implementation->addPrimitive( quad );
		m_data->implementation->attributeEnd();

	m_data->implementation->transformEnd();
}

void IECoreGL::Renderer::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECoreScene::PrimitiveVariableMap &primVars )
{
	try
	{
		IECoreScene::MeshPrimitivePtr m = new IECoreScene::MeshPrimitive;
		IECoreScene::PrimitiveVariableMap::const_iterator it = primVars.find( "P" );
		if( it == primVars.end() )
		{
			throw IECore::Exception( "Trying to render a mesh without \"P\"" );
		}

		IECore::V3fVectorDataPtr pData = runTimeCast< IECore::V3fVectorData >( it->second.data );
		if( !pData )
		{
			throw IECore::Exception( "Mesh \"P\" variable has incorrect type" );
		}

		m->setTopologyUnchecked( vertsPerFace, vertIds, pData->readable().size(), interpolation );
		m->variables = primVars;
		m_data->addPrimitive( m.get() );
	}
	catch( const std::exception &e )
	{
		msg( Msg::Warning, "Renderer::mesh", e.what() );
		return;
	}
}

void IECoreGL::Renderer::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECoreScene::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::nurbs", "Not implemented" );
}

void IECoreGL::Renderer::patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECoreScene::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::patchMesh", "Not implemented" );
}

void IECoreGL::Renderer::geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECoreScene::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::geometry", boost::format( "Geometry type \"%s\" not implemented." ) % type );
}

void IECoreGL::Renderer::procedural( IECoreScene::Renderer::ProceduralPtr proc )
{
	if ( m_data->currentInstance )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::procedural", "Procedurals currently not supported inside instances." );
		return;
	}
	if ( m_data->checkCulling( proc->bound() ) )
	{
		if( ExternalProcedural *externalProcedural = dynamic_cast<ExternalProcedural *>( proc.get() ) )
		{
			attributeBegin();
				setAttribute( "gl:primitive:wireframe", new BoolData( true ) );
				setAttribute( "gl:primitive:solid", new BoolData( false ) );
				setAttribute( "gl:curvesPrimitive:useGLLines", new BoolData( true ) );
				IECoreScene::CurvesPrimitive::createBox( externalProcedural->bound() )->render( this );
			attributeEnd();
		}
		else
		{
			m_data->implementation->addProcedural( proc, this );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void IECoreGL::Renderer::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if ( m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::instanceBegin", "Unsupported instanceBegin call after worldBegin." );
		return;
	}
	if ( m_data->currentInstance )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::instanceBegin", "Instance already being defined!" );
		return;
	}
	MemberData::InstanceMap::const_iterator it = m_data->instances.find( name );
	if ( it != m_data->instances.end() )
	{
		msg( Msg::Warning, "Renderer::instance", boost::format( "Overwriting instance named \"%s\"." ) % name );
		return;
	}
	m_data->currentInstance = new Group();
	m_data->instances[ name ] = m_data->currentInstance;
}

void IECoreGL::Renderer::instanceEnd()
{
	if ( m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::instanceEnd", "Unsupported instanceEnd call after worldBegin." );
		return;
	}
	if ( !m_data->currentInstance )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::instanceEnd", "instanceEnd called when no instances are being defined!" );
		return;
	}
	m_data->currentInstance = nullptr;
}

void IECoreGL::Renderer::instance( const std::string &name )
{
	MemberData::InstanceMap::iterator it = m_data->instances.find( name );
	if ( it == m_data->instances.end() )
	{
		msg( Msg::Warning, "Renderer::instance", boost::format( "No instance named \"%s\" was found." ) % name );
		return;
	}
	if ( m_data->currentInstance )
	{
		// instance called within another instance
		m_data->addCurrentInstanceChild( it->second );
	}
	else if ( m_data->inWorld )
	{
		m_data->implementation->addInstance( it->second );
	}
	else
	{
		msg( Msg::Warning, "Renderer::instance", "Unsupported call to instance outside world and instance block!" );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef IECore::DataPtr (*Command)( const std::string &name, const IECore::CompoundDataMap &parameters, IECoreGL::Renderer::MemberData *memberData );
typedef std::map<string, Command> CommandMap;

bool removeObjectWalk( IECoreGL::GroupPtr parent, IECoreGL::GroupPtr child, const std::string &objectName, IECoreGL::Renderer::MemberData *memberData )
{
	const NameStateComponent *stateName = child->getState()->get<NameStateComponent>();
	if( stateName && stateName->name()==objectName )
	{
		if( parent )
		{
			{
				std::lock_guard<IECoreGL::Group::Mutex> lock( parent->mutex() );
				parent->removeChild( child.get() );
			}
			{
				std::lock_guard<std::mutex> lock2( memberData->removedObjectsMutex );
				memberData->removedObjects.insert( child );
			}
		}
		else
		{
			// no parent, ie we're at the root of the Scene. just remove all the children.
			std::lock_guard<IECoreGL::Group::Mutex> lock( child->mutex() );
			std::lock_guard<std::mutex> lock2( memberData->removedObjectsMutex );
			for( IECoreGL::Group::ChildContainer::const_iterator it=child->children().begin(); it!=child->children().end(); it++ )
			{
				memberData->removedObjects.insert( *it );
			}
			child->clearChildren();
		}
		return true;
	}

	bool result = false;
	std::lock_guard<IECoreGL::Group::Mutex> lock( child->mutex() );
	IECoreGL::Group::ChildContainer::const_iterator it = child->children().begin();
	while( it!=child->children().end() )
	{
		IECoreGL::GroupPtr g = IECore::runTimeCast<IECoreGL::Group>( *it );
		it++;
		if( g )
		{
			result = result | removeObjectWalk( child, g, objectName, memberData );
		}
	}
	if ( result && child->children().size() == 0 && parent )
	{
		// group after removal became empty, remove it too.
		{
			std::lock_guard<IECoreGL::Group::Mutex> lock( parent->mutex() );
			parent->removeChild( child.get() );
		}
		{
			std::lock_guard<std::mutex> lock2( memberData->removedObjectsMutex );
			memberData->removedObjects.insert( child );
		}
	}
	return result;
}

IECore::DataPtr removeObjectCommand( const std::string &name, const IECore::CompoundDataMap &parameters, IECoreGL::Renderer::MemberData *memberData )
{
	DeferredRendererImplementationPtr r = runTimeCast<DeferredRendererImplementation>( memberData->implementation );
	if( !r )
	{
		msg( Msg::Warning, "Renderer::command", "removeObject command operates only in deferred mode" );
		return nullptr;
	}

	if( !memberData->inEdit )
	{
		msg( Msg::Warning, "Renderer::command", "removeObject command operates only within an editBegin/editEnd block" );
		return nullptr;
	}

	string objectName = parameterValue<string>( "name", parameters, "" );
	if( objectName=="" )
	{
		msg( Msg::Warning, "Renderer::command", "removeObject command expects StringData parameter \"name\"" );
		return nullptr;
	}

	ScenePtr scene = r->scene();
	bool result = removeObjectWalk( nullptr, r->scene()->root(), objectName, memberData );

	return new IECore::BoolData( result );
}

IECore::DataPtr editBeginCommand( const std::string &name, const IECore::CompoundDataMap &parameters, IECoreGL::Renderer::MemberData *memberData )
{
	DeferredRendererImplementationPtr r = runTimeCast<DeferredRendererImplementation>( memberData->implementation );
	if( !r )
	{
		msg( Msg::Warning, "Renderer::command", "editBegin command operates only in deferred mode" );
		return nullptr;
	}

	memberData->inWorld = true;
	memberData->inEdit = true;
	return new IECore::BoolData( true );
}

IECore::DataPtr editEndCommand( const std::string &name, const IECore::CompoundDataMap &parameters, IECoreGL::Renderer::MemberData *memberData )
{
	DeferredRendererImplementationPtr r = runTimeCast<DeferredRendererImplementation>( memberData->implementation );
	if( !r )
	{
		msg( Msg::Warning, "Renderer::command", "editEnd command operates only in deferred mode" );
		return nullptr;
	}

	memberData->inWorld = false;
	memberData->inEdit = false;
	// we defer final destruction of objects till now, so we don't destroy gl resources directly in removeObjectCommand.
	// we could be on any thread in removeObjectCommand (it can be called from procedurals) but we require that editEnd
	// is called on the GL thread, so this is therefore the only safe place to do the destruction.
	memberData->removedObjects.clear();
	return new IECore::BoolData( true );
}

IECore::DataPtr editQueryCommand( const std::string &name, const IECore::CompoundDataMap &parameters, IECoreGL::Renderer::MemberData *memberData )
{
	DeferredRendererImplementationPtr r = runTimeCast<DeferredRendererImplementation>( memberData->implementation );
	if( !r )
	{
		msg( Msg::Warning, "Renderer::command", "editQuery command operates only in deferred mode" );
		return nullptr;
	}

	return new IECore::BoolData( memberData->inEdit );
}

static const CommandMap &commands()
{
	static CommandMap c;
	if( !c.size() )
	{
		c["removeObject"] = removeObjectCommand;
		c["editBegin"] = editBeginCommand;
		c["editEnd"] = editEndCommand;
		c["editQuery"] = editQueryCommand;
	}
	return c;
}

IECore::DataPtr IECoreGL::Renderer::command( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if ( m_data->currentInstance )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::command", "Commands not supported inside instances." );
		return nullptr;
	}
	const CommandMap &c = commands();
	CommandMap::const_iterator it = c.find( name );
	if( it!=c.end() )
	{
		return it->second( name, parameters, m_data );
	}

	if( name.compare( 0, 3, "gl:" )==0 || name.find( ':' )==string::npos )
	{
		msg( Msg::Warning, "Renderer::command", boost::format( "Unsuppported command \"%s\"." ) % name );
		return nullptr;
	}

	return nullptr;
}

void IECoreGL::Renderer::editBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "Renderer::editBegin", "Not implemented" );
}

void IECoreGL::Renderer::editEnd()
{
	msg( Msg::Warning, "Renderer::editEnd", "Not implemented" );
}

IECoreGL::ShaderLoader *IECoreGL::Renderer::shaderLoader()
{
	return m_data->shaderLoader.get();
}

IECoreGL::TextureLoader *IECoreGL::Renderer::textureLoader()
{
	return m_data->textureLoader.get();
}
