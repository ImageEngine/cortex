//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2015, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"
#include "boost/format.hpp"
#include "boost/tokenizer.hpp"

#include "maya/MPxNode.h"
#include "maya/MPxLocatorNode.h"
#include "maya/MPxDeformerNode.h"
#include "maya/MPxObjectSet.h"
#include "maya/MPxFieldNode.h"
#include "maya/MPxSurfaceShape.h"
#include "maya/MPxComponentShape.h"
#include "maya/MPxImagePlane.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnCompoundAttribute.h"
#include "maya/MObjectArray.h"
#include "maya/MPlugArray.h"
#include "maya/MDGModifier.h"
#include "maya/MNodeMessage.h"
#include "maya/MFnPluginData.h"
#include "maya/MFnMesh.h"
#include "maya/MFnGenericAttribute.h"
#include "maya/MPxManipContainer.h"
#undef None // must come after certain Maya includes which include X11/X.h

#include "IECoreMaya/ParameterisedHolder.h"
#include "IECoreMaya/ParameterHandler.h"
#include "IECoreMaya/PythonCmd.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/ObjectData.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECore/MessageHandler.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/ObjectVector.h"
#include "IECore/ObjectParameter.h"

#include <stdlib.h>

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;
using namespace boost;
using namespace std;

template<typename B>
MObject ParameterisedHolder<B>::aParameterisedClassName;
template<typename B>
MObject ParameterisedHolder<B>::aParameterisedVersion;
template<typename B>
MObject ParameterisedHolder<B>::aParameterisedSearchPathEnvVar;

template<typename B>
const std::string ParameterisedHolder<B>::g_attributeNamePrefix = "parm_";

template<typename B>
ParameterisedHolder<B>::PLCB::PLCB( ParameterisedHolder<B> *node) : m_node(node)
{
}

template<typename B>
void ParameterisedHolder<B>::PLCB::postLoad()
{
	assert(m_node);
	m_node->getParameterised();
	// remove the callback so we don't do this again later when other files are imported etc.
	m_node->m_plcb = 0;
}

template<typename B>
ParameterisedHolder<B>::ParameterisedHolder()
	:	m_parameterised( 0 ), m_failedToLoad( false )
{
	m_plcb = new PLCB( this );
}

template<typename B>
ParameterisedHolder<B>::~ParameterisedHolder()
{
}

template<typename B>
void ParameterisedHolder<B>::postConstructor()
{
	B::setExistWithoutInConnections(true);
	B::setExistWithoutOutConnections(true);
}

template<typename B>
MStatus ParameterisedHolder<B>::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if( plug==aParameterisedClassName || plug==aParameterisedVersion || plug==aParameterisedSearchPathEnvVar )
	{
		// if the held class changes in any way then we ditch it so we're forced to reload
		// in getParameterised().
		m_parameterised = 0;
		m_failedToLoad = false;
	}
	else
	{	
		// if the plug represents a parameter then we add that parameter to a list
		// of dirty parameters. this lets us optimise setParameterisedValues so we only
		// set the values of parameters whose plugs have changed since last time.
		// we only bother doing this if we've loaded the class already, as calling plugParameter()
		// would otherwise cause a premature loading of the class. when we load the class all parameters
		// are marked as dirty anyway so there's no point worrying about it here.
		if( m_parameterised )
		{
			MPlug p = plug;
			ParameterPtr parameter = 0;
			do
			{
				parameter = plugParameter( p );
				if( p.isChild() )
				{
					p = p.parent();
				}
				else if( p.isElement() )
				{
					p = p.array();
				}
				else
				{
					p = MPlug();
				}
			} while( !parameter && !p.isNull() );
			if( parameter )
			{
				m_dirtyParameters.insert( parameter );
			}
		}
	}
	
	return B::setDependentsDirty( plug, plugArray );
}

