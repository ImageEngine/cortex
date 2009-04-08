//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
#include "IECoreGL/State.h"
#include "IECoreGL/PointsPrimitive.h"
#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/QuadPrimitive.h"
#include "IECoreGL/SpherePrimitive.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/LuminanceTexture.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/private/DeferredRendererImplementation.h"
#include "IECoreGL/private/ImmediateRendererImplementation.h"
#include "IECoreGL/private/Display.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/TextureLoader.h"
#include "IECoreGL/PerspectiveCamera.h"
#include "IECoreGL/OrthographicCamera.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/ToGLCameraConverter.h"
#include "IECoreGL/CurvesPrimitive.h"
#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/Font.h"
#include "IECoreGL/TextPrimitive.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/BoxOps.h"
#include "IECore/Camera.h"
#include "IECore/Transform.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/MeshNormalsOp.h"
#include "IECore/SplineData.h"
#include "IECore/SplineToImage.h"

#include <stack>

using namespace IECore;
using namespace IECoreGL;
using namespace Imath;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// static utility functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
typename T::ConstPtr findPrimVar( const char *name, PrimitiveVariable::Interpolation interpolation, const PrimitiveVariableMap &primVars )
{
	PrimitiveVariableMap::const_iterator it = primVars.find( name );
	if( it==primVars.end() )
	{
		return 0;
	}
	if( it->second.interpolation!=interpolation )
	{
		return 0;
	}
	return runTimeCast<T>( it->second.data );
}

template<typename T>
typename T::ConstPtr findPrimVar( const char *name, const PrimitiveVariable::Interpolation *interpolation, const PrimitiveVariableMap &primVars )
{
	while( *interpolation!=PrimitiveVariable::Invalid )
	{
		typename T::ConstPtr d = findPrimVar<T>( name, *interpolation, primVars );
		if( d )
		{
			return d;
		}
		interpolation++;
	}
	return 0;
}

template<typename T>
typename T::ConstPtr findPrimVar( const char **names, const PrimitiveVariable::Interpolation *interpolation, const PrimitiveVariableMap &primVars )
{
	while( *names!=0 )
	{
		typename T::ConstPtr d = findPrimVar<T>( *names, interpolation, primVars );
		if( d )
		{
			return d;
		}
		names++;
	}
	return 0;
}

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
		string shaderSearchPath;
		string shaderSearchPathDefault;
		string shaderIncludePath;
		string shaderIncludePathDefault;
		string textureSearchPath;
		string textureSearchPathDefault;
		vector<CameraPtr> cameras;
		vector<DisplayPtr> displays;
	} options;
	
	/// This is used only before worldBegin, so we can correctly get the transforms for cameras.
	/// After worldBegin the transform stack is taken care of by the backend implementations.
	std::stack<Imath::M44f> transformStack;
	
	struct Attributes
	{
		IECore::CompoundDataMap userAttributes;
	};
	std::stack<Attributes> attributeStack;
	
	bool inWorld;
	RendererImplementationPtr implementation;
	ShaderLoaderPtr shaderLoader;
	TextureLoaderPtr textureLoader;
	
#ifdef IECORE_WITH_FREETYPE
	typedef std::map<std::string, FontPtr> FontMap;
	FontMap fonts;
#endif // IECORE_WITH_FREETYPE

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
	m_data->options.fontSearchPath = fontPath ? fontPath : "";
	const char *shaderPath = getenv( "IECOREGL_SHADER_PATHS" );
	m_data->options.shaderSearchPath = m_data->options.shaderSearchPathDefault = shaderPath ? shaderPath : "";
	const char *shaderIncludePath = getenv( "IECOREGL_SHADER_INCLUDE_PATHS" );
	m_data->options.shaderIncludePath = m_data->options.shaderIncludePathDefault = shaderIncludePath ? shaderIncludePath : "";
	const char *texturePath = getenv( "IECOREGL_TEXTURE_PATHS" );
	m_data->options.textureSearchPath = m_data->options.textureSearchPathDefault = texturePath ? texturePath : "";
	
	m_data->transformStack.push( M44f() );
	m_data->attributeStack.push( MemberData::Attributes() );
	
	m_data->inWorld = false;
	m_data->implementation = 0;
	m_data->shaderLoader = 0;
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
			return 0;
		}
	}
	else if( name.compare( 0, 3, "gl:" )==0 || name.find( ':' )==string::npos )
	{
		msg( Msg::Warning, "Renderer::getOption", boost::format( "Unsuppported option \"%s\"." ) % name );
		return 0;
	}
	
	return 0;
}


