//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_CLASSDATA_H
#define IE_CORE_CLASSDATA_H

#include <map>

#include "IECore/Deleter.h"

namespace IECore
{

/// A simple class to allow, for example, extra data to be added to a class for use in cases where simply
/// adding member data would break the library's binary compatibility.
/// Template with the type of the class that data is being added to, the type of the data being
/// added, and policy for deleting that data when the class is destructed. The latter allows use of
/// dynamically-allocated data and arrays.
///
/// It the responsibility of the client of this class to implement all creation, deletion, and copy semantics.
///
/// Example, which effectively adds an extra member of type "int" to class "A" :
///
/// \code
/// // In file A.h :
/// class A
/// {
///    int &extraMember();
/// };
///
/// // In file A.cpp:
///
/// #include "A.h"
/// #include "IECore/ClassData.h"
///
/// static IECore::ClassData< A, int > g_classData;
///
/// A::A()
/// {
///    g_classData.create( this, 4 );
/// }
///
/// int &A::extraMember()
/// {
///    return g_classData[ this ];
/// }
/// \endcode
template< typename ClassTypeT, typename DataTypeT, typename DataDeletePolicyT = NullDeleter<DataTypeT> >
class ClassData
{
	public:

		typedef ClassTypeT        ClassType;
		typedef DataTypeT         DataType;
		typedef DataDeletePolicyT DataDeletePolicy;

		/// Exercises DataDeletePolicy on all held data
		virtual ~ClassData();

		/// Create some uninitialized data for the given class
		DataType &create( const ClassType* classOwner );

		/// Create some data with initial value as specified for the given class
		DataType &create( const ClassType* classOwner, const DataType &d );

		/// Const access to the data associated with the class. If the data has not yet been created
		/// the behaviour is undefined.
		const DataType &operator[]( const ClassType *classOwner ) const;

		/// Read/write access to the data associated with the class. If the data has not yet been created
		/// the behaviour is undefined.
		DataType &operator[]( const ClassType *classOwner );

		/// Exercise DataDeletePolicy on the classOwner's data, and remove it. If the data has not yet been created
		/// the behaviour is undefined.
		void erase( const ClassType *classOwner );

	protected:

		typedef std::map< const ClassType*, DataType > ClassDataMap;
		ClassDataMap m_classDataMap;
};

} // namespace IECore

#include "IECore/ClassData.inl"

#endif // IE_CORE_CLASSDATA_H

