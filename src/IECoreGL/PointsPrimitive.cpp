//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathFun.h"
#include "OpenEXR/ImathMatrixAlgo.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreGL/PointsPrimitive.h"
#include "IECoreGL/DiskPrimitive.h"
#include "IECoreGL/QuadPrimitive.h"
#include "IECoreGL/SpherePrimitive.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/State.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/TextureLoader.h"
#include "IECoreGL/GL.h"

using namespace IECoreGL;
using namespace IECore;
using namespace Imath;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// StateComponents
//////////////////////////////////////////////////////////////////////////

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointsPrimitive::UseGLPoints, PointsPrimitiveUseGLPointsTypeId, GLPointsUsage, ForPointsOnly );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointsPrimitive::GLPointWidth, PointsPrimitiveGLPointWidthTypeId, float, 1.0f );

} // namespace IECoreGL

//////////////////////////////////////////////////////////////////////////
// MemberData
//////////////////////////////////////////////////////////////////////////

class PointsPrimitive::MemberData : public IECore::RefCounted
{

	public :

		IECore::V3fVectorDataPtr points;

		Type type;
		static const float g_defaultWidth;
		IECore::ConstDataPtr constantWidth;
		IECore::ConstDataPtr widths;
		static const float g_defaultAspectRatio;
		IECore::ConstDataPtr patchAspectRatio;
		static const float g_defaultRotation;
		IECore::ConstDataPtr rotations;

		mutable Imath::Box3f bound;
		mutable bool recomputeBound;

		mutable bool renderSorted;
		mutable std::vector<unsigned int> depthOrder;
		mutable std::vector<float> depths;
		mutable Imath::V3f depthCameraDirection;

		struct InstancingSetup
		{
			InstancingSetup( ConstShaderPtr os, Type t, Shader::SetupPtr ss )
				:	originalShader( os ), type( t ), shaderSetup( ss )
			{
			}
			ConstShaderPtr originalShader;
			Type type;
			Shader::SetupPtr shaderSetup;
		};
		typedef std::vector<InstancingSetup> InstancingSetupVector;
		mutable InstancingSetupVector instancingSetups;

		SpherePrimitivePtr spherePrimitive;
		DiskPrimitivePtr diskPrimitive;
		QuadPrimitivePtr quadPrimitive;

};

const float PointsPrimitive::MemberData::g_defaultWidth = 1;
const float PointsPrimitive::MemberData::g_defaultAspectRatio = 1;
const float PointsPrimitive::MemberData::g_defaultRotation = 0;

//////////////////////////////////////////////////////////////////////////
// PointsPrimitive
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( PointsPrimitive );

PointsPrimitive::PointsPrimitive( Type type )
	:	m_memberData( new MemberData )
{
	m_memberData->type = type;
	m_memberData->recomputeBound = true;
}

PointsPrimitive::~PointsPrimitive()
{
}

void PointsPrimitive::updateBounds() const
{
	if( !m_memberData->recomputeBound )
	{
		return;
	}

	m_memberData->recomputeBound = false;

	if( !m_memberData->points )
	{
		m_memberData->bound = Box3f();
		return;
	}

	unsigned int cwStep = 0, wStep = 0, aStep = 0;
	const float *cw = dataAndStride( m_memberData->constantWidth.get(), &MemberData::g_defaultWidth, cwStep );
	const float *w = dataAndStride( m_memberData->widths.get(), &MemberData::g_defaultWidth, wStep );
	const float *a = dataAndStride( m_memberData->patchAspectRatio.get(), &MemberData::g_defaultAspectRatio, aStep );

	m_memberData->bound.makeEmpty();
	const vector<V3f> &pd = m_memberData->points->readable();
	for( unsigned int i=0; i<pd.size(); i++ )
	{
		float r = *cw * *w / 2.0f; cw += cwStep; w += wStep;
		if( *a < 1.0f && *a > 0.0f )
		{
			r /= *a;
		}
		a += aStep;
		m_memberData->bound.extendBy( Box3f( pd[i] - V3f( r ), pd[i] + V3f( r ) ) );
	}
}

void PointsPrimitive::addPrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primVar )
{
	if ( name == "P" )
	{
		m_memberData->recomputeBound = true;
		m_memberData->points = IECore::runTimeCast< IECore::V3fVectorData >( primVar.data->copy() );
	}
	else if( name == "constantwidth" )
	{
		m_memberData->recomputeBound = true;
		m_memberData->constantWidth = primVar.data->copy();
	}
	else if ( name == "width" )
	{
		m_memberData->recomputeBound = true;
		m_memberData->widths = primVar.data->copy();
	}
	else if ( name == "patchaspectratio" )
	{
		m_memberData->recomputeBound = true;
		m_memberData->patchAspectRatio = primVar.data->copy();
	}
	else if ( name == "patchrotation" )
	{
		m_memberData->rotations = primVar.data->copy();
	}
	Primitive::addPrimitiveVariable( name, primVar );
}

