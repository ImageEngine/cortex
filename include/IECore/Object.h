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

#ifndef IE_CORE_OBJECT_H
#define IE_CORE_OBJECT_H

#include <string>

#include "IECore/Export.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/IndexedIO.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Object );

class MurmurHash;

#define IE_CORE_DECLAREOBJECTTYPEDESCRIPTION( TYPENAME )																\
	private :																											\
		static const IECore::Object::TypeDescription<TYPENAME> m_typeDescription;										\
	public :																											\

#define IE_CORE_DECLAREOBJECTMEMBERFNS( TYPENAME )																		\
	public :																											\
		TYPENAME::Ptr copy() const { return boost::static_pointer_cast<TYPENAME>( IECore::Object::copy() ); }			\
		bool isEqualTo( const IECore::Object *other ) const override;													\
		void hash( IECore::MurmurHash &h ) const override;																\
	protected :																											\
		void copyFrom( const IECore::Object *other, IECore::Object::CopyContext *context ) override;					\
		void save( IECore::Object::SaveContext *context ) const override;								 				\
		void load( IECore::Object::LoadContextPtr context ) override;  								   					\
		void memoryUsage( IECore::Object::MemoryAccumulator & ) const override;											\

#define IE_CORE_DECLAREOBJECT( TYPE, BASETYPE ) 																		\
	IE_CORE_DECLARERUNTIMETYPED( TYPE, BASETYPE ); 																		\
	IE_CORE_DECLAREOBJECTMEMBERFNS( TYPE );																				\
	IE_CORE_DECLAREOBJECTTYPEDESCRIPTION( TYPE );																		\

#define IE_CORE_DECLAREEXTENSIONOBJECT( TYPE, TYPEID, BASETYPE ) 														\
	IE_CORE_DECLARERUNTIMETYPEDEXTENSION( TYPE, TYPEID, BASETYPE ); 													\
	IE_CORE_DECLAREOBJECTMEMBERFNS( TYPE );																				\
	IE_CORE_DECLAREOBJECTTYPEDESCRIPTION( TYPE );																		\

#define IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( TYPENAME )																	\
	const IECore::Object::TypeDescription<TYPENAME> TYPENAME::m_typeDescription											\

/// A base class defining copying and streaming.
/// \ingroup coreGroup
/// \todo rename virtual functions that match non-virtual functions so that pointers to derived classes still provide access the non-virtual ones: example hash(), memoryUsage(), save().
class IECORE_API Object : public RunTimeTyped
{
	public:

		Object();

		~Object() override;

		IE_CORE_DECLARERUNTIMETYPED( Object, RunTimeTyped );

		//! @name Object interface
		/// The following functions define the interface to which
		/// all Object subclasses must adhere. Note that the
		/// IE_CORE_DECLAREOBJECT macro is provided to simplify
		/// the declaration of these functions for your own derived classes.
		/// Note also that there is additional protected interface
		/// which you must implement - this too is declared (but not
		/// implemented) by the IE_CORE_DECLAREOBJECT macro.
		////////////////////////////////////////////////////////////
		//@{
		/// Returns a deep copy of this object. In subclasses an
		/// identical function is provided which returns a pointer
		/// to the subclass rather than to this base class.
		ObjectPtr copy() const;
		/// Copies from another object. Throws an IECore::InvalidArgumentException if
		/// other is not an instance of this object.
		void copyFrom( const Object *other );
		/// Saves the object in the current directory of ioInterface, in
		/// a subdirectory with the specified name.
		void save( IndexedIOPtr ioInterface, const IndexedIO::EntryID &name ) const;
		/// Returns true if this object is equal to the other. Should
		/// be reimplemented appropriately in derived classes, first calling
		/// your base class isEqualTo() and returning false straight away
		/// if that returns false. The Object level implementation checks that
		/// the types are identical, so you can safely perform a
		/// boost::static_pointer_cast<YourClass>( other ) if your base class isEqualTo()
		/// doesn't return false.
		virtual bool isEqualTo( const Object *other ) const = 0;
		/// Returns true if this object is not equal to the other. A default
		/// implementation for this returns the negation of isEqualTo(), but you
		/// may wish to override it if you can provide a faster implementation
		/// for a specific subclass.
		virtual bool isNotEqualTo( const Object *other ) const;
		/// Calls isEqualTo() for people who prefer to use the operator syntax.
		bool operator==( const Object &other ) const;
		/// Calls isNotEqualTo() for people who prefer to use the operator syntax.
		bool operator!=( const Object &other ) const;
		/// Returns the number of bytes this instance occupies in memory.
		size_t memoryUsage() const;
		/// Returns a hash computed from all the member data of this object.
		/// This convenience function simply creates a MurmurHash object, appends
		/// to it using the virtual function below and then returns it.
		MurmurHash hash() const;
		/// Must be implemented by subclasses to append all member data into the
		/// given hash. Implementations must first call the base class implementation
		/// before appending their own data.
		virtual void hash( MurmurHash &h ) const = 0;
		//@}

