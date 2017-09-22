//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

// This must come before the Cortex includes, because on OSX headers included
// by TBB define macros which conflict with the inline functions in ai_types.h.
#include "ai.h"

#include "OpenEXR/ImathBoxAlgo.h"

#include "boost/format.hpp"
#include "boost/algorithm/string/predicate.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/Camera.h"
#include "IECore/Transform.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/SpherePrimitive.h"

#include "IECoreArnold/private/RendererImplementation.h"
#include "IECoreArnold/ParameterAlgo.h"
#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/CameraAlgo.h"

using namespace IECore;
using namespace IECoreArnold;
using namespace Imath;
using namespace std;
using namespace boost;

namespace
{

InternedString g_aiAutomaticInstancingAttributeName( "ai:automaticInstancing" );
InternedString g_automaticInstancingAttributeName( "automaticInstancing" );

} // namespace

////////////////////////////////////////////////////////////////////////
// AttributeState implementation
////////////////////////////////////////////////////////////////////////

RendererImplementation::AttributeState::AttributeState()
{
	surfaceShader = AiNode( "utility" );
	displacementShader = 0;
	attributes = new CompoundData;
	attributes->writable()["ai:visibility:camera"] = new BoolData( true );
	attributes->writable()["ai:visibility:shadow"] = new BoolData( true );
	attributes->writable()["ai:visibility:diffuseReflect"] = new BoolData( true );
	attributes->writable()["ai:visibility:specularReflect"] = new BoolData( true );
	attributes->writable()["ai:visibility:diffuseTransmit"] = new BoolData( true );
	attributes->writable()["ai:visibility:specularTransmit"] = new BoolData( true );
	attributes->writable()["ai:visibility:volume"] = new BoolData( true );
	attributes->writable()["ai:visibility:subsurface"] = new BoolData( true );
}

RendererImplementation::AttributeState::AttributeState( const AttributeState &other )
{
	surfaceShader = other.surfaceShader;
	displacementShader = other.displacementShader;
	shaders = other.shaders;
	attributes = other.attributes->copy();
}

////////////////////////////////////////////////////////////////////////
// RendererImplementation implementation
////////////////////////////////////////////////////////////////////////

extern AtNodeMethods *ieDisplayDriverMethods;

IECoreArnold::RendererImplementation::RendererImplementation()
	:	m_defaultFilter( 0 ), m_motionBlockSize( 0 )
{
	constructCommon( Render );
}

IECoreArnold::RendererImplementation::RendererImplementation( const std::string &assFileName )
	:	m_defaultFilter( 0 ), m_motionBlockSize( 0 )
{
	m_assFileName = assFileName;
	constructCommon( AssGen );
}

IECoreArnold::RendererImplementation::RendererImplementation( const RendererImplementation &other )
	:	m_transformStack( other.m_transformStack, /* flatten = */ true ), m_motionBlockSize( 0 )
{
	constructCommon( Procedural );
	m_instancingConverter = other.m_instancingConverter;
	m_attributeStack.push( AttributeState( other.m_attributeStack.top() ) );
}

IECoreArnold::RendererImplementation::RendererImplementation( const AtNode *proceduralNode )
	:	m_motionBlockSize( 0 )
{
	constructCommon( Procedural );
	m_instancingConverter = new InstancingConverter;
	/// \todo Initialise stacks properly!!
	m_attributeStack.push( AttributeState() );
	// the AttributeState constructor makes a surface shader node, and
	// it's essential that we return that as one of the nodes created by
	// the procedural - otherwise arnold hangs.
	addNode( m_attributeStack.top().surfaceShader );
}

void IECoreArnold::RendererImplementation::constructCommon( Mode mode )
{
	m_mode = mode;
	if( mode != Procedural )
	{
		m_universe = boost::shared_ptr<UniverseBlock>( new UniverseBlock( /* writable = */ true ) );
		m_instancingConverter = new InstancingConverter;

		// create a generic filter we can use for all displays
		m_defaultFilter = AiNode( "gaussian_filter" );
		AiNodeSetStr( m_defaultFilter, "name", "ieCoreArnold:defaultFilter" );

		m_attributeStack.push( AttributeState() );
	}
}

