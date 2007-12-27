//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_BOXTRAITS_H
#define IE_COREMAYA_BOXTRAITS_H

#include "OpenEXR/ImathLimits.h"

#include "IECore/BoxTraits.h"
#include "IECoreMaya/VectorTraits.h"

#include "maya/MBoundingBox.h"

namespace IECore
{

template<>
struct BoxTraits<MBoundingBox>
{
	typedef MPoint BaseType;
	
	/// Create a box from the minimum and maximum corner points
	static MBoundingBox create( const MPoint &min, const MPoint &max )
	{
		return MBoundingBox( min, max );
	}

	/// Return the box's minimum corner point
	static MPoint min( const MBoundingBox& box )
	{
		return box.min();
	}

	/// Return the box's maximum corner point	
	static MPoint max( const MBoundingBox& box )
	{
		return box.max();
	}	
	
	/// Return the dimensions of the box
	static MPoint size( const MBoundingBox& box )
	{
		return MPoint( box.width(), box.height(), box.depth() );
	}
	
	/// Return the center point of the box
	static MPoint center( const MBoundingBox& box )
	{
		return box.center();
	}	
	
	/// Return true if the box is considered to be empty
	static bool isEmpty( const MBoundingBox& box )
	{	
		return box.width() * box.height() * box.depth() <= Imath::limits<double>::epsilon() ;				
	}
	
	/// Modify the box such that it is considered to be empty
	static void makeEmpty( MBoundingBox& box )
	{
		box.clear();
		
		assert( isEmpty(box) );
	}

	/// Enlarge the box to include the given point
	static void extendBy( MBoundingBox& box, const MPoint& p )
	{
		box.expand( p );
		
		assert( intersects( box, p ) );
	}

	/// Enlarge the box to include the given box	
	static void extendBy( MBoundingBox& box, const MBoundingBox& box2 )
	{
		box.expand( box2 );
		
		assert( intersects( box, box2 ) );
	}
		
	/// Return true if the box contains the given box	
	static bool intersects( const MBoundingBox& box, const MPoint& p )
	{
		return box.contains( p );
	}
	
	/// Return true if the two boxes intersect
	static bool intersects( const MBoundingBox& box, const MBoundingBox& box2 )
	{
		assert( box.intersects( box2 ) == box2.intersects( box ) );
		
		return box.intersects( box2 );
	}
};

} // namespace IECore

#endif // IE_COREMAYA_BOXTRAITS_H
