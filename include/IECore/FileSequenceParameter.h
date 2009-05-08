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

#ifndef IE_CORE_FILESEQUENCEPARAMETER_H
#define IE_CORE_FILESEQUENCEPARAMETER_H

#include <vector>

#include "IECore/FrameList.h"
#include "IECore/FileSequence.h"
#include "IECore/PathParameter.h"

namespace IECore
{

/// The FileSequenceParameter class implements a Parameter to define FileSequences.
/// As it can't store FileSequence objects as its value (they're not derived from Object) it stores
/// a string representing the sequence instead, but provides methods for turning this into a FileSequence.
/// \todo Have this support the specification of frame ranges as well (in a form like "fileName.#.ext 1-20")
/// This should be pretty easy to achieve as the FrameList class already defines the serialisation and parsing
/// for frame ranges.
class FileSequenceParameter : public PathParameter
{
	public:

		typedef std::vector< std::string > ExtensionList;

		IE_CORE_DECLAREOBJECT( FileSequenceParameter, PathParameter );

		FileSequenceParameter( const std::string &name, const std::string &description,	const std::string &defaultValue = "", bool allowEmptyString = true, CheckType check = PathParameter::DontCare,
			const StringParameter::PresetsContainer &presets = StringParameter::PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0,
			const ExtensionList &extensions = ExtensionList() );

		virtual ~FileSequenceParameter();

		const ExtensionList &getExtensions() const;
		void setExtensions( const ExtensionList &extensions );

		/// Returns true only if the value is StringData and matches the FileSequence::fileNameValidator
		/// regex. Also checks that the sequence exists or doesn't exist based on the CheckType passed to
 		/// the constructor.
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;

		void setFileSequenceValue( ConstFileSequencePtr frameList );

		/// Gets the internal StringData value and creates a FileSequence
		/// from it using the ls() function. Note that this can return 0
		/// if check is DontCare and no matching sequence exists on disk.
		FileSequencePtr getFileSequenceValue() const;

	protected :

		FileSequenceParameter();
		friend class TypeDescription<FileSequenceParameter>;

		/// Find the longest space-delimited tail substring that is a parseable FrameList and
		/// return a FileSequence instance which contains that FrameList. Everything before that is considered to
		/// be part of the filename. Previous implementations would just split on the first space character
		/// encountered, but this wouldn't allow for the filename portion of the value to include spaces itself.
		FileSequencePtr parseFileSequence( const std::string &fileSequenceString ) const;

		ExtensionList m_extensions;

	private :

		static const unsigned int g_ioVersion;

};

IE_CORE_DECLAREPTR( FileSequenceParameter );

} // namespace IECore

#endif // IE_CORE_FILESEQUENCEPARAMETER_H
