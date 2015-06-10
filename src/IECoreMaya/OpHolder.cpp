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
#include <iostream>
#include <cassert>

#include "boost/format.hpp"

#include "maya/MPxNode.h"
#include "maya/MPlugArray.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnAttribute.h"
#include "maya/MDGModifier.h"
#include "maya/MGlobal.h"
#include "maya/MFnNumericAttribute.h"

#include "IECoreMaya/OpHolder.h"
#include "IECoreMaya/ParameterHandler.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECore/MessageHandler.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/TypedParameter.h"
#include "IECore/Parameterised.h"

#include "IECorePython/ScopedGILLock.h"

using namespace IECore;
using namespace IECoreMaya;

template<typename B>
MObject OpHolder<B>::aResultDependency;

// this post load callback is used to dirty the result plug
// following loading - see further  comments in initialize.
template<typename B>
class OpHolder<B>::PostLoadCallback : public IECoreMaya::PostLoadCallback
{

	public :
	
		PostLoadCallback( OpHolder<B> *node )
			:	m_node( node )
		{
		}
	
	protected :

		OpHolder<B> *m_node;

		virtual void postLoad()
		{	
			MFnDependencyNode fnDN( m_node->thisMObject() );
	
			MPlug plug = fnDN.findPlug( aResultDependency );
			plug.setValue( 1 );

			m_node->m_postLoadCallback = 0; // remove this callback
		}
		
};
		
template<typename B>
OpHolder<B>::OpHolder()
{
	m_postLoadCallback = new PostLoadCallback( this );
}

template<typename B>
OpHolder<B>::~OpHolder()
{
}

template<typename B>
void *OpHolder<B>::creator()
{
	return new OpHolder<B>();
}

template<typename B>
MStatus OpHolder<B>::initialize()
{
	MStatus s = OpHolder<B>::inheritAttributesFrom( ParameterisedHolder<B>::typeName );
	if( !s )
	{
		return s;
	}
	
	// when we create the result attribute in createResultAttribute(), we make it
	// non-storable. it's common for the result to be a large object such as a mesh so
	// storing it would be prohibitively expensive, and in any case it doesn't make sense to
	// store something which can be recomputed.
	// 
	// however, that causes problems because maya seems incapable of realising that because
	// it didn't save the attribute in the file, it can't possibly have a valid value.
	// maya also doesn't call setDependentsDirty() during scene open, so we don't get a
	// chance to dirty the result at that point either. to work around this, we create this dummy dependency
	// attribute, which we change the value of in a post load callback. this value change
	// triggers setDependentsDirty and we're able to dirty the result before anyone tries to use
	// it. see FnParameterisedHolderTest.testResultAttrSaveLoad() for a test case exercising this
	// workaround.
	MFnNumericAttribute fnNAttr;
	aResultDependency = fnNAttr.create( "resultDependency", "rdep", MFnNumericData::kInt, 0 );
	fnNAttr.setStorable( false );
	fnNAttr.setHidden( true );
	
	return B::addAttribute( aResultDependency );
}

template<typename B>
MStatus OpHolder<B>::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	
	/// This isn't the best way of doing it, but at this point we can't even be sure that the Op has been loaded,
	/// so calling plugParameter() may not work. We can't call getOp() or getParameterised() here, as it seems
	/// we can't do things such as adding/removing attributes within this function
	if( std::string( plug.partialName().substring( 0, 4 ).asChar() ) == ParameterisedHolder<B>::g_attributeNamePrefix || plug==aResultDependency )
	{
		MFnDependencyNode fnDN( B::thisMObject() );
		MStatus s;
		MPlug resultPlug = fnDN.findPlug( "result" , &s);
		if ( s && !resultPlug.isNull() )
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
			ParameterisedHolder<B>::setParameterisedValues( true /* lazy */ );
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
			IECorePython::ScopedGILLock lock;
			PyErr_Print();
			return MS::kFailure;
		}
		catch (...)
		{
			return MS::kFailure;
		}

		assert( result );
		MStatus s = ParameterHandler::setValue( op->resultParameter(), resultPlug );

		block.setClean( resultPlug );

		return s;
	}

	return MS::kFailure;
}

template<typename B>
MStatus OpHolder<B>::setParameterised( IECore::RunTimeTypedPtr p )
{
	MStatus s = ParameterisedHolder<B>::setParameterised( p );
	if( !s )
	{
		return s;
	}

	return createResultAttribute();
}

template<typename B>
IECore::RunTimeTypedPtr OpHolder<B>::getParameterised( std::string *classNameOut, int *classVersionOut, std::string *searchPathEnvVarOut )
{
	bool mustCreateResultAttribute = !ParameterisedHolder<B>::m_parameterised;

	IECore::RunTimeTypedPtr result = ParameterisedHolder<B>::getParameterised( classNameOut, classVersionOut, searchPathEnvVarOut );
	if( result && mustCreateResultAttribute )
	{
		MStatus s = createResultAttribute();
		if( !s )
		{
			ParameterisedHolder<B>::m_parameterised = 0;
			ParameterisedHolder<B>::m_failedToLoad = true;
		}
	}
	
	return result;
}

template<typename B>
MStatus OpHolder<B>::createResultAttribute()
{
	IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( ParameterisedHolder<B>::m_parameterised );

	if( !op )
	{
		MString nodeName = ParameterisedHolder<B>::name();
		msg( Msg::Error, "OpHolder::createResultAttribute", boost::format( "No Op found on node \"%s\"." ) % nodeName.asChar() );
		return MStatus::kFailure;
	}
	
	MStatus s = ParameterisedHolder<B>::createOrUpdateAttribute( const_cast<IECore::Parameter *>( op->resultParameter() ), "result" );
	if( !s )
	{
		MString nodeName = ParameterisedHolder<B>::name();
		msg( Msg::Error, "OpHolder::createResultAttribute", boost::format( "Unable to update result attribute to represent class \"%s\" on node \"%s\"." ) % op->typeName() % nodeName.asChar() );
		return s;
	}
			
	MFnDependencyNode fnDN( B::thisMObject() );
	MObject attribute = fnDN.attribute( "result" );

	MFnAttribute fnAttr( attribute );
	fnAttr.setWritable( false );
	fnAttr.setStorable( false );
	
	return MStatus::kSuccess;
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

