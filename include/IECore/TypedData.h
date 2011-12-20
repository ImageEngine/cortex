//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TYPEDDATA_H
#define IE_CORE_TYPEDDATA_H

#include "IECore/Data.h"
#include "IECore/Exception.h"
#include "IECore/TypedDataTraits.h"

namespace IECore
{

/// A templated class which can be used to wrap useful data
/// types and containers. The copyFrom() function is implemented
/// so as to perform a lazy copy on write type behaviour - this
/// makes the creation of read only copies of significant amounts
/// of data relatively cheap. Note that as this class derives from
/// Object, a proper instantiation requires a fair bit of specialisation
/// of the methods reimplemented from Object. For this reason the
/// template implementation is hidden from the public interface, and
/// you can therefore only use only the typedefs in SimpleTypedData.h,
/// VectorTypedData.h and CompoundData.h rather than being able
/// to instantiate the template for arbitrary data types.
/// It also provides low level access to its data through functions
/// like baseReadable, baseWritable and baseSize. They are available
/// only when the data can be seen as an array of a base type. Use
/// hasBase for checking that.
/// \ingroup coreGroup
template <class T>
class TypedData : public Data
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
		virtual bool isEqualTo( const Object *other ) const;
		virtual void hash( MurmurHash &h ) const;
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
		/// copied behind the scenes, it may not be called while
		/// other threads are operating on the same instance.
		T &writable();

		/// Base type used in the internal data structure.
		typedef typename TypedDataTraits< TypedData<T> >::BaseType BaseType;

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

		virtual ~TypedData();

		static Object::TypeDescription<TypedData<T> > m_typeDescription;

		virtual void copyFrom( const Object *other, CopyContext *context );
		virtual void save( SaveContext *context ) const;
		virtual void load( LoadContextPtr context );
		/// If the memory usage is anything other than sizeof(T) for
		/// a given T then you must provide an accurate specialisation
		/// for this function.
		virtual void memoryUsage( Object::MemoryAccumulator &accumulator ) const;

		class DataHolder : public RefCounted
		{
			public:
				DataHolder() : data() {}
				DataHolder(const T &initData) : data(initData) {}
			public:
				T data;
		};
		IE_CORE_DECLAREPTR( DataHolder );
		DataHolderPtr m_data;
};

} // namespace IECore

#endif // IE_CORE_TYPEDDATA_H