		//! @name Object factory
		/// The following static functions provide the ability to
		/// create an Object of a given type or typeId, as well
		/// as providing conversions between type names and TypeIds.
		/////////////////////////////////////////////////////////////
		//@{
		/// Returns true if typeId is a valid registered Object type.
		static bool isType( TypeId typeId );
		/// Returns true if typeName is a valid registered Object type.
		static bool isType( const std::string &typeName );
		/// Returns true if typeId is a valid registered abstract Object type -
		/// one which cannot be instantiated with create().
		static bool isAbstractType( TypeId typeId );
		/// As above but taking a type name.
		static bool isAbstractType( const std::string &typeName );
		/// Creates an instance of an object of the specified type. Throws an
		/// Exception if typeId is not a valid type.
		static ObjectPtr create( TypeId typeId );
		/// Creates an instance of an object of the specified type.
		/// Throws an Exception if typeName is not a valid type.
		static ObjectPtr create( const std::string &typeName );
		/// Loads an object previously saved with the given name in the current directory
		/// of ioInterface.
		static ObjectPtr load( ConstIndexedIOPtr ioInterface, const IndexedIO::EntryID &name );
		//@}

		typedef std::function<ObjectPtr ()> CreatorFn;

		/// Register a new Object-derived type with the system.
		static void registerType( TypeId typeId, const std::string &typeName, CreatorFn creator = CreatorFn() );

	protected :

		/// Instantiating an instance of TypeDescription<YourClass>
		/// causes the registration of your class with the IECore type
		/// system. It's essential that all subclasses of
		/// Object are registered this way. The best way of doing this
		/// is as a private static member of the class being registered.
		template<class T>
		class TypeDescription : protected RunTimeTyped::TypeDescription<T>
		{
			public :
				/// Registers the object using its static typeId and static typename
				TypeDescription();
				/// Registers the object using a specified typeId and typename
				TypeDescription( TypeId alternateTypeId, const std::string &alternateTypeName );
		};

		/// A simple class used in the copyFrom() method to provide
		/// a means of copying Object derived member data while
		/// ensuring the uniqueness of copies of objects in the case
		/// that an object is referred to more than once.
		class IECORE_API CopyContext : private boost::noncopyable
		{
			public :
				CopyContext();
				/// Returns a copy of the specified object.
				template<class T>
				typename T::Ptr copy( const T *toCopy );
			private :
				ObjectPtr copyInternal( const Object *toCopy );
				struct CopiedObjects;
				std::unique_ptr<CopiedObjects> m_copies;
		};

		/// Must be implemented in all subclasses to make a deep copy of
		/// all member data, after calling BaseClass::copyFrom() to allow
		/// the base class to do the same. When making copies of held member
		/// data derived from Object, you /must/ use the context object provided,
		/// rather than calling copy() or copyFrom() yourself.
		virtual void copyFrom( const Object *other, CopyContext *context ) = 0;

		/// The class provided to the save() method implemented by subclasses.
		class IECORE_API SaveContext : private boost::noncopyable
		{
			public :
				SaveContext( IndexedIOPtr ioInterface );
				/// Returns an interface to a container in which you can save your class data. You should save
				/// your data directly into the root of this container. The "filesystem" below the
				/// root is guaranteed to be empty and immune to writes from any badly behaved Object
				/// subclasses.
				/// @param typeName The typename of your class.
				/// @param ioVersion The current file format version for your class. This should be incremented
				/// each time the format you save in changes, and is the same as the version retrieved
				/// in the LoadContext::ioInterface() method. It is recommended that you store your
				/// ioVersion as a private static const member of your class.
				IndexedIOPtr container( const std::string &typeName, unsigned int ioVersion );
				/// Saves an Object instance, saving only a reference in the case that the object has
				/// already been saved.
				void save( const Object *toSave, IndexedIO *o, const IndexedIO::EntryID &name );
				/// Returns an interface to an alternative container in which to save class data. This container
				/// is provided for optimisation reasons and should be used only in extreme cases. The container
				/// provides no protection from overwriting of your class data by base or derived classes, and
				/// provides no versioning. Furthermore you can only use raw IndexedIO methods
				/// for saving in it - SaveContext::save() may not be used and therefore child Objects may not
				/// be saved. This interface is provided primarily for the SimpleTypedData classes, which save
				/// very small amounts of unstructured data where the metadata associated with the standard
				/// container becomes relatively expensive in both disk space and time. Think carefully before
				/// using this container, it provides performance benefits only in extreme cases!
				IndexedIO *rawContainer();
			private :
				struct SavedObjects;
				SaveContext( IndexedIOPtr ioInterface, std::shared_ptr<SavedObjects> savedObjects );
				IndexedIOPtr m_ioInterface;
				std::shared_ptr<SavedObjects> m_savedObjects;
		};

