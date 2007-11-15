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

#include "IECoreMaya/Parameter.h"
#include "IECoreMaya/NumericTraits.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/CompoundParameterHandler.h"

#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/CompoundParameter.h"

#include "maya/MFnCompoundAttribute.h"


using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< CompoundParameterHandler > registrar( IECore::CompoundParameter::staticTypeId() );

MStatus CompoundParameterHandler::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	intrusive_ptr<const IECore::CompoundParameter> p = dynamic_pointer_cast<const IECore::CompoundParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MFnCompoundAttribute fnCAttr( attribute );
	if( !fnCAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	const IECore::CompoundParameter::ParameterMap &childParameters = p->parameters();

	// remove any children who aren't represented in the parameter
	bool removedOne = false;
	do {
		removedOne = false;
		for( unsigned i=0; i<fnCAttr.numChildren(); i++ )
		{
			MObject childAttr = fnCAttr.child( i );
			MFnAttribute fnAttr( childAttr );
			if( !childParameters.count( fnAttr.name().asChar() ) )
			{
				fnCAttr.removeChild( childAttr );
				removedOne = true;
				break;
			}
		}
	} while( removedOne );
	
	// add/update children for each child of the parameter
	for( IECore::CompoundParameter::ParameterMap::const_iterator it=childParameters.begin(); it!=childParameters.end(); it++ )
	{
		bool updatedOk = false;
		for( unsigned i=0; i<fnCAttr.numChildren(); i++ )
		{
			MObject childAttr = fnCAttr.child( i );
			MFnAttribute fnChildAttr( childAttr );
			if( fnChildAttr.name()==it->first.c_str() )
			{
				if( Parameter::update( it->second, childAttr ) ) {
					updatedOk = true;
				} else {
					fnCAttr.removeChild( childAttr );
				}
				break;
			}
		}
		if( !updatedOk )
		{
			MObject c = Parameter::create( it->second, it->first.c_str() );
			if( c.isNull() )
			{
				return MS::kFailure;
			} else {
				MStatus s = fnCAttr.addChild( c );
				if( !s )
				{
					return s;
				}
			}
		}
	}
	
	return MS::kSuccess;
}

MObject CompoundParameterHandler::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	intrusive_ptr<const IECore::CompoundParameter> p = dynamic_pointer_cast<const IECore::CompoundParameter>( parameter );
	
	if( !p || !p->parameters().size())
	{
		return MObject::kNullObj;
	}
	
	MFnCompoundAttribute fnCAttr;
	MObject result = fnCAttr.create( attributeName, attributeName );
	update( parameter, result );
	return result;
}
		
MStatus CompoundParameterHandler::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	intrusive_ptr<const IECore::CompoundParameter> p = dynamic_pointer_cast<const IECore::CompoundParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	const IECore::CompoundParameter::ParameterMap &childParameters = p->parameters();
	if( childParameters.size()!=plug.numChildren() )
	{
		return MS::kFailure;
	}
	
	for( unsigned i=0; i<plug.numChildren(); i++ )
	{
		MPlug childPlug = plug.child( i );
		IECore::CompoundParameter::ParameterMap::const_iterator it = childParameters.find( childPlug.partialName().asChar() );
		if( it==childParameters.end() )
		{
			return MS::kFailure;
		}
		MStatus s = Parameter::setValue( it->second, childPlug );
		if( !s )
		{
			return s;
		}
	}

	return MS::kSuccess;
}

MStatus CompoundParameterHandler::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	intrusive_ptr<IECore::CompoundParameter> p = dynamic_pointer_cast<IECore::CompoundParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	const IECore::CompoundParameter::ParameterMap &childParameters = p->parameters();
	if( childParameters.size()!=plug.numChildren() )
	{
		return MS::kFailure;
	}
	
	for( unsigned i=0; i<plug.numChildren(); i++ )
	{
		MPlug childPlug = plug.child( i );
		IECore::CompoundParameter::ParameterMap::const_iterator it = childParameters.find( childPlug.partialName().asChar() );
		if( it==childParameters.end() )
		{
			return MS::kFailure;
		}
		MStatus s = Parameter::setValue( childPlug, it->second );
		if( !s )
		{
			return s;
		}
	}

	return MS::kSuccess;
}
