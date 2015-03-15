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

#ifndef IECORE_FONT_H
#define IECORE_FONT_H

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"

#include "IECore/Export.h"
#include "IECore/RunTimeTyped.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( MeshPrimitive );
IE_CORE_FORWARDDECLARE( ImagePrimitive );
IE_CORE_FORWARDDECLARE( Group );

/// The Font class allows the loading of fonts and their
/// conversion to MeshPrimitives and ImagePrimitives.
/// \ingroup renderingGroup
class IECORE_API Font : public RunTimeTyped
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( Font, RunTimeTyped );

		Font( const std::string &fontFile );
		virtual ~Font();

		const std::string &fileName() const;

		void setKerning( float kerning );
		float getKerning() const;

		/// Sets the tolerance used when converting
		/// curved segments of glyphs into triangle
		/// meshes. Smaller values produce denser
		/// meshes. Tolerance is specified in the
		/// same coordinate system as the resulting
		/// mesh - that is one unit in the mesh is
		/// equal to one em.
		void setCurveTolerance( float tolerance );
		float getCurveTolerance() const;

		/// Sets the resolution used in converting
		/// glyphs into images.
		void setResolution( float pixelsPerEm );
		float getResolution() const;

		/// Returns a mesh for the specified character, using
		/// the current curve tolerance. This returns a reference
		/// into an internal cache and hence the resulting mesh
		/// is const.
		const MeshPrimitive *mesh( char c ) const;
		/// Returns a mesh representing the specified string,
		/// using the current curve tolerance and kerning.
		MeshPrimitivePtr mesh( const std::string &text ) const;
		/// Returns a group representing the specified string,
		/// using the current curve tolerance and kerning.
		GroupPtr meshGroup( const std::string &text ) const;
		/// Returns the necessary appropriate offset between the
		/// origins of the first and second characters, taking
		/// into account the current kerning.
		Imath::V2f advance( char first, char second ) const;
		/// Returns a bounding box guaranteed to be large enough
		/// to contain all characters from the font. 1 unit in this
		/// bound is equal to 1 em.
		Imath::Box2f bound() const;
		/// Returns the bounding box for the specified character -
		/// units are as above.
		Imath::Box2f bound( char c ) const;
		/// Returns the bounding box for the specified string taking
		/// into account the current kerning settings - units are as
		/// above.
		Imath::Box2f bound( const std::string &text ) const;
		/// Returns an ImagePrimitive to represent the specified
		/// character, using the current resolution. The image will have
		/// a single channel named "Y". The display window is the same for all
		/// characters, and will bound any character in the font. The data window
		/// will differ for each character and covers the bounding box of the
		/// individual character. 0,0 in pixel coordinates corresponds to the
		/// origin of the character on the baseline - bear in mind that image coordinates
		/// increase from top to bottom, so the top of the character will typically
		/// have a negative y coordinate in pixel space.
		const ImagePrimitive *image( char c ) const;
		/// Returns an image containing a grid of 16x8 characters containing
		/// all the chars from 0-127 inclusive. This too has a single "Y" channel.
		/// \todo These images currently return a straight conversion of the data
		/// from FreeType, which is intended for direct display without colour conversion.
		/// I think we should linearise the data before returning it.
		ImagePrimitivePtr image() const;

	private :

		IE_CORE_FORWARDDECLARE( Implementation );
		ImplementationPtr m_implementation;

		class Mesher;
		
};

IE_CORE_DECLAREPTR( Font );

}

#endif // IECORE_FONT_H
