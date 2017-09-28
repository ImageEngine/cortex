//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaDagNodeConverter.h"

#include "IECore/CompoundObject.h"
#include "IECore/MessageHandler.h"

#include "maya/MPxNode.h"
#include "maya/MFnDagNode.h"

using namespace IECoreMaya;

IE_CORE_DEFINERUNTIMETYPED( FromMayaDagNodeConverter );

FromMayaDagNodeConverter::FromMayaDagNodeConverter( const std::string &description, const MDagPath &dagPath )
	:	FromMayaObjectConverter( description, dagPath.node() ), m_dagPath( dagPath )
{
}

IECore::ObjectPtr FromMayaDagNodeConverter::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	if( !m_dagPath.isValid() )
	{
		return 0;
	}
	return doConversion( m_dagPath, operands );
}

FromMayaDagNodeConverterPtr FromMayaDagNodeConverter::create( const MDagPath &dagPath, IECore::TypeId resultType )
{

	MayaType typeId( dagPath.apiType(), 0 );

	MPxNode* userNode = MFnDagNode( dagPath ).userNode();
	if( userNode )
	{
		typeId.first = MFn::kInvalid;
		typeId.second = userNode->typeId().id();
	}

	const TypesToFnsMap &m = typesToFns();
	TypesToFnsMap::const_iterator it = m.find( Types( typeId, resultType ) );
	if( it!=m.end() )
	{
		return it->second( dagPath );
	}

	// if not then see if the default converter is suitable
	DefaultConvertersMap &dc = defaultConverters();
	DefaultConvertersMap::const_iterator dcIt = dc.find( typeId );
	if( dcIt != dc.end() )
	{
		if( resultType==IECore::InvalidTypeId || RunTimeTyped::inheritsFrom( dcIt->second->first.second, resultType ) )
		{
			return dcIt->second->second( dagPath );
		}
	}

	return 0;
}

FromMayaDagNodeConverter::TypesToFnsMap &FromMayaDagNodeConverter::typesToFns()
{
	static TypesToFnsMap m;
	return m;
}

FromMayaDagNodeConverter::DefaultConvertersMap &FromMayaDagNodeConverter::defaultConverters()
{
	static DefaultConvertersMap m;
	return m;
}

void FromMayaDagNodeConverter::registerConverter( const MayaType fromType, IECore::TypeId resultType, bool defaultConversion, CreatorFn creator )
{
	TypesToFnsMap &m = typesToFns();
	TypesToFnsMap::const_iterator it = m.insert( TypesToFnsMap::value_type( Types( fromType, resultType), creator ) ).first;
	if( defaultConversion )
	{
		DefaultConvertersMap &dc = defaultConverters();
		if( ! dc.insert( DefaultConvertersMap::value_type( fromType, it ) ).second )
		{
			IECore::msg( IECore::Msg::Error, "FromMayaDagNodeConverter::registerConverter", boost::format( "Default conversion for MFn::Type %d already registered - ignoring second registration." ) % fromType.first );
		}
	}
}