void IECoreGL::Renderer::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if( m_data->inWorld )
	{
		msg( Msg::Warning, "IECoreGL::Renderer::camera", "Cameras can not be specified after worldBegin." );
		return;
	}
	
	try
	{
		IECore::CameraPtr coreCamera = new IECore::Camera( name, 0, new CompoundData( parameters ) );
		IECoreGL::CameraPtr camera = IECore::runTimeCast<IECoreGL::Camera>( ToGLCameraConverter( coreCamera ).convert() );
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
		IECore::SearchPath includePaths( m_data->options.shaderIncludePath, ":" );
		m_data->shaderLoader = new ShaderLoader( IECore::SearchPath( m_data->options.shaderSearchPath, ":" ), &includePaths );
	}
	
	if( m_data->options.textureSearchPath==m_data->options.textureSearchPathDefault )
	{
		// use the shared default cache if we can
		m_data->textureLoader = TextureLoader::defaultTextureLoader();
	}
	else
	{
		m_data->textureLoader = new TextureLoader( IECore::SearchPath( m_data->options.textureSearchPath, ":" ) );
	}
	
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
		IECore::CameraPtr defaultCamera = new IECore::Camera();
		defaultCamera->addStandardParameters();
		IECoreGL::CameraPtr camera = IECore::runTimeCast<IECoreGL::Camera>( ToGLCameraConverter( defaultCamera ).convert() );
		m_data->implementation->addCamera( camera );
	}
	
	for( unsigned int i=0; i<m_data->options.displays.size(); i++ )
	{
		m_data->implementation->addDisplay( m_data->options.displays[i] );
	}
	m_data->implementation->worldBegin();
}

void IECoreGL::Renderer::worldEnd()
{
	if( !m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::worldEnd", "Cannot call worldEnd() before worldBegin()." );
		return;
	}
	m_data->implementation->worldEnd();
	m_data->inWorld = false;
}

ScenePtr IECoreGL::Renderer::scene()
{
	DeferredRendererImplementationPtr r = runTimeCast<DeferredRendererImplementation>( m_data->implementation );
	if( r )
	{
		return r->scene();
	}
	return 0;
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
		/// \todo We need to reverse the rightHandedOrientation oojamaflip here if the
		/// old transform is flipped relative to the new one. to do that we have to implement
		/// getTransform() properly.
		m_data->implementation->transformEnd();
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
	msg( Msg::Warning, "Renderer::setTransform", "Not implemented" );
}

void IECoreGL::Renderer::setTransform( const std::string &coordinateSystem )
{
	msg( Msg::Warning, "Renderer::setTransform", "Not implemented" );
}

Imath::M44f IECoreGL::Renderer::getTransform() const
{
	msg( Msg::Warning, "Renderer::getTransform", "Not implemented" );
	return M44f();
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
	msg( Msg::Warning, "Renderer::coordinateSystem", "Not implemented" );
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
	typename T::ConstPtr a = memberData->implementation->template getState<T>();
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
	IECoreGL::ConstColorPtr a = memberData->implementation->getState<Color>();
	Color4f c = a->value();
	return new Color3fData( Color3f( c[0], c[1], c[2] ) );
}

static IECore::ConstDataPtr opacityAttributeGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	IECoreGL::ConstColorPtr a = memberData->implementation->getState<Color>();
	Color4f c = a->value();
	return new Color3fData( Color3f( c[3] ) );
}

static void opacityAttributeSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
	ConstColor3fDataPtr d = castWithWarning<const Color3fData>( value, name, "Renderer::setAttribute" );
	if( d )
	{
		Color::ConstPtr c = memberData->implementation->getState<Color>();
		Color4f cc = c->value();
		cc[3] = (d->readable()[0] + d->readable()[1] + d->readable()[2]) / 3.0f;
		memberData->implementation->addState( new Color( cc ) );
	}
}

static IECore::ConstDataPtr blendFactorGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	ConstBlendFuncStateComponentPtr b = memberData->implementation->getState<BlendFuncStateComponent>();
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
	ConstBlendFuncStateComponentPtr b = memberData->implementation->getState<BlendFuncStateComponent>();
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

static IECore::ConstDataPtr blendEquationGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	ConstBlendEquationStateComponentPtr b = memberData->implementation->getState<BlendEquationStateComponent>();
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
	ConstPointsPrimitiveUseGLPointsPtr b = memberData->implementation->getState<PointsPrimitiveUseGLPoints>();
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
	UseGLPoints u;
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
	memberData->implementation->addState( new PointsPrimitiveUseGLPoints( u ) );
}

static IECore::ConstDataPtr nameGetter( const std::string &name, const IECoreGL::Renderer::MemberData *memberData )
{
	ConstNameStateComponentPtr n = memberData->implementation->getState<NameStateComponent>();
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
	TextPrimitive::ConstTypePtr b = memberData->implementation->getState<TextPrimitive::Type>();
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
}

