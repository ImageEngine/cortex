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

#ifndef IE_CORE_FILESEQUENCE_H
#define IE_CORE_FILESEQUENCE_H

#include <vector>
#include <string>

#include "boost/format.hpp"
#include "boost/regex.hpp"

#include "IECore/RunTimeTyped.h"
#include "IECore/FrameList.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( FileSequence );

/// The FrameList class defines an abstract base class for classes which are
/// able to generate a list of frame numbers.
class FileSequence : public RunTimeTyped
{
	public:
	
		IE_CORE_DECLARERUNTIMETYPED( FileSequence, RunTimeTyped );
	
		/// Constructs a new FileSequence object with the specified fileName, and FrameList.
		/// fileName must be a string containing a single sequence of at least # character - this
		/// represents the location and padding of the frame numbers. 
		FileSequence( const std::string &fileName, FrameListPtr frameList );
		virtual ~FileSequence();
		
		const std::string &getFileName() const;
		void setFileName( const std::string &fileName );
		
		FrameListPtr getFrameList();			
		void setFrameList( FrameListPtr frameList );
		
		std::string asString() const;
		
		/// Returns the frame number padding - this is calculated by looking at the
		/// number of # characters in the fileName
		unsigned getPadding() const;
		
		/// Sets the frame number padding - this is achieved by modifying the number of # characters
		/// in the fileName
		void setPadding( unsigned padding );
		
		/// Returns the part of the fileName before the # sequence representing the frame number.
		std::string getPrefix() const;
		
		/// Sets the part of fileName before the # sequence to the string specified by prefix.
		void setPrefix( const std::string &prefix );
		
		/// Returns the part of fileName following the # sequence representing the frame number.
		std::string getSuffix() const;
		
		/// Sets the part of fileName following the # sequence to the string specified by suffix.
		void setSuffix( const std::string &suffix );
		
		/// Returns a filename for the sequence represented by this object, with the frame number
		/// specified by the frameNumber parameter
		std::string fileNameForFrame( FrameList::Frame frameNumber ) const;
		
		/// Returns the frame number of a filename from this sequence.
		FrameList::Frame frameForFileName( const std::string &fileName ) const;
		
		/// Generates a list of all filenames represented by this object, ordered according to
		/// the ordering of frames in the FrameList.
		void fileNames( std::vector< std::string > &f ) const;
		
		/// Generates a list of lists of filenames represented by this object, with no more
		/// than clumpSize filenames in each sublist.
		void clumpedFileNames( unsigned clumpSize, std::vector< std::vector < std::string > > &f ) const;
		
		/// Returns a deep copy of this object.
		FileSequencePtr copy() const;
		
		void mapTo( ConstFileSequencePtr other, std::vector< std::pair< std::string, std::string > > &result ) const;
		
		void mapTo( ConstFileSequencePtr other, std::map< std::string, std::string > &result ) const;
	
		/// Returns a regular expression that matches only valid filenames. Matches 1, 2, and 3 of any
		/// resulting matches are the prefix, padding and suffix of the matched filename.
		static boost::regex fileNameValidator();				
		
		bool operator ==( const FileSequence &other ) const;
		
	protected :
	
		std::string m_fileName;
		FrameListPtr m_frameList;
		
		/// Returns a boost::format for expanding out the filename with a frame number. Due to limitations in boost::format
		/// we need use a different template depending on the sign of the frame number.
		boost::format fileNameTemplate( bool negativeFrame ) const;
			
		
};
	
} // namespace IECore

#endif // IE_CORE_FILESEQUENCE_H
