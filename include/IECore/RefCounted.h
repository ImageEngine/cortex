//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_REFCOUNTED_H
#define IE_CORE_REFCOUNTED_H

#include <cassert>

#include "tbb/atomic.h"

#include "IECore/Export.h"
#include "boost/noncopyable.hpp"
#include "boost/intrusive_ptr.hpp"

namespace IECore
{

#define IE_CORE_DECLAREPTR( TYPENAME ) \
typedef boost::intrusive_ptr< TYPENAME > TYPENAME ## Ptr; \
typedef boost::intrusive_ptr< const TYPENAME > Const ## TYPENAME ## Ptr; \

#define IE_CORE_DECLAREMEMBERPTR( TYPENAME ) \
		typedef boost::intrusive_ptr< TYPENAME > Ptr; \
		typedef boost::intrusive_ptr< const TYPENAME > ConstPtr;

/// This macro can be used when declaring member pointers in template classes with 2 template parameters,
/// where the comma in the template arguments would otherwise confuse the pre-processor
#define IE_CORE_DECLAREMEMBERPTR2( PART1, PART2 ) \
		typedef boost::intrusive_ptr< PART1, PART2 > Ptr; \
		typedef boost::intrusive_ptr< const PART1, PART2 > ConstPtr;

/// This macro can be used when declaring member pointers in template classes with 3 template parameters,
/// where the commas in the template arguments would otherwise confuse the pre-processor
#define IE_CORE_DECLAREMEMBERPTR3( PART1, PART2, PART3 ) \
		typedef boost::intrusive_ptr< PART1, PART2, PART3 > Ptr; \
		typedef boost::intrusive_ptr< const PART1, PART2, PART3 > ConstPtr;

#define IE_CORE_FORWARDDECLARE( TYPENAME )									\
	class TYPENAME;															\
	IE_CORE_DECLAREPTR( TYPENAME )											\

/// A simple reference counted base class, intended for use with boost::intrusive_ptr.
///
/// An intrusive_ptr should be used anywhere in Cortex where it is necessary to maintain
/// ownership of an instance derived from RefCounted, or to share ownership between
/// several interested parties. The construction of an intrusive_ptr
/// increments the reference count and the destruction of an intrusive_ptr decrements
/// the reference count - when the count drops to 0 the RefCounted object will self
/// destruct. All RefCounted derived types define both a Type::Ptr and TypePtr typedef
/// for an intrusive_ptr pointing to that type.
///
/// There is some overhead involved in the reference counting associated with
/// RefCounted, and for this reason, and in an attempt to strengthen the semantics of
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
/// Pass an intrusive_ptr (TypePtr) if the reference count will be changed following the function
/// call. For instance Group::addChild( VisibleRenderablePtr child ) takes an intrusive_ptr as the
/// group will maintain a reference to the child following the call.
///
/// When returning a pointer from a method :
///
/// Return a raw (Type *) pointer if the called object will continue to maintain a reference 
/// to the returned object following the method call. For instance CompoundObject::member() returns
/// a raw pointer, because the CompoundObject will continue to hold a reference to the returned object
/// following the call. The caller can assign the raw pointer to an intrusive_ptr if it wishes to own
/// its own reference to the result, but if it merely wants to use the result temporarily it can
/// rely on the owner to maintain a reference until the owner dies.
///
/// Return an intrusive_ptr (TypePtr) if the method or function is returning an object for which no
/// other references are currently held. For instance Object::create() returns an ObjectPtr so that
/// a reference to the new object exists, and to encourage the caller to maintain ownership of the new
/// object.
///
/// When calling operator new :
///
/// Always assign the result of operator new for a RefCounted class to an intrusive_ptr immediately, to
/// assume ownership of the new object.
///
/// \note Not all of Cortex has been updated to match these conventions. Please add todo items for any
/// nonconforming APIs.
///
/// \todo Disallow construction on the heap by having a private destructor - do we
/// need to do this for all derived classes as well?
/// \ingroup utilityGroup
class IECORE_API RefCounted : private boost::noncopyable
{
	public:

		IE_CORE_DECLAREMEMBERPTR( RefCounted );

		typedef size_t RefCount;

		RefCounted();

		/// Add a reference to the current object
		inline void addRef() const { m_numRefs++; };

		/// Remove a reference from the current object
		inline void removeRef() const
		{
			assert( m_numRefs > 0 );
			if( --m_numRefs==0 )
			{
				delete this;
			}
		};

		/// Returns the current reference count.
		inline RefCount refCount() const { return m_numRefs; };

	protected:

		virtual ~RefCounted();

	private :

		mutable tbb::atomic<RefCount> m_numRefs;

};

IE_CORE_DECLAREPTR( RefCounted )

/// Functions required to allow use of RefCounted with boost::intrusive_ptr
IECORE_API inline void intrusive_ptr_add_ref( const IECore::RefCounted *r )
{
	r->addRef();
}

IECORE_API inline void intrusive_ptr_release(const IECore::RefCounted *r)
{
	r->removeRef();
}

/// Implementation of tbb_hasher to allow intrusive_ptrs to be used
/// with tbb_concurrent_* containers.
template<typename T>
inline size_t tbb_hasher( const boost::intrusive_ptr<T> &ptr )
{
	// This is the same as what tbb uses for raw pointers
	const size_t h = reinterpret_cast<size_t>( ptr.get() );
	return (h >> 3) ^ h;
}

} // namespace IECore


#endif // IE_CORE_REFCOUNTED_H
