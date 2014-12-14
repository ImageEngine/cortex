//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_FILESEQUENCEVECTORPARAMETER_H
#define IE_CORE_FILESEQUENCEVECTORPARAMETER_H

#include <vector>

#include "IECore/Export.h"
#include "IECore/FrameList.h"
#include "IECore/FileSequence.h"
#include "IECore/PathVectorParameter.h"

namespace IECore
{

/// The FileSequenceParameter class implements a Parameter to hold a list of FileSequences.
/// As it can't store FileSequence objects as its value (they're not derived from Object) it stores
/// a list of string representing the sequences instead, but provides methods for turning these
///  into a list of FileSequence objects.
/// \todo Have this support the specification of frame ranges as well (in a form like "fileName.#.ext 1-20")
/// This should be pretty easy to achieve as the FrameList class already defines the serialisation and parsing
/// for frame ranges.
class IECORE_API FileSequenceVectorParameter : public PathVectorParameter
{
	public:

		typedef std::vector< std::string > ExtensionList;

		IE_CORE_DECLARERUNTIMETYPED( FileSequenceVectorParameter, PathVectorParameter );

		FileSequenceVectorParameter( const std::string &name, const std::string &description, const std::vector< std::string > &defaultValue, bool allowEmptyList = true, CheckType check = PathVectorParameter::DontCare,
			const StringVectorParameter::PresetsContainer &presets = StringVectorParameter::PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0,
			const ExtensionList &extensions = ExtensionList() );

		FileSequenceVectorParameter( const std::string &name, const std::string &description, ObjectTypePtr defaultValue, bool allowEmptyList = true, CheckType check = PathVectorParameter::DontCare,
			const StringVectorParameter::ObjectPresetsContainer &presets = StringVectorParameter::ObjectPresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0,
			const ExtensionList &extensions = ExtensionList() );

		virtual ~FileSequenceVectorParameter();

		const ExtensionList &getExtensions() const;
		void setExtensions( const ExtensionList &extensions );

		/// Returns true only if the value is StringVectorData and each string matches the FileSequence::fileNameValidator
		/// regex. Also checks that the sequences exist or don't exist based on the CheckType passed to
 		/// the constructor.
		virtual bool valueValid( const Object *value, std::string *reason = 0 ) const;

		void setFileSequenceValues( const std::vector< FileSequencePtr > &sequences );

		/// Creates a FileSequence from the given StringVectorData using the ls() function.
		/// Note that this can return 0 if check is DontCare and no matching sequence exists on disk.
		void getFileSequenceValues( const StringVectorData *value, std::vector< FileSequencePtr > &sequences ) const;
		
		/// Gets fileSequence values using the internal StringVectorData value
		void getFileSequenceValues( std::vector< FileSequencePtr > &sequences ) const;
		

	protected :

		/// Find the longest space-delimited tail substring that is a parseable FrameList and
		/// return a FileSequence instance which contains that FrameList. Everything before that is considered to
		/// be part of the filename. Previous implementations would just split on the first space character
		/// encountered, but this wouldn't allow for the filename portion of the value to include spaces itself.
		FileSequencePtr parseFileSequence( const std::string &fileSequenceString ) const;

		ExtensionList m_extensions;

};

IE_CORE_DECLAREPTR( FileSequenceVectorParameter );

} // namespace IECore

#endif // IE_CORE_FILESEQUENCEVECTORPARAMETER_H