IECoreArnold::RendererImplementation::~RendererImplementation()
{
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	if( 0 == name.compare( 0, 3, "ai:" ) )
	{
		AtNode *options = AiUniverseGetOptions();
		const AtParamEntry *parameter = AiNodeEntryLookUpParameter( AiNodeGetNodeEntry( options ), name.c_str() + 3 );
		if( parameter )
		{
			ParameterAlgo::setParameter( options, name.c_str() + 3, value.get() );
			return;
		}
	}
	else if( 0 == name.compare( 0, 5, "user:" ) )
	{
		AtNode *options = AiUniverseGetOptions();
		ParameterAlgo::setParameter( options, name.c_str(), value.get() );
		return;
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// ignore options prefixed for some other renderer
		return;
	}

	msg( Msg::Warning, "IECoreArnold::RendererImplementation::setOption", format( "Unknown option \"%s\"." ) % name );
}

IECore::ConstDataPtr IECoreArnold::RendererImplementation::getOption( const std::string &name ) const
{
	if( 0 == name.compare( 0, 3, "ai:" ) )
	{
		AtNode *options = AiUniverseGetOptions();
		return ParameterAlgo::getParameter( options, name.c_str() + 3 );
	}
	else if( 0 == name.compare( 0, 5, "user:" ) )
	{
		AtNode *options = AiUniverseGetOptions();
		return ParameterAlgo::getParameter( options, name.c_str() );
	}
	else if( name == "shutter" )
	{
		AtNode *camera = AiUniverseGetCamera();
		float start = AiNodeGetFlt( camera, "shutter_start" );
		float end = AiNodeGetFlt( camera, "shutter_end" );
		return new V2fData( V2f( start, end ) );
	}

	return 0;
}

void IECoreArnold::RendererImplementation::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	CameraPtr cortexCamera = new Camera( name, 0, new CompoundData( parameters ) );
	cortexCamera->addStandardParameters();

	AtNode *arnoldCamera = CameraAlgo::convert( cortexCamera.get() );

	string nodeName = boost::str( boost::format( "ieCoreArnold:camera:%s" ) % name );
	AiNodeSetStr( arnoldCamera, "name", nodeName.c_str() );

	AtNode *options = AiUniverseGetOptions();
	AiNodeSetPtr( options, "camera", arnoldCamera );

	applyTransformToNode( arnoldCamera );

	const V2iData *resolution = cortexCamera->parametersData()->member<V2iData>( "resolution" );
	AiNodeSetInt( options, "xres", resolution->readable().x );
	AiNodeSetInt( options, "yres", resolution->readable().y );

	const FloatData *pixelAspectRatio = cortexCamera->parametersData()->member<FloatData>( "pixelAspectRatio" );
	AiNodeSetFlt( options, "pixel_aspect_ratio", pixelAspectRatio->readable() );
}

void IECoreArnold::RendererImplementation::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	AtNode *driver = 0;
	if( AiNodeEntryLookUp( type.c_str() ) )
	{
		driver = AiNode( type.c_str() );
	}
	else
	{
		// automatically map tiff to driver_tiff and so on, to provide a degree of
		// compatibility with existing renderman driver names.
		std::string prefixedType = "driver_" + type;
		if( AiNodeEntryLookUp( prefixedType.c_str() ) )
		{
			driver = AiNode( prefixedType.c_str() );
		}
	}

	if( !driver )
	{
		msg( Msg::Error, "IECoreArnold::RendererImplementation::display", boost::format( "Unable to create display of type \"%s\"" ) % type );
		return;
	}

	string nodeName = boost::str( boost::format( "ieCoreArnold:display%d" ) % m_outputDescriptions.size() );
	AiNodeSetStr( driver, "name", nodeName.c_str() );

	const AtParamEntry *fileNameParameter = AiNodeEntryLookUpParameter( AiNodeGetNodeEntry( driver ), "filename" );
	if( fileNameParameter )
	{
		AiNodeSetStr( driver, AiParamGetName( fileNameParameter ), name.c_str() );
	}

	ParameterAlgo::setParameters( driver, parameters );

	string d = data;
	if( d=="rgb" )
	{
		d = "RGB RGB";
	}
	else if( d=="rgba" )
	{
		d = "RGBA RGBA";
	}

	std::string outputDescription = str( format( "%s %s %s" ) % d.c_str() % AiNodeGetName( m_defaultFilter ) % nodeName.c_str() );
	m_outputDescriptions.push_back( outputDescription );
}

