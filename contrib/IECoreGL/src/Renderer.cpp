//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

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
		string shaderSearchPath;
		string shaderSearchPathDefault;
		string textureSearchPath;
		string textureSearchPathDefault;
		vector<CameraPtr> cameras;
		vector<DisplayPtr> displays;
	} options;
	
	bool inWorld;
	RendererImplementationPtr implementation;
	ShaderLoaderPtr shaderLoader;
	TextureLoaderPtr textureLoader;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// structors
/////////////////////////////////////////////////////////////////////////////////////////////////////////

IECoreGL::Renderer::Renderer()
{
	m_data = new MemberData;
	
	m_data->options.mode = MemberData::Immediate;
	m_data->options.shutter = V2f( 0 );
	
	const char *shaderPath = getenv( "IECOREGL_SHADER_PATHS" );
	m_data->options.shaderSearchPath = m_data->options.shaderSearchPathDefault = shaderPath ? shaderPath : "";
	const char *texturePath = getenv( "IECOREGL_TEXTURE_PATHS" );
	m_data->options.textureSearchPath = m_data->options.textureSearchPathDefault = texturePath ? texturePath : "";
	
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
		(*o)["gl:searchPath:shader"] = shaderSearchPathOptionSetter;
		(*o)["searchPath:shader"] = shaderSearchPathOptionSetter;
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
		(*o)["gl:searchPath:shader"] = shaderSearchPathOptionGetter;
		(*o)["searchPath:shader"] = shaderSearchPathOptionGetter;
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


void IECoreGL::Renderer::camera( const std::string &name, IECore::CompoundDataMap &parameters )
{
	if( m_data->inWorld )
	{
		msg( Msg::Warning, "Renderer::camera", "Cameras can not be specified after worldBegin." );
		return;
	}
	
	CameraPtr camera = 0;
	string projection = parameterValue<string>( "projection", parameters, "perspective" );
	if( projection=="orthographic" )
	{
		camera = new OrthographicCamera;
	}
	else 
	{
		if( projection!="perspective" )
		{
			msg( Msg::Warning, "Renderer::camera", boost::format( "Unsupported projection \"%s\" - reverting to perspective." ) % projection );
		}
		PerspectiveCameraPtr p = new PerspectiveCamera;
		float fov = parameterValue<float>( "projection:fov", parameters, 90.0f );
		p->setFOV( fov );
		camera = p;
	}
	
	camera->setResolution( parameterValue<V2i>( "resolution", parameters, V2i( 640, 480 ) ) );
	camera->setScreenWindow( parameterValue<Box2f>( "screenWindow", parameters, Box2f( V2f( -1 ), V2f( 1 ) ) ) );
	camera->setClippingPlanes( parameterValue<V2f>( "clippingPlanes", parameters, V2f( 0.1f, 10000) ) );
	
	// we have to store these till worldBegin, as only then i we sure what sort of renderer backend we have
	m_data->options.cameras.push_back( camera );
}


void IECoreGL::Renderer::display( const std::string &name, const std::string &type, const std::string &data, IECore::CompoundDataMap &parameters )
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
	
	if( m_data->options.shaderSearchPath==m_data->options.shaderSearchPathDefault )
	{
		// use the shared default cache if we can
		m_data->shaderLoader = ShaderLoader::defaultShaderLoader();
	}
	else
	{
		m_data->shaderLoader = new ShaderLoader( IECore::SearchPath( m_data->options.shaderSearchPath, ":" ) );
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
	
	for( unsigned int i=0; i<m_data->options.cameras.size(); i++ )
	{
		m_data->implementation->addCamera( m_data->options.cameras[i] );
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
	m_data->implementation->transformBegin();
}

void IECoreGL::Renderer::transformEnd()
{
	m_data->implementation->transformEnd();
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
	m_data->implementation->concatTransform( m );
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
	}
	return a;
}

void IECoreGL::Renderer::attributeBegin()
{
	m_data->implementation->attributeBegin();
}

void IECoreGL::Renderer::attributeEnd()
{
	m_data->implementation->attributeEnd();
}

/// \todo Support user attributes and ignore attributes destined for other renderers
void IECoreGL::Renderer::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	const AttributeSetterMap *s = attributeSetters();
	AttributeSetterMap::const_iterator it = s->find( name );
	if( it!=s->end() )
	{
		it->second( name, value, m_data );
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
static void addPrimitive( IECoreGL::PrimitivePtr primitive, const IECore::PrimitiveVariableMap &primVars, IECoreGL::Renderer::MemberData *memberData )
{
	// add vertex attributes to the primitive if it supports them
	if( primitive->vertexAttributeSize() )
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

void IECoreGL::Renderer::curves( const std::string &interpolation, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::curves", "Not implemented" );
}

Imath::Box3f IECoreGL::Renderer::textExtents(const std::string & t, const float width )
{
	msg( Msg::Warning, "Renderer::textExtents", "Not implemented" );
	return Box3f();
}

void IECoreGL::Renderer::text(const std::string &t, const float width )
{
	msg( Msg::Warning, "Renderer::text", "Not implemented" );
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
	bool t = false;
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
			ShaderStateComponentPtr state = new ShaderStateComponent( imageShader(), 0, &textures );
			m_data->implementation->addState( state );
		}
	}
	else
	{
		/// \todo Support a fixed pipeline fallback when we have support for a fixed pipeline
		/// in a StateComponent
	}
	
	QuadPrimitivePtr quad = new QuadPrimitive( dataWindow.size().x + 1, dataWindow.size().y + 1 );
	m_data->implementation->addPrimitive( quad );
}

void IECoreGL::Renderer::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	ConstV3fVectorDataPtr points = findPrimVar<V3fVectorData>( "P", PrimitiveVariable::Vertex, primVars );
	if( !points )
	{
		msg( Msg::Warning, "Renderer::mesh", "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
		return;
	}

	MeshPrimitivePtr prim = new MeshPrimitive( vertsPerFace, vertIds, points );
	addPrimitive( prim, primVars, m_data );
}

void IECoreGL::Renderer::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "Renderer::nurbs", "Not implemented" );
}

void IECoreGL::Renderer::geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars )
{
	if( type=="sphere" )
	{
		float radius = parameterValue<float>( "radius", topology, 1 );
		float zMin = parameterValue<float>( "zMin", topology, -1 );
		float zMax = parameterValue<float>( "zMax", topology, 1 );
		float thetaMax = parameterValue<float>( "thetaMax", topology, 360 );
		SpherePrimitivePtr prim = new SpherePrimitive( radius, zMin, zMax, thetaMax );
		addPrimitive( prim, primVars, m_data );
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
// commands
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void IECoreGL::Renderer::command( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "Renderer::setOption", "Not implemented" );
}

