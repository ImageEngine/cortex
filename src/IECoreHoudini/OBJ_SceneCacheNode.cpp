//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "OBJ/OBJ_Geometry.h"
#include "OBJ/OBJ_SubNet.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/OBJ_SceneCacheNode.h"

using namespace IECore;
using namespace IECoreHoudini;

template<typename BaseType>
OBJ_SceneCacheNode<BaseType>::OBJ_SceneCacheNode( OP_Network *net, const char *name, OP_Operator *op ) : SceneCacheNode<BaseType>( net, name, op )
{
}

template<typename BaseType>
OBJ_SceneCacheNode<BaseType>::~OBJ_SceneCacheNode()
{
}

template<typename BaseType>
PRM_Name OBJ_SceneCacheNode<BaseType>::pBuild( "build", "Build Hierarchy" );

static void copyAndHideParm( PRM_Template &src, PRM_Template &dest )
{
	PRM_Name *name = new PRM_Name( src.getToken(), src.getLabel(), src.getExpressionFlag() );
	name->harden();
	
	dest.initialize(
		(PRM_Type) (src.getType() | PRM_TYPE_INVISIBLE),
		src.getTypeExtended(),
		src.exportLevel(),
		src.getVectorSize(),
		name,
		src.getFactoryDefaults(),
		src.getChoiceListPtr(),
		src.getRangePtr(),
		src.getCallback(),
		src.getSparePtr(),
		src.getParmGroup(),
		(const char *)src.getHelpText(),
		src.getConditionalBasePtr()
	);
}

template<typename BaseType>
OP_TemplatePair *OBJ_SceneCacheNode<BaseType>::buildParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		PRM_Template *objTemplate = BaseType::getTemplateList( OBJ_PARMS_PLAIN );
		unsigned numObjParms = PRM_Template::countTemplates( objTemplate );
		unsigned numSCCParms = PRM_Template::countTemplates( SceneCacheNode<BaseType>::parameters );
		thisTemplate = new PRM_Template[ numObjParms + numSCCParms + 2 ];
		
		for ( unsigned i = 0; i < numObjParms; ++i )
		{
			thisTemplate[i] = objTemplate[i];
			copyAndHideParm( objTemplate[i], thisTemplate[i] );
		}
		
		for ( unsigned i = 0; i < numSCCParms; ++i )
		{
			thisTemplate[numObjParms+i] = SceneCacheNode<BaseType>::parameters[i];
		}
		
		thisTemplate[numObjParms + numSCCParms] = PRM_Template(
			PRM_CALLBACK, 1, &pBuild, 0, 0, 0, &OBJ_SceneCacheNode<BaseType>::buildButtonCallback, 0, 0,
			"Build the hierarchy below the specified root path.\n"
			"Some nodes may define additional options that are used during the build process."
		);
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( thisTemplate );
	}
	
	return templatePair;
}

template<typename BaseType>
int OBJ_SceneCacheNode<BaseType>::buildButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	std::string file;
	OBJ_SceneCacheNode<BaseType> *node = reinterpret_cast<OBJ_SceneCacheNode<BaseType>*>( data );
	if ( !node || !node->ensureFile( file ) )
	{
		return 0;
	}
	
	node->cleanHierarchy();
	node->buildHierarchy( node->scene( file, node->getPath() ) );
	
	return 1;
}

template<typename BaseType>
void OBJ_SceneCacheNode<BaseType>::cleanHierarchy()
{
	OP_NodeList childNodes;
	for ( int i=0; i < this->getNchildren(); ++i )
	{
		childNodes.append( this->getChild( i ) );
	}
	
	this->destroyNodes( childNodes );
}

template<typename BaseType>
void OBJ_SceneCacheNode<BaseType>::sceneChanged()
{
	SceneCacheNode<BaseType>::sceneChanged();
	
	std::string file;
	if ( !OBJ_SceneCacheNode<BaseType>::ensureFile( file ) )
	{
		m_static = false;
		return;
	}
	
	std::string path = this->getPath();
	
	ConstSceneInterfacePtr scene = this->scene( file, path );
	const SampledSceneInterface *sampledScene = IECore::runTimeCast<const SampledSceneInterface>( scene );
	if ( !sampledScene )
	{
		m_static = false;
		return;
	}
	
	m_static = ( sampledScene->numTransformSamples() < 2 );
}

template<typename BaseType>
bool OBJ_SceneCacheNode<BaseType>::getParmTransform( OP_Context &context, UT_DMatrix4 &xform )
{
	std::string file = this->getFile();
	std::string path = this->getPath();
	OBJ_SceneCacheNode<OP_Node>::Space space = (OBJ_SceneCacheNode<OP_Node>::Space)this->getSpace();
	
	MurmurHash hash;
	hash.append( file );
	hash.append( path );
	hash.append( space );
	
	if ( m_static )
	{
		if ( this->m_loaded && this->m_hash == hash )
		{
			xform = m_xform;
			return true;
		}
	}
	else
	{
		BaseType::flags().setTimeDep( true );
		BaseType::getParmList()->setCookTimeDependent( true );
	}
	
	if ( !SceneCacheNode<BaseType>::ensureFile( file ) )
	{
		SceneCacheNode<BaseType>::addError( OBJ_ERR_CANT_FIND_OBJ, ( file + " is not a valid .scc" ).c_str() );
		return false;
	}
	
	ConstSceneInterfacePtr scene = this->scene( file, path );
	if ( !scene )
	{
		SceneCacheNode<BaseType>::addError( OBJ_ERR_CANT_FIND_OBJ, ( path + " is not a valid location in " + file ).c_str() );
		return false;
	}
	
	Imath::M44d transform;
	if ( space == SceneCacheNode<OP_Node>::World )
	{
		transform = SceneCacheNode<BaseType>::worldTransform( file, path, context.getTime() );
	}
	else if ( space == SceneCacheNode<OP_Node>::Local )
	{
		transform = scene->readTransformAsMatrix( context.getTime() );
	}
	
	xform = IECore::convert<UT_Matrix4D>( transform );
	m_xform = xform;
	this->m_hash = hash;
	this->m_loaded = true;
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Known Specializations
//////////////////////////////////////////////////////////////////////////////////////////

template class OBJ_SceneCacheNode<OBJ_Geometry>;
template class OBJ_SceneCacheNode<OBJ_SubNet>;
