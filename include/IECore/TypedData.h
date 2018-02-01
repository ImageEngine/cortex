//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_TYPEDDATA_H
#define IECORE_TYPEDDATA_H

#include "IECore/Data.h"
#include "IECore/Export.h"
#include "IECore/TypedDataInternals.h"

namespace IECore
{

/// A templated class which can be used to wrap useful data
/// types and containers, while implementing all the io, copying
/// and hashing abilities of the Object base class. The internal
/// data may be accessed using the readable() and writable() accessors
/// and in the case of it being composed of one or more contiguous
/// elements of a simple base type it may also be accessed as raw data
/// using the baseReadable() and baseWritable() methods.
///
/// Note that to use instantiations of this class you should utilise the
/// appropriate typedefs as provided by headers such as SimpleTypedData.h
/// or VectorTypedData.h, rather than include this header directly.
///
/// Also note that you cannot simply instantiate this class with an
/// arbitrary type without performing some specialisation of the functions
/// for implementing IO and hashing. DateTimeData.h and the matching
/// DateTimeData.cpp provide a good example for creating your own
/// TypedData class - see comments in those files for further details.
/// \ingroup coreGroup
template <class T>
class IECORE_EXPORT TypedData : public Data
{
	public:

		typedef T ValueType;

		/// Default constructor.
		TypedData();
		/// Constructor based on the stored data type.
		TypedData(const T &data);

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( TypedData<T>, Data );

		//! @name Object interface
		////////////////////////////////////////////////////////////
		//@{
		typename TypedData<T>::Ptr copy() const;
		bool isEqualTo( const Object *other ) const override;
		void hash( MurmurHash &h ) const override;
		//@}

		/// Equivalent to writable() = data
		void operator = (const T &data);
		/// Equivalent to writable() = typedData.readable()
		void operator = (const TypedData<T> &typedData);

		/// Gives read-only access to the internal data structure.
		/// \threading It's safe for multiple concurrent threads to
		/// call readable() on the same instance, provided that no
		/// concurrent modifications are being made to that instance.
		const T &readable() const;
		/// Gives read-write access to the internal data structure.
		/// \threading Because calling writable() may cause data to be
		/// modified behind the scenes, it may not be called while
		/// other threads are operating on the same instance.
		T &writable();

		/// Base type used in the internal data structure.
		typedef typename TypedDataTraits<T>::BaseType BaseType;

		/// Defines whether the internal data structure has a single base type.
		static bool hasBase();

		/// Get low level read-only access to the internal data structure as a reference to the first element on an array of base type.
		/// Throws an Exception if this type has no single base type.
		const BaseType *baseReadable() const;

		/// Get low level read-write access to the internal data structure as a reference to the first element on an array of base type.
		/// Throws an Exception if this type has no single base type.
		BaseType *baseWritable();

		/// Return the size of the internal data structure in terms of base type elements.
		/// Throws an Exception if this type has no single base type.
		size_t baseSize() const;

	protected:

		typedef typename TypedDataTraits<T>::DataHolder DataHolder;

		~TypedData() override;

		static Object::TypeDescription<TypedData<T> > m_typeDescription;

		void copyFrom( const Object *other, CopyContext *context ) override;
		void save( SaveContext *context ) const override;
		void load( LoadContextPtr context ) override;
		/// If the memory usage is anything other than sizeof(T) for
		/// a given T then you must provide an accurate specialisation
		/// for this function.
		void memoryUsage( Object::MemoryAccumulator &accumulator ) const override;

		DataHolder m_data;

};

/// Macro for the declaration of a new TypedData instantiation, holding
/// a type T. TYPENAME is the name of the new TypedData class, T is the
/// held type, BASETYPE is the base type or void if there is no base, and
/// DataHolder is either SimpleDataHolder or ShareableDataHolder.
/// SimpleDataHolder is appropriate where T is of a small fixed size and
/// SharedDataHolder is appropriate where T is larger and/or varies
/// in size. The difference being that ShareableDataHolder implements
/// a lazy-copy-on-write behaviour useful for large types, but also incurs
/// the cost of an extra allocation and reference count inappropriate
/// for small types.
#define IECORE_DECLARE_TYPEDDATA( TYPENAME, T, BASETYPE, DATAHOLDER ) \
	template <> \
	class IECORE_EXPORT TypedDataTraits<T> \
	{ \
		public : \
			typedef BASETYPE BaseType; \
			typedef DATAHOLDER<T> DataHolder; \
	}; \
	\
	typedef TypedData<T> TYPENAME; \
	\
	IE_CORE_DECLAREPTR( TYPENAME ); \

} // namespace IECore

#endif // IECORE_TYPEDDATA_H