template<typename B>
MStatus ParameterisedHolder<B>::shouldSave( const MPlug &plug, bool &isSaving )
{
	ParameterPtr parameter = plugParameter( plug );
	if( parameter )
	{
		// For parameters we found in the parameter map, we always handle
		// them ourselves, setting isSaving to either true or false,
		// and returning kSuccess, which means "Ignore the default behaviour"
		// There is no particular reason not use the standard behaviour
		// of calling the parent shouldSave, but instead we set isSaving
		// to always true and store our parm_ attributes even when they
		// are at default values.  This is because it used to work that way
		// for all parms, and Lucio is concerned that changing this
		// could break something
		isSaving = true;


		/// Maya 8.5 and 2009 crash when saving a GenericAttribute (such as that
		/// created by the MeshParameterHandler) containing an "empty" mesh.
		/// This only applies to ASCII files, saving to binary works. Here
		/// we prevent Maya saving the value.
		MFnGenericAttribute fnGA;
		if( fnGA.hasObj( plug.attribute() ) )
		{
			MObject value = plug.asMObject();
			MFnMesh fnMesh( value );
			if( fnMesh.hasObj( value ) && fnMesh.numPolygons()==0 )
			{
				isSaving = false;
			}
		}

		return MS::kSuccess;
	}
	else
	{
		// For parameters that aren't special, use the default behaviour of
		// the base class.
		// NOTE: This is not very clear in the documentation, but for most
		// parameters, the default behaviour is to not touch isSaving,
		// and return kUnknownParameter, which Maya interprets as meaning
		// that we're not doing anything special, so use the default 
		// behaviour.  Maya then checks whether the plug has changed from
		// default, and exports it if it has
		return B::shouldSave( plug, isSaving );
	}
}

template<typename B>
void *ParameterisedHolder<B>::creator()
{
	return new ParameterisedHolder<B>();
}