/////////////////////////////////////////////////////////////////////////////////////////
// world
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::worldBegin()
{
	// reset transform stack
	if( m_transformStack.size() > 1 )
	{
		msg( Msg::Warning, "IECoreArnold::RendererImplementation::worldBegin", "Missing transformEnd() call detected." );
	}
	m_transformStack = TransformStack();

	// specify default camera if none has been specified yet
	AtNode *options = AiUniverseGetOptions();

	if( !AiNodeGetPtr( options, "camera" ) )
	{
		// no camera has been specified - make a default one
		camera( "default", CompoundDataMap() );
	}

	// specify all the outputs
	AtArray *outputsArray = AiArrayAllocate( m_outputDescriptions.size(), 1, AI_TYPE_STRING );
	for( int i = 0, e = m_outputDescriptions.size(); i < e; i++ )
	{
		AiArraySetStr( outputsArray, i, m_outputDescriptions[i].c_str() );
	}
	AiNodeSetArray( options, "outputs", outputsArray );

}

void IECoreArnold::RendererImplementation::worldEnd()
{
	if( m_mode == Render )
	{
		AiRender( AI_RENDER_MODE_CAMERA );
	}
	else if( m_mode == AssGen )
	{
		AiASSWrite( m_assFileName.c_str(), AI_NODE_ALL, false );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::transformBegin()
{
	m_transformStack.push();
}

void IECoreArnold::RendererImplementation::transformEnd()
{
	try
	{
		m_transformStack.pop();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Warning, "IECoreArnold::RendererImplementation::transformEnd", e.what() );
		return;
	}
}

void IECoreArnold::RendererImplementation::setTransform( const Imath::M44f &m )
{
	if( m_motionBlockSize && !m_transformStack.inMotion() )
	{
		// We call motionBegin on the transform stack with a dummy time sample vector of the correct size
		// Arnold doesn't support non-uniform time sampling, so we won't use the actual values, we just
		// need the transform block to know how many values to expect.
		m_transformStack.motionBegin( std::vector<float>( m_motionBlockSize, 0.0f ) );
	}
	m_transformStack.set( m );
}

void IECoreArnold::RendererImplementation::setTransform( const std::string &coordinateSystem )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::setTransform", "Not implemented" );
}

Imath::M44f IECoreArnold::RendererImplementation::getTransform() const
{
	return m_transformStack.get();
}

Imath::M44f IECoreArnold::RendererImplementation::getTransform( const std::string &coordinateSystem ) const
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::getTransform", "Not implemented" );
	return M44f();
}

void IECoreArnold::RendererImplementation::concatTransform( const Imath::M44f &m )
{
	if( m_motionBlockSize && !m_transformStack.inMotion() )
	{
		// See comment in setTransform
		m_transformStack.motionBegin( std::vector<float>( m_motionBlockSize, 0.0f ) );
	}
	m_transformStack.concatenate( m );
}

void IECoreArnold::RendererImplementation::coordinateSystem( const std::string &name )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::coordinateSystem", "Not implemented" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// attribute code
//////////////////////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::attributeBegin()
{
	transformBegin();
	m_attributeStack.push( AttributeState( m_attributeStack.top() ) );
}

void IECoreArnold::RendererImplementation::attributeEnd()
{
	m_attributeStack.pop();
	transformEnd();
}

void IECoreArnold::RendererImplementation::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	m_attributeStack.top().attributes->writable()[name] = value->copy();
}

IECore::ConstDataPtr IECoreArnold::RendererImplementation::getAttribute( const std::string &name ) const
{
	return m_attributeStack.top().attributes->member<Data>( name );
}

