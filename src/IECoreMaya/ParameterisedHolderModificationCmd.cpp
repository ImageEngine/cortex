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
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreMaya/ParameterisedHolderModificationCmd.h"
#include "IECoreMaya/ClassParameterHandler.h"
#include "IECoreMaya/ClassVectorParameterHandler.h"

using namespace IECoreMaya;
using namespace IECore;

IECore::ConstObjectPtr ParameterisedHolderModificationCmd::g_originalValue = 0;
IECore::ConstCompoundDataPtr ParameterisedHolderModificationCmd::g_originalClasses = 0;
IECore::ConstObjectPtr ParameterisedHolderModificationCmd::g_newValue = 0;
IECore::ConstCompoundDataPtr ParameterisedHolderModificationCmd::g_newClasses = 0;

ParameterisedHolderModificationCmd::ParameterisedHolderModificationCmd()
	:	m_parameterisedHolder( 0 ), m_originalValues( 0 ), m_newValues( 0 ), m_changingClass( false )
{
}

ParameterisedHolderModificationCmd::~ParameterisedHolderModificationCmd()
{
}

void *ParameterisedHolderModificationCmd::creator()
{
	return new ParameterisedHolderModificationCmd;
}

bool ParameterisedHolderModificationCmd::isUndoable() const
{
	return true;
}

bool ParameterisedHolderModificationCmd::hasSyntax() const
{
	return false;
}

MStatus ParameterisedHolderModificationCmd::doIt( const MArgList &argList )
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
		
	// store the original and new values of everything. these are just passed in from
	// the FnParameterisedHolder. in the case of changing the held class we won't have
	// any new values.
	
	m_originalValues = g_originalValue;
	m_originalClasses = g_originalClasses;
	m_newValues = g_newValue;
	m_newClasses = g_newClasses;
	
	g_originalValue = 0;
	g_originalClasses = 0;
	g_newValue = 0;
	g_newClasses = 0;
	
	// change the maya side class or monkey with the maya side class parameters as requested. then remember the new values
	// of everything and which parameters are changing so we can push them in and out during undo and redo.
	
	if( m_changingClass )
	{
		MStatus s = m_parameterisedHolder->setParameterised( m_newClassName.asChar(), m_newClassVersion, m_newSearchPathEnvVar.asChar() );
		if ( !s )
		{
			return s;
		}
		m_newValues = m_parameterisedHolder->getParameterisedInterface()->parameters()->getValue()->copy();
		storeParametersWithNewValues( m_originalValues.get(), m_newValues.get(), "" );
		despatchSetParameterisedCallbacks();
	}
	else
	{
		storeParametersWithNewValues( m_originalValues.get(), m_newValues.get(), "" );	
		m_parameterisedHolder->updateParameterised();
		setNodeValuesForParametersWithNewValues();
		despatchClassSetCallbacks();
	}
		
	return MS::kSuccess;
}

