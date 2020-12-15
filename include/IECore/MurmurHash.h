//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MURMURHASH_H
#define IECORE_MURMURHASH_H

#include "IECore/Export.h"
#include "IECore/InternedString.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "tbb/concurrent_hash_map.h"

#include <iostream>

#include <stdint.h>

namespace IECore
{

/// A nice class for hashing arbitrary chunks of data, based on
/// code available at http://code.google.com/p/smhasher.
///
/// From that page :
///
/// "All MurmurHash versions are public domain software, and the
/// author disclaims all copyright to their code."
///
/// \todo Deal with endian-ness.
class IECORE_API MurmurHash
{

	public :

		MurmurHash();
		MurmurHash( const MurmurHash &other );

		// Construct directly from known internal values
		MurmurHash( uint64_t h1, uint64_t h2 );

		inline MurmurHash &append( char data );
		inline MurmurHash &append( unsigned char data );
		inline MurmurHash &append( short data );
		inline MurmurHash &append( unsigned short data );
		inline MurmurHash &append( int data );
		inline MurmurHash &append( unsigned int data );
		inline MurmurHash &append( int64_t data );
		inline MurmurHash &append( uint64_t data );
		inline MurmurHash &append( half data );
		inline MurmurHash &append( float data );
		inline MurmurHash &append( double data );
		inline MurmurHash &append( const char *data );
		inline MurmurHash &append( const std::string &data );
		inline MurmurHash &append( const InternedString &data );
		inline MurmurHash &append( const Imath::V2i &data );
		inline MurmurHash &append( const Imath::V2f &data );
		inline MurmurHash &append( const Imath::V2d &data );
		inline MurmurHash &append( const Imath::V3i &data );
		inline MurmurHash &append( const Imath::V3f &data );
		inline MurmurHash &append( const Imath::V3d &data );
		inline MurmurHash &append( const Imath::Color3f &data );
		inline MurmurHash &append( const Imath::Color3<double> &data );
		inline MurmurHash &append( const Imath::Color4f &data );
		inline MurmurHash &append( const Imath::Color4<double> &data );
		inline MurmurHash &append( const Imath::M33f &data );
		inline MurmurHash &append( const Imath::M33d &data );
		inline MurmurHash &append( const Imath::M44f &data );
		inline MurmurHash &append( const Imath::M44d &data );
		inline MurmurHash &append( const Imath::Box2i &data );
		inline MurmurHash &append( const Imath::Box2f &data );
		inline MurmurHash &append( const Imath::Box2d &data );
		inline MurmurHash &append( const Imath::Box3i &data );
		inline MurmurHash &append( const Imath::Box3f &data );
		inline MurmurHash &append( const Imath::Box3d &data );
		inline MurmurHash &append( const Imath::Quatf &data );
		inline MurmurHash &append( const Imath::Quatd &data );
		inline MurmurHash &append( const MurmurHash &data );

		inline MurmurHash &append( const char *data, size_t numElements );
		inline MurmurHash &append( const unsigned char *data, size_t numElements );
		inline MurmurHash &append( const short *data, size_t numElements );
		inline MurmurHash &append( const unsigned short *data, size_t numElements );
		inline MurmurHash &append( const int *data, size_t numElements );
		inline MurmurHash &append( const unsigned int *data, size_t numElements );
		inline MurmurHash &append( const int64_t *data, size_t numElements );
		inline MurmurHash &append( const uint64_t *data, size_t numElements );
		inline MurmurHash &append( const half *data, size_t numElements );
		inline MurmurHash &append( const float *data, size_t numElements );
		inline MurmurHash &append( const double *data, size_t numElements );
		inline MurmurHash &append( const std::string *data, size_t numElements );
		inline MurmurHash &append( const InternedString *data, size_t numElements );
		inline MurmurHash &append( const Imath::V2i *data, size_t numElements );
		inline MurmurHash &append( const Imath::V2f *data, size_t numElements );
		inline MurmurHash &append( const Imath::V2d *data, size_t numElements );
		inline MurmurHash &append( const Imath::V3i *data, size_t numElements );
		inline MurmurHash &append( const Imath::V3f *data, size_t numElements );
		inline MurmurHash &append( const Imath::V3d *data, size_t numElements );
		inline MurmurHash &append( const Imath::Color3f *data, size_t numElements );
		inline MurmurHash &append( const Imath::Color3<double> *data, size_t numElements );
		inline MurmurHash &append( const Imath::Color4f *data, size_t numElements );
		inline MurmurHash &append( const Imath::Color4<double> *data, size_t numElements );
		inline MurmurHash &append( const Imath::M33f *data, size_t numElements );
		inline MurmurHash &append( const Imath::M33d *data, size_t numElements );
		inline MurmurHash &append( const Imath::M44f *data, size_t numElements );
		inline MurmurHash &append( const Imath::M44d *data, size_t numElements );
		inline MurmurHash &append( const Imath::Box2i *data, size_t numElements );
		inline MurmurHash &append( const Imath::Box2f *data, size_t numElements );
		inline MurmurHash &append( const Imath::Box2d *data, size_t numElements );
		inline MurmurHash &append( const Imath::Box3i *data, size_t numElements );
		inline MurmurHash &append( const Imath::Box3f *data, size_t numElements );
		inline MurmurHash &append( const Imath::Box3d *data, size_t numElements );
		inline MurmurHash &append( const Imath::Quatf *data, size_t numElements );
		inline MurmurHash &append( const Imath::Quatd *data, size_t numElements );

		inline const MurmurHash &operator = ( const MurmurHash &other );

		inline bool operator == ( const MurmurHash &other ) const;
		inline bool operator != ( const MurmurHash &other ) const;

		inline bool operator < ( const MurmurHash &other ) const;

		std::string toString() const;

		// Access internal storage for special cases
		inline uint64_t h1() const;
		inline uint64_t h2() const;

	private :

		inline void append( const void *data, size_t bytes, int elementSize );

		uint64_t m_h1;
		uint64_t m_h2;

		friend size_t tbb_hasher( const MurmurHash &h );
		friend size_t hash_value( const MurmurHash &h );

};

IECORE_API std::ostream &operator << ( std::ostream &o, const MurmurHash &hash );

} // namespace IECore

#include "IECore/MurmurHash.inl"

#endif // IECORE_MURMURHASH_H