static void textPrimitiveTypeSetter( const std::string &name, IECore::ConstDataPtr value, IECoreGL::Renderer::MemberData *memberData )
{
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
}

static const AttributeSetterMap *attributeSetters()
{
	static AttributeSetterMap *a = new AttributeSetterMap;
	if( !a->size() )
	{
		(*a)["gl:primitive:wireframe"] = typedAttributeSetter<PrimitiveWireframe>;
		(*a)["gl:primitive:wireframeWidth"] = typedAttributeSetter<PrimitiveWireframeWidth>;
		(*a)["gl:primitive:bound"] = typedAttributeSetter<PrimitiveBound>;
		(*a)["gl:primitive:solid"] = typedAttributeSetter<PrimitiveSolid>;
		(*a)["gl:primitive:outline"] = typedAttributeSetter<PrimitiveOutline>;
		(*a)["gl:primitive:outlineWidth"] = typedAttributeSetter<PrimitiveOutlineWidth>;
		(*a)["gl:primitive:points"] = typedAttributeSetter<PrimitivePoints>;
		(*a)["gl:primitive:pointWidth"] = typedAttributeSetter<PrimitivePointWidth>;
		(*a)["gl:primitive:sortForTransparency"] = typedAttributeSetter<PrimitiveTransparencySortStateComponent>;
		(*a)["gl:primitive:wireframeColor"] = typedAttributeSetter<WireframeColorStateComponent>;
		(*a)["gl:primitive:boundColor"] = typedAttributeSetter<BoundColorStateComponent>;
		(*a)["gl:primitive:outlineColor"] = typedAttributeSetter<OutlineColorStateComponent>;
		(*a)["gl:primitive:pointColor"] = typedAttributeSetter<PointColorStateComponent>;
		(*a)["gl:color"] = typedAttributeSetter<Color>;
		(*a)["color"] = colorAttributeSetter;
		(*a)["opacity"] = opacityAttributeSetter;
		(*a)["gl:blend:color"] = typedAttributeSetter<BlendColorStateComponent>;
		(*a)["gl:blend:srcFactor"] = blendFactorSetter;
		(*a)["gl:blend:dstFactor"] = blendFactorSetter;
		(*a)["gl:blend:equation"] = blendEquationSetter;
		(*a)["gl:shade:transparent"] = typedAttributeSetter<TransparentShadingStateComponent>;
		(*a)["gl:pointsPrimitive:useGLPoints"] = pointsPrimitiveUseGLPointsSetter;
		(*a)["gl:pointsPrimitive:glPointWidth"] = typedAttributeSetter<PointsPrimitiveGLPointWidth>;
		(*a)["name"] = nameSetter;
		(*a)["doubleSided"] = typedAttributeSetter<DoubleSidedStateComponent>;
		(*a)["rightHandedOrientation"] = typedAttributeSetter<RightHandedOrientationStateComponent>;
		(*a)["gl:curvesPrimitive:useGLLines"] = typedAttributeSetter<CurvesPrimitive::UseGLLines>;
		(*a)["gl:curvesPrimitive:glLineWidth"] = typedAttributeSetter<CurvesPrimitive::GLLineWidth>;
		(*a)["gl:curvesPrimitive:ignoreBasis"] = typedAttributeSetter<CurvesPrimitive::IgnoreBasis>;
		(*a)["gl:smoothing:points"] = typedAttributeSetter<PointSmoothingStateComponent>;
		(*a)["gl:smoothing:lines"] = typedAttributeSetter<LineSmoothingStateComponent>;
		(*a)["gl:smoothing:polygons"] = typedAttributeSetter<PolygonSmoothingStateComponent>;
		(*a)["gl:textPrimitive:type"] = textPrimitiveTypeSetter;
	}
	return a;
}

