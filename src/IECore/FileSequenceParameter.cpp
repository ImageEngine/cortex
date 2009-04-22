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

#include "IECore/FileSequenceParameter.h"
#include "IECore/FileSequenceFunctions.h"
#include "IECore/FrameList.h"
#include "IECore/CompoundObject.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( FileSequenceParameter );

FileSequenceParameter::FileSequenceParameter( 
	const std::string &name, const std::string &description, const std::string &defaultValue, 
	bool allowEmptyString, CheckType check, const StringParameter::PresetsContainer &presets, bool presetsOnly, 
	ConstCompoundObjectPtr userData, const ExtensionList &extensions
) : PathParameter( name, description, defaultValue, allowEmptyString, check, presets, presetsOnly, userData ),
    m_extensions( extensions )
{
}		

FileSequenceParameter::~FileSequenceParameter()
{
}

const FileSequenceParameter::ExtensionList &FileSequenceParameter::getExtensions() const
{
	return m_extensions;
}

void FileSequenceParameter::setExtensions( const ExtensionList &extensions )
{
	m_extensions = extensions;
}

bool FileSequenceParameter::valueValid( ConstObjectPtr value, std::string *reason ) const
{
	/// we can't call PathParameter::valueValid() because that would do existence checking on
	/// our path specifier with the # characters in it, and that would yield the wrong results
	/// so we call StringParameter.valueValid and do the rest ourselves.	
	if ( !StringParameter::valueValid( value ) )
	{
		return false;
	}
	
	ConstStringDataPtr stringDataValue = assertedStaticCast< const StringData >( value );
	const std::string &stringValue = stringDataValue->readable();
		
	if ( allowEmptyString() && !stringValue.size() )
	{
		return true;
	}
	
	if ( ! boost::regex_match( stringValue, FileSequence::fileNameValidator() ) )
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
		fileSequence = parseFileSequence( stringValue );
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
	
	return true;
}
		
void FileSequenceParameter::setFileSequenceValue( ConstFileSequencePtr fileSequence )
{
	//// \todo Don't throw away the FrameList here!
	setValue( new StringData( fileSequence->getFileName() ) );
}
		
FileSequencePtr FileSequenceParameter::getFileSequenceValue() const
{
	const std::string &fileSequenceStr = getTypedValue();
	
	if ( fileSequenceStr.find_first_of( ' ' ) == std::string::npos )
	{
		FileSequencePtr result = 0;
		ls( fileSequenceStr, result );
		return result;
	}
	
	return parseFileSequence( fileSequenceStr );
}

FileSequencePtr FileSequenceParameter::parseFileSequence( const std::string &fileSequenceStr ) const
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