const Shader::Setup *PointsPrimitive::shaderSetup( const Shader *shader, State *state ) const
{
	Type type = effectiveType( state );
	if( type == Point )
	{
		// rendering as gl points goes through the normal process
		return Primitive::shaderSetup( shader, state );
	}
	else
	{
		// for rendering as disks, quads or spheres, we build a custom setup which we use
		// for instancing primitives onto our points.
		for( MemberData::InstancingSetupVector::const_iterator it = m_memberData->instancingSetups.begin(), eIt = m_memberData->instancingSetups.end(); it != eIt; it++ )
		{
			if( it->originalShader == shader && it->type == type )
			{
				return it->shaderSetup.get();
			}
		}

		ConstShaderPtr instancingShader = shader;
		ShaderStateComponent *shaderStateComponent = state->get<ShaderStateComponent>();
		if( instancingShader->vertexSource() == "" )
		{
			// if the current shader has specific vertex source, then we assume the user has provided
			// a shader capable of performing the instancing, but if not then we substitute in our own
			// instancing vertex shader.
			ShaderLoader *shaderLoader = shaderStateComponent->shaderLoader();
			instancingShader = shaderLoader->create( instancingVertexSource(), "", shader->fragmentSource() );
		}

		Shader::SetupPtr instancingShaderSetup = new Shader::Setup( instancingShader );
		shaderStateComponent->addParametersToShaderSetup( instancingShaderSetup.get() );
		addPrimitiveVariablesToShaderSetup( instancingShaderSetup.get(), "vertex", 1 );

		instancingShaderSetup->addUniformParameter( "useWidth", new BoolData( static_cast<bool>( m_memberData->widths ) ) );
		if( !m_memberData->constantWidth )
		{
			instancingShaderSetup->addUniformParameter( "constantwidth", new FloatData( 1.0f ) );
		}
		instancingShaderSetup->addUniformParameter( "useAspectRatio", new BoolData( static_cast<bool>( m_memberData->patchAspectRatio ) ) );
		instancingShaderSetup->addUniformParameter( "useRotation", new BoolData( static_cast<bool>( m_memberData->rotations ) ) );

		switch( type )
		{
			case Disk :
				if( !m_memberData->diskPrimitive )
				{
					m_memberData->diskPrimitive = new DiskPrimitive( 0.5f );
				}
				m_memberData->diskPrimitive->addPrimitiveVariablesToShaderSetup( instancingShaderSetup.get(), "instance" );
				break;
			case Sphere :
				if( !m_memberData->spherePrimitive )
				{
					m_memberData->spherePrimitive = new SpherePrimitive( 0.5f );
				}
				m_memberData->spherePrimitive->addPrimitiveVariablesToShaderSetup( instancingShaderSetup.get(), "instance" );
				break;
			case Quad :
				if( !m_memberData->quadPrimitive )
				{
					m_memberData->quadPrimitive = new QuadPrimitive();
				}
				m_memberData->quadPrimitive->addPrimitiveVariablesToShaderSetup( instancingShaderSetup.get(), "instance" );
				break;
			default :
				break;
		}

		m_memberData->instancingSetups.push_back( MemberData::InstancingSetup( shader, type, instancingShaderSetup ) );

		return instancingShaderSetup.get();
	}
}

void PointsPrimitive::render( const State *currentState, IECore::TypeId style ) const
{
	if( !m_memberData->points->readable().size() )
	{
		// early out if no points - some drivers crash otherwise
		return;
	}

	/*if( depthSortRequested( state ) )
	{
		depthSort();
		m_memberData->renderSorted = true;
	}
	else
	{
		m_memberData->renderSorted = false;
	}*/

	switch( effectiveType( currentState ) )
	{
		case Point :
			glPointSize( currentState->get<GLPointWidth>()->value() );
			renderInstances( 1 );
			break;
		case Disk :
			m_memberData->diskPrimitive->renderInstances( m_memberData->points->readable().size() );
			break;
		case Quad :
			m_memberData->quadPrimitive->renderInstances( m_memberData->points->readable().size() );
			break;
		case Sphere :
			m_memberData->spherePrimitive->renderInstances( m_memberData->points->readable().size() );
			break;
	}
}

void PointsPrimitive::renderInstances( size_t numInstances ) const
{
	glDrawArraysInstancedARB( GL_POINTS, 0, m_memberData->points->readable().size(), numInstances );
}

Imath::Box3f PointsPrimitive::bound() const
{
	updateBounds();
	return m_memberData->bound;
}

