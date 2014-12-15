//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_COMPOUNDOBJECT_H
#define IE_CORE_COMPOUNDOBJECT_H

#include "IECore/Export.h"
#include "IECore/Object.h"

namespace IECore
{

/// A simple class representing compounds of named
/// child Objects.
class IECORE_API CompoundObject : public Object
{
	public:

		CompoundObject();
		virtual ~CompoundObject();

		IE_CORE_DECLAREOBJECT( CompoundObject, Object );

		typedef std::map<InternedString, ObjectPtr> ObjectMap;

		/// Gives const access to the member object map.
		const ObjectMap &members() const;
		/// Gives access to the member Object map for
		/// direct manipulation.
		ObjectMap &members();

		/// Convenience function to find an object in members(). If the named object doesn't exist
		/// or doesn't match the type specified as the template argument, behavior
		/// is defined by the throwExceptions parameter. When this parameter is true a descriptive
		/// Exception is thrown, and when false 0 is returned.
		template<typename T>
		T *member( const InternedString &name, bool throwExceptions = false );
		template<typename T>
		const T *member( const InternedString &name, bool throwExceptions = false ) const;
		
		/// A Convenience function to find an object in members().
		/// If the named object doesn't exist, if createIfMissing is true, an object will be added
		/// with the type's object factory create method. If false, or the named entry does not match the 
		/// type specified as the template argument, behavior is defined by the throwExceptions parameter.
		/// When this parameter is true a descriptive Exception is thrown, and when false 0 is returned.
		template<typename T>
		T *member( const InternedString &name, bool throwExceptions, bool createIfMissing );

		/// Returns an instance of CompoundObject which can be shared by everyone - for instance a procedural
		/// might like to populate it with objects to be used subsequently by a shadeop.
		static CompoundObject *defaultInstance();

	protected:

		ObjectMap m_members;

	private :

		static const unsigned int m_ioVersion;

};

IE_CORE_DECLAREPTR( CompoundObject );

}

#include "IECore/CompoundObject.inl"

#endif // IE_CORE_COMPOUNDOBJECT_H