static const AttributeGetterMap *attributeGetters()
{
	static AttributeGetterMap *a = new AttributeGetterMap;
	if( !a->size() )
	{
		(*a)["gl:primitive:wireframe"] = typedAttributeGetter<PrimitiveWireframe>;
		(*a)["gl:primitive:wireframeWidth"] = typedAttributeGetter<PrimitiveWireframeWidth>;
		(*a)["gl:primitive:bound"] = typedAttributeGetter<PrimitiveBound>;
		(*a)["gl:primitive:solid"] = typedAttributeGetter<PrimitiveSolid>;
		(*a)["gl:primitive:outline"] = typedAttributeGetter<PrimitiveOutline>;
		(*a)["gl:primitive:outlineWidth"] = typedAttributeGetter<PrimitiveOutlineWidth>;
		(*a)["gl:primitive:points"] = typedAttributeGetter<PrimitivePoints>;
		(*a)["gl:primitive:pointWidth"] = typedAttributeGetter<PrimitivePointWidth>;
		(*a)["gl:primitive:sortForTransparency"] = typedAttributeGetter<PrimitiveTransparencySortStateComponent>;
		(*a)["gl:primitive:wireframeColor"] = typedAttributeGetter<WireframeColorStateComponent>;
		(*a)["gl:primitive:boundColor"] = typedAttributeGetter<BoundColorStateComponent>;
		(*a)["gl:primitive:outlineColor"] = typedAttributeGetter<OutlineColorStateComponent>;
		(*a)["gl:primitive:pointColor"] = typedAttributeGetter<PointColorStateComponent>;
		(*a)["gl:color"] = typedAttributeGetter<Color>;
		(*a)["color"] = colorAttributeGetter;
		(*a)["opacity"] = opacityAttributeGetter;
		(*a)["gl:blend:color"] = typedAttributeGetter<BlendColorStateComponent>;
		(*a)["gl:blend:srcFactor"] = blendFactorGetter;
		(*a)["gl:blend:dstFactor"] = blendFactorGetter;
		(*a)["gl:blend:equation"] = blendEquationGetter;
		(*a)["gl:shade:transparent"] = typedAttributeGetter<TransparentShadingStateComponent>;
		(*a)["gl:pointsPrimitive:useGLPoints"] = pointsPrimitiveUseGLPointsGetter;
		(*a)["gl:pointsPrimitive:glPointWidth"] = typedAttributeGetter<PointsPrimitiveGLPointWidth>;
		(*a)["name"] = nameGetter;
		(*a)["doubleSided"] = typedAttributeGetter<DoubleSidedStateComponent>;
		(*a)["rightHandedOrientation"] = typedAttributeGetter<RightHandedOrientationStateComponent>;
		(*a)["gl:curvesPrimitive:useGLLines"] = typedAttributeGetter<CurvesPrimitive::UseGLLines>;
		(*a)["gl:curvesPrimitive:glLineWidth"] = typedAttributeGetter<CurvesPrimitive::GLLineWidth>;
		(*a)["gl:curvesPrimitive:ignoreBasis"] = typedAttributeGetter<CurvesPrimitive::IgnoreBasis>;
		(*a)["gl:smoothing:points"] = typedAttributeGetter<PointSmoothingStateComponent>;
		(*a)["gl:smoothing:lines"] = typedAttributeGetter<LineSmoothingStateComponent>;
		(*a)["gl:smoothing:polygons"] = typedAttributeGetter<PolygonSmoothingStateComponent>;
		(*a)["gl:textPrimitive:type"] = textPrimitiveTypeGetter;
	}
	return a;
}

void IECoreGL::Renderer::attributeBegin()
{
	m_data->implementation->attributeBegin();
	m_data->attributeStack.push( m_data->attributeStack.top() );
}

void IECoreGL::Renderer::attributeEnd()
{
	if( !m_data->attributeStack.size() )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::Renderer::attributeEnd", "No matching attributeBegin." );
		return;
	}
	m_data->attributeStack.pop();
	m_data->implementation->attributeEnd();
}

void IECoreGL::Renderer::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	const AttributeSetterMap *s = attributeSetters();
	AttributeSetterMap::const_iterator it = s->find( name );
	if( it!=s->end() )
	{
		it->second( name, value, m_data );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		m_data->attributeStack.top().userAttributes[name] = value->copy();
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
	const AttributeGetterMap *g = attributeGetters();
	AttributeGetterMap::const_iterator it = g->find( name );
	if( it!=g->end() )
	{
		return it->second( name, m_data );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		MemberData::Attributes &attributes = m_data->attributeStack.top();
		IECore::CompoundDataMap::const_iterator it = attributes.userAttributes.find( name );
		if( it != attributes.userAttributes.end() )
		{
			return it->second;
		}
		return 0;
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// prefixed for some other renderer, so we can ignore it
		return 0;
	}
	else
	{
		msg( Msg::Warning, "Renderer::getAttribute", boost::format( "Unsupported attribute \"%s\"." ) % name );
	}
	return 0;
}

