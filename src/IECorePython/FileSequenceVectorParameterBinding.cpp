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

#include "IECorePython/FileSequenceVectorParameterBinding.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/ParameterBinding.h"

#include "IECore/FileSequenceVectorParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"

#include "boost/python.hpp"
#include "boost/tokenizer.hpp"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;

namespace
{

class FileSequenceVectorParameterWrapper : public ParameterWrapper< FileSequenceVectorParameter >
{
	protected:

		static FileSequenceVectorParameter::ExtensionList makeExtensions( object extensions )
		{
			FileSequenceVectorParameter::ExtensionList result;

			extract<list> ee( extensions );
			if ( ee.check() )
			{
				list ext = ee();

				for ( long i = 0; i < IECorePython::len( ext ); i++ )
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

		static StringVectorDataPtr makeDefault( object defaultValue )
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
		static StringVectorDataPtr makeFromObject( object defaultValue )
		{
			StringVectorDataPtr data = new StringVectorData();
			std::vector<std::string> &result = data->writable();

			extract<list> de( defaultValue );
			if( de.check() )
			{
				list l = de();

				for ( long i = 0; i < IECorePython::len( l ); i++ )
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
					return de();
				}
				else
				{
					throw InvalidArgumentException( "FileSequenceParameter: Invalid value" );
				}
			}

			return data;
		}

	public :

		FileSequenceVectorParameterWrapper( PyObject *self, const std::string &n, const std::string &d, object dv = list(), bool allowEmptyList = true, FileSequenceVectorParameter::CheckType check = FileSequenceVectorParameter::DontCare, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = nullptr, object extensions = list() )
			: ParameterWrapper< FileSequenceVectorParameter >( self, n, d, makeDefault( dv ), allowEmptyList, check, parameterPresets<FileSequenceVectorParameter::ObjectPresetsContainer>( p ), po, ud, makeExtensions( extensions ) )
		{
		};

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
			for ( long i = 0; i < IECorePython::len( ext ); i++)
			{
				FileSequenceVectorParameter::setExtensions( makeExtensions( ext ) );
			}
		}

		void setFileSequenceValuesWrap( list l )
		{
			std::vector< FileSequencePtr > seqs;
			for ( long i = 0; i < IECorePython::len( l ); i++ )
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

		list getFileSequenceValuesInternalDataWrap() const
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

		list getFileSequenceValuesDataWrap( const StringVectorData *value ) const
		{
			list r;

			std::vector< FileSequencePtr > sequences;
			getFileSequenceValues( value, sequences );

			for ( std::vector< FileSequencePtr >::const_iterator it = sequences.begin(); it != sequences.end(); ++it )
			{
				r.append( *it );
			}

			return r;
		}

};

} // namespace

namespace IECorePython
{

void bindFileSequenceVectorParameter()
{

	ParameterClass<FileSequenceVectorParameter, FileSequenceVectorParameterWrapper>()
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
					arg( "userData" ) = CompoundObject::Ptr( nullptr ),
					arg( "extensions" ) = list()
				)
			)
		)
		.def( "getFileSequenceValues", &FileSequenceVectorParameterWrapper::getFileSequenceValuesInternalDataWrap )
		.def( "getFileSequenceValues", &FileSequenceVectorParameterWrapper::getFileSequenceValuesDataWrap )
		.def( "setFileSequenceValues", &FileSequenceVectorParameterWrapper::setFileSequenceValuesWrap )
		.add_property( "extensions",&FileSequenceVectorParameterWrapper::getExtensionsWrap, &FileSequenceVectorParameterWrapper::setExtensionsWrap )
	;

}

} // namespace IECorePython
