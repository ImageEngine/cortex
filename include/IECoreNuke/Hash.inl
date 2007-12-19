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

#ifndef IECORENUKE_HASH_INL
#define IECORENUKE_HASH_INL

namespace IECoreNuke
{

template<class T>
void append( DD::Image::Hash &hash, const Imath::Vec2<T> &v )
{
	for( unsigned i=0; i<v.dimensions(); i++ )
	{
		hash.append( v[i] );
	}
}

template<class T>
void append( DD::Image::Hash &hash, const Imath::Vec3<T> &v )
{
	for( unsigned i=0; i<v.dimensions(); i++ )
	{
		hash.append( v[i] );
	}
}

template<class T>
void append( DD::Image::Hash &hash, const Imath::Color4<T> &v )
{
	for( unsigned i=0; i<v.dimensions(); i++ )
	{
		hash.append( v[i] );
	}
}

template<class T>
void append( DD::Image::Hash &hash, const Imath::Box<T> &b )
{
	append( hash, b.min );
	append( hash, b.max );
}

template<class T>
void append( DD::Image::Hash &hash, const Imath::Matrix33<T> &m )
{
	for( int i=0; i<9; i++ )
	{
		hash.append( m.getValue()[i] );
	}
}

template<class T>
void append( DD::Image::Hash &hash, const Imath::Matrix44<T> &m )
{
	for( int i=0; i<16; i++ )
	{
		hash.append( m.getValue()[i] );
	}
}

} // namespace IECoreNuke

#endif // IECORENUKE_HASH_INL
