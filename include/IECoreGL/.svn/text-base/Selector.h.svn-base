//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_SELECTOR_H
#define IECOREGL_SELECTOR_H

#include <vector>

#include "OpenEXR/ImathBox.h"

#include "IECoreGL/HitRecord.h"

namespace IECoreGL
{

/// The Selector class simplifies the process of selecting objects
/// rendered with OpenGL.
class Selector
{

	public :

		Selector();
		
		/// Starts an operation to select objects in the specified
		/// region of NDC space (0,0-1,1 top left to bottom right).
		/// Set up the GL camera, call this function, then render
		/// the objects with appropriate glLoadName() calls with names
		/// generated using NameStateComponent. Call selectEnd() to
		/// retrieve the resulting selection hits. It is your responsibility
		/// to keep the selector alive in between begin() and end() calls.
		void begin( const Imath::Box2f &region );
		/// Ends a selection operation, filling the provided vector
		/// with records of all the objects hit. Returns the new size
		/// of the hits vector.
		size_t end( std::vector<HitRecord> &hits );

	private :
	
		std::vector<GLuint> m_selectBuffer;
		
};

} // namespace IECoreGL

#endif // IECOREGL_SELECTOR_H