/// Returns true if the value was added successfully
static bool checkAndAddShaderParameter( ShaderStateComponentPtr shaderState, const std::string &name, const IECore::DataPtr value,
	IECoreGL::Renderer::MemberData *memberData, const std::string &context, bool ignoreMissingParameters )
{
	try
	{
		if( !shaderState->shader()->hasParameter( name ) )
		{
			if( ignoreMissingParameters )
			{
				return false;
			}
			else
			{
				msg( Msg::Error, context, boost::format( "Shader parameter \"%s\" doesn't exist." ) % name );
				return false;
			}
		}
	
		if( value->isInstanceOf( StringData::staticTypeId() ) )
		{
			// should be a texture parameter
			if( shaderState->shader()->parameterType( name )==Texture::staticTypeId() )
			{
				StringDataPtr s = boost::static_pointer_cast<StringData>( value );
				if( s->readable()!="" )
				{
					TexturePtr t = memberData->textureLoader->load( s->readable() );
					if( t )
					{
						shaderState->textureValues()[name] = t;
						return true;
					}
				}
			}
			else
			{
				msg( Msg::Error, context, boost::format( "Shader parameter \"%s\" is not a texture parameter." ) % name );
				return false;
			}
		}
		else if( value->isInstanceOf( SplinefColor3fDataTypeId ) || value->isInstanceOf( SplineffDataTypeId ) )
		{
			// turn splines into textures
			if( shaderState->shader()->parameterType( name )==Texture::staticTypeId() )
			{
				SplineToImagePtr op = new SplineToImage();
				op->splineParameter()->setValue( value );
				op->resolutionParameter()->setTypedValue( V2i( 8, 512 ) );
				ImagePrimitivePtr image = boost::static_pointer_cast<ImagePrimitive>( op->operate() );
								
				TexturePtr texture = 0;
				if( image->variables.find( "R" )!=image->variables.end() )
				{
					texture = new ColorTexture( image );
				}
				else
				{
					texture = new LuminanceTexture( image );
				}
				shaderState->textureValues()[name] = texture;	
			}
			else
			{
				msg( Msg::Error, context, boost::format( "Shader parameter \"%s\" is not a texture parameter." ) % name );
				return false;
			}
		}
		else
		{
			// a standard parameter
			if( shaderState->shader()->valueValid( name, value ) )
			{
				shaderState->parameterValues()->writable()[name] = value;
				return true;
			}
			else
			{
				msg( Msg::Error, context, boost::format( "Invalid value for shader parameter \"%s\"." ) % name );
			}
		}
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, context, boost::format( "Invalid or unsupported name or value for shader parameter \"%s\" (%s)." ) % name % e.what() );
	}
	catch( ... )
	{
		msg( Msg::Error, context, boost::format( "Invalid or unsupported name or value for shader parameter \"%s\"." ) % name );
	}
	return false;
}

void IECoreGL::Renderer::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if( type=="surface" || type=="gl:surface" )
	{
		ShaderPtr s = 0;
		
		string fragSrc = parameterValue<string>( "gl:fragmentSource", parameters, "" );
		string vertSrc = parameterValue<string>( "gl:vertexSource", parameters, "" );
		if( fragSrc!="" || vertSrc!="" )
		{
			// compile from src parameters
			try
			{
				s = new Shader( vertSrc, fragSrc );
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "Renderer::shader", boost::format( "Failed to compile shader \"%s\" (%s)." ) % name % e.what() );
			}
		}
		else
		{
			// load from disk
			try
			{
				s = m_data->shaderLoader->load( name );
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "Renderer::shader", boost::format( "Failed to load shader \"%s\" (%s)." ) % name % e.what() );
			}
			catch( ... )
			{
				msg( Msg::Error, "Renderer::shader", boost::format( "Failed to load shader \"%s\"." ) % name );
			}
		}
		
		if( s )
		{
			// validate the parameter types and load any texture parameters.
			ShaderStateComponentPtr shaderState = new ShaderStateComponent( s, new CompoundData );
			for( CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
			{
				if( it->first!="gl:fragmentSource" && it->first!="gl:vertexSource" )
				{
					checkAndAddShaderParameter( shaderState, it->first, it->second, m_data, "Renderer::shader", false );
				}
			}

			m_data->implementation->addState( shaderState );
		}

	}
	else
	{
		msg( Msg::Warning, "Renderer::shader", boost::format( "Unsupported shader type \"%s\"." ) % type );
	}
}