void IECoreArnold::RendererImplementation::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	if(
		type=="shader" || type=="ai:shader" ||
		type=="surface" || type=="ai:surface" ||
		type=="displacement" || type=="ai:displacement"
	)
	{
		AtNode *s = 0;
		if( 0 == name.compare( 0, 10, "reference:" ) )
		{
			s = AiNodeLookUpByName( name.c_str() + 10 );
			if( !s )
			{
				msg( Msg::Warning, "IECoreArnold::RendererImplementation::shader", boost::format( "Couldn't find shader \"%s\"" ) % name );
				return;
			}
		}
		else
		{
			s = AiNode( name.c_str() );
			if( !s )
			{
				msg( Msg::Warning, "IECoreArnold::RendererImplementation::shader", boost::format( "Couldn't load shader \"%s\"" ) % name );
				return;
			}
			for( CompoundDataMap::const_iterator parmIt=parameters.begin(); parmIt!=parameters.end(); parmIt++ )
			{
				if( parmIt->second->isInstanceOf( IECore::StringDataTypeId ) )
				{
					const std::string &potentialLink = static_cast<const StringData *>( parmIt->second.get() )->readable();
					if( 0 == potentialLink.compare( 0, 5, "link:" ) )
					{
						std::string linkHandle = potentialLink.c_str() + 5;
						AttributeState::ShaderMap::const_iterator shaderIt = m_attributeStack.top().shaders.find( linkHandle );
						if( shaderIt != m_attributeStack.top().shaders.end() )
						{
							AiNodeLinkOutput( shaderIt->second, "", s, parmIt->first.value().c_str() );
						}
						else
						{
							msg( Msg::Warning, "IECoreArnold::RendererImplementation::shader", boost::format( "Couldn't find shader handle \"%s\" for linking" ) % linkHandle );
						}
						continue;
					}
				}
				ParameterAlgo::setParameter( s, parmIt->first.value().c_str(), parmIt->second.get() );
			}
			addNode( s );
		}

		if( type=="shader" || type == "ai:shader" )
		{
			CompoundDataMap::const_iterator handleIt = parameters.find( "__handle" );
			if( handleIt != parameters.end() && handleIt->second->isInstanceOf( IECore::StringDataTypeId ) )
			{
				const std::string &handle = static_cast<const StringData *>( handleIt->second.get() )->readable();
				m_attributeStack.top().shaders[handle] = s;
			}
			else
			{
				msg( Msg::Warning, "IECoreArnold::RendererImplementation::shader", "No __handle parameter specified." );
			}
		}
		else if( type=="surface" || type == "ai:surface" )
		{
			m_attributeStack.top().surfaceShader = s;
		}
		else
		{
			m_attributeStack.top().displacementShader = s;
		}
	}
	else
	{
		if( type.find( ':' ) == string::npos )
		{
			msg( Msg::Warning, "IECoreArnold::RendererImplementation::shader", boost::format( "Unsupported shader type \"%s\"" ) % type );
		}
	}
}

void IECoreArnold::RendererImplementation::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	const char *unprefixedName = name.c_str();
	if( name.find( ':' ) != string::npos )
	{
		if( boost::starts_with( name, "ai:" ) )
		{
			unprefixedName += 3;
		}
		else
		{
			return;
		}
	}

	AtNode *l = AiNode( unprefixedName );
	if( !l )
	{
		msg( Msg::Warning, "IECoreArnold::RendererImplementation::light", boost::format( "Couldn't load light \"%s\"" ) % unprefixedName );
		return;
	}
	for( CompoundDataMap::const_iterator parmIt=parameters.begin(); parmIt!=parameters.end(); parmIt++ )
	{
		ParameterAlgo::setParameter( l, parmIt->first.value().c_str(), parmIt->second.get() );
	}
	applyTransformToNode( l );
	addNode( l );
}

void IECoreArnold::RendererImplementation::illuminate( const std::string &lightHandle, bool on )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::illuminate", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// motion blur
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::motionBegin( const std::set<float> &times )
{
	if( m_motionBlockSize )
	{
		msg( Msg::Error, "IECoreArnold::RendererImplementation::motionBegin", "Already in a motion block." );
		return;
	}

	std::vector<float> timesVector;
	timesVector.insert( timesVector.end(), times.begin(), times.end() );
	NodeAlgo::ensureUniformTimeSamples( timesVector );

	m_motionStart = timesVector.front();
	m_motionEnd = timesVector.back();
	m_motionBlockSize = times.size();

}

