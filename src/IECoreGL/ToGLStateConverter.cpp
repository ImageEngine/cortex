//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/ToGLStateConverter.h"

#include "IECoreGL/CurvesPrimitive.h"
#include "IECoreGL/PointsPrimitive.h"
#include "IECoreGL/Primitive.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/State.h"
#include "IECoreGL/TextureLoader.h"

#include "IECoreScene/Shader.h"
#include "IECoreScene/ShaderNetwork.h"

#include "IECore/CompoundObject.h"
#include "IECore/ObjectVector.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace IECoreGL;

//////////////////////////////////////////////////////////////////////////
// Individual state converters
//////////////////////////////////////////////////////////////////////////

namespace
{

template<class T>
StateComponentPtr attributeToTypedState( const IECore::Object *attribute )
{
	typedef IECore::TypedData<typename T::ValueType> DataType;
	const DataType *d = runTimeCast<const DataType>( attribute );
	if( !d )
	{
		throw IECore::Exception( boost::str( boost::format( "Expected data of type \"%s\"" ) % DataType::staticTypeName() ) );
	}

	return new T( d->readable() );
}

StateComponentPtr attributeToUseGLPointsState( const IECore::Object *attribute )
{
	const StringData *d = runTimeCast<const StringData>( attribute );
	if( !d )
	{
		throw IECore::Exception( "Expected data of type StringData" );
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
		throw IECore::Exception( boost::str( boost::format( "Unsupported value \"%s\"." ) % v ) );
	}

	return new PointsPrimitive::UseGLPoints( u );
}

StateComponentPtr attributeToShaderState( const IECore::Object *attribute )
{
	const IECoreScene::Shader *shader = runTimeCast<const IECoreScene::Shader>( attribute );
	if( !shader )
	{
		if( const IECoreScene::ShaderNetwork *n = runTimeCast<const IECoreScene::ShaderNetwork>( attribute ) )
		{
			shader = n->outputShader();
		}
	}
	if( !shader )
	{
		throw IECore::Exception( "Expected a Shader" );
	}

	const StringData *vertexSourceData = shader->parametersData()->member<StringData>( "gl:vertexSource" );
	const StringData *geometrySourceData = shader->parametersData()->member<StringData>( "gl:geometrySource" );
	const StringData *fragmentSourceData = shader->parametersData()->member<StringData>( "gl:fragmentSource" );

	std::string vertexSource = vertexSourceData ? vertexSourceData->readable() : "";
	std::string geometrySource = geometrySourceData ? geometrySourceData->readable() : "";
	std::string fragmentSource = fragmentSourceData ? fragmentSourceData->readable() : "";

	if( vertexSource == "" && geometrySource == "" && fragmentSource == "" )
	{
		ShaderLoader::defaultShaderLoader()->loadSource( shader->getName(), vertexSource, geometrySource, fragmentSource );
	}

	CompoundObjectPtr parametersData = new CompoundObject;
	for( CompoundDataMap::const_iterator it=shader->parameters().begin(); it!=shader->parameters().end(); it++ )
	{
		if( it->first!="gl:fragmentSource" && it->first!="gl:geometrySource" && it->first!="gl:vertexSource" )
		{
			parametersData->members()[it->first] = it->second;
		}
	}

	return new ShaderStateComponent( ShaderLoader::defaultShaderLoader(), TextureLoader::defaultTextureLoader(), vertexSource, geometrySource, fragmentSource, parametersData );
}

typedef StateComponentPtr (*AttributeToState)( const IECore::Object *attribute );
typedef std::map<IECore::InternedString, AttributeToState> AttributeToStateMap;

const AttributeToStateMap &attributeToStateMap()
{
	static AttributeToStateMap m;
	if( !m.size() )
	{
		m["gl:primitive:wireframe"] = attributeToTypedState<IECoreGL::Primitive::DrawWireframe>;
		m["gl:primitive:wireframeWidth"] = attributeToTypedState<IECoreGL::Primitive::WireframeWidth>;
		m["gl:primitive:bound"] = attributeToTypedState<IECoreGL::Primitive::DrawBound>;
		m["gl:primitive:solid"] = attributeToTypedState<IECoreGL::Primitive::DrawSolid>;
		m["gl:primitive:outline"] = attributeToTypedState<IECoreGL::Primitive::DrawOutline>;
		m["gl:primitive:outlineWidth"] = attributeToTypedState<IECoreGL::Primitive::OutlineWidth>;
		m["gl:primitive:points"] = attributeToTypedState<IECoreGL::Primitive::DrawPoints>;
		m["gl:primitive:pointWidth"] = attributeToTypedState<IECoreGL::Primitive::PointWidth>;
		m["gl:primitive:wireframeColor"] = attributeToTypedState<WireframeColorStateComponent>;
		m["gl:primitive:boundColor"] = attributeToTypedState<BoundColorStateComponent>;
		m["gl:primitive:outlineColor"] = attributeToTypedState<OutlineColorStateComponent>;
		m["gl:primitive:pointColor"] = attributeToTypedState<PointColorStateComponent>;
		m["gl:pointsPrimitive:useGLPoints"] = attributeToUseGLPointsState;
		m["gl:pointsPrimitive:glPointWidth"] = attributeToTypedState<IECoreGL::PointsPrimitive::GLPointWidth>;
		m["doubleSided"] = attributeToTypedState<DoubleSidedStateComponent>;
		m["gl:curvesPrimitive:useGLLines"] = attributeToTypedState<IECoreGL::CurvesPrimitive::UseGLLines>;
		m["gl:curvesPrimitive:glLineWidth"] = attributeToTypedState<IECoreGL::CurvesPrimitive::GLLineWidth>;
		m["gl:curvesPrimitive:ignoreBasis"] = attributeToTypedState<IECoreGL::CurvesPrimitive::IgnoreBasis>;
		m["gl:smoothing:points"] = attributeToTypedState<PointSmoothingStateComponent>;
		m["gl:smoothing:lines"] = attributeToTypedState<LineSmoothingStateComponent>;
		m["gl:smoothing:polygons"] = attributeToTypedState<PolygonSmoothingStateComponent>;
		m["gl:surface"] = attributeToShaderState;
		m["gl:depthTest"] = attributeToTypedState<DepthTestStateComponent>;
	}
	return m;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// ToGLStateConverter implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( ToGLStateConverter );

ToGLConverter::ConverterDescription<ToGLStateConverter> ToGLStateConverter::g_description;

ToGLStateConverter::ToGLStateConverter( IECore::ConstCompoundObjectPtr toConvert )
	:	ToGLConverter( "Converts IECore::CompoundObject objects to IECoreGL::State objects.", IECore::CompoundObjectTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::CompoundObject>( toConvert ) );
}

ToGLStateConverter::~ToGLStateConverter()
{
}

IECore::RunTimeTypedPtr ToGLStateConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	const CompoundObject *co = runTimeCast<const CompoundObject>( src.get() );
	if( !co )
	{
		throw IECore::Exception( "Expected a CompoundObject" );
	}

	const AttributeToStateMap &m = attributeToStateMap();

	const StatePtr result = new State( false );
	for( CompoundObject::ObjectMap::const_iterator it = co->members().begin(), eIt = co->members().end(); it != eIt; ++it )
	{
		AttributeToStateMap::const_iterator mIt = m.find( it->first );
		if( mIt != m.end() )
		{
			StateComponentPtr s = mIt->second( it->second.get() );
			result->add( s );
		}
	}
	return result;
}
