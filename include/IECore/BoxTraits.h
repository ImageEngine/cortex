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

#ifndef IE_CORE_BOXTRAITS_H
#define IE_CORE_BOXTRAITS_H

#include <boost/static_assert.hpp>

#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathBoxAlgo.h"

#include "IECore/VectorTraits.h"

namespace IECore
{

/// The BoxTypeTraits struct provides a means of deriving the vector base types of different box
/// classes within templated code. 
template<typename T>
struct BoxTypeTraits
{
	typedef typename T::BaseType BaseType;
};

/// The BoxTraits struct provides a means of using different box classes within templated code. The default
/// implementation is compatible with the Imath library's Box classes.
/// \todo Separate this out into BoxTraits and BoxOps
template<typename T>
struct BoxTraits
{
	typedef typename BoxTypeTraits<T>::BaseType BaseType;
	typedef VectorTraits< typename BoxTypeTraits<T>::BaseType > VectorTraits;
	
	/// Create a box from the minimum and maximum corner points
	static T create( const BaseType &min, const BaseType &max )
	{
		return T( min, max );
	}

	/// Return the box's minimum corner point
	static BaseType min( const T& box )
	{
		return box.min;
	}

	/// Return the box's maximum corner point	
	static BaseType max( const T& box )
	{
		return box.max;
	}	
	
	/// Return the dimensions of the box
	static BaseType size( const T& box )
	{
		return box.size();
	}
	
	/// Return the center point of the box
	static BaseType center( const T& box )
	{
		return box.center();
	}	
	
	/// Return true if the box is considered to be empty
	static bool isEmpty( const T& box )
	{
		return box.isEmpty();
	}
	
	/// Modify the box such that it is considered to be empty
	static void makeEmpty( T& box )
	{
		box.makeEmpty();
		
		assert( isEmpty(box) );
	}

	/// Enlarge the box to include the given point
	static void extendBy( T& box, const BaseType& p )
	{
		box.extendBy( p );
		
		assert( intersects( box, p ) );
	}

	/// Enlarge the box to include the given box	
	static void extendBy( T& box, const T& box2 )
	{
		box.extendBy( box2 );
		
		assert( intersects( box, box2 ) );
	}
		
	/// Return true if the box contains the given box	
	static bool intersects( const T& box, const BaseType& p )
	{
		return box.intersects( p );
	}
	
	/// Return true if the two boxes intersect
	static bool intersects( const T& box, const T& box2 )
	{
		assert( box.intersects( box2 ) == box2.intersects( box ) );
		
		return box.intersects( box2 );
	}
};

template<>
struct BoxTypeTraits<Imath::Box3s>
{
	typedef Imath::V3s BaseType;

};

template<>
struct BoxTypeTraits<Imath::Box3i>
{
	typedef Imath::V3i BaseType;

};

template<>
struct BoxTypeTraits<Imath::Box3f>
{
	typedef Imath::V3f BaseType;
	
};

template<>
struct BoxTypeTraits<Imath::Box3d>
{
	typedef Imath::V3d BaseType;	

};

template<>
struct BoxTypeTraits<Imath::Box2s>
{
	typedef Imath::V2s BaseType;
	
};

template<>
struct BoxTypeTraits<Imath::Box2i>
{
	typedef Imath::V2i BaseType;

};

template<>
struct BoxTypeTraits<Imath::Box2f>
{
	typedef Imath::V2f BaseType;	

};

template<>
struct BoxTypeTraits<Imath::Box2d>
{
	typedef Imath::V2d BaseType;	
	
};


} // namespace IECore

#endif // IE_CORE_BOXTRAITS_H
