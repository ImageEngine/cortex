//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/bind.hpp"
#include "boost/python.hpp"

#include "GA/GA_AIFBlindData.h"

#include "IECore/CapturingRenderer.h"
#include "IECore/Group.h"
#include "IECore/Op.h"
#include "IECore/ObjectParameter.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/WorldBlock.h"

#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/ScopedGILRelease.h"

#include "IECoreHoudini/FromHoudiniGeometryConverter.h"
#include "IECoreHoudini/NodePassData.h"
#include "IECoreHoudini/SOP_ParameterisedHolder.h"

using namespace boost;
using namespace boost::python;

using namespace IECore;
using namespace IECoreHoudini;

SOP_ParameterisedHolder::SOP_ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op ) : ParameterisedHolder<SOP_Node>( net, name, op )
{
	IECoreHoudini::MessageHandler::HandlerFn errorFn = boost::bind( &SOP_ParameterisedHolder::addError, this, SOP_MESSAGE, _1 );
	IECoreHoudini::MessageHandler::HandlerFn warningFn = boost::bind( &SOP_ParameterisedHolder::addWarning, this, SOP_MESSAGE, _1 );
	IECoreHoudini::MessageHandler::HandlerFn infoFn = boost::bind( &SOP_ParameterisedHolder::addMessage, this, SOP_MESSAGE, _1 );
	IECore::MessageHandlerPtr h = new IECoreHoudini::MessageHandler( errorFn, warningFn, infoFn, infoFn );
	setMessageHandler( h );
}

SOP_ParameterisedHolder::~SOP_ParameterisedHolder()
{
}

void SOP_ParameterisedHolder::refreshInputConnections()
{
	// store the existing input nodes
	unsigned numInputs = nInputs();
	std::vector<OP_Node*> inputNodes;
	for ( unsigned i=0; i < numInputs; i++ )
	{
		OP_Node *input = getInput( i );
		if ( !input )
		{
			continue;
		}
		
		inputNodes.push_back( input );
	}
	
	// clear the input parameters and break the node connections
	disconnectAllInputs();
	m_inputParameters.clear();

	ParameterisedInterface *parameterised = getParameterisedInterface();
	if ( !parameterised )
	{
		return;
	}

	// add inputs for the appropriate parameters
	const CompoundParameter::ParameterVector &parameters = parameterised->parameters()->orderedParameters();
	for ( CompoundParameter::ParameterVector::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		// adding ObjectParameters explicitly so generic data can be passed through
		// ParameterisedHolders even if they don't make particular sense in SOPs.
		if ( (*it)->typeId() == ObjectParameterTypeId )
		{
			m_inputParameters.push_back( *it );
		}
		else
		{
			const ObjectParameter *objectParam = IECore::runTimeCast<ObjectParameter>( *it );
			if ( !objectParam )
			{
				continue;
			}
			
			FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( GU_DetailHandle(), objectParam->validTypes() );
			if ( converter )
			{
				m_inputParameters.push_back( *it );
				/// \todo: add converter parameters here once ParameterHandlers are defined in c++
			}
		}
		
		/// \todo: try using the messageHandler(). it might not work outside of a cook though...
		if ( m_inputParameters.size() > 4 )
		{
			std::cerr << "ieParameterisedHolder does not support more than 4 input parameter connections." << std::endl;
		}
	}

	// remake the connections to inputs that still exist
	numInputs = std::min( inputNodes.size(), m_inputParameters.size() );
	for ( unsigned i=0; i < numInputs; i++ )
	{
		setInput( i, inputNodes[i] );
	}
	
	/// \todo: Is this really the only we can get the gui to update the input connections?
	setXY( getX()+.0001, getY()+.0001 );
	setXY( getX()-.0001, getY()-.0001 );
}

void SOP_ParameterisedHolder::setInputParameterValues( float now )
{
	for ( unsigned int i=0; i < m_inputParameters.size(); i++ )
	{
		useInputSource( i, m_dirty, false );
		
		IECore::ParameterPtr inputParameter = m_inputParameters[i];
		
		GU_DetailHandle inputHandle = inputGeoHandle( i );
		GU_DetailHandleAutoReadLock readHandle( inputHandle );
		const GU_Detail *inputGdp = readHandle.getGdp();
		if ( !inputGdp )
		{
			continue;
		}
		
		const GA_ROAttributeRef attrRef = inputGdp->findAttribute( GA_ATTRIB_DETAIL, GA_SCOPE_PRIVATE, "IECoreHoudiniNodePassData" );
		if ( attrRef.isValid() )
		{
			// looks like data passed from another ParameterisedHolder
			const GA_Attribute *attr = attrRef.getAttribute();
			const GA_AIFBlindData *blindData = attr->getAIFBlindData();
			const NodePassData passData = blindData->getValue<NodePassData>( attr, 0 );
			SOP_ParameterisedHolder *sop = dynamic_cast<SOP_ParameterisedHolder*>( const_cast<OP_Node*>( passData.nodePtr() ) );
			
			IECore::ConstObjectPtr result = 0;
			if ( passData.type() == IECoreHoudini::NodePassData::CORTEX_OPHOLDER )
			{
				IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( sop->getParameterised() );
				result = op->resultParameter()->getValue();
			}
			else if ( passData.type() == IECoreHoudini::NodePassData::CORTEX_PROCEDURALHOLDER )
			{
				IECore::ParameterisedProcedural *procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( sop->getParameterised() );
				IECore::CapturingRendererPtr renderer = new IECore::CapturingRenderer();
				// We are acquiring and releasing the GIL here to ensure that it is released when we render. This has
				// to be done because a procedural might jump between c++ and python a few times (i.e. if it spawns
				// subprocedurals that are implemented in python). In a normal call to cookMySop, this wouldn't be an
				// issue, but if cookMySop was called from HOM, hou.Node.cook appears to be holding onto the GIL.
				IECorePython::ScopedGILLock gilLock;
				{
					IECorePython::ScopedGILRelease gilRelease;
					{
						IECore::WorldBlock worldBlock( renderer );
						procedural->render( renderer );
					}
				}
				result = IECore::runTimeCast<const IECore::Object>( renderer->world() );
			}
			else
			{
				continue;
			}
			
			try
			{
				inputParameter->setValidatedValue( IECore::constPointerCast<IECore::Object>( result ) );
			}
			catch ( const IECore::Exception &e )
			{
				addError( SOP_MESSAGE, e.what() );
			}
		}
		else
		{
			// looks like a regular Houdini detail
			IECore::ObjectParameterPtr objectParameter = IECore::runTimeCast<IECore::ObjectParameter>( inputParameter );
			if ( !objectParameter )
			{
				continue;
			}
			
			FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( inputHandle, objectParameter->validTypes() );
			if ( !converter )
			{
				continue;
			}
			
			// set converter parameters from the node values
			const CompoundParameter::ParameterVector &converterParameters = converter->parameters()->orderedParameters();
			for ( CompoundParameter::ParameterVector::const_iterator it=converterParameters.begin(); it != converterParameters.end(); ++it )
			{
				updateParameter( *it, now, "parm_" + inputParameter->name() + "_" );
			}
			
			try
			{
				IECore::ObjectPtr converted = converter->convert();
				if ( converted )
				{
					inputParameter->setValidatedValue( converted );
				}
			}
			catch ( const IECore::Exception &e )
			{
				addError( SOP_MESSAGE, e.what() );
			}
			catch ( std::runtime_error &e )
			{
				addError( SOP_MESSAGE, e.what() );
			}
		}
	}
}
