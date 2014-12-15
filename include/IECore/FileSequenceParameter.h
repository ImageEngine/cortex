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

#ifndef IE_CORE_FILESEQUENCEPARAMETER_H
#define IE_CORE_FILESEQUENCEPARAMETER_H

#include <vector>

#include "IECore/Export.h"
#include "IECore/FrameList.h"
#include "IECore/FileSequence.h"
#include "IECore/PathParameter.h"

namespace IECore
{

/// The FileSequenceParameter class implements a Parameter to define FileSequences.
/// As it can't store FileSequence objects as its value (they're not derived from Object) it stores
/// a string representing the sequence instead, but provides methods for turning this into a FileSequence.
class IECORE_API FileSequenceParameter : public PathParameter
{
	public:

		typedef std::vector< std::string > ExtensionList;

		IE_CORE_DECLARERUNTIMETYPED( FileSequenceParameter, PathParameter );

		/// Constructs a FileSequenceParameter
		/// \param minSequenceSize Specifies how many files must exist on the file sequence in order to validate the parameter (only used if check type is MustExist).
		FileSequenceParameter( const std::string &name, const std::string &description,	const std::string &defaultValue = "", bool allowEmptyString = true, CheckType check = PathParameter::DontCare,
			const StringParameter::PresetsContainer &presets = StringParameter::PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0,
			const ExtensionList &extensions = ExtensionList(), size_t minSequenceSize = 2 );

		virtual ~FileSequenceParameter();

		const ExtensionList &getExtensions() const;
		void setExtensions( const ExtensionList &extensions );

		size_t getMinSequenceSize() const;
		void setMinSequenceSize( size_t size );

		/// Returns true only if the value is StringData and matches the FileSequence::fileNameValidator
		/// regex. Also checks that the sequence exists or doesn't exist based on the CheckType passed to
 		/// the constructor.
		virtual bool valueValid( const Object *value, std::string *reason = 0 ) const;

		/// Sets the internal StringData with the textual representation of the given file sequence object.
		void setFileSequenceValue( ConstFileSequencePtr fileSequence );

		/// Creates a FileSequence using the given StringData with the current frame list
		/// If no frame list is given on the parameter and it is set to MustExist than
		/// this function uses the ls() to list from the file system. Note that it could return NULL
		/// in case it doesn't exist.
		/// If the parameter is not set to MustExist and no frame list is defined, then it returns a
		/// FileSequence with EmptyFrameList object.
		FileSequencePtr getFileSequenceValue( const StringData *value ) const;
		
		/// Returns a FileSequence using the internal StringData value
		FileSequencePtr getFileSequenceValue() const;

	protected :

		ExtensionList m_extensions;
		size_t m_minSequenceSize;

};

IE_CORE_DECLAREPTR( FileSequenceParameter );

} // namespace IECore

#endif // IE_CORE_FILESEQUENCEPARAMETER_H
