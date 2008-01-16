//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/bindings/PlugBinding.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/StatusException.h"

#include "maya/MFnDagNode.h"
#include "maya/MFnAttribute.h"
#include "maya/MSelectionList.h"
#include "maya/MString.h"

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;

///////////////////////////////////////////////////////////////////////
// Plug implementation
///////////////////////////////////////////////////////////////////////

Plug::Plug( const MPlug &plug )
	:	m_plug( plug )
{
}

Plug::Plug( const std::string &name )
{
	MStatus s;
	MSelectionList list;
	s = list.add( name.c_str() );
	StatusException::throwIfError( list.getPlug( 0, m_plug ) );
}

Plug::Plug( const Plug &other )
{
	m_plug = other.m_plug;
}
		
FromMayaConverterPtr Plug::converter()
{
	return FromMayaPlugConverter::create( m_plug );
}

FromMayaConverterPtr Plug::converter( IECore::TypeId resultType )
{
	return FromMayaPlugConverter::create( m_plug, resultType );
}

IECore::ObjectPtr Plug::convert()
{
	FromMayaConverterPtr c = converter();
	if( !c )
	{
		return 0;
	}
	return c->convert();
}

IECore::ObjectPtr Plug::convert( IECore::TypeId resultType )
{
	FromMayaConverterPtr c = converter( resultType );
	if( !c )
	{
		return 0;
	}
	return c->convert();
}

std::string Plug::name() const
{
	if( m_plug.isNull() )
	{
		return "";
	}
	return m_plug.name().asChar();
}

std::string Plug::fullPathName() const
{
	if( m_plug.isNull() )
	{
		return "";
	}
	
	if (m_plug.node().hasFn( MFn::kDagNode ) )
	{
		MFnDagNode fnDagNode( m_plug.node() );
		MString name = fnDagNode.fullPathName() + ".";		
		name += m_plug.partialName(false, true, true, false, true, true);
		return name.asChar();
	}
	else
	{
		return name();
	}
}

std::string Plug::partialPathName() const
{
	if( m_plug.isNull() )
	{
		return "";
	}
	
	if (m_plug.node().hasFn( MFn::kDagNode ) )
	{
		MFnDagNode fnDagNode( m_plug.node() );
		MString name = fnDagNode.partialPathName() + ".";		
		name += m_plug.partialName(false, true, true, false, true, true);
		return name.asChar();
	}
	else
	{
		return name();
	}
}

boost::python::list Plug::childNames() const
{
	boost::python::list result;
	
	for (unsigned int i = 0; i < m_plug.numChildren(); i++)
	{
		MPlug child = m_plug.child(i);
		
		MFnAttribute fnAttr( child.attribute() );
		
		result.append( fnAttr.name().asChar() );
	}
	
	return result;
}

Plug Plug::child( const std::string &name ) const
{
	for (unsigned int i = 0; i < m_plug.numChildren(); i++)
	{
		MPlug child = m_plug.child(i);
		
		MFnAttribute fnAttr( child.attribute() );
		
		std::string childName = fnAttr.name().asChar();
		
		if (childName == name)
		{
			return Plug( child );
		}

	}

	throw StatusException( MS::kFailure );
}

bool Plug::isArray() const
{
	return m_plug.isArray();
}

unsigned Plug::numElements()
{
	return m_plug.evaluateNumElements();
}

Plug Plug::elementByPhysicalIndex( unsigned i )
{
	/// we to need to call numElements so that the plug is up to date
	/// in the number of elements it has. otherwise we get exceptions
	/// accessing elements which should exist.
	numElements();
	
	MStatus s;
	MPlug p = m_plug.elementByPhysicalIndex( i, &s );
	StatusException::throwIfError( s );
	return p;
}
				
///////////////////////////////////////////////////////////////////////
// Plug binding
///////////////////////////////////////////////////////////////////////

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( PlugWrapperConvertOverloads, convert, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( PlugWrapperConverterOverloads, converter, 0, 1);

void IECoreMaya::bindPlug()
{
	class_<Plug>( "Plug", init<const std::string &>() )
		.def( "converter", (FromMayaConverterPtr (Plug::*)( TypeId ))&Plug::converter, PlugWrapperConverterOverloads() )
		.def( "convert", (ObjectPtr (Plug::*)( TypeId ))&Plug::convert, PlugWrapperConvertOverloads() )
		.def( "name", &Plug::name)
		.def( "fullPathName", &Plug::fullPathName)
		.def( "__str__", &Plug::fullPathName )
		.def( "partialPathName", &Plug::partialPathName)
		.def( "childNames", &Plug::childNames)
		.def( "child", &Plug::child)
		.def( "isArray", &Plug::isArray )		
		.def( "numElements", &Plug::numElements )		
		.def( "elementByPhysicalIndex", &Plug::elementByPhysicalIndex )
		.def( "__len__", &Plug::numElements )
		.def( "__getitem__", &Plug::elementByPhysicalIndex )		
	;
}