void IECoreGL::Renderer::light( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "Renderer::light", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// motion 
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void IECoreGL::Renderer::motionBegin( const std::set<float> times )
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

// adds a primitive into the renderer implementation, first extracting any primitive variables that represent
// shader parameters and applying them in the renderer state.
// \todo I broke a lot of const correctness stuff to make this work - in particular I'm not sure it's good for
// ShaderStateComponent to expose it's shader and parameters in non-const form, and i'm not sure RendererImplementation::getState()
// should return non-const data either. When we do varying primvars look into storing the uniform ones on the primitive too,
// and see if that might solve our problem somewhat.
/// \todo the addVertexAttributes is bit of a hack - MeshPrimitives have their own mechanisms for adding vertex attributes to take into
/// account changes of detail from varying->facevarying. 
/// \todo Ditch this entire function. Vertex attributes should be added by the relevant converter classes (like MeshPrimitive does), and the
/// uniform primvar shader overrides should be stored on the primitive and dealt with at draw time.
static void addPrimitive( IECoreGL::PrimitivePtr primitive, const IECore::PrimitiveVariableMap &primVars, IECoreGL::Renderer::MemberData *memberData, bool addVertexAttributes = true )
{
	// add vertex attributes to the primitive if it supports them
	if( addVertexAttributes && primitive->vertexAttributeSize() )
	{
		for( IECore::PrimitiveVariableMap::const_iterator it=primVars.begin(); it!=primVars.end(); it++ )
		{
			if( it->second.interpolation==IECore::PrimitiveVariable::Vertex || it->second.interpolation==IECore::PrimitiveVariable::Varying )
			{
				try
				{
					primitive->addVertexAttribute( it->first, it->second.data );
				}
				catch( const std::exception &e )
				{
					IECore::msg( IECore::Msg::Error, "Renderer::addPrimitive", boost::format( "Failed to add primitive variable (%s)." ) % e.what() );
				}
			}
		}
	}

	ShaderStateComponentPtr ss = memberData->implementation->getState<ShaderStateComponent>();
	if( ss && ss->shader() )
	{
		ShaderStateComponentPtr shaderState = new ShaderStateComponent( ss->shader(), ss->parameterValues(), &ss->textureValues() );
		
		unsigned int parmsAdded = 0;
		for( PrimitiveVariableMap::const_iterator it = primVars.begin(); it!=primVars.end(); it++ )
		{
			if( it->second.interpolation==PrimitiveVariable::Constant )
			{
				parmsAdded += checkAndAddShaderParameter( shaderState, it->first, it->second.data, memberData, "Renderer::addPrimitive", true );
			}
		}
		if( parmsAdded )
		{
			memberData->implementation->attributeBegin();
			memberData->implementation->addState( shaderState );
		}
			memberData->implementation->addPrimitive( primitive );
		if( parmsAdded )
		{
			memberData->implementation->attributeEnd();
		}
	}
	else
	{
		// no shader so no need to worry
		memberData->implementation->addPrimitive( primitive );
	}
}

void IECoreGL::Renderer::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	// get positions
	ConstV3fVectorDataPtr points = findPrimVar<V3fVectorData>( "P", PrimitiveVariable::Vertex, primVars );
	if( !points )
	{
		msg( Msg::Warning, "Renderer::points", "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
		return;
	}
	
	// get type
	PointsPrimitive::Type type = PointsPrimitive::Disk;
	if( ConstStringDataPtr t = findPrimVar<StringData>( "type", PrimitiveVariable::Uniform, primVars ) )
	{
		if( t->readable()=="particle" || t->readable()=="disk" || t->readable()=="blobby" )
		{
			type = PointsPrimitive::Disk;
		}
		else if( t->readable()=="sphere" )
		{
			type = PointsPrimitive::Sphere;
		}
		else if( t->readable()=="patch" )
		{
			type = PointsPrimitive::Quad;
		}
		else if( t->readable()=="gl:point" )
		{
			type = PointsPrimitive::Point;
		}
		else
		{
			msg( Msg::Warning, "Renderer::points", boost::format( "Unknown type \"%s\" - reverting to particle type." ) % t->readable() );
		}
	}
	
	// get colors
	PrimitiveVariable::Interpolation colorInterpolations[] = { PrimitiveVariable::Vertex, PrimitiveVariable::Varying, PrimitiveVariable::Invalid };
	ConstColor3fVectorDataPtr colors = findPrimVar<Color3fVectorData>( "Cs", colorInterpolations, primVars );
	
	// get widths
	ConstFloatDataPtr constantWidth = findPrimVar<FloatData>( "constantwidth", PrimitiveVariable::Constant, primVars );
	PrimitiveVariable::Interpolation widthInterpolations[] = { PrimitiveVariable::Vertex, PrimitiveVariable::Varying, PrimitiveVariable::Invalid };
	ConstFloatVectorDataPtr widths = findPrimVar<FloatVectorData>( "width", widthInterpolations, primVars );
	
	if( constantWidth )
	{
		if( widths )
		{
			FloatVectorDataPtr newWidths = widths->copy();
			vector<float> &w = newWidths->writable();
			float ww = constantWidth->readable();
			for( vector<float>::iterator it=w.begin(); it!=w.end(); it++ )
			{
				*it *= ww;
			}
		}
		else
		{
			FloatVectorDataPtr newWidths = new FloatVectorData();
			newWidths->writable().push_back( constantWidth->readable() );
			widths = newWidths;
		}
	}
	
	// compute heights
	ConstFloatDataPtr constantAspectData = findPrimVar<FloatData>( "patchaspectratio", PrimitiveVariable::Constant, primVars );
	PrimitiveVariable::Interpolation aspectInterpolations[] = { PrimitiveVariable::Vertex, PrimitiveVariable::Varying, PrimitiveVariable::Invalid }; 
	ConstFloatVectorDataPtr aspectData = findPrimVar<FloatVectorData>( "patchaspectratio", aspectInterpolations, primVars );
	
	ConstFloatVectorDataPtr heights = 0;
	if( !constantAspectData && !aspectData )
	{
		heights = widths;
	}
	else
	{
		if( constantAspectData )
		{
			float aspect = constantAspectData->readable();
			FloatVectorDataPtr h = widths->copy();
			vector<float> &hV = h->writable();
			for( unsigned int i=0; i<hV.size(); i++ )
			{
				hV[i] /= aspect;
			}
			heights = h;
		}
		else
		{
			// we have varying aspect data
			FloatVectorDataPtr h = aspectData->copy();
			vector<float> &hV = h->writable();
			float defaultWidth = 1;
			const float *widthsP = &defaultWidth;
			unsigned int widthStride = 0;
			if( widths )
			{
				widthsP = &widths->readable()[0];
				widthStride = widths->readable().size() > 1 ? 1 : 0;
			}
			for( unsigned int i=0; i<hV.size(); i++ )
			{
				hV[i] = *widthsP / hV[i];
				widthsP += widthStride;
			}
			heights = h;
		}
	}
	
	// get rotations
	PrimitiveVariable::Interpolation rotationInterpolations[] = { PrimitiveVariable::Vertex, PrimitiveVariable::Varying, PrimitiveVariable::Invalid };
	ConstFloatVectorDataPtr rotations = findPrimVar<FloatVectorData>( "patchrotation", rotationInterpolations, primVars );

	// make the primitive
	PointsPrimitivePtr prim = new PointsPrimitive( type, points, colors, 0, widths, heights, rotations );
	addPrimitive( prim, primVars, m_data );
}

void IECoreGL::Renderer::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::disk", "Not implemented" );
}

