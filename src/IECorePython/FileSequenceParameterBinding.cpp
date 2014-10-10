//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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
#include "boost/tokenizer.hpp"

#include "IECore/FileSequenceParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/FileSequenceParameterBinding.h"
#include "IECorePython/ParameterBinding.h"
#include "IECorePython/Wrapper.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

class FileSequenceParameterWrap : public FileSequenceParameter, public Wrapper< FileSequenceParameter >
{
	public:

		IE_CORE_DECLAREMEMBERPTR( FileSequenceParameterWrap );

		static FileSequenceParameter::ExtensionList makeExtensions( object extensions )
		{
			FileSequenceParameter::ExtensionList result;

			extract<list> ee( extensions );
			if ( ee.check() )
			{
				list ext = ee();

				for ( long i = 0; i < IECorePython::len( ext ); i++ )
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

	public :

		FileSequenceParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv = object( std::string("") ), bool allowEmptyString = true, FileSequenceParameter::CheckType check = FileSequenceParameter::DontCare, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0, object extensions = list(), size_t minSequenceSize = 2 )
			:	FileSequenceParameter( n, d, makeDefault( dv ), allowEmptyString, check, parameterPresets<FileSequenceParameter::PresetsContainer>( p ), po, ud, makeExtensions( extensions ), minSequenceSize ), Wrapper< FileSequenceParameter >( self, this ) {};

		IECOREPYTHON_PARAMETERWRAPPERFNS( FileSequenceParameter );
};

static list getFileSequenceExtensionsWrap( FileSequenceParameter &param )
{
	FileSequenceParameter::ExtensionList extensions = param.getExtensions();

	list result;
	for ( FileSequenceParameter::ExtensionList::const_iterator it = extensions.begin(); it != extensions.end(); ++it )
	{
		result.append( *it );
	}

	return result;
}

static void setFileSequenceExtensionsWrap( FileSequenceParameter &param, object ext )
{
	for ( long i = 0; i < IECorePython::len( ext ); i++)
	{
		param.setExtensions( FileSequenceParameterWrap::makeExtensions( ext ) );
	}
}


void bindFileSequenceParameter()
{
	FileSequencePtr (FileSequenceParameter::*getFileSequenceValueInternalData)() const = &FileSequenceParameter::getFileSequenceValue;
	FileSequencePtr (FileSequenceParameter::*getFileSequenceValueStringData)( const StringData *value ) const = &FileSequenceParameter::getFileSequenceValue;
	
	RunTimeTypedClass<FileSequenceParameter, FileSequenceParameterWrap>()
		.def(
			init< const std::string &, const std::string &, boost::python::optional< object, bool, FileSequenceParameter::CheckType, const object &, bool, CompoundObjectPtr, object, int > >
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ) = object( std::string("") ),
					arg( "allowEmptyString" ) = true,
					arg( "check" ) = FileSequenceParameter::DontCare,
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false ,
					arg( "userData" ) = CompoundObject::Ptr( 0 ),
					arg( "extensions" ) = list(),
					arg( "minSequenceSize" ) = 2
				)
			)
		)
		.def( "getFileSequenceValue", getFileSequenceValueInternalData )
		.def( "getFileSequenceValue", getFileSequenceValueStringData )
		.def( "setFileSequenceValue", &FileSequenceParameter::setFileSequenceValue )
		.def( "setMinSequenceSize", &FileSequenceParameter::setMinSequenceSize )
		.def( "getMinSequenceSize", &FileSequenceParameter::getMinSequenceSize )
		.add_property( "extensions",&getFileSequenceExtensionsWrap, &setFileSequenceExtensionsWrap )
		.IECOREPYTHON_DEFPARAMETERWRAPPERFNS( FileSequenceParameter )
	;

}

}