template<typename T>
const T *PointsPrimitive::dataAndStride( const IECore::Data *data, const T *defaultValue, unsigned int &stride )
{
	stride = 0;
	if( !data )
	{
		stride = 0;
		return defaultValue;
	}
	IECore::TypeId t = data->typeId();
	if ( t == IECore::TypedData< T >::staticTypeId() )
	{
		return &(static_cast< const IECore::TypedData< T > * >( data )->readable());
	}
	if ( t == IECore::TypedData< std::vector< T > >::staticTypeId() )
	{
		stride = 1;
		return &(static_cast< const IECore::TypedData< std::vector<T> > * >( data )->readable()[0]);
	}
	return defaultValue;
}

PointsPrimitive::Type PointsPrimitive::effectiveType( const State *state ) const
{
	Type result = m_memberData->type;
	switch( state->get<UseGLPoints>()->value() )
	{
		case ForPointsOnly :
			break;
		case ForPointsAndDisks :
			if( result==Disk )
			{
				result = Point;
			}
			break;
		case ForAll :
			result = Point;
			break;
	}
	return result;
}

std::string &PointsPrimitive::instancingVertexSource()
{
	static std::string s =

		"#version 120\n"
		""
		"#include \"IECoreGL/PointsPrimitive.h\"\n"
		"#include \"IECoreGL/VertexShader.h\"\n"
		""
		"IECOREGL_POINTSPRIMITIVE_DECLAREVERTEXPARAMETERS\n"
		""
		"IECOREGL_VERTEXSHADER_IN vec3 vertexCs;"
		"uniform bool vertexCsActive = false;"
		""
		"uniform vec3 Cs = vec3( 1, 1, 1 );"
		""
		"IECOREGL_VERTEXSHADER_IN vec3 instanceP;"
		"IECOREGL_VERTEXSHADER_IN vec3 instanceN;"
		"IECOREGL_VERTEXSHADER_IN vec2 instanceuv;"
		""
		"IECOREGL_VERTEXSHADER_OUT vec3 fragmentI;"
		"IECOREGL_VERTEXSHADER_OUT vec3 fragmentN;"
		"IECOREGL_VERTEXSHADER_OUT vec2 fragmentuv;"
		"IECOREGL_VERTEXSHADER_OUT vec3 fragmentCs;"
		""
		"void main()"
		"{"
		"	mat4 instanceMatrix = IECOREGL_POINTSPRIMITIVE_INSTANCEMATRIX;"
		""
		"	vec4 pCam = instanceMatrix * vec4( instanceP, 1 );"
		"	gl_Position = gl_ProjectionMatrix * pCam;"
		""
		"	fragmentN = normalize( instanceMatrix * vec4( instanceN, 0.0 ) ).xyz;"
		""
		"	if( gl_ProjectionMatrix[2][3] != 0.0 )"
		"	{"
		"		fragmentI = normalize( -pCam.xyz );"
		"	}"
		"	else"
		"	{"
		"		fragmentI = vec3( 0.0, 0.0, -1.0 );"
		"	}"
		""
		"	fragmentCs = mix( Cs, vertexCs, float( vertexCsActive ) );"
		"	fragmentuv = instanceuv;"
		""
		"}";

	return s;
}

struct SortFn
{
	SortFn( const vector<float> &d ) : depths( d ) {};
	bool operator()( int f, int s ) { return depths[f] > depths[s]; };
	const vector<float> &depths;
};

/// \todo Use IECore::RadixSort (might still want to use std::sort for small numbers of points - profile to check this)
void PointsPrimitive::depthSort() const
{
	V3f cameraDirection = Camera::viewDirectionInObjectSpace();
	cameraDirection.normalize();

	const vector<V3f> &points = m_memberData->points->readable();
	if( !m_memberData->depthOrder.size() )
	{
		// never sorted before. initialize space.
		m_memberData->depthOrder.resize( points.size() );
		for( unsigned int i=0; i<m_memberData->depthOrder.size(); i++ )
		{
			m_memberData->depthOrder[i] = i;
		}
		m_memberData->depths.resize( points.size() );
	}
	else
	{
		// sorted before. see if the camera direction has changed enough
		// to warrant resorting.
		if( cameraDirection.dot( m_memberData->depthCameraDirection ) > 0.95 )
		{
			return;
		}
	}

	m_memberData->depthCameraDirection = cameraDirection;

	// calculate all distances
	for( unsigned int i=0; i<m_memberData->depths.size(); i++ )
	{
		m_memberData->depths[i] = points[i].dot( m_memberData->depthCameraDirection );
	}

	// sort based on those distances
	SortFn sorter( m_memberData->depths );
	sort( m_memberData->depthOrder.begin(), m_memberData->depthOrder.end(), sorter );
}