void IECoreGL::Renderer::curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	ConstV3fVectorDataPtr points = findPrimVar<V3fVectorData>( "P", PrimitiveVariable::Vertex, primVars );
	if( !points )
	{
		msg( Msg::Warning, "Renderer::curves", "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
		return;
	}

	ConstFloatDataPtr widthData = findPrimVar<FloatData>( "width", PrimitiveVariable::Constant, primVars );
	if( !widthData  )
	{
		widthData = findPrimVar<FloatData>( "constantwidth", PrimitiveVariable::Constant, primVars );
	}
	
	float width = 1;
	if( widthData )
	{
		width = widthData->readable();
	}
	
	CurvesPrimitivePtr prim = new CurvesPrimitive( basis, periodic, numVertices, points, width );
	addPrimitive( prim, primVars, m_data );
}

void IECoreGL::Renderer::text( const std::string &font, const std::string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{

#ifdef IECORE_WITH_FREETYPE
	FontPtr f = 0;
	MemberData::FontMap::const_iterator it = m_data->fonts.find( font );
	if( it!=m_data->fonts.end() )
	{
		f = it->second;
	}
	else
	{
		IECore::SearchPath s( m_data->options.fontSearchPath, ":" );
		string file = s.find( font ).string();
		if( file!="" )
		{
			try
			{
				IECore::FontPtr cf = new IECore::Font( file );
				cf->setResolution( 128 ); // makes for better texture resolutions - maybe it could be an option?
				f = new Font( cf );
			}
			catch( const std::exception &e )
			{
				IECore::msg( IECore::Msg::Warning, "Renderer::text", e.what() ); 
			}
		}
		m_data->fonts[font] = f;
	}
	
	if( !f )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::text", boost::format( "Font \"%s\" not found." ) % font ); 	
		return;
	}
	
	f->coreFont()->setKerning( kerning );
	
	TextPrimitivePtr prim = new TextPrimitive( text, f );
	addPrimitive( prim, primVars, m_data );
#else
	IECore::msg( IECore::Msg::Warning, "Renderer::text", "IECore was not built with FreeType support." ); 	
#endif // IECORE_WITH_FREETYPE
}

void IECoreGL::Renderer::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	SpherePrimitivePtr prim = new SpherePrimitive( radius, zMin, zMax, thetaMax );
	addPrimitive( prim, primVars, m_data );
}

static IECoreGL::ShaderPtr imageShader()
{
	static const char *fragSrc = 
		"uniform sampler2D texture;"
		""
		"void main()"
		"{"
		"	gl_FragColor = texture2D( texture, gl_TexCoord[0].xy );"
		"}";
	static bool t = false;
	static ShaderPtr s = 0;
	if( !t )
	{
		try
		{
			s = new Shader( "", fragSrc );
		}
		catch( const std::exception &e )
		{
			msg( Msg::Error, "Renderer::image", boost::format( "Unable to create image shader (%s)." ) % e.what() );
		}
		t = true;
	}
	return s;	
}

