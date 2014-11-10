//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_HITRECORD_H
#define IECOREGL_HITRECORD_H

#include "IECore/InternedString.h"

#include "IECoreGL/GL.h"

namespace IECoreGL
{

/// The HitRecord struct represents hit records
/// found in the glSelectBuffer.
class HitRecord
{

	public :

		/// Construct from a hit record in the format specified
		/// for the OpenGL select buffer. Raises an exception if
		/// more than one name is specified in the record.
		HitRecord( const GLuint *hitRecord );
		HitRecord( float dMin, float dMax, GLuint name );

		/// The minimum and maximum depths of the hit, normalised
		/// in the 0-1 range between the near and far clipping planes.
		float depthMin;
		float depthMax;

		/// Identifier for the hit object.
		GLuint name;

		/// Performs comparison based on the depth.min member.
		bool operator < ( const HitRecord &other ) const;

		/// Returns the offset to the next hit record in the select
		/// buffer - this is a constant as the constructor accepts
		/// only hit records with a single name.
		size_t offsetToNext() const;

};

} // namespace IECoreGL

#endif // IECOREGL_HITRECORD_H
