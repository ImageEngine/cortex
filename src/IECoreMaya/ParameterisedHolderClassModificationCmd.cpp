//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include "maya/MArgList.h"
#include "maya/MSelectionList.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnDagNode.h"
#include "maya/MGlobal.h"

#include "IECore/Object.h"
#include "IECore/CompoundParameter.h"

#include "IECoreMaya/ParameterisedHolderClassModificationCmd.h"
#include "IECoreMaya/ClassParameterHandler.h"
#include "IECoreMaya/ClassVectorParameterHandler.h"

using namespace IECoreMaya;
using namespace IECore;

IECore::ObjectPtr ParameterisedHolderClassModificationCmd::g_undoValue = 0;

ParameterisedHolderClassModificationCmd::ParameterisedHolderClassModificationCmd()
	:	m_parameterisedHolder( 0 ), m_originalValues( 0 ), m_changingClass( false )
{
}

ParameterisedHolderClassModificationCmd::~ParameterisedHolderClassModificationCmd()
{
}

void *ParameterisedHolderClassModificationCmd::creator()
{
	return new ParameterisedHolderClassModificationCmd;
}

bool ParameterisedHolderClassModificationCmd::isUndoable() const
{
	return true;
}

bool ParameterisedHolderClassModificationCmd::hasSyntax() const
{
	return false;
}

MStatus ParameterisedHolderClassModificationCmd::doIt( const MArgList &argList )
{	
	// get the node we're operating on
	
	MSelectionList selection;
	selection.add( argList.asString( 0 ) );
	
	selection.getDependNode( 0, m_node );
	if( m_node.isNull() )
	{
		return MS::kFailure;
	}
	
	MFnDependencyNode fnNode( m_node );
	MPxNode *userNode = fnNode.userNode();
	m_parameterisedHolder = dynamic_cast<ParameterisedHolderInterface *>( userNode );
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	// if we're being asked to change class then store the details of the class we want to set
	// and the one we're replacing

	if( argList.length() == 4 )
	{
		std::string originalClassName;
		std::string originalSearchPathEnvVar;
		m_parameterisedHolder->getParameterised( &originalClassName, &m_originalClassVersion, &originalSearchPathEnvVar );
		m_originalClassName = originalClassName.c_str();
		m_originalSearchPathEnvVar = originalSearchPathEnvVar.c_str();
	
		m_newClassName = argList.asString( 1 );
		m_newClassVersion = argList.asInt( 2 );
		m_newSearchPathEnvVar = argList.asString( 3 );
		
		m_changingClass = true;
	}
	else if( argList.length() != 1 )
	{
		displayError( "ieParameterisedHolderSetParameterised : wrong number of arguments." );
		return MS::kFailure;
	}	
		
	// store the current (maya side) values of everything
	
	ParameterisedInterface *parameterised = m_parameterisedHolder->getParameterisedInterface();
	if( parameterised )
	{
		if( m_changingClass )
		{
			m_parameterisedHolder->setParameterisedValues();
			m_originalValues = parameterised->parameters()->getValue()->copy();
			storeClassParameterStates( m_originalClassInfo, parameterised->parameters(), "", false /* all of em */ );
		}
		else
		{
			assert( g_undoValue );
			m_originalValues = g_undoValue;
			g_undoValue = 0;
			storeClassParameterStates( m_originalClassInfo, parameterised->parameters(), "", true /* only the ones that have been changed */ );
		}
	}
	
	// change the class or monkey with the class parameters as requested
	
	if( m_changingClass )
	{
		m_parameterisedHolder->setParameterised( m_newClassName.asChar(), m_newClassVersion, m_newSearchPathEnvVar.asChar() );
		despatchSetParameterisedCallbacks();
	}
	else
	{
		m_parameterisedHolder->updateParameterised();
		storeClassParameterStates( m_newClassInfo, m_parameterisedHolder->getParameterisedInterface()->parameters(), "", false );
		despatchClassSetCallbacks();
	}
		
	return MS::kSuccess;
}

