//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//   * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//   * Neither the name of Image Engine Design nor the names of any
//    other contributors to this software may be used to endorse or
//    promote products derived from this software without specific prior
//    written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "boost/python.hpp"

#include "IECore/FileSequenceVectorParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/FileSequenceVectorParameterBinding.h"
#include "IECore/bindings/ParameterBinding.h"

using namespace boost::python;

namespace IECore 
{

class FileSequenceVectorParameterWrap : public FileSequenceVectorParameter, public Wrapper< FileSequenceVectorParameter >
{
	public:
	
		IE_CORE_DECLAREMEMBERPTR( FileSequenceVectorParameterWrap );
	
	protected:
	
		static FileSequenceVectorParameter::ExtensionList makeExtensions( object extensions )
		{
			FileSequenceVectorParameter::ExtensionList result;
			
			extract<list> ee( extensions );
			if ( ee.check() )
			{			
				list ext = ee();
				
				for ( long i = 0; i < len( ext ); i++ )
				{
					extract< std::string > ex( ext[i] );
					if ( !ex.check() )
					{
						throw InvalidArgumentException( "FileSequenceVectorParameter: Invalid extensions value" );
					}

					result.push_back( ex() );
				}
			
			}
			else
			{
				extract<std::string> ee( extensions );
				if ( ee.check() )
				{
					std::string ext = ee();
					boost::tokenizer< boost::char_separator<char> > t( ext, boost::char_separator<char>( " " ) );
					
					for ( boost::tokenizer<boost::char_separator<char> >::const_iterator it = t.begin(); it != t.end(); ++it )
					{
						result.push_back( *it );
					}
				}
				else
				{
					throw InvalidArgumentException( "FileSequenceVectorParameter: Invalid extensions value" );
				}
			}
			
			return result;
		}
		
		static std::vector<std::string> makeDefault( object defaultValue )
		{
			try
			{
				return makeFromObject( defaultValue );
			}
			catch ( InvalidArgumentException &e )
			{
				throw InvalidArgumentException( "FileSequenceVectorParameter: Invalid default value" );
			}
			
		}

		/// Allow construction from either a list of strings/FileSequences, or a StringVectorData
		static std::vector<std::string> makeFromObject( object defaultValue )
		{
			std::vector<std::string> result;
		
			extract<list> de( defaultValue );
			if( de.check() )
			{
				list l = de();
				
				for ( long i = 0; i < len( l ); i++ )
				{
					
					extract<std::string> ee( l[i] );
					
					if ( ee.check() )
					{
						result.push_back( ee() );
					}
					else
					{					
						extract<FileSequence *> ee( l[i] );

						if ( ee.check() )
						{
							result.push_back( ee()->asString() );
						}
						else
						{
							throw InvalidArgumentException( "FileSequenceVectorParameter: Invalid value" );
						}
					}
					
				}
			}
			else			
			{
				extract<StringVectorData *> de( defaultValue );
				if( de.check() )
				{
					return de()->readable();
				}
				else
				{
					throw InvalidArgumentException( "FileSequenceParameter: Invalid value" );
				}
			}
			
			return result;
		}
				
	public :

		FileSequenceVectorParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv = list(), bool allowEmptyList = true, FileSequenceVectorParameter::CheckType check = FileSequenceVectorParameter::DontCare, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0, object extensions = list() )	
			:	FileSequenceVectorParameter( n, d, makeDefault( dv ), allowEmptyList, check, parameterPresets<FileSequenceVectorParameter::PresetsContainer>( p ), po, ud, makeExtensions( extensions ) ), Wrapper< FileSequenceVectorParameter >( self, this ) {};
					
		list getExtensionsWrap() const
		{
			FileSequenceVectorParameter::ExtensionList extensions = FileSequenceVectorParameter::getExtensions();
			
			list result;
			for ( FileSequenceVectorParameter::ExtensionList::const_iterator it = extensions.begin(); it != extensions.end(); ++it )
			{
				result.append( *it );
			}
			
			return result;
		}
		
		void setExtensionsWrap( object ext )
		{
			for ( long i = 0; i < len( ext ); i++)
			{
				FileSequenceVectorParameter::setExtensions( makeExtensions( ext ) );
			}
		}
		
		void setFileSequenceValuesWrap( list l )
		{
			std::vector< FileSequencePtr > seqs;
			for ( long i = 0; i < len( l ); i++ )
			{
				extract< FileSequence *> e( l[i] );
				if ( e.check() )
				{
					seqs.push_back( e() );
				}
				else
				{
					throw InvalidArgumentException( "FileSequenceVectorParameter: Invalid argument to setFileSequenceValues" );
				}								
			}
									
			setFileSequenceValues( seqs );		
		}
		
		list getFileSequenceValuesWrap() const
		{
			list r;
			
			std::vector< FileSequencePtr > sequences;
			getFileSequenceValues( sequences );
			
			for ( std::vector< FileSequencePtr >::const_iterator it = sequences.begin(); it != sequences.end(); ++it )
			{
				r.append( *it );
			}
			
			return r;
		}

		IE_COREPYTHON_PARAMETERWRAPPERFNS( FileSequenceVectorParameter );
};

void bindFileSequenceVectorParameter()
{	

	typedef class_< FileSequenceVectorParameter, FileSequenceVectorParameterWrap::Ptr, boost::noncopyable, bases< PathVectorParameter > > FileSequenceVectorParameterPyClass;
	FileSequenceVectorParameterPyClass ( "FileSequenceVectorParameter", no_init )
		.def(
			init< const std::string &, const std::string &, boost::python::optional< object, bool, FileSequenceVectorParameter::CheckType, const object &, bool, CompoundObjectPtr, object > >
			( 
				( 
					arg( "name" ), 
					arg( "description" ), 
					arg( "defaultValue" ) = list(),
					arg( "allowEmptyList" ) = true,
					arg( "check" ) = FileSequenceVectorParameter::DontCare, 
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false , 
					arg( "userData" ) = CompoundObject::Ptr( 0 ),
					arg( "extensions" ) = list()
				) 
			) 
		)
		.def( "getFileSequenceValues", &FileSequenceVectorParameterWrap::getFileSequenceValuesWrap )	
		.def( "setFileSequenceValues", &FileSequenceVectorParameterWrap::setFileSequenceValuesWrap )
		.add_property( "extensions",&FileSequenceVectorParameterWrap::getExtensionsWrap, &FileSequenceVectorParameterWrap::setExtensionsWrap )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( FileSequenceVectorParameter )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( FileSequenceVectorParameter )
	;
	
	WrapperToPython< FileSequenceVectorParameterWrap::Ptr >();
	INTRUSIVE_PTR_PATCH( FileSequenceVectorParameter, FileSequenceVectorParameterPyClass );
	implicitly_convertible<FileSequenceVectorParameterWrap::Ptr, FileSequenceVectorParameterPtr>();
	implicitly_convertible<FileSequenceVectorParameterPtr, PathVectorParameterPtr>();
}

}