		/// The class provided to the load() method implemented by subclasses.
		class IECORE_API LoadContext : public RefCounted
		{
			public :
				LoadContext( ConstIndexedIOPtr ioInterface );
				/// Returns an interface to the container created by SaveContext::container().
				/// @param typeName The typename of your class.
				/// @param ioVersion On entry this should contain the current file format version
				/// for your class. On exit it will contain the file format version of the file being
				/// read. If the latter is greater than the former an exception is thrown (the file is
				/// newer than the library) - this should not be caught.
				/// @param throwIfMissing If false will and the container does not carry the entry for the type name, returns a null pointer.
				ConstIndexedIOPtr container( const std::string &typeName, unsigned int &ioVersion, bool throwIfMissing = true );
				template<class T>
				/// Load an Object instance previously saved by SaveContext::save().
				typename T::Ptr load( const IndexedIO *container, const IndexedIO::EntryID &name );
				/// Returns an interface to a raw container created by SaveContext::rawContainer() - please see
				/// documentation and cautionary notes for that function.
				const IndexedIO *rawContainer();

			private :
				struct LoadedObjects;
				LoadContext( ConstIndexedIOPtr ioInterface, std::shared_ptr<LoadedObjects> loadedObjects );
				ObjectPtr loadObjectOrReference( const IndexedIO *container, const IndexedIO::EntryID &name );
				ObjectPtr loadObject( const IndexedIO *container );

				ConstIndexedIOPtr m_ioInterface;
				std::shared_ptr<LoadedObjects> m_loadedObjects;
		};
		IE_CORE_DECLAREPTR( LoadContext );

		/// Must be implemented in all derived classes. Implementations should first call the parent class
		/// save() method, then call context->container() before filling the returned container with
		/// their member data. Classes with no member data may omit the call to container(), resulting
		/// in smaller file sizes.
		virtual void save( SaveContext *context ) const = 0;
		/// Must be implemented in all derived classes. Implementations should first call the parent class load() method,
		/// then call context->container() before loading their member data from that container.
		/// context is a smart pointer to a reference counted object to allow you to keep the
		/// context and perform lazy loading at a later date - although this is not yet used
		/// by any of the core types. A call to context->container() will throw an Exception if the corresponding
		/// save() method did not create a container.
		virtual void load( LoadContextPtr context ) = 0;

		/// The class provided to the memoryUsage() virtual method implemented
		/// by subclasses.
		class IECORE_API MemoryAccumulator : private boost::noncopyable
		{
			public :
				MemoryAccumulator();
				/// Adds the specified number of bytes to the total.
				void accumulate( size_t bytes );
				/// Adds object->memoryUsage() to the total, but only
				/// if that object hasn't been accumulated already.
				void accumulate( const Object *object );
				/// Adds bytes to the total, but only if the specified
				/// pointer hasn't been passed to this call already.
				void accumulate( const void *ptr, size_t bytes );
				/// Returns the total accumulated to date.
				size_t total() const;
			private :
				size_t m_total;
				struct Accumulated;
				std::unique_ptr<Accumulated> m_accumulated;
		};

		/// Must be implemented in all derived classes to specify the amount of memory they are
		/// using. An implementation must add it's memory usage to the accumulator before calling
		/// memoryUsage() on its base class.
		virtual void memoryUsage( MemoryAccumulator &accumulator ) const = 0;

	private :

		static const TypeDescription<Object> m_typeDescription;
		static const unsigned int m_ioVersion;

};

} // namespace IECore

#include "IECore/Object.inl"

#endif // IE_CORE_OBJECT_H