MStatus ParameterisedHolderClassModificationCmd::undoIt()
{
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	MStatus s;
	if( m_changingClass ) 
	{
		s = m_parameterisedHolder->setParameterised( m_originalClassName.asChar(), m_originalClassVersion, m_originalSearchPathEnvVar.asChar() );
		if( !s )
		{
			return s;
		}
	}
	
	if( m_originalClassInfo.classParameters.size() || m_originalClassInfo.classVectorParameters.size() )
	{
		restoreClassParameterStates( m_originalClassInfo, m_parameterisedHolder->getParameterisedInterface()->parameters(), "" );
		m_parameterisedHolder->updateParameterised();
	}
	
	if( m_originalValues )
	{
		m_parameterisedHolder->getParameterisedInterface()->parameters()->setValue( m_originalValues->copy() );
		m_parameterisedHolder->setNodeValues();
	}

	// despatch callbacks only when the dust has settled
	if( m_changingClass )
	{
		despatchSetParameterisedCallbacks();
	}
	else
	{
		despatchClassSetCallbacks();
	}
	
	return s;
}

MStatus ParameterisedHolderClassModificationCmd::redoIt()
{	
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	if( m_changingClass )
	{
		m_parameterisedHolder->setParameterised( m_newClassName.asChar(), m_newClassVersion, m_newSearchPathEnvVar.asChar() );
		despatchSetParameterisedCallbacks();
	}
	else
	{
		restoreClassParameterStates( m_newClassInfo, m_parameterisedHolder->getParameterisedInterface()->parameters(), "" );
		m_parameterisedHolder->updateParameterised();
		despatchClassSetCallbacks();
	}
	
	return MS::kSuccess;
}

void ParameterisedHolderClassModificationCmd::storeClassParameterStates( ClassInfo &classInfo, const IECore::Parameter *parameter, const std::string &parentParameterPath, bool changedOnly )
{
	std::string parameterPath = parentParameterPath;
	if( parentParameterPath.size() )
	{
		parameterPath += ".";
	}
	parameterPath += parameter->name();
	
	if( parameter->isInstanceOf( "ClassParameter" ) )
	{
		MPlug parameterPlug = m_parameterisedHolder->parameterPlug( parameter );
		if( !parameterPlug.isNull() )
		{
			ClassParameterInfo mayaInfo;
			ClassParameterHandler::currentClass( parameterPlug, mayaInfo.className, mayaInfo.classVersion, mayaInfo.searchPathEnvVar );
			ClassParameterInfo realInfo;
			ClassParameterHandler::getClass( parameter, realInfo.className, realInfo.classVersion, realInfo.searchPathEnvVar );
			if( !changedOnly || mayaInfo != realInfo )
			{
				classInfo.classParameters[parameterPath] = mayaInfo;
			}
		}
	}
	else if( parameter->isInstanceOf( "ClassVectorParameter" ) )
	{
		MPlug parameterPlug = m_parameterisedHolder->parameterPlug( parameter );
		if( !parameterPlug.isNull() )
		{
			ClassVectorParameterInfo mayaInfo;
			ClassVectorParameterHandler::currentClasses( parameterPlug, mayaInfo.parameterNames, mayaInfo.classNames, mayaInfo.classVersions );
			ClassVectorParameterInfo realInfo;
			ClassVectorParameterHandler::getClasses( parameter, realInfo.parameterNames, realInfo.classNames, realInfo.classVersions );
			if( !changedOnly || mayaInfo != realInfo )
			{
				classInfo.classVectorParameters[parameterPath] = mayaInfo;
			}
		}
	}
	
	if( parameter->isInstanceOf( IECore::CompoundParameter::staticTypeId() ) )
	{
		const CompoundParameter *compoundParameter = static_cast<const CompoundParameter *>( parameter );
		const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
		for( CompoundParameter::ParameterVector::const_iterator it = childParameters.begin(); it!=childParameters.end(); it++ )
		{
			storeClassParameterStates( classInfo, *it, parameterPath, changedOnly );
		}
	}
}

