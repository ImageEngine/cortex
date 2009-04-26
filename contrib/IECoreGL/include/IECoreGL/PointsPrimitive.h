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

#ifndef IECOREGL_POINTSPRIMITIVE_H
#define IECOREGL_POINTSPRIMITIVE_H

#include "IECoreGL/Primitive.h"

#include "IECore/VectorTypedData.h"

namespace IECoreGL
{

/// \todo Camera facing seems to fail in orthographic mode.
class PointsPrimitive : public Primitive
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::PointsPrimitive, PointsPrimitiveTypeId, Primitive );

		/// \todo We should consider removing the Point enum value and always using the PointsPrimitiveUseGLPoints
		/// attribute instead.
		enum Type {
			Point,	// GL_POINTS
			Disk,	// camera facing disks
			Quad,	// camera facing quads
			Sphere,	// spheres
		};

		/// Other than points, each argument can have size() equal to the size of
		/// points, or a size of 1.
		/// widths are used by all types.
		/// heights and rotations are used only by the Quad type.
		/// Copies of all data are taken.
		PointsPrimitive( Type type,
			IECore::ConstV3fVectorDataPtr points,
			IECore::ConstColor3fVectorDataPtr colors = 0,
			IECore::ConstFloatVectorDataPtr alphas = 0,
			IECore::ConstFloatVectorDataPtr widths = 0,
			IECore::ConstFloatVectorDataPtr heights = 0,
			IECore::ConstFloatVectorDataPtr rotations = 0 );
			
		virtual ~PointsPrimitive();

		virtual Imath::Box3f bound() const;

		virtual size_t vertexAttributeSize() const;
		
	protected :
		
		virtual void render( ConstStatePtr state, IECore::TypeId style ) const;
		
	private :
	
		const Imath::Color3f *setOrReturnColor() const;
	
		template<typename T>
		static const T *dataAndStride( typename IECore::TypedData<std::vector<T> >::ConstPtr data, T *defaultValue, unsigned int &stride );
	
		void renderPoints( ConstStatePtr state, IECore::TypeId style ) const;
		void renderDisks( ConstStatePtr state, IECore::TypeId style ) const;
		void renderQuads( ConstStatePtr state, IECore::TypeId style ) const;
		void renderSpheres( ConstStatePtr state, IECore::TypeId style ) const;
	
		IECore::V3fVectorDataPtr m_points;
		IECore::Color3fVectorDataPtr m_colors;
		IECore::FloatVectorDataPtr m_alphas;
		
		Type m_type;
		static float g_defaultWidth;
		IECore::FloatVectorDataPtr m_widths;
		static float g_defaultHeight;
		IECore::FloatVectorDataPtr m_heights;
		static float g_defaultRotation;
		IECore::FloatVectorDataPtr m_rotations;
		
		Imath::Box3f m_bound;
		
		void depthSort() const;
		mutable bool m_renderSorted;
		mutable std::vector<unsigned int> m_depthOrder;
		mutable std::vector<float> m_depths;
		mutable Imath::V3f m_depthCameraDirection;

};

IE_CORE_DECLAREPTR( PointsPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_POINTSPRIMITIVE_H