/// \todo This positions images incorrectly when dataWindow!=displayWindow. This is because the texture
/// contains only the dataWindow contents, but we've positioned the card as if it will contain the whole
/// displayWindow.
void IECoreGL::Renderer::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	ImagePrimitivePtr image = new ImagePrimitive( dataWindow, displayWindow );
	image->variables = primVars;
	
	ShaderPtr shader = imageShader();
	if( shader )
	{
		TexturePtr texture = 0;
		try
		{
			texture = new ColorTexture( image );
		}
		catch( const std::exception &e )
		{
			msg( Msg::Warning, "Renderer::image", boost::format( "Texture conversion failed (%s)." ) % e.what() );
		}
		catch( ... )
		{
			msg( Msg::Warning, "Renderer::image", "Texture conversion failed." );
		}
	
		if( texture )
		{
			ShaderStateComponent::TexturesMap textures;
			textures["texture"] = texture;
			ShaderStateComponentPtr state = new ShaderStateComponent( shader, 0, &textures );
			m_data->implementation->addState( state );
		}
	}
	else
	{
		/// \todo Support a fixed pipeline fallback when we have support for a fixed pipeline
		/// in a StateComponent
		msg( Msg::Warning, "Renderer::image", "Unable to create shader to display image." );
	}
	
	m_data->implementation->transformBegin();
	
		Box3f bound = image->bound();
		V3f center = bound.center();
		
		M44f xform;
		xform[3][0] = center.x;
		xform[3][1] = center.y;		
		xform[3][2] = center.z;
		
		xform[0][0] = boxSize( bound ).x ;
		xform[1][1] = boxSize( bound ).y ;
		xform[2][2] = 1.0;		
			
		m_data->implementation->concatTransform( xform );
		QuadPrimitivePtr quad = new QuadPrimitive( 1.0, 1.0 );
		m_data->implementation->addPrimitive( quad );
		
	m_data->implementation->transformEnd();	
}

void IECoreGL::Renderer::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	try
	{
		IECore::MeshPrimitivePtr m = new IECore::MeshPrimitive( vertsPerFace, vertIds, interpolation );
		m->variables = primVars;
		
		if( interpolation!="linear" )
		{
			// it's a subdivision mesh. in the absence of a nice subdivision algorithm to display things with,
			// we can at least make things look a bit nicer by calculating some smooth shading normals. 
			// if interpolation is linear and no normals are provided then we assume the faceted look is intentional.
			if( primVars.find( "N" )==primVars.end() )
			{
				MeshNormalsOpPtr normalOp = new MeshNormalsOp();
				normalOp->inputParameter()->setValue( m );
				normalOp->copyParameter()->setTypedValue( false );
				normalOp->operate();
			}
		}
		
		MeshPrimitivePtr prim = boost::static_pointer_cast<MeshPrimitive>( ToGLMeshConverter( m ).convert() );
		addPrimitive( prim, primVars, m_data, false );
	}
	catch( const std::exception &e )
	{
		msg( Msg::Warning, "Renderer::mesh", e.what() );
		return;
	}
}

void IECoreGL::Renderer::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::nurbs", "Not implemented" );
}

void IECoreGL::Renderer::patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::patchMesh", "Not implemented" );
}

void IECoreGL::Renderer::geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars )
{
	if( type=="sphere" )
	{
		float radius = parameterValue<float>( "radius", topology, 1 );
		float zMin = parameterValue<float>( "zMin", topology, -1 );
		float zMax = parameterValue<float>( "zMax", topology, 1 );
		float thetaMax = parameterValue<float>( "thetaMax", topology, 360 );
		sphere( radius, zMin, zMax, thetaMax, primVars );
	}
	else
	{
		msg( Msg::Warning, "Renderer::geometry", boost::format( "Geometry type \"%s\" not implemented." ) % type );
	}
}

void IECoreGL::Renderer::procedural( IECore::Renderer::ProceduralPtr proc )
{
	/// \todo Frustum culling, with an option to enable/disable it (we'd need to disable it when
	/// building scenes for interactive display).
	proc->render( this );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void IECoreGL::Renderer::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "Renderer::instanceBegin", "Not implemented" );
}

void IECoreGL::Renderer::instanceEnd()
{
	msg( Msg::Warning, "Renderer::instanceEnd", "Not implemented" );
}

void IECoreGL::Renderer::instance( const std::string &name )
{
	msg( Msg::Warning, "Renderer::instance", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////////////////////

IECore::DataPtr IECoreGL::Renderer::command( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "Renderer::command", "Not implemented" );
	return 0;
}

