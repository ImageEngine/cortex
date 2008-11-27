//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

namespace IECore
{

#define IE_CORE_DECLARETYPEFNS( TYPENAME )										\
	virtual IECore::TypeId typeId() const { return TYPENAME ## TypeId; };		\
	virtual std::string typeName() const { return #TYPENAME; };					\
	static IECore::TypeId staticTypeId() { return TYPENAME ## TypeId; };		\
	static std::string staticTypeName() { return #TYPENAME; };

#define IE_CORE_DECLAREEXTENSIONTYPEFNS( TYPENAME, TYPEID )							\
	virtual IECore::TypeId typeId() const { return IECore::TypeId(TYPEID); };		\
	virtual std::string typeName() const { return #TYPENAME; };						\
	static IECore::TypeId staticTypeId() { return IECore::TypeId(TYPEID); };		\
	static std::string staticTypeName() { return #TYPENAME; };
		
#define IE_CORE_DECLARETYPEISINSTANCEFUNCTIONS( BASETYPE )																										\
	virtual bool isInstanceOf( IECore::TypeId typeId ) const { return typeId==staticTypeId() ? true : BASETYPE::isInstanceOf( typeId ); };						\
	virtual bool isInstanceOf( const std::string &typeName ) const { return typeName==staticTypeName() ? true : BASETYPE::isInstanceOf( typeName ); };			\
	
#define IE_CORE_DECLAREINHERITSFROMFUNCTIONS( BASETYPE )																										\
	static bool inheritsFrom( IECore::TypeId typeId ) { return BASETYPE::staticTypeId()==typeId ? true : BASETYPE::inheritsFrom( typeId ); };					\
	static bool inheritsFrom( const std::string &typeName ) { return BASETYPE::staticTypeName()==typeName ? true : BASETYPE::inheritsFrom( typeName ); }		\
	
/// Use this macro within the public section of an IECore class declaration to
/// implement all necessary RunTimeTyped functions. TYPE is the name of
/// the class and BASETYPE is the name of the base class. The TypeId
/// enum must contain an entry TYPETypeId.
#define IE_CORE_DECLARERUNTIMETYPED( TYPE, BASETYPE )		\
	IE_CORE_DECLAREMEMBERPTR( TYPE )  						\
	IE_CORE_DECLARETYPEFNS( TYPE )							\
	IE_CORE_DECLARETYPEISINSTANCEFUNCTIONS( BASETYPE )		\
	IE_CORE_DECLAREINHERITSFROMFUNCTIONS( BASETYPE )		\
	
/// Use this macro within the public section of an extension library class declaration to
/// implement all necessary RunTimeTyped functions. TYPE is the name of
/// the class and BASETYPE is the name of the base class. TYPEID is a unique numeric type
/// identifier that must not clash with any other.
#define IE_CORE_DECLARERUNTIMETYPEDEXTENSION( TYPE, TYPEID, BASETYPE )		\
	IE_CORE_DECLAREMEMBERPTR( TYPE )  										\
	IE_CORE_DECLAREEXTENSIONTYPEFNS( TYPE, TYPEID )							\
	IE_CORE_DECLARETYPEISINSTANCEFUNCTIONS( BASETYPE )						\
	IE_CORE_DECLAREINHERITSFROMFUNCTIONS( BASETYPE )						\

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
		virtual std::string typeName() const;
		
		/// Returns the TypeId for this class, without needing an instance.
		static TypeId staticTypeId();
		/// Returns the type name for this class, without needing an instance.
		static std::string staticTypeName();
		
		/// Returns true if this object is an instance of the specified type,
		/// or of a class inherited from the specified type.
		virtual bool isInstanceOf( TypeId typeId ) const;
		/// Returns true if this object is an instance of the specified type,
		/// or of a class inherited from the specified type.
		virtual bool isInstanceOf( const std::string &typeName ) const;
		
		/// Returns true if this class inherits from the specified type.
		static bool inheritsFrom( TypeId typeId );
		/// Returns true if this class inherits from the specified type.
		static bool inheritsFrom( const std::string &typeName );
		
		//@}

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
