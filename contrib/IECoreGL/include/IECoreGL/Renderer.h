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

#ifndef IECOREGL_RENDERER_H
#define IECOREGL_RENDERER_H

#include "IECoreGL/TypeIds.h"

#include "IECore/Renderer.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Scene );

class Renderer : public IECore::Renderer
{
	public :

		Renderer();
		virtual ~Renderer();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Renderer, RendererTypeId, IECore::Renderer );

		/// \par Standard options supported :
		///
		/// \li <b>"searchPath:font"</b>
		///
		/// \par Implementation specific options supported :
		///
		/// "gl:mode" StringData 
		/// Valid values are "immediate" or "deferred". In immediate mode rendering is
		/// performed in a streaming fashion, drawing each primitive as it is
		/// specified. In deferred mode rendering is performed by building a Scene
		/// object which can be retrieved for further use by using the scene() method
		/// after the render is complete. Future versions may support advanced features
		/// such as motion blur and depth of field when in deferred mode but not in immediate mode.
		///
		/// "searchPath:shader" StringData
		///	"gl:searchPath:shader" StringData
		/// Specifies a set of colon separated paths on which to search for GLSL shaders. The default
		/// value is obtained from the environment variable IECOREGL_SHADER_PATHS.
		///
		/// "searchPath:shaderInclude" StringData
		///	"gl:searchPath:shaderInclude" StringData
		/// Specifies a set of colon separated paths on which to search for include files for GLSL shaders. The default
		/// value is obtained from the environment variable IECOREGL_SHADER_INCLUDE_PATHS.
		///
		/// "searchPath:texture" StringData
		/// "gl:searchPath:texture" StringData
		/// Specifies a set of colon separated paths on which to search for textures. The default
		/// value is obtained from the environment variable IECOREGL_TEXTURE_PATHS. Any image file format
		/// for which an IECore::Reader is available is suitable for use as a texture.
		///
		/// "shutter" V2fData
		virtual void setOption( const std::string &name, IECore::ConstDataPtr value );
		virtual IECore::ConstDataPtr getOption( const std::string &name ) const;
		/// \par Standard parameters supported :
		///
		/// \li <b>"resolution"</b>
		///	\li <b>"projection"</b> (orthographic and perspective)
		/// \li <b>"projection:fov"</b>
		/// \li <b>"resolution"</b>
		/// \li <b>"screenWindow"</b>
		/// \li <b>"clippingPlanes"</b>
		virtual void camera( const std::string &name, const IECore::CompoundDataMap &parameters );
		virtual void display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters );

		virtual void worldBegin();
		virtual void worldEnd();
		/// When in deferred mode (see setOption above), this method will return the Scene that
		/// was generated.
		ScenePtr scene();

		virtual void transformBegin();
		virtual void transformEnd();
		virtual void setTransform( const Imath::M44f &m );
		virtual void setTransform( const std::string &coordinateSystem );
		virtual Imath::M44f getTransform() const;
		virtual Imath::M44f getTransform( const std::string &coordinateSystem ) const;
		virtual void concatTransform( const Imath::M44f &m );
		virtual void coordinateSystem( const std::string &name );

		virtual void attributeBegin();
		virtual void attributeEnd();
		/// \par Standard attributes supported :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"color"</b><br>
		/// Sets the rgb components of the current OpenGL
		/// color.
		/// \li <b>"opacity"</b><br>
		/// Sets the alpha component of the current OpenGL
		/// color to the average of the rgb components of
		/// opacity.
		///
		/// \li <b>"doubleSided"</b>
		/// \li <b>"rightHandedOrientation"</b>
		/// \li <b>"name"</b>
		///
		/// \par Implementation specific attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:color"	Color4fData	Color4f( 1.0f )</b>
		/// Sets the current OpenGL color including the
		/// alpha component.
		///
		/// \par Implementation specific shading attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:shade:transparent" BoolData false</b><br>
		/// Signifies that shading may result in
		/// transparent results. This is necessary as
		/// it's nontrivial to determine this
		/// information by querying the shader, and
		/// other parts of the system may need to
		/// know when transparency is present - for
		/// example to trigger depth sorting.
		///
		/// \par Implementation specific primitive style attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:primitive:wireframe" BoolData false</b><br>
		/// Draw a wireframe for each primitive.
		///
		/// \li <b>"gl:primitive:wireframeWidth" FloatData 1.0f</b><br>
		/// The line width of the wireframe of the primitive.
		///
		/// \li <b>"gl:primitive:wireframeColor" Color4fData</b><br>
		/// The color of the wireframes drawn.
		///
		/// \li <b>"gl:primitive:bound" BoolData false</b><br>
		/// Draw a bounding box for each primitive.
		///
		/// \li <b>"gl:primitive:boundColor" Color4fData </b><br>
		/// The color of the bounding boxes drawn.
		///
		/// \li <b>"gl:primitive:filled" BoolData true</b><br>
		/// Draw each primitive filled.
		///
		/// \li <b>"gl:primitive:outline" BoolData false</b><br>
		/// Draw an outline for each primitive. Note that the results
		/// of having this on with filled mode off will probably be
		/// undesirable.
		///
		/// \li <b>"gl:primitive:outlineColor" Color4fData</b><br>
		/// The color of the outlines drawn.
		///
		/// \li <b>"gl:primitive:outlineWidth" FloatData 1.0f</b><br>
		/// The line width of the outlines of the primitive.
		///
		/// \li <b>"gl:primitive:points" BoolData false</b><br>
		/// Draw the vertices of each primitive as points.
		///
		/// \li <b>"gl:primitive:pointWidth" FloatData 1.0f</b><br>
		/// The width of the points used to draw vertices.
		///
		/// \li <b>"gl:primitive:pointColor" Color4fData</b><br>
		/// The color of the points drawn.
		///
		/// \li <b>"gl:primitive:sortForTransparency" BoolData true</b><br>
		/// Causes the individual components of a 
		/// primitive to be sorted in depth when the "gl:shade:transparent"
		/// attribute is true.
		/// This is currently supported only by the
		/// points primitive.
		///
		/// \par Implementation specific points primitive attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:pointsPrimitive:useGLPoints" StringData "forGLPoints"</b><br>
		/// Can be used to force the use of lightweight glPoints
		/// representation of points primitives with types other than "gl:point".
		/// Valid values are :<br><br>
		///	"forGLPoints"<br>
		/// Use lightweight points only if type is "gl:point"<br><br>
		/// "forParticlesAndDisks"<br>
		/// Use lightweight points as a stand in for particle and disk types.<br><br>
		/// "forAll"<br>
		/// Use lightweight points as a stand in for all point types. <br><br>
		///
		/// \li <b>"gl:pointsPrimitive:glPointWidth" FloatData 1.0f</b><br>
		/// The size of the points (in pixels) used when rendering lightweight
		/// points.
		///
		/// \par Implementation specific curves primitive attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:curvesPrimitive:useGLLines" BoolData false</b><br>
		/// When this is true then lightweight OpenGL line primitives are used
		/// for representing curves.
		///
		/// \li <b>"gl:curvesPrimitive:glLineWidth" FloatData 1.0f</b><br>
		/// Specifies the line width (in pixels) which is used when
		/// rendering lightweight line primitives.
		///
		/// \li <b>"gl:pointsPrimitive:ignoreBasis" BoolData false</b><br>
		/// When this is true, all curves are rendered as if they were linear.
		///
		/// \par Implementation specific blending attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:blend:srcFactor" StringData "srcAlpha"</b><br>
		///	\li <b>"gl:blend:dstFactor" StringData "oneMinusSrcAlpha"</b><br>
		/// These attributes are mapped onto calls to glBlendFunc.
		/// They accept only the values below, which
		/// correspond directly to one of the GLenum blending
		/// values.<br><br>
		///	"zero"<br>
		/// "one"<br>
		/// "srcColor"<br>
		/// "oneMinusSrcColor"<br>
		/// "dstColor"<br>
		/// "oneMinusDstColor"<br>
		/// "srcAlpha"<br>
		/// "oneMinusSrcAlpha"<br>
		/// "dstAlpha"<br>
		/// "oneMinusDstAlpha"<br>
		/// "constantColor"<br>
		/// "oneMinusConstantColor"<br>
		/// "constantAlpha"<br>
		/// "oneMinusConstantAlpha"<br>
		///
		/// \li <b>"gl:blend:color" Color4fData 1 1 1 1</b><br>
		/// Used to specify glBlendColor(), only taking effect when using
		/// either "constantColor" or "oneMinusConstantColor" for one or
		/// other of the blend factors above.
		///
		/// \li <b>"gl:blend:equation" StringData "add"</b><br>
		/// Controls how the src and dst values are combined after being weighted
		/// by srcFactor and dstFactor - this maps onto a call to glBlendEquation.
		/// Valid values are listed below, and map directly onto the corresponding
		/// GLenum values.<br><br>
		///	"add"<br>
		/// "subtract"<br>
		/// "reverseSubtract"<br>
		/// "min"<br>
		/// "max"<br>
		virtual void setAttribute( const std::string &name, IECore::ConstDataPtr value );
		virtual IECore::ConstDataPtr getAttribute( const std::string &name ) const;
		/// Supports only shader type "surface" or "gl:surface", looking for "name.vert" and  "name.frag" GLSL source files
		/// in the paths defined by the "searchPath:shader" option. Alternatively if the parameter list contains 
		/// "gl:vertexSource" and/or a "gl:fragmentSource" StringData then a new shader is created using the source provided.
		/// For shaders with sampler2D parameters, texture files for these parameters may be specified by passing the filename
		/// to an image as StringData.
		virtual void shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters );
		virtual void light( const std::string &name, const IECore::CompoundDataMap &parameters );

		virtual void motionBegin( const std::set<float> times );
		virtual void motionEnd();

		/// Supports the following primitive variables :
		///
		///	Vertex V3fVectorData "P"
		///
		/// Uniform StringData "type"
		/// Used to determine how the points are rendered. Supported types are :
		///
		///		"gl:point"
		///		Rendered as GL_POINTS
		///
		///		"particle" (the default)
		///		"disk"
		///		"blobby"
		///		Rendered as camera facing disks. The "width" and "constantwidth" variables are supported.
		///		Blobby is provided for vague compatibility with the IECoreRI::RIRenderer and 3delight.
		///
		///		"patch"
		///		Rendered as camera facing patches. Supports the "width" and "constantwidth" variables
		///		and in addition the "patchaspectratio" and "patchrotation" variables. See the 3delight
		///		documentation for a description of how these can be used.
		///
		///		"sphere"
		///		Rendered as spheres. Supports "width" and "constantwidth" variables to define the
		///		sizes of the spheres.
		///
		///	Constant FloatData "constantwidth"
		/// 
		/// Vertex|Varying FloatVectorData "width"
		///
		/// Constant|Vertex|Varying FloatData|FloatVectorData "patchaspectratio"
		/// Constant|Vertex|Varying FloatData|FloatVectorData "patchrotation"
		/// These two are used only by the "patch" type.
		virtual void points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars );
		virtual void disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars );
		/// Supports the following primitive variables :
		///
		/// Vertex V3fVectorData "P"
		/// Constant FloatData "width"
		virtual void curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars );
		virtual void text( const std::string &font, const std::string &text, float kerning = 1.0f, const IECore::PrimitiveVariableMap &primVars=IECore::PrimitiveVariableMap() );
		virtual void sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars );
		/// Supports the following image formats specified as primitive variables :
		///
		/// 	"R", "G", "B", "A"	:	UCharVectorData
		/// 	"R", "G", "B", "A"	:	CharVectorData
		/// 	"R", "G", "B", "A"	:	UIntVectorData
		/// 	"R", "G", "B", "A"	:	IntVectorData
		/// 	"R", "G", "B", "A"	:	HalfVectorData
		/// 	"R", "G", "B", "A"	:	FloatVectorData
		/// 	"R", "G", "B", "A"	:	DoubleVectorData
		///
		/// As a convenience the names "r", "g", "b", "a" or "red", "green", "blue", "alpha" can
		/// appear in place of "R", "G", "B", "A".
		///
		/// Currently assumes dataWindow==displayWindow.
		virtual void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars );
		/// All meshes are treated as having interpolation=="linear".
		/// \todo Support normals and st.
		virtual void mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars );
		virtual void nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars );
		/// Supports the following geometry types with the specified topology data :
		///
		/// "sphere"
		/// "radius"	FloatData	1
		/// "zMin"		FloatData	-1
		/// "zMax"		FloatData	1
		///	"thetaMax"	FloatData	360
		///
		/// \deprecated Use the sphere() method instead.
		virtual void geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars );
		virtual void procedural( IECore::Renderer::ProceduralPtr proc );

		virtual void instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters );
		virtual void instanceEnd();
		virtual void instance( const std::string &name );

		virtual IECore::DataPtr command( const std::string &name, const IECore::CompoundDataMap &parameters );

		struct MemberData;
		
	private :
	
		MemberData *m_data;
		
};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreGL

#endif // IECOREGL_RENDERER_H
