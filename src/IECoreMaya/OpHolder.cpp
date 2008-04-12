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

#include "boost/python.hpp"
#include <iostream>
#include <cassert>

#include "boost/format.hpp"

#include "maya/MPxNode.h"
#include "maya/MPlugArray.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnAttribute.h"
#include "maya/MDGModifier.h"
#include "maya/MGlobal.h"

#include "IECoreMaya/OpHolder.h"
#include "IECoreMaya/Parameter.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECore/MessageHandler.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/TypedParameter.h"

#include "IECore/Parameterised.h"

using namespace IECore;
using namespace IECoreMaya;

template<typename B>
OpHolder<B>::OpHolder()
{	
}

template<typename B>
OpHolder<B>::~OpHolder()
{
}

template<typename B>
bool OpHolder<B>::isAbstractClass()
{
	return false;
}

template<typename B>
void *OpHolder<B>::creator()
{
	return new OpHolder<B>();
}

template<typename B>
MStatus OpHolder<B>::initialize()
{
	return inheritAttributesFrom( ParameterisedHolder<B>::typeName );
}

template<typename B>
MStatus OpHolder<B>::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	/// This isn't the best way of doing it, but at this point we can't even be sure that the Op has been loaded,
	/// so calling plugParameter() may not work. We can't call getOp() or getParameterised() here, as it seems
	/// we can't do things such as adding/removing attributes within this function
	if( std::string( plug.partialName().substring( 0, 4 ).asChar() ) == "parm_" )
	{
		MFnDependencyNode fnDN( B::thisMObject() );
		MStatus s;
		MPlug resultPlug = fnDN.findPlug( "result" , &s);
		if ( s && !plug.isNull() )
		{
			plugArray.append( resultPlug );
		}
	}
		
	return ParameterisedHolder<B>::setDependentsDirty( plug, plugArray );
}

template<typename B>
MStatus OpHolder<B>::compute( const MPlug &plug, MDataBlock &block )
{
	IECore::OpPtr op = getOp();
	
	if (op)
	{	
		MFnDependencyNode fnDN( B::thisMObject() );
		MPlug resultPlug( B::thisMObject(), fnDN.attribute( "result" ) );
	
		if (plug != resultPlug)
		{
			return MS::kUnknownParameter;
		}
	
		IECore::ObjectPtr result;
		
		try
		{
			ParameterisedHolder<B>::setParameterisedValues();
			result = op->operate();
			if (!result)
			{
				return MS::kFailure;
			}
		} 		
		catch( std::exception &e )
		{
			MGlobal::displayError( e.what() );
			return MS::kFailure;
		}
		catch( boost::python::error_already_set & )
		{
			PyErr_Print();
			return MS::kFailure;			
		}		
		catch (...)
		{
			return MS::kFailure;		
		}
			
		assert( result );				
		MStatus s = Parameter::setValue( op->resultParameter(), resultPlug );
		
		block.setClean( resultPlug );
		
		return s;
	}

	return MS::kFailure;
}

template<typename B>
IECore::ParameterisedPtr OpHolder<B>::getParameterised( std::string *classNameOut, int *classVersionOut, std::string *searchPathEnvVarOut )
{
	if (!ParameterisedHolder<B>::m_parameterised && !ParameterisedHolder<B>::m_failedToLoad)
	{
		ParameterisedHolder<B>::getParameterised( classNameOut, classVersionOut, searchPathEnvVarOut );

		if (ParameterisedHolder<B>::m_parameterised && !ParameterisedHolder<B>::m_failedToLoad)
		{
			ParameterisedHolder<B>::m_failedToLoad = true;
		
			IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( ParameterisedHolder<B>::m_parameterised );
		
			if (op)
			{
				MFnDependencyNode fnDN( B::thisMObject() );
							
				MObject attribute = fnDN.attribute( "result" );
								
				MPlugArray connectionsFromMe, connectionsToMe;
				
				if( !attribute.isNull() )
				{
					MFnAttribute fnAttr( attribute );
					fnAttr.setWritable( false );
					fnAttr.setStorable( false );					
					
					MStatus s = IECoreMaya::Parameter::update( op->resultParameter(), attribute );
					
					if (s)
					{
						ParameterisedHolder<B>::m_failedToLoad = false;
						
						return ParameterisedHolder<B>::m_parameterised;
					}
					// failed to update (parameter type probably changed).
					// remove the current attribute and fall through to the create
					// code
					
					MPlug plug( B::thisMObject(), attribute );
					plug.connectedTo( connectionsFromMe, false, true );
					plug.connectedTo( connectionsToMe, true, false );
					
					/// Make sure we keep the parameter's value as held in the attribute before we remove it!
					ParameterisedHolder<B>::setParameterisedValues();

					fnDN.removeAttribute( attribute );
				}
			
				attribute = IECoreMaya::Parameter::create( op->resultParameter(), "result" );
				MStatus s = fnDN.addAttribute( attribute );
				
				MFnAttribute fnAttr( attribute );
				fnAttr.setWritable( false );
				fnAttr.setStorable( false );	
			
				if( s )
				{	
					if ( connectionsFromMe.length() || connectionsToMe.length() )
					{
						MDGModifier dgMod;
						MPlug plug( B::thisMObject(), attribute );	
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
					
					ParameterisedHolder<B>::setNodeValues();
					
					ParameterisedHolder<B>::m_failedToLoad = false;
					
					return ParameterisedHolder<B>::m_parameterised;
				}

				// Can't deal with the result attribute - consider this a failure.
				
				MPlug pClassName( B::thisMObject(), ParameterisedHolder<B>::aParameterisedClassName );
				MPlug pVersion( B::thisMObject(), ParameterisedHolder<B>::aParameterisedVersion );
	
				MString className;
				int version;
	
				pClassName.getValue( className );
				pVersion.getValue( version );
			
				msg( Msg::Error, "OpHolder::getParameterised",
					boost::format( "Unable to update result attribute to represent class \"%s\" version %d." ) % className.asChar() % version );
				ParameterisedHolder<B>::m_parameterised = 0;
			}	
		}
		
	}

	return ParameterisedHolder<B>::getParameterised( classNameOut, classVersionOut, searchPathEnvVarOut );
}

template<typename B>
MStatus OpHolder<B>::setOp( const std::string &className, int classVersion )
{
        return ParameterisedHolder<B>::setParameterised( className, classVersion, "IECORE_OP_PATHS");
}

template<typename B>          
IECore::OpPtr OpHolder<B>::getOp( std::string *className, int *classVersion, std::string *searchPathEnvVar )
{
        return IECore::runTimeCast<IECore::Op>( getParameterised( className, classVersion, searchPathEnvVar ) );       
}


template<>
MTypeId OpHolderNode::id( OpHolderNodeId );

template class OpHolder<MPxNode>;

