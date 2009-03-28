//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
#include "boost/python/make_constructor.hpp"

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/FrameListBinding.h"

#include "IECore/CompoundFrameList.h"
#include "IECore/Exception.h"

using namespace boost::python;

namespace IECore 
{

static CompoundFrameListPtr constructFromList( list l )
{
	std::vector< FrameListPtr > frameLists;
	
	for ( long i = 0; i < len( l ); i++ )
	{
		extract< FrameListPtr > ex( l[i] );
		
		if ( !ex.check() )
		{
			throw Exception( "Not a FrameList" );
		}
	
		frameLists.push_back( ex() );
	}
	
	return new CompoundFrameList( frameLists );
}

static list getFrameLists( CompoundFrameList &x )
{
	const std::vector<FrameListPtr> &frameLists = x.getFrameLists();
	
	list result;
	
	for ( std::vector<FrameListPtr>::const_iterator it = frameLists.begin(); it != frameLists.end(); ++it )
	{
		result.append( *it );
	}
	
	return result;	
}

static void setFrameLists( CompoundFrameList &x, list l )
{
	std::vector< FrameListPtr > frameLists;
	
	for ( long i = 0; i < len( l ); i++ )
	{
		extract< FrameListPtr > ex( l[i] );
		
		if ( !ex.check() )
		{
			throw Exception( "Not a FrameList" );
		}
	
		frameLists.push_back( ex() );
	}
	
	x.setFrameLists( frameLists );
}

template<>
std::string repr( CompoundFrameList &x )
{	
	std::stringstream s;
	
	s << "IECore.CompoundFrameList( [ ";
	
	const std::vector< FrameListPtr > &frameLists = x.getFrameLists();
	
	for ( std::vector< FrameListPtr >::const_iterator it = frameLists.begin(); it != frameLists.end(); ++it )
	{
		object item( *it );
		
		if ( it != frameLists.begin() )
		{
			s << ", ";
		}
		
		s << call_method< std::string >( item.ptr(), "__repr__" );
	}

	s << " ] ) ";
	
	return s.str();
}

void bindCompoundFrameList()
{	
	typedef class_< CompoundFrameList, CompoundFrameList::Ptr, bases< FrameList >, boost::noncopyable > CompoundFrameListPyClass;
	CompoundFrameListPyClass ( "CompoundFrameList", no_init )		
		.def( init<>() )
		.def( "__init__", make_constructor( &constructFromList ) )
		.add_property( "frameLists", getFrameLists, setFrameLists )
		.def( "__repr__", repr< CompoundFrameList > )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(CompoundFrameList)	
	;
		
	INTRUSIVE_PTR_PATCH( CompoundFrameList, CompoundFrameListPyClass );
	implicitly_convertible<CompoundFrameListPtr, FrameListPtr>();	
}

}