void ParameterisedHolderClassModificationCmd::restoreClassParameterStates( const ClassInfo &classInfo, IECore::Parameter *parameter, const std::string &parentParameterPath )
{
	std::string parameterPath = parentParameterPath;
	if( parentParameterPath.size() )
	{
		parameterPath += ".";
	}
	parameterPath += parameter->name();
	
	if( parameter->isInstanceOf( "ClassParameter" ) )
	{		
		ClassParameterInfoMap::const_iterator it = classInfo.classParameters.find( parameterPath );
		if( it!=classInfo.classParameters.end() )
		{
			ClassParameterHandler::setClass( parameter, it->second.className, it->second.classVersion, it->second.searchPathEnvVar );
		}
	}
	else if( parameter->isInstanceOf( "ClassVectorParameter" ) )
	{		
		ClassVectorParameterInfoMap::const_iterator it = classInfo.classVectorParameters.find( parameterPath );
		if( it!=classInfo.classVectorParameters.end() )
		{
			ClassVectorParameterHandler::setClasses( parameter, it->second.parameterNames, it->second.classNames, it->second.classVersions );
		}
	}
	
	if( parameter->isInstanceOf( IECore::CompoundParameter::staticTypeId() ) )
	{
		CompoundParameter *compoundParameter = static_cast<CompoundParameter *>( parameter );
		const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
		for( CompoundParameter::ParameterVector::const_iterator it = childParameters.begin(); it!=childParameters.end(); it++ )
		{
			restoreClassParameterStates( classInfo, constPointerCast<Parameter>( *it ), parameterPath );
		}
	}
}

void ParameterisedHolderClassModificationCmd::despatchSetParameterisedCallbacks() const
{
	MFnDependencyNode fnNode( m_node );
	MString nodeName = fnNode.name();
	MFnDagNode fnDN( m_node );
	if( fnDN.hasObj( m_node ) )
	{
		nodeName = fnDN.fullPathName();
	}
	
	MGlobal::executePythonCommand( "import IECoreMaya; IECoreMaya.FnParameterisedHolder._despatchSetParameterisedCallbacks( \"" + nodeName + "\" )" );
}

void ParameterisedHolderClassModificationCmd::despatchClassSetCallbacks() const
{
	MFnDependencyNode fnNode( m_node );
	MString nodeName = fnNode.name();
	MFnDagNode fnDN( m_node );
	if( fnDN.hasObj( m_node ) )
	{
		nodeName = fnDN.fullPathName();
	}
	
	ParameterisedInterface *parameterised = m_parameterisedHolder->getParameterisedInterface();

	ClassParameterInfoMap::const_iterator it;
	for( it = m_originalClassInfo.classParameters.begin(); it != m_originalClassInfo.classParameters.end(); it++ )
	{
		Parameter *parameter = parameterFromPath( parameterised, it->first );
		MPlug parameterPlug = m_parameterisedHolder->parameterPlug( parameter );
		MString plugName = nodeName + "." + parameterPlug.partialName();
		MGlobal::executePythonCommand( "import IECoreMaya; IECoreMaya.FnParameterisedHolder._despatchSetClassParameterClassCallbacks( \"" + plugName + "\" )" );
	}
	
	ClassVectorParameterInfoMap::const_iterator vit;
	for( vit = m_originalClassInfo.classVectorParameters.begin(); vit != m_originalClassInfo.classVectorParameters.end(); vit++ )
	{
		Parameter *parameter = parameterFromPath( parameterised, vit->first );
		MPlug parameterPlug = m_parameterisedHolder->parameterPlug( parameter );
		MString plugName = nodeName + "." + parameterPlug.partialName();
		MGlobal::executePythonCommand( "import IECoreMaya; IECoreMaya.FnParameterisedHolder._despatchSetClassVectorParameterClassesCallbacks( \"" + plugName + "\" )" );
	}
}

IECore::Parameter *ParameterisedHolderClassModificationCmd::parameterFromPath( ParameterisedInterface *parameterised, const std::string &path ) const
{
	std::vector<std::string> names;
	boost::split( names, path, boost::is_any_of( "." ) );
	
	CompoundParameter *parent = parameterised->parameters();
	for( int i=0; i<(int)names.size(); i++ )
	{
		if( i==((int)names.size()-1) )
		{
			return parent->parameter<Parameter>( names[i] );
		}
		else
		{
			parent = parent->parameter<CompoundParameter>( names[i] );
		}
	}
	
	return 0;
}
