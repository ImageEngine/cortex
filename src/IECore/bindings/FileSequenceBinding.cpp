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
#include "boost/format.hpp"

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/FileSequenceBinding.h"

#include "IECore/FileSequence.h"
#include "IECore/Exception.h"

using namespace boost::python;

namespace IECore 
{

template<>
std::string repr( FileSequence &x )
{
	std::stringstream s;
	
	s << "IECore.FileSequence( '";
	
	s << x.getFileName();
	
	s <<"', ";
	
	object item( x.getFrameList() );
	
	s << call_method< std::string >( item.ptr(), "__repr__" );

	s << " ) ";
	
	return s.str();
}

template<>
std::string str( FileSequence &x )
{
	return x.asString();
}

struct FileSequenceHelper
{	
	static list fileNames( const FileSequence &x )
	{
		list result;
		
		std::vector< std::string > f ;
		
		x.fileNames( f );
		
		for ( std::vector< std::string >::const_iterator it = f.begin(); it != f.end(); ++it )
		{
			result.append( *it );
		}
		
		return result;
	}
	
	static list clumpedFileNames( const FileSequence &x, unsigned clumpSize )
	{
		list result;
		
		std::vector< std::vector< std::string > > f;
		x.clumpedFileNames( clumpSize, f );
		
		for ( std::vector< std::vector< std::string > >::const_iterator it = f.begin(); it != f.end(); ++it )
		{
			const std::vector< std::string > &clump = *it;
			
			list clumpList;
			
			for ( std::vector< std::string >::const_iterator cit = clump.begin(); cit != clump.end(); ++cit )
			{
				clumpList.append( *cit );
			}
		
			result.append( clumpList );
		}
		
		return result;
	}
			
	static object mapTo( const FileSequence &x, ConstFileSequencePtr other, bool asList )	
	{			
		if ( asList )
		{
			std::vector< std::pair< std::string, std::string > > result;
		
			x.mapTo( other, result );
		
			list l;
			for (std::vector< std::pair< std::string, std::string > >::const_iterator it = result.begin(); it != result.end(); ++it )
			{
				l.append( make_tuple( it->first, it->second ) );
			}

			return l;
		}
		else
		{
			std::map< std::string, std::string > result;
		
			x.mapTo( other, result );
		
			dict d;
			for ( std::map< std::string, std::string >::const_iterator it = result.begin(); it != result.end(); ++it )
			{
				d[ it->first ] = it->second;
			}

			return d;
		}
	}
	
	static object mapTo( const FileSequence &x, ConstFileSequencePtr other )	
	{
		return mapTo( x, other, false );
	}
};


void bindFileSequence()
{	
	object (*mapTo1)( const FileSequence &, ConstFileSequencePtr, bool ) = &FileSequenceHelper::mapTo;
	object (*mapTo2)( const FileSequence &, ConstFileSequencePtr ) = &FileSequenceHelper::mapTo;	

	RunTimeTypedClass<FileSequence>()	
		.def( init< const std::string &, FrameListPtr >() )
		.add_property( "frameList", &FileSequence::getFrameList, &FileSequence::setFrameList )
		.add_property( "fileName", make_function( &FileSequence::getFileName, return_value_policy<copy_const_reference>() ), &FileSequence::setFileName )							
		.def( "getPadding", &FileSequence::getPadding )
		.def( "setPadding", &FileSequence::setPadding )		
		.def( "getPrefix", &FileSequence::getPrefix )
		.def( "setPrefix", &FileSequence::setPrefix )				
		.def( "getSuffix", &FileSequence::getSuffix )
		.def( "setSuffix", &FileSequence::setSuffix )						
		.def( "copy", &FileSequence::copy )	
		.def( "fileNameForFrame", &FileSequence::fileNameForFrame )
		.def( "frameForFileName", &FileSequence::frameForFileName )
		.def( "fileNames", &FileSequenceHelper::fileNames )
		.def( "clumpedFileNames", &FileSequenceHelper::clumpedFileNames )					
		.def( "mapTo", mapTo1 )					
		.def( "mapTo", mapTo2 )							
		.def( "__repr__", repr< FileSequence > )
		.def( "__str__", str< FileSequence > )	
		.def( self == self )	
	;

}

}