MStatus ParameterisedHolderModificationCmd::undoIt()
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
	
	if( m_originalClasses->readable().size() )
	{
		restoreClassParameterStates( m_originalClasses.get(), m_parameterisedHolder->getParameterisedInterface()->parameters().get(), "" );
		m_parameterisedHolder->updateParameterised();
	}
	
	if( m_originalValues )
	{
		m_parameterisedHolder->getParameterisedInterface()->parameters()->setValue( m_originalValues->copy() );
		setNodeValuesForParametersWithNewValues();
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

MStatus ParameterisedHolderModificationCmd::redoIt()
{	
	if( !m_parameterisedHolder )
	{
		return MStatus::kFailure;
	}

	if( m_changingClass )
	{
		MStatus s = m_parameterisedHolder->setParameterised( m_newClassName.asChar(), m_newClassVersion, m_newSearchPathEnvVar.asChar() );
		if ( !s )
		{
			return s;
		}
		despatchSetParameterisedCallbacks();
	}
	else
	{
		restoreClassParameterStates( m_newClasses.get(), m_parameterisedHolder->getParameterisedInterface()->parameters().get(), "" );
		m_parameterisedHolder->getParameterisedInterface()->parameters()->setValue( m_newValues->copy() );
		m_parameterisedHolder->updateParameterised();
		setNodeValuesForParametersWithNewValues();
		despatchClassSetCallbacks();
	}
	
	return MS::kSuccess;
}

void ParameterisedHolderModificationCmd::restoreClassParameterStates( const IECore::CompoundData *classes, IECore::Parameter *parameter, const std::string &parentParameterPath )
{
	std::string parameterPath = parentParameterPath;
	if( parentParameterPath.size() )
	{
		parameterPath += ".";
	}
	parameterPath += parameter->name();
		
	if( parameter->isInstanceOf( "ClassParameter" ) )
	{				
		const CompoundData *c = classes->member<const CompoundData>( parameterPath );
		if( c )
		{
			ClassParameterHandler::setClass(
				parameter,
				c->member<const IECore::StringData>( "className" )->readable().c_str(),
				c->member<const IECore::IntData>( "classVersion" )->readable(),
				c->member<const IECore::StringData>( "searchPathEnvVar" )->readable().c_str()
			);
		}
	}
	else if( parameter->isInstanceOf( "ClassVectorParameter" ) )
	{		
		const CompoundData *c = classes->member<const CompoundData>( parameterPath );
		if( c )
		{
			IECore::ConstStringVectorDataPtr parameterNames = c->member<const IECore::StringVectorData>( "parameterNames" );
			IECore::ConstStringVectorDataPtr classNames = c->member<const IECore::StringVectorData>( "classNames" );
			IECore::ConstIntVectorDataPtr classVersions = c->member<const IECore::IntVectorData>( "classVersions" );
			MStringArray mParameterNames;
			MStringArray mClassNames;
			MIntArray mClassVersions;
			int numClasses = parameterNames->readable().size();
			for( int i=0; i<numClasses; i++ )
			{
				mParameterNames.append( parameterNames->readable()[i].c_str() );
				mClassNames.append( classNames->readable()[i].c_str() );
				mClassVersions.append( classVersions->readable()[i] );
			}
			ClassVectorParameterHandler::setClasses( parameter, mParameterNames, mClassNames, mClassVersions );
		}
	}
	
	if( parameter->isInstanceOf( IECore::CompoundParameter::staticTypeId() ) )
	{
		CompoundParameter *compoundParameter = static_cast<CompoundParameter *>( parameter );
		const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
		for( CompoundParameter::ParameterVector::const_iterator it = childParameters.begin(); it!=childParameters.end(); it++ )
		{
			restoreClassParameterStates( classes, it->get(), parameterPath );
		}
	}
}

void ParameterisedHolderModificationCmd::storeParametersWithNewValues( const IECore::Object *originalValue, const IECore::Object *newValue, const std::string &parameterPath )
{
	if( !(originalValue && newValue) )
	{
		// if either of the values isn't present then a parameter is appearing and disappearing due to
		// Class*Parameter edits. we treat it as a parameter with a new value so that it'll get it's value
		// transferred into maya appropriately.
		m_parametersWithNewValues.insert( parameterPath );
	}
	else if( originalValue->typeId() != newValue->typeId() )
	{
		// types are different so there's clearly a new value involved
		m_parametersWithNewValues.insert( parameterPath );
	}
	else if( originalValue->isInstanceOf( CompoundObject::staticTypeId() ) )
	{
		// compound value, representing several child parameters - attempt to recurse.
		// we need to consider children of both the original and new parameters in case a parameter
		// exists only on one side.
		const CompoundObject *originalCompound = static_cast<const CompoundObject *>( originalValue );
		const CompoundObject *newCompound = static_cast<const CompoundObject *>( newValue );
		const CompoundObject::ObjectMap &originalChildren = originalCompound->members();
		for( CompoundObject::ObjectMap::const_iterator it = originalChildren.begin(); it!=originalChildren.end(); it++ )
		{
			std::string childParameterPath;
			if( parameterPath.size() )
			{
				childParameterPath = parameterPath + "." + it->first.value();
			}
			else
			{
				childParameterPath = it->first;
			}
			storeParametersWithNewValues( it->second.get(), newCompound->member<Object>( it->first ), childParameterPath );
		}
		
		const CompoundObject::ObjectMap &newChildren = static_cast<const CompoundObject *>( newValue )->members();
		for( CompoundObject::ObjectMap::const_iterator it = newChildren.begin(); it!=newChildren.end(); it++ )
		{
			if( originalChildren.find( it->first )==originalChildren.end() )
			{
				// this is a child we didn't encounter in the first iteration.
				std::string childParameterPath;
				if( parameterPath.size() )
				{
					childParameterPath = parameterPath + "." + it->first.value();
				}
				else
				{
					childParameterPath = it->first;
				}
				storeParametersWithNewValues( 0, it->second.get(), childParameterPath );
			}
		}
	}
	else
	{
		if( !originalValue->isEqualTo( newValue ) )
		{
			m_parametersWithNewValues.insert( parameterPath );
		}
	}
}

void ParameterisedHolderModificationCmd::setNodeValuesForParametersWithNewValues() const
{
	ParameterisedInterface *parameterised = m_parameterisedHolder->getParameterisedInterface();
	for( std::set<std::string>::const_iterator it=m_parametersWithNewValues.begin(); it!=m_parametersWithNewValues.end(); it++ )
	{
		Parameter *p = parameterFromPath( parameterised, *it );
		if( p )
		{
			setNodeValue( p );
		}
	}
}

void ParameterisedHolderModificationCmd::setNodeValue( IECore::Parameter *parameter ) const
{
	m_parameterisedHolder->setNodeValue( parameter );
	
	if( parameter->isInstanceOf( CompoundParameter::staticTypeId() ) )
	{
		// recurse to the children - this is the only reason this function
		// is necessary as ParameterisedHolder::setNodeValue() doesn't recurse.
		CompoundParameter *compoundParameter = static_cast<CompoundParameter *>( parameter );
		const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
		for( CompoundParameter::ParameterVector::const_iterator it = childParameters.begin(); it!=childParameters.end(); it++ )
		{
			setNodeValue( it->get() );
		}
	}
}

void ParameterisedHolderModificationCmd::despatchSetParameterisedCallbacks() const
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

void ParameterisedHolderModificationCmd::despatchClassSetCallbacks() const
{
	MFnDependencyNode fnNode( m_node );
	MString nodeName = fnNode.name();
	MFnDagNode fnDN( m_node );
	if( fnDN.hasObj( m_node ) )
	{
		nodeName = fnDN.fullPathName();
	}
	
	ParameterisedInterface *parameterised = m_parameterisedHolder->getParameterisedInterface();

	std::set<IECore::InternedString> names;
	for( IECore::CompoundDataMap::const_iterator it=m_originalClasses->readable().begin(); it!=m_originalClasses->readable().end(); it++ )
	{
		names.insert( it->first );
	}
	for( IECore::CompoundDataMap::const_iterator it=m_newClasses->readable().begin(); it!=m_newClasses->readable().end(); it++ )
	{
		names.insert( it->first );
	}

	for( std::set<IECore::InternedString>::const_iterator it=names.begin(); it!=names.end(); it++ )
	{
		Parameter *parameter = parameterFromPath( parameterised, *it );
		if( parameter )
		{
			IECore::CompoundDataMap::const_iterator it1 = m_originalClasses->readable().find( *it );
			IECore::CompoundDataMap::const_iterator it2 = m_newClasses->readable().find( *it );			

			if( it1==m_originalClasses->readable().end() || it2==m_newClasses->readable().end() || !(it1->second->isEqualTo( it2->second.get() ) ) )
			{
				MPlug parameterPlug = m_parameterisedHolder->parameterPlug( parameter );
				MString plugName = nodeName + "." + parameterPlug.partialName();
				if( parameter->isInstanceOf( "ClassParameter" ) )
				{
					MGlobal::executePythonCommand( "import IECoreMaya; IECoreMaya.FnParameterisedHolder._despatchSetClassParameterClassCallbacks( \"" + plugName + "\" )" );
				}
				else
				{
					MGlobal::executePythonCommand( "import IECoreMaya; IECoreMaya.FnParameterisedHolder._despatchSetClassVectorParameterClassesCallbacks( \"" + plugName + "\" )" );
				}
			}		
		}
	}
}

IECore::Parameter *ParameterisedHolderModificationCmd::parameterFromPath( ParameterisedInterface *parameterised, const std::string &path ) const
{
	std::vector<std::string> names;
	boost::split( names, path, boost::is_any_of( "." ) );
	
	CompoundParameter *parent = parameterised->parameters().get();
	for( int i=0; i<(int)names.size(); i++ )
	{
		if( i==((int)names.size()-1) )
		{
			return parent->parameter<Parameter>( names[i] );
		}
		else
		{
			parent = parent->parameter<CompoundParameter>( names[i] );
			if( !parent )
			{
				return 0;
			}
		}
	}
	
	return 0;
}