template<typename B>
MStatus ParameterisedHolder<B>::initialize()
{
	MStatus s;
	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;

	aParameterisedClassName = tAttr.create( "className", "clas", MFnData::kString );
	tAttr.setReadable(true);
	tAttr.setWritable(true);
	tAttr.setStorable(true);
	tAttr.setConnectable(false);
	tAttr.setHidden(true);

	s = B::addAttribute( aParameterisedClassName );
	assert(s);

	aParameterisedVersion = nAttr.create("version", "ver", MFnNumericData::kInt, 1, &s );
	assert(s);
	nAttr.setReadable(true);
	nAttr.setWritable(true);
	nAttr.setStorable(true);
	nAttr.setConnectable(false);
	nAttr.setHidden(true);


	s = B::addAttribute( aParameterisedVersion );
	assert(s);

	aParameterisedSearchPathEnvVar = tAttr.create("searchPathEnvVar", "spev", MFnData::kString );
	tAttr.setReadable(true);
	tAttr.setWritable(true);
	tAttr.setStorable(true);
	tAttr.setConnectable(false);
	tAttr.setHidden(true);

	s = B::addAttribute( aParameterisedSearchPathEnvVar );
	assert(s);
	
	MPxManipContainer::addToManipConnectTable( id );

	return MS::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::setParameterised( IECore::RunTimeTypedPtr p )
{
	MPlug pClassName( B::thisMObject(), aParameterisedClassName );
	MPlug pVersion( B::thisMObject(), aParameterisedVersion );
	MPlug pSearchPathEnvVar( B::thisMObject(), aParameterisedSearchPathEnvVar );
	pClassName.setValue( "" );
	pVersion.setValue( 0 );
	pSearchPathEnvVar.setValue( "" );

	m_parameterised = p;
	m_failedToLoad = false;

	MStatus s = createAndRemoveAttributes();
	if( !s )
	{
		m_parameterised = 0;
		return s;
	}

	return MStatus::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar )
{
	MPlug pClassName( B::thisMObject(), aParameterisedClassName );
	MPlug pVersion( B::thisMObject(), aParameterisedVersion );
	MPlug pSearchPathEnvVar( B::thisMObject(), aParameterisedSearchPathEnvVar );
	pClassName.setValue( className.c_str() );
	pVersion.setValue( classVersion );
	pSearchPathEnvVar.setValue( searchPathEnvVar.c_str() );

	m_parameterised = 0;
	m_failedToLoad = false;

	if( getParameterised() )
	{
		return MStatus::kSuccess;
	}

	return MStatus::kFailure;
}

template<typename B>
MStatus ParameterisedHolder<B>::updateParameterised()
{
	return createAndRemoveAttributes();
}

template<typename B>
IECore::RunTimeTypedPtr ParameterisedHolder<B>::getParameterised( std::string *classNameOut, int *classVersionOut, std::string *searchPathEnvVarOut )
{
	MPlug pClassName( B::thisMObject(), aParameterisedClassName );
	MPlug pVersion( B::thisMObject(), aParameterisedVersion );
	MPlug pSearchPathEnvVar( B::thisMObject(), aParameterisedSearchPathEnvVar );

	MString className;
	int version;
	MString searchPathEnvVar;

	pClassName.getValue( className );

	pVersion.getValue( version );
	pSearchPathEnvVar.getValue( searchPathEnvVar );

	if( !m_parameterised && !m_failedToLoad )
	{
		m_failedToLoad = true;
		if( className!="" )
		{
			m_parameterised = loadClass( className, version, searchPathEnvVar );
			if( m_parameterised )
			{
				// we'll only create and remove attributes if we loaded successfully.
				// this avoids the situation where the loading fails due to some
				// correctable error, but we've just deleted all the attributes with
				// all the settings and connections important to the user.
				if( createAndRemoveAttributes( true ) )
				{
					m_failedToLoad = false;
				}
			}
		}
	}

	// fill output parameters
	if( m_parameterised )
	{
		if( classNameOut )
		{
			*classNameOut = className.asChar();
		}
		if( classVersionOut )
		{
			*classVersionOut = version;
		}
		if( searchPathEnvVarOut )
		{
			*searchPathEnvVarOut = searchPathEnvVar.asChar();
		}
	}

	return m_parameterised;
}

template<typename B>
MStatus ParameterisedHolder<B>::setNodeValues()
{
	// to update the parameter->name map if necessary
	getParameterised();

	MFnDependencyNode fnDN( B::thisMObject() );

	ParameterToAttributeNameMap::const_iterator it;
	for( it=m_parametersToAttributeNames.begin(); it!=m_parametersToAttributeNames.end(); it++ )
	{
		MPlug p = fnDN.findPlug( it->second );
		if( p.isNull() )
		{
			return MStatus::kFailure;
		}
		try
		{
			MStatus s = ParameterHandler::setValue( it->first, p );
			if( !s )
			{
				return s;
			}
		}
		catch( std::exception &e )
		{
			msg( Msg::Error, "ParameterisedHolder::setNodeValues", boost::format( "Caught exception while setting parameter value to attribute %s : %s" ) % p.name().asChar() % e.what());
			return MStatus::kFailure;
		}
		catch( ... )
		{
			msg( Msg::Error, "ParameterisedHolder::setNodeValues", boost::format( "Caught exception while setting parameter value to attribute %s." ) % p.name().asChar() );
			return MStatus::kFailure;
		}
	}
	return MStatus::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::setNodeValue( ParameterPtr pa )
{
	MPlug p = parameterPlug( pa );
	if( p.isNull() )
	{
		return MStatus::kFailure;
	}

	MStatus s = MS::kSuccess;

	try
	{
		s = IECoreMaya::ParameterHandler::setValue( pa, p );
	}
	catch ( std::exception &e )
	{
		msg( Msg::Error, "ParameterisedHolder::setNodeValues", boost::format( "Caught exception while setting parameter value to attribute %s : %s" ) % p.name().asChar() % e.what());
		s = MS::kFailure;
	}
	catch (...)
	{
		msg( Msg::Error, "ParameterisedHolder::setNodeValues", boost::format( "Caught exception while setting parameter value to attribute %s" ) % p.name().asChar());
		s = MS::kFailure;
	}

	return s;
}

template<typename B>
MStatus ParameterisedHolder<B>::setParameterisedValues()
{
	return setParameterisedValues( false /* not lazy */ );
}

template<typename B>
MStatus ParameterisedHolder<B>::setParameterisedValues( bool lazy )
{
	ParameterisedInterface *parameterisedInterface = getParameterisedInterface();
	if( !parameterisedInterface )
	{
		return MS::kFailure;
	}
	
	MStatus s;
	setParameterisedValuesWalk( lazy, parameterisedInterface->parameters(), s );
	return s;
}

template<typename B>
bool ParameterisedHolder<B>::setParameterisedValuesWalk( bool lazy, IECore::ParameterPtr parameter, MStatus &status )
{
	MFnDependencyNode fnDN( B::thisMObject() );
	
	// traverse child parameters if we have them
	
	bool childParametersWereSet = false;
	if( parameter->isInstanceOf( CompoundParameter::staticTypeId() ) )
	{
		CompoundParameterPtr compoundParameter = boost::static_pointer_cast<CompoundParameter>( parameter );
		const CompoundParameter::ParameterVector &childParameters = compoundParameter->orderedParameters();
		for( CompoundParameter::ParameterVector::const_iterator cIt=childParameters.begin(); cIt!=childParameters.end(); cIt++ )
		{
			bool b = setParameterisedValuesWalk( lazy, *cIt, status );
			childParametersWereSet = childParametersWereSet || b;
		}
	}

	// then set this parameter if necessary
	
	bool thisParameterWasSet = false;
	if( parameter->name()!="" && (!lazy || m_dirtyParameters.find( parameter )!=m_dirtyParameters.end()) )
	{
		ParameterToAttributeNameMap::const_iterator nIt = m_parametersToAttributeNames.find( parameter );
		if( nIt==m_parametersToAttributeNames.end() )
		{
			msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", boost::format( "Unable to find plug name for parameter %s" ) % parameter->name() );
			status = MS::kFailure;
		}
		else
		{
		
			MPlug p = fnDN.findPlug( nIt->second );
			if( p.isNull() )
			{
				msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", boost::format( "Unable to find plug for parameter %s" ) %  parameter->name() );
				status = MS::kFailure;
			}
			else
			{
				try
				{
					MStatus s = ParameterHandler::setValue( p, parameter );
					if( !s )
					{
						msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", boost::format( "Failed to set parameter value from %s" ) % p.name().asChar() );
						status = s;
					}
					else
					{
						m_dirtyParameters.erase( parameter );
						thisParameterWasSet = true;
					}
				}
				catch( std::exception &e )
				{
					msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", boost::format( "Caught exception while setting parameter value from %s : %s" ) % p.name().asChar() % e.what());
					status = MS::kFailure;
				}
				catch( ... )
				{
					msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", boost::format( "Caught unknown exception while setting parameter value from %s" ) % p.name().asChar() );
					status = MS::kFailure;
				}
			}
		}
	}
	
	// increment the updateCount if necessary
	
	if( thisParameterWasSet || childParametersWereSet )
	{
		CompoundObjectPtr userData = parameter->userData();
		IntDataPtr updateCount = userData->member<IntData>( "updateCount" );
		if( !updateCount )
		{
			updateCount = new IntData( 0 );
			userData->members()["updateCount"] = updateCount;
		}
		else
		{
			updateCount->writable()++;
		}
	}
	
	return childParametersWereSet || thisParameterWasSet;
}

template<typename B>
MStatus ParameterisedHolder<B>::setParameterisedValue( ParameterPtr pa )
{
	MPlug p = parameterPlug( pa );
	if( p.isNull() )
	{
		return MStatus::kFailure;
	}

	MStatus s = MS::kSuccess;

	try
	{
		s = IECoreMaya::ParameterHandler::setValue( p, pa );
		if( s )
		{
			m_dirtyParameters.erase( pa );
		}
	}
	catch ( std::exception &e )
	{
		msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", boost::format( "Caught exception while setting parameter value from %s : %s" ) % p.name().asChar() % e.what());
		s = MS::kFailure;
	}
	catch (...)
	{
		msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", boost::format( "Caught exception while setting parameter value from %s" ) % p.name().asChar());
		s = MS::kFailure;
	}

	return s;
}

template<typename B>
MPlug ParameterisedHolder<B>::parameterPlug( IECore::ConstParameterPtr parameter )
{
	// to update the parameter->name map if necessary
	getParameterised();

	ParameterToAttributeNameMap::const_iterator it = m_parametersToAttributeNames.find( boost::const_pointer_cast<IECore::Parameter>( parameter ) );
	if( it==m_parametersToAttributeNames.end() )
	{
		return MPlug();
	}

	MFnDependencyNode fnDN( B::thisMObject() );
	return MPlug( B::thisMObject(), fnDN.attribute( it->second ) );
}

template<typename B>
IECore::ParameterPtr ParameterisedHolder<B>::plugParameter( const MPlug &plug )
{
	assert( ! plug.isNull() );

	// to update the parameter->name map if necessary
	getParameterised();
	AttributeNameToParameterMap::const_iterator it = m_attributeNamesToParameters.find( plug.partialName() );
	if( it==m_attributeNamesToParameters.end() )
	{
		return 0;
	}
	return it->second;
}

template<typename B>
IECore::RunTimeTypedPtr ParameterisedHolder<B>::loadClass( const MString &className, int classVersion, const MString &searchPathEnvVar )
{
	string toExecute = boost::str( format(
			"IECore.ClassLoader.defaultLoader( \"%s\" ).load( \"%s\", %d )()\n"
		) % searchPathEnvVar.asChar() % className.asChar() % classVersion
	);

	IECorePython::ScopedGILLock gilLock;

	try
	{
		handle<> resultHandle( PyRun_String(
			toExecute.c_str(),
			Py_eval_input, PythonCmd::globalContext().ptr(),
			PythonCmd::globalContext().ptr() )
		);
		object result( resultHandle );
		return extract<RunTimeTypedPtr>( result )();
	}
	catch( error_already_set & )
	{
		MFnDependencyNode fnDN( B::thisMObject() );

		msg( Msg::Error, "ParameterisedHolder::loadClass",
			boost::format( "Unable to load class \"%s\" version %d into node %s." ) % className.asChar() % classVersion % fnDN.name().asChar());

		PyErr_Print();
	}
	catch( ... )
	{
		MFnDependencyNode fnDN( B::thisMObject() );

		msg( Msg::Error, "ParameterisedHolder::loadClass",
			boost::format( "Unable to load class \"%s\" version %d into node %s." ) % className.asChar() % classVersion % fnDN.name().asChar());
	}
	return 0;
}

template<typename B>
MStatus ParameterisedHolder<B>::createAndRemoveAttributes( bool callRestore )
{
	m_attributeNamesToParameters.clear();
	m_parametersToAttributeNames.clear();

	MStatus s;
	if( m_parameterised )
	{
		ParameterisedInterface *parameterisedInterface = dynamic_cast<ParameterisedInterface *>( m_parameterised.get() );
		s = createAttributesWalk( parameterisedInterface->parameters(), "parm", callRestore );
		if( !s )
		{
			msg( Msg::Error, "ParameterisedHolder::createAndRemoveAttributes", boost::format( "Unable to create attributes to represent class." ) );
			return s;
		}
	}

	s = removeUnecessaryAttributes();
	if( !s )
	{
		msg( Msg::Error, "ParameterisedHolder::createAndRemoveAttributes", "Failed to remove unecessary attributes." );
		return s;
	}

	return MS::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::createAttributesWalk( IECore::ConstCompoundParameterPtr parameter, const std::string &rootName, bool callRestore )
{

	const CompoundParameter::ParameterVector &children = parameter->orderedParameters();
	for( size_t i=0; i<children.size(); i++ )
	{
		string attributeName = rootName + "_" + children[i]->name();
		MString mAttributeName = attributeName.c_str();

		m_attributeNamesToParameters[mAttributeName] = children[i];
		m_parametersToAttributeNames[children[i]] = mAttributeName;
		m_dirtyParameters.insert( children[i] );

		MStatus s = createOrUpdateAttribute( children[i], mAttributeName, callRestore );
		if( !s )
		{
			return s;
		}	

		// recurse to the children if this is a compound child
		CompoundParameterPtr compoundChild = runTimeCast<CompoundParameter>( children[i] );
		if( compoundChild )
		{
			MStatus s = createAttributesWalk( compoundChild, rootName + "_" + compoundChild->name(), callRestore );
			if( !s )
			{
				return s;
			}
		}
	}

	return MS::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::createOrUpdateAttribute( IECore::ParameterPtr parameter, const MString &attributeName, bool callRestore )
{
	MObject node =  B::thisMObject();
	MFnDependencyNode fnDN( node );

	MPlugArray connectionsFromMe, connectionsToMe;

	// try to reuse an old plug if we can
	MPlug plug = fnDN.findPlug( attributeName, false /* no networked plugs please */ );
	if( !plug.isNull() )
	{
		MStatus s = MS::kSuccess;
		if( callRestore )
		{
			ParameterHandler::restore( plug, parameter );
		}
	
		if( s )
		{
			s = IECoreMaya::ParameterHandler::update( parameter, plug );
			if( s )
			{
				return MS::kSuccess;
			}
		}
		
		// failed to restore and/or update (parameter type probably changed).
		// remove the current attribute and fall through to the create
		// code

		// remember connections so we can remake them for the new
		// attribute. we have to be careful to only store non-networked plugs as
		// networked plugs are invalidated by the removal of the attribute.
		nonNetworkedConnections( plug, connectionsFromMe, connectionsToMe );
		
		fnDN.removeAttribute( plug.attribute() );
	}

	// reuse failed - create a new attribute
	/// \todo: update ParameterisedHolder to accept null plugs when the todo in ParameterHandler::create is addressed
	plug = IECoreMaya::ParameterHandler::create( parameter, attributeName, node );
	if( plug.isNull() )
	{
		msg( Msg::Error, "ParameterisedHolder::createOrUpdateAttribute", boost::format( "Failed to create attribute to represent parameter \"%s\" of type \"%s\"" ) % parameter->name() % parameter->typeName() );
		return MS::kFailure;
	}

	// restore any existing connections
	if( connectionsFromMe.length() || connectionsToMe.length() )
	{
		MDGModifier dgMod;
		for (unsigned i = 0; i < connectionsFromMe.length(); i++)
		{
			dgMod.connect( plug, connectionsFromMe[i] );
		}
		for (unsigned i = 0; i < connectionsToMe.length(); i++)
		{
			dgMod.connect( connectionsToMe[i], plug );
		}

		dgMod.doIt();
	}

	/// and set the value of the attribute, in case it differs from the default
	return IECoreMaya::ParameterHandler::setValue( parameter, plug );
}

template<typename B>
MStatus ParameterisedHolder<B>::removeUnecessaryAttributes()
{
	MObjectArray toRemove;
	MFnDependencyNode fnDN( B::thisMObject() );
	for( unsigned i=0; i<fnDN.attributeCount(); i++ )
	{
		MObject attr = fnDN.attribute( i );
		MFnAttribute fnAttr( attr );

		MString attrName = fnAttr.name();
		if( 0==strncmp( attrName.asChar(), g_attributeNamePrefix.c_str(), g_attributeNamePrefix.size() ) )
		{
			if( m_attributeNamesToParameters.find( fnAttr.name() )==m_attributeNamesToParameters.end() )
			{
				MPlug plug( B::thisMObject(), attr );
				plug.setLocked( false ); // we can't remove things if they're locked
				if( fnAttr.parent().isNull() )
				{
					toRemove.append( attr );
				}
				else
				{
					// we don't need to remove attributes which are the children
					// of compounds as they'll be removed when their parent is removed
				}
			}
		}
	}
	for( unsigned i=0; i<toRemove.length(); i++ )
	{
		MStatus s = fnDN.removeAttribute( toRemove[i] );
		if( !s )
		{
			return s;
		}
	}

	return MStatus::kSuccess;
}

template<typename B>
void ParameterisedHolder<B>::nonNetworkedConnections( const MPlug &plug, MPlugArray &connectionsFromPlug, MPlugArray &connectionsToPlug ) const
{
	MPlugArray from;
	MPlugArray to;
	
	// the MPlug.connectedTo() method is documented as always returning networked plugs.
	plug.connectedTo( from, false, true );
	plug.connectedTo( to, true, false );

	connectionsFromPlug.clear(); connectionsFromPlug.setLength( from.length() );
	connectionsToPlug.clear(); connectionsToPlug.setLength( to.length() );

	for( unsigned i=0; i<from.length(); i++ )
	{
		// the MPlug( node, attribute ) constructor is documented as always returning non-networked plugs.
		connectionsFromPlug.set( MPlug( from[i].node(), from[i].attribute() ), i );
	}
	
	for( unsigned i=0; i<to.length(); i++ )
	{
		connectionsToPlug.set( MPlug( to[i].node(), to[i].attribute() ), i );
	}
}

// specialisations of the different typeIds
template<>
MTypeId ParameterisedHolderNode::id( ParameterisedHolderNodeId );

template<>
MString ParameterisedHolderNode::typeName( "ieParameterisedHolderNode" );

template<>
MTypeId ParameterisedHolderLocator::id( ParameterisedHolderLocatorId );

template<>
MString ParameterisedHolderLocator::typeName( "ieParameterisedHolderLocator" );

template<>
MTypeId ParameterisedHolderDeformer::id( ParameterisedHolderDeformerId );

template<>
MString ParameterisedHolderDeformer::typeName( "ieParameterisedHolderDeformer" );

template<>
MTypeId ParameterisedHolderField::id( ParameterisedHolderFieldId );

template<>
MString ParameterisedHolderField::typeName( "ieParameterisedHolderField" );

template<>
MTypeId ParameterisedHolderSet::id( ParameterisedHolderSetId );

template<>
MString ParameterisedHolderSet::typeName( "ieParameterisedHolderSet" );

template<>
MTypeId ParameterisedHolderSurfaceShape::id( ParameterisedHolderSurfaceShapeId );

template<>
MString ParameterisedHolderSurfaceShape::typeName( "ieParameterisedHolderSurfaceShape" );

template<>
MTypeId ParameterisedHolderComponentShape::id( ParameterisedHolderComponentShapeId );

template<>
MString ParameterisedHolderComponentShape::typeName( "ieParameterisedHolderComponentShape" );

template<>
MTypeId ParameterisedHolderImagePlane::id( ParameterisedHolderImagePlaneId );

template<>
MString ParameterisedHolderImagePlane::typeName( "ieParameterisedHolderImagePlane" );

// explicit instantiation
template class ParameterisedHolder<MPxNode>;
template class ParameterisedHolder<MPxLocatorNode>;
template class ParameterisedHolder<MPxDeformerNode>;
template class ParameterisedHolder<MPxFieldNode>;
template class ParameterisedHolder<MPxObjectSet>;
template class ParameterisedHolder<MPxSurfaceShape>;
template class ParameterisedHolder<MPxComponentShape>;
template class ParameterisedHolder<MPxImagePlane>;
