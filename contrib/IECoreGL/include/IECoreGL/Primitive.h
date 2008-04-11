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

#ifndef IECOREGL_PRIMITIVE_H
#define IECOREGL_PRIMITIVE_H

#include "IECoreGL/Renderable.h"
#include "IECoreGL/GL.h"

#include "IECore/Primitive.h"

#include "OpenEXR/ImathBox.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( Shader );

/// The Primitive class represents geometric objects that can
/// be rendered in OpenGL. Primitives may be rendered in a variety
/// of styles defined by State objects, or just rendered as raw geometry
/// in the current OpenGL state.
class Primitive : public Renderable
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( Primitive, PrimitiveTypeId, Renderable );

		Primitive();
		virtual ~Primitive();
		
		/// Renders the Primitive in the current
		/// OpenGL context. The Primitive will draw itself
		/// using the style represented by state, allowing
		/// representations such as wireframe over shaded etc.
		/// An exception is thrown if state->isComplete() is not
		/// true.
		///
		/// The default implementation for this function calls
		/// the protected render() method several times
		/// in different OpenGL states, once for each style
		/// present in state.
		virtual void render( ConstStatePtr state ) const;
	
		/// Returns the number of expected data values for
		/// vertex attributes. Returns 0 if vertex attributes
		/// are not supported. The default implementation
		/// returns 0.
		virtual size_t vertexAttributeSize() const;
		/// Takes a copy of data. Throws an Exception if this primitive doesn't support
		/// vertex attributes, or if the data supplied is not suitable.
		void addVertexAttribute( const std::string &name, IECore::ConstDataPtr data );
	
	protected :
	
		/// Must be implemented by subclasses. This function is called several
		/// times by the standard render() call, once for each style of rendering
		/// requested in state (wireframe, solid etc). The TypeId of the StateComponent
		/// representing that style is passed so that the drawing can be optimised
		/// for the particular style (e.g. PrimitiveWireframeTypeId is passed for
		/// wireframe rendering).
		virtual void render( ConstStatePtr state, IECore::TypeId style ) const = 0;
		
		/// Can be called from a derived class' render() method to set
		/// varying parameters of the current shader based on the
		/// data from vertex attributes. This must /not/ be called unless the style
		/// parameter passed to render is PrimitiveSolid - in all other cases no shader
		/// is bound and an Exception will result.
		void setVertexAttributes( ConstStatePtr state ) const;
		/// Can be called from a derived class' render() method to
		/// set uniform parameters of the current shader based on a single element of
		/// data from the vertex attributes. This must /not/ be called unless the
		/// style parameter passed to render is PrimitiveSolid - in all other cases
		/// no shader is bound and an Exception will result.
		void setVertexAttributesAsUniforms( unsigned int vertexIndex ) const;
		/// Convenience function for use in render() implementations. Returns
		/// true if TransparentShadingStateComponent is true and
		/// PrimitiveTransparencySortStateComponent is true.
		bool depthSortRequested( ConstStatePtr state ) const;
		
	private :
	
		struct SetVertexAttribute;
		
		struct IntData
		{
			IntData() {};
			IntData( const int *d, unsigned int dim ) : data( d ), dimensions( dim ) {};
			const int *data;
			unsigned int dimensions;
		};
		struct FloatData
		{
			FloatData() {};
			FloatData( const float *d, unsigned int dim ) : data( d ), dimensions( dim ) {};
			const float *data;
			unsigned int dimensions;
		};
		void setupVertexAttributesAsUniform( Shader *s ) const;
		mutable struct {
			Shader *shader;
			std::map<GLint, IntData> intDataMap;
			std::map<GLint, FloatData> floatDataMap;
		} m_vertexToUniform;
	
		typedef std::map<std::string, IECore::ConstDataPtr> VertexAttributeMap;
		VertexAttributeMap m_vertexAttributes;
		
};

IE_CORE_DECLAREPTR( Primitive );

} // namespace IECoreGL

#endif // IECOREGL_PRIMITIVE_H
