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

#include <algorithm>
#include <cassert>

#include "boost/filesystem/convenience.hpp"

#include "IECore/FileSequenceVectorParameter.h"
#include "IECore/FileSequenceFunctions.h"
#include "IECore/FrameList.h"
#include "IECore/CompoundObject.h"

using namespace IECore;


FileSequenceVectorParameter::FileSequenceVectorParameter( const std::string &name, const std::string &description, 
	const std::vector< std::string > &defaultValue, bool allowEmptyList, CheckType check,
	const StringVectorParameter::PresetsMap &presets, bool presetsOnly, ConstCompoundObjectPtr userData,
	const ExtensionList &extensions )
 : PathVectorParameter( name, description, defaultValue, allowEmptyList, check, presets, presetsOnly, userData ),
    m_extensions( extensions )
{
}		

FileSequenceVectorParameter::~FileSequenceVectorParameter()
{
}

const FileSequenceVectorParameter::ExtensionList &FileSequenceVectorParameter::getExtensions() const
{
	return m_extensions;
}

void FileSequenceVectorParameter::setExtensions( const ExtensionList &extensions )
{
	m_extensions = extensions;
}

bool FileSequenceVectorParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	/// we can't call PathParameter::valueValid() because that would do existence checking on
	/// our path specifier with the # characters in it, and that would yield the wrong results
	/// so we call StringParameter.valueValid and do the rest ourselves.	
	if ( !StringVectorParameter::valueValid( value ) )
	{
		return false;
	}
	
	ConstStringVectorDataPtr stringVectorDataValue = assertedStaticCast< const StringVectorData >( value );
	const StringVectorData::ValueType &stringVectorValue = stringVectorDataValue->readable();
		
	if ( allowEmptyList() && !stringVectorValue.size() )
	{
		return true;
	}
	
	for ( StringVectorData::ValueType::const_iterator it = stringVectorValue.begin(); it != stringVectorValue.end(); ++it )
	{		
		if ( ! boost::regex_match( *it, FileSequence::fileNameValidator() ) )
		{
			if ( reason )
			{
				*reason = "Value must contain one sequence of at least one # character to specify frame number.";
			}
			return false;
		}

		FileSequencePtr fileSequence = 0;
		try
		{
			fileSequence = parseFileSequence( *it );
		}
		catch ( Exception &e )
		{
			if ( reason )
			{
				*reason = "Not a valid file sequence specification";
			}
			return false;
		}

		assert( fileSequence );

		if ( m_extensions.size() )
		{	
			std::string ext = boost::filesystem::extension( fileSequence->getFileName() );
			if ( ext.size() && ext[0] == '.' )
			{
				ext = ext.substr( 1, ext.size() - 1 );
			}

			if ( std::find( m_extensions.begin(), m_extensions.end(), ext ) == m_extensions.end() )
			{
				if ( reason )
				{
					*reason = "File sequence extension not valid";
				}
				return false;
			}
		}

		if ( mustExist() )
		{
			FileSequencePtr s = 0;
			ls( fileSequence->getFileName(), s );

			if ( !s )
			{
				if ( reason )
				{
					*reason = "File sequence does not exist";
				}
				return false;
			}
		}
		else if ( mustNotExist() )
		{
			FileSequencePtr s = 0;
			ls( fileSequence->getFileName(), s );

			if ( s )
			{
				if ( reason )
				{
					*reason = "File sequence already exists";
				}
				return false;
			}
		}
	
	}
	
	return true;
}
		
void FileSequenceVectorParameter::setFileSequenceValues( const std::vector<FileSequencePtr> &fileSequences )
{
	//// \todo Don't throw away the FrameList here!
	
	StringVectorDataPtr data = new StringVectorData();
	
	data->writable().reserve( fileSequences.size() );
	
	for ( std::vector<FileSequencePtr>::const_iterator it = fileSequences.begin(); it != fileSequences.end(); ++it )
	{
		data->writable().push_back( (*it)->getFileName() );
	}
	
	setValue( data );
}
		
void FileSequenceVectorParameter::getFileSequenceValues( std::vector<FileSequencePtr> &fileSequences ) const
{
	fileSequences.clear();	

	const std::vector< std::string > &sequences = getTypedValue();
	
	for ( std::vector< std::string >::const_iterator it =sequences.begin(); it != sequences.end(); ++it )
	{
		FileSequencePtr value;
	
		if ( it->find_first_of( ' ' ) == std::string::npos )
		{
			ls( *it, value );			
		}
		else
		{
			value = parseFileSequence( *it );
		}
		
		if ( value )
		{
			fileSequences.push_back( value );
		}
	}
}

FileSequencePtr FileSequenceVectorParameter::parseFileSequence( const std::string &fileSequenceStr ) const
{
	std::string fileSequenceCopy = fileSequenceStr;
	
	std::string::size_type spaceIndex = fileSequenceCopy.find_first_of( " " );
	
	bool found = false;
	
	std::string filename = fileSequenceStr;
	
	FrameListPtr frameList = FrameList::parse( "" );
	
	while ( !found && spaceIndex != std::string::npos )
	{
		
		std::string head = fileSequenceStr.substr( 0, spaceIndex );
		std::string tail = fileSequenceStr.substr( spaceIndex+1, fileSequenceStr.size() - spaceIndex - 1 );		
		assert( head + " " + tail == fileSequenceStr );
		
		filename = head;
		
		try
		{
			frameList = FrameList::parse( tail );
			found = true;
		}
		catch ( Exception &e )
		{
			fileSequenceCopy = fileSequenceCopy.substr( 0, spaceIndex ) 
				+ "*"
				+ fileSequenceCopy.substr( spaceIndex+1, fileSequenceStr.size() - spaceIndex - 1 )
			;
			
			spaceIndex = fileSequenceCopy.find_first_of( " " );
		}	
	}
	
	return new FileSequence( filename, frameList );
	
}
