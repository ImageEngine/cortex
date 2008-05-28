//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/RunTimeTyped.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"

#include "boost/shared_ptr.hpp"

#include <map>

/// Forward declarations for freetype types
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_FaceRec_ *FT_Face;

namespace IECore
{

IE_CORE_FORWARDDECLARE( MeshPrimitive );
IE_CORE_FORWARDDECLARE( ImagePrimitive );
IE_CORE_FORWARDDECLARE( Group );

/// The Font class allows the loading of fonts and their
/// conversion to MeshPrimitives.
class Font : public RunTimeTyped
{
	
	public :

		IE_CORE_DECLARERUNTIMETYPED( Font, RunTimeTyped );

		Font( const std::string &fontFile );
		virtual ~Font();

		const std::string &fileName() const;

		void setKerning( float kerning );
		float getKerning() const;

		void setCurveTolerance( float tolerance );
		float getCurveTolerance() const;
		
		void setResolution( float pixelsPerEm );
		float getResolution() const;

		/// \todo All these methods should be const. The internal cache
		/// is an implementation detail which is irrelevant to clients
		/// of the class.
		ConstMeshPrimitivePtr mesh( char c );
		MeshPrimitivePtr mesh( const std::string &text );
		GroupPtr meshGroup( const std::string &text );
		Imath::V2f advance( char first, char second );
		Imath::Box2f bound( char c );
		Imath::Box2f bound( const std::string &text );
		
	private :

		class Mesher;
		
		static FT_Library library();

		FT_Face m_face;
		float m_kerning;
		float m_curveTolerance;
		float m_pixelsPerEm;
		
		struct Mesh;
		typedef boost::shared_ptr<Mesh> MeshPtr;
		typedef boost::shared_ptr<const Mesh> ConstMeshPtr;
		typedef std::map<char, ConstMeshPtr> MeshMap;
		MeshMap m_meshes;
		
		ConstMeshPtr cachedMesh( char c );
		
};

IE_CORE_DECLAREPTR( Font );

}

#endif // IECORE_FONT_H
