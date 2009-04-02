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

#include "IECore/FileSequenceParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/FileSequenceParameterBinding.h"
#include "IECore/bindings/ParameterBinding.h"

using namespace boost::python;

namespace IECore 
{

class FileSequenceParameterWrap : public FileSequenceParameter, public Wrapper< FileSequenceParameter >
{
	public:
	
		IE_CORE_DECLAREMEMBERPTR( FileSequenceParameterWrap );
	
	protected:
	
		static FileSequenceParameter::ExtensionList makeExtensions( object extensions )
		{
			FileSequenceParameter::ExtensionList result;
			
			extract<list> ee( extensions );
			if ( ee.check() )
			{			
				list ext = ee();
				
				for ( long i = 0; i < len( ext ); i++ )
				{
					extract< std::string > ex( ext[i] );
					if ( !ex.check() )
					{
						throw InvalidArgumentException( "FileSequenceParameter: Invalid extensions value" );
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
					throw InvalidArgumentException( "FileSequenceParameter: Invalid extensions value" );
				}
			}
			
			return result;
		}

		/// Allow construction from either a string, StringData, or a FileSequence
		static std::string makeDefault( object defaultValue )
		{
			extract<std::string> de( defaultValue );
			if( de.check() )
			{
				return de();
			}
			else			
			{
				extract<StringData *> de( defaultValue );
				if( de.check() )
				{
					return de()->readable();
				}
				else
				{
					extract<FileSequence *> de( defaultValue );
					if( de.check() )
					{
						return de()->asString();
					}
					else
					{
						throw InvalidArgumentException( "FileSequenceParameter: Invalid default value" );
					}
				}
			}
		}

		static StringParameter::PresetsMap makePresets( const dict &d )
		{
			StringParameter::PresetsMap p;
			boost::python::list keys = d.keys();
			boost::python::list values = d.values();
			for( int i = 0; i<keys.attr( "__len__" )(); i++ )
			{			
				const std::string key = extract<std::string>( keys[i] )();
				extract<std::string> de( values[i] );
				if( de.check() )
				{
					p.insert( StringParameter::PresetsMap::value_type( key, de() ) );
				}
				else
				{				
					extract<StringData *> de( values[i] );
					if( de.check() )
					{
						p.insert( StringParameter::PresetsMap::value_type( key, de()->readable() ) );
					}				
					else	
					{
						extract<FileSequence *> de( values[i] );
						if( de.check() )
						{
							p.insert( StringParameter::PresetsMap::value_type( key, de()->asString() ) );
						}
						else
						{
							throw InvalidArgumentException( "FileSequenceParameter: Invalid preset value" );
						}
					}
				}
			}
			return p;
		}
				
	public :

		FileSequenceParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv = object( std::string("") ), bool allowEmptyString = true, FileSequenceParameter::CheckType check = FileSequenceParameter::DontCare, const dict &p = dict(), bool po = false, CompoundObjectPtr ud = 0, object extensions = list() )	
			:	FileSequenceParameter( n, d, makeDefault( dv ), allowEmptyString, check, makePresets( p ), po, ud, makeExtensions( extensions ) ), Wrapper< FileSequenceParameter >( self, this ) {};
					
		list getExtensionsWrap()
		{
			FileSequenceParameter::ExtensionList extensions = FileSequenceParameter::getExtensions();
			
			list result;
			for ( FileSequenceParameter::ExtensionList::const_iterator it = extensions.begin(); it != extensions.end(); ++it )
			{
				result.append( *it );
			}
			
			return result;
		}
		
		void setExtensionsWrap( object ext )
		{
			for ( long i = 0; i < len( ext ); i++)
			{
				FileSequenceParameter::setExtensions( makeExtensions( ext ) );
			}
		}	

		IE_COREPYTHON_PARAMETERWRAPPERFNS( FileSequenceParameter );
};

void bindFileSequenceParameter()
{	

	typedef class_< FileSequenceParameter, FileSequenceParameterWrap::Ptr, boost::noncopyable, bases< PathParameter > > FileSequenceParameterPyClass;
	FileSequenceParameterPyClass ( "FileSequenceParameter", no_init )
		.def(
			init< const std::string &, const std::string &, boost::python::optional< object, bool, FileSequenceParameter::CheckType, const dict &, bool, CompoundObjectPtr, object > >
			( 
				( 
					arg( "name" ), 
					arg( "description" ), 
					arg( "defaultValue" ) = object( std::string("") ),
					arg( "allowEmptyString" ) = true,
					arg( "check" ) = FileSequenceParameter::DontCare, 
					arg( "presets" ) = dict(),
					arg( "presetsOnly" ) = false , 
					arg( "userData" ) = CompoundObjectPtr(),
					arg( "extensions" ) = list()
				) 
			) 
		)
		.def( "getFileSequenceValue", &FileSequenceParameter::getFileSequenceValue )	
		.def( "setFileSequenceValue", &FileSequenceParameter::setFileSequenceValue )
		.add_property( "extensions",&FileSequenceParameterWrap::getExtensionsWrap, &FileSequenceParameterWrap::setExtensionsWrap )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( FileSequenceParameter )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( FileSequenceParameter )
	;
	
	WrapperToPython< FileSequenceParameterWrap::Ptr >();
	INTRUSIVE_PTR_PATCH( FileSequenceParameter, FileSequenceParameterPyClass );
	implicitly_convertible<FileSequenceParameterWrap::Ptr, FileSequenceParameterPtr>();
	implicitly_convertible<FileSequenceParameterPtr, PathParameterPtr>();
}

}
