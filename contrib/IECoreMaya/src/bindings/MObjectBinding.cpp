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

#include "maya/MSelectionList.h"
#include "maya/MString.h"

#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/bindings/MObjectBinding.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;

///////////////////////////////////////////////////////////////////////
// MObjectWrapper implementation
///////////////////////////////////////////////////////////////////////

MObjectWrapper::MObjectWrapper( const MObject &object )
	:	m_objectHandle( object )
{
}

MObjectWrapper::MObjectWrapper( const char *name )
{
	MSelectionList list;
	list.add( name );
	MObject node;
	list.getDependNode( 0, node );
	if( node.isNull() )
	{
		throw IECore::Exception( std::string( "Object \"" ) + name + "\" does not exist or is ambiguous." );
	}
	m_objectHandle = node;
}

MObject MObjectWrapper::object( bool throwIfNotValid )
{
	if( !m_objectHandle.isAlive() )
	{
		throw IECore::Exception( "MObject not alive." );
	}
	if( throwIfNotValid && !m_objectHandle.isValid() )
	{
		throw IECore::Exception( "MObject not valid." );
	}
	return m_objectHandle.object();
}

const MObjectHandle &MObjectWrapper::objectHandle()
{
	return m_objectHandle;
}

FromMayaConverterPtr MObjectWrapper::converter()
{
	return FromMayaObjectConverter::create( object() );
}

FromMayaConverterPtr MObjectWrapper::converter( IECore::TypeId resultType )
{
	return FromMayaObjectConverter::create( object(), resultType );
}

IECore::ObjectPtr MObjectWrapper::convert()
{
	FromMayaConverterPtr c = converter();
	if( !c )
	{
		return 0;
	}
	return c->convert();
}

IECore::ObjectPtr MObjectWrapper::convert( IECore::TypeId resultType )
{
	FromMayaConverterPtr c = converter( resultType );
	if( !c )
	{
		return 0;
	}
	return c->convert();
}
		
///////////////////////////////////////////////////////////////////////
// MObjectWrapper binding
///////////////////////////////////////////////////////////////////////

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( MObjectWrapperConvertOverloads, convert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( MObjectWrapperConverterOverloads, converter, 0, 1);

void IECoreMaya::bindMObject()
{
	class_<MObjectWrapper, boost::noncopyable>( "MObject", init<const char *>() )
		.def( "convert", (ObjectPtr (MObjectWrapper::*)( TypeId ))&MObjectWrapper::convert, MObjectWrapperConvertOverloads() )
		.def( "converter", (FromMayaConverterPtr (MObjectWrapper::*)( TypeId ))&MObjectWrapper::converter, MObjectWrapperConverterOverloads() )
	;
}
