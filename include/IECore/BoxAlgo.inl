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
//	     other contributors to this software may be used to endorse or
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

#ifndef IE_CORE_BOXALGO_INL
#define IE_CORE_BOXALGO_INL

namespace IECore
{

template<class T>
std::ostream &operator <<( std::ostream &os, const Imath::Box<T> &obj )
{
      os << "[ " << obj.min << ", " << obj.max << " ]";
      return os;
}

template <class T>
Imath::Vec2<T> closestPointInBox(const Imath::Vec2<T>& p, const Imath::Box< Imath::Vec2<T> >& box )
{
	Imath::Vec2<T> b;

	if (p.x < box.min.x)
	{
		b.x = box.min.x;
	}
	else if (p.x > box.max.x)
	{
		b.x = box.max.x;
	}
	else
	{
		b.x = p.x;
	}

	if (p.y < box.min.y)
	{
		b.y = box.min.y;
	}
	else if (p.y > box.max.y)
	{
		b.y = box.max.y;
	}
	else
	{
		b.y = p.y;
	}	

	return b;	
}	
	
} // namespace IECore

#endif // IE_CORE_BOXALGO_INL
