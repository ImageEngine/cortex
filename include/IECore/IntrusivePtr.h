//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_INTRUSIVEPTR_H
#define IE_CORE_INTRUSIVEPTR_H

#include <iosfwd>

namespace IECore
{

/// Templated class following the same interface defined by boost::intrusive_ptr with 
/// one major change: implicit conversion to raw pointers.
/// It uses the same reference increment and decrement functions used by boost:
/// intrusive_ptr_add_ref and intrusive_ptr_release.
///
/// IntrusivePtr should be used anywhere in Cortex where it is necessary to maintain
/// ownership of an instance derived from RefCounted, or to share ownership between
/// several interested parties. The construction of an IntrusivePtr pointer
/// increments the reference count and the destruction of an IntrusivePtr decrements
/// the reference count - when the count drops to 0 the RefCounted object will self
/// destruct. All RefCounted derived types define both a Type::Ptr and TypePtr typedef
/// for an IntrusivePtr pointing to that type.
///
/// There is some limited overhead involved in the reference counting associated with
/// IntrusivePtr, and for this reason, and in an attempt to strengthen the semantics of
/// pointer usage in Cortex, we try to use the following conventions when defining
/// programming interfaces :
///
/// When receiving a pointer as a function argument :
///
/// Pass a raw (Type *) pointer if the called function has no need
/// to increment the reference count. For instance Renderable::render( Renderer *renderer )
/// takes a raw pointer as the Renderable should not need to hold a reference to the Renderer
/// following the call.
///
/// Pass an IntrusivePtr (TypePtr) if the reference count will be changed following the function
/// call. For instance Group::addChild( VisibleRenderablePtr child ) takes an IntrusivePtr as the
/// group will maintain a reference to the child following the call.
///
/// When returning a pointer from a method :
///
/// Return a raw (Type *) pointer if the called object will continue to maintain a reference 
/// to the returned object following the method call. For instance CompoundObject::member() returns
/// a raw pointer, because the CompoundObject will continue to hold a reference to the returned object
/// following the call. The caller can assign the raw pointer to an IntrusivePtr if it wishes to own
/// its own reference to the result, but if it merely wants to use the result temporarily it can
/// rely on the owner to maintain a reference until the owner dies.
///
/// Return an IntrusivePtr (TypePtr) if the method or function is returning an object for which no
/// other references are currently held. For instance Object::create() returns an ObjectPtr so that
/// a reference to the new object exists, and to encourage the caller to maintain ownership of the new
/// object.
///
/// When calling operator new :
///
/// Always assign the result of operator new for a RefCounted class to an IntrusivePtr immediately, to
/// assume ownership of the new object.
///
/// \note Not all of Cortex has been updated to match these conventions. Please add todo items for any
/// nonconforming APIs.
template<class T> 
class IntrusivePtr
{
	public:

		typedef T element_type;

		IntrusivePtr();

		IntrusivePtr( T * p );

		template<typename U>
		IntrusivePtr( IntrusivePtr<U> const & rhs );

		IntrusivePtr( IntrusivePtr const & rhs);

		~IntrusivePtr();

		template<typename U> 
		inline IntrusivePtr & operator=( IntrusivePtr<U> const & rhs);

		inline IntrusivePtr & operator=( IntrusivePtr const & rhs);

		inline IntrusivePtr & operator=(T * rhs);

		inline void reset();

		inline void reset( T * rhs );

		inline T * get() const;

		inline T & operator*() const;

		inline T * operator->() const;

		inline operator T *();

		inline operator T const * () const;

		template<class U> 
		inline bool operator==(IntrusivePtr<U> const & b) const;

		template<class U> 
		inline bool operator!=(IntrusivePtr<U> const & b) const;

		template<class U> 
		inline bool operator==(U * b) const;

		template<class U> 
		inline bool operator!=(U * b) const;

		inline bool operator<(IntrusivePtr const & b) const;

		inline void swap(IntrusivePtr & rhs);

	private:

		T * m_p;
};

template<class T, class U> 
inline bool operator==(T * a, IntrusivePtr<U> const & b);

template<class T, class U> 
inline bool operator!=(T * a, IntrusivePtr<U> const & b);

template<class T> 
void swap( IntrusivePtr<T> & lhs, IntrusivePtr<T> & rhs);

// mem_fn support
template<class T> 
T * get_pointer( IntrusivePtr<T> const & p);

template<class T, class U> 
IntrusivePtr<T> staticPointerCast( IntrusivePtr<U> const & p);

template<class T, class U> 
IntrusivePtr<T> constPointerCast( IntrusivePtr<U> const & p);

template<class T, class U> 
IntrusivePtr<T> dynamicPointerCast(IntrusivePtr<U> const & p);

template<class E, class T, class Y> 
std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, IntrusivePtr<Y> const & p);

} // namespace IECore

#include "IECore/IntrusivePtr.inl"

#endif  // IE_CORE_INTRUSIVEPTR_H