void IECoreArnold::RendererImplementation::motionEnd()
{
	if( !m_motionBlockSize )
	{
		msg( Msg::Error, "IECoreArnold::RendererImplementation::motionEnd", "Not in a motion block." );
		return;
	}

	m_motionBlockSize = 0;
	m_motionPrimitives.clear();
	if( m_transformStack.inMotion() )
	{
		m_transformStack.motionEnd();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	PointsPrimitivePtr points = new IECore::PointsPrimitive( numPoints );
	points->variables = primVars;
	addPrimitive( points.get(), "ai:points:" );
}

void IECoreArnold::RendererImplementation::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::disk", "Not implemented" );
}

void IECoreArnold::RendererImplementation::curves( const IECore::CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	CurvesPrimitivePtr curves = new IECore::CurvesPrimitive( numVertices, basis, periodic );
	curves->variables = primVars;
	addPrimitive( curves.get(), "ai:curves:" );
}

void IECoreArnold::RendererImplementation::text( const std::string &font, const std::string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::text", "Not implemented" );
}

void IECoreArnold::RendererImplementation::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	SpherePrimitivePtr sphere = new IECore::SpherePrimitive( radius, zMin, zMax, thetaMax );
	sphere->variables = primVars;
	addPrimitive( sphere.get(), "ai:sphere:" );
}

void IECoreArnold::RendererImplementation::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::image", "Not implemented" );
}

void IECoreArnold::RendererImplementation::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	MeshPrimitivePtr mesh = new IECore::MeshPrimitive( vertsPerFace, vertIds, interpolation );
	mesh->variables = primVars;
	addPrimitive( mesh.get(), "ai:polymesh:" );
}

void IECoreArnold::RendererImplementation::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::nurbs", "Not implemented" );
}

void IECoreArnold::RendererImplementation::patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::patchMesh", "Not implemented" );
}

void IECoreArnold::RendererImplementation::geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::geometry", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// procedurals
/////////////////////////////////////////////////////////////////////////////////////////
int IECoreArnold::RendererImplementation::procFunc( AtProceduralNodeMethods *methods )
{
	methods->Init = procInit;
	methods->Cleanup = procCleanup;
	methods->NumNodes = procNumNodes;
	methods->GetNode = procGetNode;
	return 1;
}

int IECoreArnold::RendererImplementation::procInit( AtNode *node, void **userPtr )
{
	ProceduralData *data = (ProceduralData *)( AiNodeGetPtr( node, "userptr" ) );
	data->procedural->render( data->renderer.get() );
	data->procedural = 0;
	*userPtr = data;
	return 1;
}

int IECoreArnold::RendererImplementation::procCleanup( const AtNode *node, void *userPtr )
{
	ProceduralData *data = (ProceduralData *)( userPtr );
	delete data;
	return 1;
}

int IECoreArnold::RendererImplementation::procNumNodes( const AtNode *node, void *userPtr )
{
	ProceduralData *data = (ProceduralData *)( userPtr );
	return data->renderer->m_implementation->m_nodes.size();
}

AtNode* IECoreArnold::RendererImplementation::procGetNode( const AtNode *node, void *userPtr, int i )
{
	ProceduralData *data = (ProceduralData *)( userPtr );
	return data->renderer->m_implementation->m_nodes[i];
}

