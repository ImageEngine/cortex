//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_RUNTIMETYPED_H
#define IE_CORE_RUNTIMETYPED_H

#include "IECore/RefCounted.h"
#include "IECore/TypeIds.h"

#include <string>
#include <map>
#include <vector>
#include <set>

namespace IECore
{

#define IE_CORE_DECLARETYPEFNS( TYPENAME )\
	virtual IECore::TypeId typeId() const { return TYPENAME ## TypeId; };\
	virtual const char *typeName() const { return #TYPENAME; };\
	static IECore::TypeId staticTypeId() { return TYPENAME ## TypeId; };\
	static const char *staticTypeName() { return #TYPENAME; };\
	static IECore::TypeId baseTypeId() { return BaseClass::staticTypeId(); };\
	static const char *baseTypeName() { return BaseClass::staticTypeName(); };	

#define IE_CORE_DECLAREEXTENSIONTYPEFNS( TYPENAME, TYPEID )\
	virtual IECore::TypeId typeId() const { return IECore::TypeId(TYPEID); };\
	virtual const char *typeName() const { return #TYPENAME; };\
	static IECore::TypeId staticTypeId() { return IECore::TypeId(TYPEID); };\
	static const char *staticTypeName() { return #TYPENAME; };\
	static IECore::TypeId baseTypeId() { return BaseClass::staticTypeId(); };\
	static const char *baseTypeName() { return BaseClass::staticTypeName(); };	
		
#define IE_CORE_DECLARETYPEISINSTANCEFUNCTIONS()																										\
	virtual bool isInstanceOf( IECore::TypeId typeId ) const { return typeId==staticTypeId() ? true : BaseClass::isInstanceOf( typeId ); };						\
	virtual bool isInstanceOf( const char *typeName ) const { return !strcmp( typeName, staticTypeName() ) ? true : BaseClass::isInstanceOf( typeName ); };			\
	
#define IE_CORE_DECLAREINHERITSFROMFUNCTIONS()																										\
	static bool inheritsFrom( IECore::TypeId typeId ) { return BaseClass::staticTypeId()==typeId ? true : BaseClass::inheritsFrom( typeId ); };					\
	static bool inheritsFrom( const char *typeName ) { return !strcmp(BaseClass::staticTypeName(), typeName ) ? true : BaseClass::inheritsFrom( typeName ); }		\

#define IE_CORE_DECLARERUNTIMETYPEDDESCRIPTION( TYPE )		\
	private: \
	static const IECore::RunTimeTyped::TypeDescription<TYPE> g_typeDescription;\
	public:
	
#define IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( TYPE )																	\
	const IECore::RunTimeTyped::TypeDescription<TYPE> TYPE::g_typeDescription;		
	
#define IE_CORE_DEFINERUNTIMETYPED( TYPE )																	\
	IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( TYPE )
		
/// Use this macro within the public section of an IECore class declaration to
/// implement all necessary RunTimeTyped functions. TYPE is the name of
/// the class and BASETYPE is the name of the base class. The TypeId
/// enum must contain an entry TYPETypeId.
#define IE_CORE_DECLARERUNTIMETYPED( TYPE, BASETYPE )\
	typedef BASETYPE BaseClass;\
	IE_CORE_DECLAREMEMBERPTR( TYPE )\
	IE_CORE_DECLARETYPEFNS( TYPE )\
	IE_CORE_DECLARETYPEISINSTANCEFUNCTIONS()\
	IE_CORE_DECLAREINHERITSFROMFUNCTIONS()\
	IE_CORE_DECLARERUNTIMETYPEDDESCRIPTION( TYPE ) \
	
	
	
/// Use this macro within the public section of an extension library class declaration to
/// implement all necessary RunTimeTyped functions. TYPE is the name of
/// the class and BASETYPE is the name of the base class. TYPEID is a unique numeric type
/// identifier that must not clash with any other.
#define IE_CORE_DECLARERUNTIMETYPEDEXTENSION( TYPE, TYPEID, BASETYPE )\
	typedef BASETYPE BaseClass; \
	IE_CORE_DECLAREMEMBERPTR( TYPE )  \
	IE_CORE_DECLAREEXTENSIONTYPEFNS( TYPE, TYPEID )\
	IE_CORE_DECLARETYPEISINSTANCEFUNCTIONS()\
	IE_CORE_DECLAREINHERITSFROMFUNCTIONS()\
	IE_CORE_DECLARERUNTIMETYPEDDESCRIPTION( TYPE )
	

/// An abstract base class for objects whose type we wish to determine at runtime.
/// The rationale for using such a type system rather than the std c++
/// typeid() stuff is as follows :
///
///	1) The GCC implentation of the C++ system breaks down with
///	templated types across module boundaries.
///
/// 2) We wish to use the type system to identify the type
/// of serialised objects in files (see the serialisation
/// interface defined in Object), but the C++ type_info object
/// provides us with no information we can usefully use for that.
class RunTimeTyped : public RefCounted
{
	public:

		/// A typedef for the class this class derives from. All RunTimeTyped classes define this typedef.
		typedef RefCounted BaseClass;
		
		IE_CORE_DECLAREMEMBERPTR( RunTimeTyped );
		
		RunTimeTyped();
		virtual ~RunTimeTyped();
		
		//! @name Type identification functions.
		/// These functions provide useful queries about the typing
		/// of both classes and instances. They must be reimplemented
		/// appropriately in all derived classes. This is achieved
		/// through the use of the IE_CORE_DECLARERUNTIMETYPED
		/// macro.
		////////////////////////////////////////////////////////////
		//@{
		
		/// Returns a unique numeric identifier for the type
		/// of this instance. For classes defined in the core library
		///  this should be a member of the TypeId enum defined in IECore/TypeIds.h
		virtual TypeId typeId() const;
		/// Returns a unique name for the type of this instance. This should be
		/// implemented to return the class name.
		virtual const char *typeName() const;
		
		/// Returns the TypeId for this class, without needing an instance.
		static TypeId staticTypeId();
		/// Returns the type name for this class, without needing an instance.
		static const char *staticTypeName();
		
		/// Returns the TypeId of the base of this class, without needing an instance. 
		/// The base type of RunTimeTyped itself is defined to be InvalidTypeId;
		static TypeId baseTypeId();
		
		/// Returns the type name of the base of this class, without needing an instance. 
		/// The base type name of RunTimeTyped itself is defined to be "InvalidType";
		static const char *baseTypeName();		
		
		/// Returns true if this object is an instance of the specified type,
		/// or of a class inherited from the specified type.
		virtual bool isInstanceOf( TypeId typeId ) const;
		/// Returns true if this object is an instance of the specified type,
		/// or of a class inherited from the specified type.
		virtual bool isInstanceOf( const char *typeName ) const;
		
		/// Returns true if this class inherits from the specified type.
		static bool inheritsFrom( TypeId typeId );
		/// Returns true if this class inherits from the specified type.
		static bool inheritsFrom( const char *typeName );
		
		/// Returns the base type of the given type, or InvalidTypeId if no such base exists.
		static TypeId baseTypeId( TypeId typeId );
		
		/// Returns all bases of the given type, or an empty vector if no such bases exist.
		/// The elements are ordered by "distance" from the given TypeId. That is to say, the first
		/// element will be the immediate base class, and the last elemenet will be RunTimeTyped.
		/// Should not be called during static initialization as it's likely that not all types will
		/// have been registered at that point, so to do so would yield an incomplete list.
		static const std::vector<TypeId> &baseTypeIds( TypeId typeId );		
		
		/// Returns all derived types of the given type, or an empty set if no such derived types exist.
		/// Should not be called during static initialization as it's likely that not all types will
		/// have been registered at that point, so to do so would yield an incomplete list.
		static const std::set<TypeId> &derivedTypeIds( TypeId typeId );	
		
		/// Returns the corresponding TypeId for the specified
		/// typeName, or InvalidTypeId if typeName is not a
		/// registered type.
		static TypeId typeIdFromTypeName( const char *typeName );
		/// Returns the corresponding typeName for the specified
		/// TypeId, or InvalidTypeId if typeId is not a
		/// registered type.
		static const char *typeNameFromTypeId( TypeId typeId );
		
		//@}
		
	protected :
	
		template<class T>
		struct TypeDescription
		{
			TypeDescription();
		};
		
		typedef std::map< TypeId, TypeId > BaseTypeRegistryMap;
		typedef std::map< TypeId, std::vector<TypeId> > BaseTypesRegistryMap;
		typedef std::map< TypeId, std::set< TypeId > > DerivedTypesRegistryMap;		
		
		static BaseTypeRegistryMap &baseTypeRegistry();
		static DerivedTypesRegistryMap &derivedTypesRegistry();
		
		static BaseTypesRegistryMap &completeBaseTypesRegistry();
		static DerivedTypesRegistryMap &completeDerivedTypesRegistry();
		
		static void derivedTypeIdsWalk( TypeId typeId, std::set<TypeId> & );	
		
		static void registerType( TypeId derivedTypeId, const char *derivedTypeName, TypeId baseTypeId );	
		
		typedef std::map<TypeId, std::string> TypeIdsToTypeNamesMap;
		typedef std::map<std::string, TypeId> TypeNamesToTypeIdsMap;
		
		static TypeIdsToTypeNamesMap &typeIdsToTypeNames();
		static TypeNamesToTypeIdsMap &typeNamesToTypeIds();
			
};

IE_CORE_DECLAREPTR( RunTimeTyped );

/// Equivalent to boost::dynamic_pointer_cast but using the type identification
/// system implemented in RunTimeTyped. This should be used in preference to
/// both dynamic_cast and boost::dynamic_pointer_cast wherever possible.
template<typename T, typename S>
boost::intrusive_ptr<T> runTimeCast( const boost::intrusive_ptr<S> &src );
/// Equivalent to dynamic_cast but using the type identification system
/// implemented in RunTimeTyped. This should be used in preference to
/// dynamic_cast wherever possible.
template<typename T, typename S>
T *runTimeCast( S *src );

/// Equivalent to boost::static_pointer_cast, but using the type identifaction
/// system implemented in RunTimeTyped to fire an assert if the equivalent runTimeCast
/// would not succeed. In a non-asserted build this will compile directly down to
/// a single boost::static_pointer_cast.
template<typename T, typename S>
inline boost::intrusive_ptr<T> assertedStaticCast( const boost::intrusive_ptr<S> &src );

/// Equivalent to static_cast, but using the type identifaction system implemented in 
/// RunTimeTyped to fire an assert if the equivalent runTimeCast would not succeed. 
/// In a non-asserted build this will compile directly down to a single static_cast.
template<typename T, typename S>
inline T* assertedStaticCast( S* src );

} // namespace IECore

#include "IECore/RunTimeTyped.inl"

#endif // IE_CORE_RUNTIMETYPED_H
