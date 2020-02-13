//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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


#ifndef IECORE_COMPOUNDDATA_H
#define IECORE_COMPOUNDDATA_H

#include "IECore/CompoundDataBase.h"
#include "IECore/Export.h"

namespace IECore
{

/// A class for storing a map of named Data items.
/// \ingroup coreGroup
class IECORE_API CompoundData : public CompoundDataBase
{
	public :

		CompoundData();
		CompoundData( const CompoundDataMap &members );

		IE_CORE_DECLAREOBJECT( CompoundData, CompoundDataBase );

		/// Convenience functions to find a child Data object. If the named child doesn't exist
		/// or doesn't match the type specified as the template argument, behavior
		/// is defined by the throwExceptions parameter. When this parameter is true a descriptive
		/// Exception is thrown, and when false 0 is returned.
		template<typename T = Data>
		T *member( const InternedString &name, bool throwExceptions = false );
		template<typename T = Data>
		const T *member( const InternedString &name, bool throwExceptions = false ) const;

		/// A Convenience function to find a child Data object.
		/// If the named child doesn't exist, if createIfMissing is true, a child will be added
		/// with the type's object factory create method. If false, or the named child does not match the
		/// type specified as the template argument, behavior is defined by the throwExceptions parameter.
		/// When this parameter is true a descriptive Exception is thrown, and when false 0 is returned.
		template<typename T = Data>
		T *member( const InternedString &name, bool throwExceptions, bool createIfMissing );
};

IE_CORE_DECLAREPTR( CompoundData );

} // namespace IECore

#include "IECore/CompoundData.inl"

#endif // IE_CORE_COMPOUNDDATA_H