void IECoreArnold::RendererImplementation::procedural( IECore::Renderer::ProceduralPtr proc )
{
	Box3f bound = proc->bound();
	if( bound.isEmpty() )
	{
		return;
	}

	AtNode *node = NULL;
	std::string nodeType = "procedural";

	if( const ExternalProcedural *externalProc = dynamic_cast<ExternalProcedural *>( proc.get() ) )
	{
		// In Arnold, external procedurals register node types, and then we use the node types
		// just like built in nodes - we don't reference the filename of the dso that defines the node type.
		// So here we just interpret "filename" as the node type to create.
		// \todo : Change the name of the parameter to ExternalProcedural
		// to be just "name" instead of "filename" in Cortex 10?
		node = AiNode( externalProc->fileName().c_str() );
		ParameterAlgo::setParameters( node, externalProc->parameters() );
		applyTransformToNode( node );
	}
	else
	{
		node = AiNode( nodeType.c_str() );

		// we have to transform the bound, as we're not applying the current transform to the
		// procedural node, but instead applying absolute transforms to the shapes the procedural
		// generates.
		if( bound != Procedural::noBound )
		{
			Box3f transformedBound;
			for( size_t i = 0, e = m_transformStack.numSamples(); i < e; ++i )
			{
				transformedBound.extendBy( transform( bound, m_transformStack.sample( i ) ) );
			}
			bound = transformedBound;
		}

		AiNodeSetPtr( node, "funcptr", (void *)procFunc );

		ProceduralData *data = new ProceduralData;
		data->procedural = proc;
		data->renderer = new IECoreArnold::Renderer( new RendererImplementation( *this ) );

		AiNodeSetPtr( node, "userptr", data );
	}

	if( nodeType == "procedural" )
	{
		// We call addNode() rather than addShape() as we don't want to apply transforms and
		// shaders and attributes to procedurals. If we do, they override the things we set
		// on the nodes generated by the procedurals, which is frankly useless.
		addNode( node );
	}
	else
	{
		addShape( node );
	}
}

bool IECoreArnold::RendererImplementation::automaticInstancing() const
{
	const CompoundDataMap &attributes = m_attributeStack.top().attributes->readable();
	CompoundDataMap::const_iterator it = attributes.find( g_aiAutomaticInstancingAttributeName );
	if( it != attributes.end() && it->second->typeId() == IECore::BoolDataTypeId )
	{
		return static_cast<const IECore::BoolData *>( it->second.get() )->readable();
	}
	else
	{
		it = attributes.find( g_automaticInstancingAttributeName );
		if( it != attributes.end() && it->second->typeId() == IECore::BoolDataTypeId )
		{
			return static_cast<const IECore::BoolData *>( it->second.get() )->readable();
		}
	}
	return true;
}

void IECoreArnold::RendererImplementation::addPrimitive( const IECore::Primitive *primitive, const std::string &attributePrefix )
{
	if( m_motionBlockSize )
	{
		// We're in a motion block. Just store samples
		// until we have all of them.
		m_motionPrimitives.push_back( primitive );
		if( m_motionPrimitives.size() != m_motionBlockSize )
		{
			return;
		}
	}

	const CompoundDataMap &attributes = m_attributeStack.top().attributes->readable();

	AtNode *shape = NULL;
	if( automaticInstancing() )
	{
		IECore::MurmurHash hash;
		for( CompoundDataMap::const_iterator it = attributes.begin(), eIt = attributes.end(); it != eIt; it++ )
		{
			if(
				boost::starts_with( it->first.value(), attributePrefix ) ||
				boost::starts_with( it->first.c_str(), "ai:shape:" )
			)
			{
				hash.append( it->first.value() );
				it->second->hash( hash );
			}
		}
		if( m_motionBlockSize )
		{
			vector<const Primitive *> prims;
			for( vector<ConstPrimitivePtr>::const_iterator it = m_motionPrimitives.begin(), eIt = m_motionPrimitives.end(); it != eIt; ++it )
			{
				prims.push_back( it->get() );
			}
			shape = m_instancingConverter->convert( prims, m_motionStart, m_motionEnd, hash );
		}
		else
		{
			shape = m_instancingConverter->convert( primitive, hash );
		}
	}
	else
	{
		if( m_motionBlockSize )
		{
			vector<const Object *> prims;
			for( vector<ConstPrimitivePtr>::const_iterator it = m_motionPrimitives.begin(), eIt = m_motionPrimitives.end(); it != eIt; ++it )
			{
				prims.push_back( it->get() );
			}
			shape = NodeAlgo::convert( prims, m_motionStart, m_motionEnd );
		}
		else
		{
			shape = NodeAlgo::convert( primitive );
		}
	}

	if( strcmp( AiNodeEntryGetName( AiNodeGetNodeEntry( shape ) ), "ginstance" ) )
	{
		// it's not an instance, copy over attributes destined for this object type.
		const CompoundDataMap &attributes = m_attributeStack.top().attributes->readable();
		for( CompoundDataMap::const_iterator it = attributes.begin(), eIt = attributes.end(); it != eIt; it++ )
		{
			if( boost::starts_with( it->first.value(), attributePrefix ) )
			{
				ParameterAlgo::setParameter( shape, it->first.value().c_str() + attributePrefix.size(), it->second.get() );
			}
			else if( boost::starts_with( it->first.c_str(), "ai:shape:" ) )
			{
				ParameterAlgo::setParameter( shape, it->first.value().c_str() + 9, it->second.get() );
			}
		}
	}
	else
	{
		// it's an instance - make sure we don't get double transformations.
		AiNodeSetBool( shape, "inherit_xform", false );
	}

	addShape( shape );
}

