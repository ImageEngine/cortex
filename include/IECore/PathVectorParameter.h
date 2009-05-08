//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PATHVECTORPARAMETER_H
#define IE_CORE_PATHVECTORPARAMETER_H

#include "IECore/VectorTypedParameter.h"

namespace IECore
{

/// This class implements a StringVectorParameter object with validation
/// of its elements based on it representing a list of file/directory paths.
class PathVectorParameter : public StringVectorParameter
{
	public :

		IE_CORE_DECLAREOBJECT( PathVectorParameter, StringVectorParameter )

		typedef enum
		{
			DontCare,
			MustExist,
			MustNotExist,
		} CheckType;

		PathVectorParameter( const std::string &name, const std::string &description,
		                     const std::vector<std::string> &defaultValue, bool allowEmptyList = true, CheckType check = PathVectorParameter::DontCare,
		                     const StringVectorParameter::PresetsContainer &presets = StringVectorParameter::PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0 );

		bool allowEmptyList() const;
		bool mustExist() const;
		bool mustNotExist() const;

		/// Returns false if :
		///
		/// * allowEmptyList() is false and the string vector is empty.
		/// * The value does not form a valid path name.
		/// * mustExist() is true and the file/dir doesn't exist.
		/// * mustNotExist() is true and the file/dir exists.
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;

	protected :

		// for io and copying
		PathVectorParameter();
		friend class TypeDescription<PathVectorParameter>;

	private :

		bool m_allowEmptyList;
		CheckType m_check;

		static const unsigned int g_ioVersion;

};

IE_CORE_DECLAREPTR( PathVectorParameter )

} // namespace IECore

#endif // IE_CORE_PATHVECTORPARAMETER_H