void IECoreArnold::RendererImplementation::addShape( AtNode *shape )
{
	applyTransformToNode( shape );
	applyVisibilityToNode( shape );

	AiNodeSetPtr( shape, "shader", m_attributeStack.top().surfaceShader );

	if( AiNodeEntryLookUpParameter( AiNodeGetNodeEntry( shape ), "disp_map" ) )
	{
		if( m_attributeStack.top().displacementShader )
		{
			AiNodeSetPtr( shape, "disp_map", m_attributeStack.top().displacementShader );
		}
	}

	addNode( shape );
}

void IECoreArnold::RendererImplementation::applyTransformToNode( AtNode *node )
{
	const size_t numSamples = m_transformStack.numSamples();
	if( numSamples == 1 )
	{
		M44f m = m_transformStack.get();
		AiNodeSetMatrix( node, "matrix", reinterpret_cast<AtMatrix&>( m.x ) );
	}
	else
	{
		AtArray *matrices = AiArrayAllocate( 1, numSamples, AI_TYPE_MATRIX );
		for( size_t i = 0; i < numSamples; ++i )
		{
			M44f m = m_transformStack.sample( i );
			AiArraySetMtx( matrices, i, reinterpret_cast<AtMatrix&>( m.x ) );
		}
		AiNodeSetArray( node, "matrix", matrices );
		AiNodeSetFlt( node, "motion_start", m_motionStart );
		AiNodeSetFlt( node, "motion_end", m_motionEnd );
	}
}

void IECoreArnold::RendererImplementation::applyVisibilityToNode( AtNode *node )
{
	uint8_t visibility = 0;
	const BoolData *visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:camera" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_CAMERA;
	}

	visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:shadow" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_SHADOW;
	}

	visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:diffuseReflect" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_DIFFUSE_REFLECT;
	}

	visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:specularReflect" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_SPECULAR_REFLECT;
	}

	visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:diffuseTransmit" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_DIFFUSE_TRANSMIT;
	}

	visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:specularTransmit" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_SPECULAR_TRANSMIT;
	}

	visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:volume" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_VOLUME;
	}

	visData = m_attributeStack.top().attributes->member<BoolData>( "ai:visibility:subsurface" );
	if( visData->readable() )
	{
		visibility |= AI_RAY_SUBSURFACE;
	}

	AiNodeSetByte( node, "visibility", visibility );
}

void IECoreArnold::RendererImplementation::addNode( AtNode *node )
{
	m_nodes.push_back( node );
}

/////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::instanceBegin", "Not implemented" );
}

void IECoreArnold::RendererImplementation::instanceEnd()
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::instanceEnd", "Not implemented" );
}

void IECoreArnold::RendererImplementation::instance( const std::string &name )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::instance", "Not implemented" );
}

/////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////

IECore::DataPtr IECoreArnold::RendererImplementation::command( const std::string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::command", "Not implemented" );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// rerendering
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreArnold::RendererImplementation::editBegin( const std::string &editType, const IECore::CompoundDataMap &parameters )
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::editBegin", "Not implemented" );
}

void IECoreArnold::RendererImplementation::editEnd()
{
	msg( Msg::Warning, "IECoreArnold::RendererImplementation::editEnd", "Not implemented" );
}
