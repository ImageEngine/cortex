//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

#include "IECoreGL/Export.h"
#include "IECoreGL/TypeIds.h"

#include "IECoreScene/Renderer.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Scene );
IE_CORE_FORWARDDECLARE( TextureLoader );
IE_CORE_FORWARDDECLARE( ShaderLoader );

/// \addtogroup environmentGroup
///
/// <b>IECOREGL_SHADER_PATHS</b><br>
/// <b>IECOREGL_SHADER_INCLUDE_PATHS</b><br>
/// <b>IECOREGL_TEXTURE_PATHS</b><br>
/// <b>IECORE_FONT_PATHS</b><br>
/// Used to specify default resource locations for the IECoreGL::Renderer.

/// The IECoreGL::Renderer class implements the IECore::Renderer interface to
/// allow rendering to OpenGL. Immediate mode rendering allows the generation of
/// images and deferred mode rendering allows scenes to be captured for later interactive
/// display.
/// \ingroup renderingGroup
class IECOREGL_API Renderer : public IECoreScene::Renderer
{
	public :

		Renderer();
		~Renderer() override;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Renderer, RendererTypeId, IECoreScene::Renderer );

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
		///
		/// "gl:drawCoordinateSystems" BoolData false
		/// When this is true, coordinate systems created with the coordinateSystem() method
		/// will be visualised.
		void setOption( const std::string &name, IECore::ConstDataPtr value ) override;
		IECore::ConstDataPtr getOption( const std::string &name ) const override;
		/// \par Standard parameters supported :
		///
		/// \li <b>"resolution"</b>
		///	\li <b>"projection"</b> (orthographic and perspective)
		/// \li <b>"projection:fov"</b>
		/// \li <b>"resolution"</b>
		/// \li <b>"screenWindow"</b>
		/// \li <b>"clippingPlanes"</b>
		void camera( const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters ) override;

		void worldBegin() override;
		void worldEnd() override;
		/// When in deferred mode (see setOption above), this method will return the Scene that
		/// was generated.
		/// \threading The Renderer tries very hard not to need a GL context to operate in when in deferred mode. This allows it to evaluate multiple
		/// procedurals concurrently in separate threads (GL wants only one threading talking to a context). When the scene is rendered for the first
		/// time it will instantiate various OpenGL resources (textures and shaders and the like) in the current GL context. It is therefore important that
		/// the scene is destroyed from the same thread that renders it, so that the resources are released in the correct context. As the resources are
		/// also shared by caches (TextureLoaders and ShaderLoaders) in the Renderer, it is also important that the renderer is destroyed from this same
		/// thread.
		ScenePtr scene();

		void transformBegin() override;
		void transformEnd() override;
		void setTransform( const Imath::M44f &m ) override;
		void setTransform( const std::string &coordinateSystem ) override;
		Imath::M44f getTransform() const override;
		Imath::M44f getTransform( const std::string &coordinateSystem ) const override;
		void concatTransform( const Imath::M44f &m ) override;
		void coordinateSystem( const std::string &name ) override;

		void attributeBegin() override;
		void attributeEnd() override;
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
		/// \li <b>"user:*"</b>
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
		/// \li <b>"gl:primitive:solid" BoolData true</b><br>
		/// Draw each primitive solid.
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
		/// \li <b>"gl:primitive:selectable" BoolData true</b><br>
		/// Allow the primitive to be selected. ( ie. it will be drawn when we render for selection purposes )
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
		/// \li <b>"gl:curvesPrimitive:ignoreBasis" BoolData false</b><br>
		/// When this is true, all curves are rendered as if they were linear.
		///
		/// \par Implementation specific text primitive attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:textPrimitive:type" StringData "mesh"</b><br>
		/// Controls the basic method used for text rendering. A value
		/// of "mesh" specifies that text primitives are rendered as
		/// triangulated meshes, and a value of "sprite" specifies rendering
		/// as textured planes. The former allows completely customisable shading
		/// using the current shader, whereas the latter is shaded constantly
		/// using the current colour, but may offer better anti-aliasing and/or
		/// speed.
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
		///
		/// \li <b>"gl:alphaTest" BoolData false</b><br>
		/// When this is true, gl alpha testing will be enabled.
		///
		/// \li <b>"gl:alphaTest:value" FloatData 0</b><br>
		/// Alpha test comparison value for glAlphaFunc
		///
		/// \li <b>"gl:alphaTest:mode" StringData "always"</b><br>
		/// Alpha test comparison mode for glAlphaFunc.
		/// Valid values are listed below, and map directly onto the corresponding
		/// GLenum values.<br><br>
		/// "never"<br>
		/// "less"<br>
		/// "equal"<br>
		/// "lequal"<br>
		/// "greater"<br>
		/// "notequal"<br>
		/// "gequal"<br>
		/// "always"<br>
		///
		/// \par Implementation specific antialiasing attributes :
		///////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:smoothing:points" BoolData false</b><br>
		/// \li <b>"gl:smoothing:lines" BoolData false</b><br>
		/// \li <b>"gl:smoothing:polygons" BoolData false</b><br>
		/// These attributes enable GL_POINT_SMOOTH, GL_LINE_SMOOTH and
		/// GL_POLYGON_SMOOTH respectively.
		///
		/// \par Implementation specific procedural attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:procedural:reentrant" BoolData true</b><br>
		/// When true, procedurals may be evaluated in multiple parallel threads.
		/// When false they will be evaluated from the same thread they were specified
		/// from.
		/// \par Implementation specific depth buffer attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:depthTest" BoolData true</b><br>
		/// Maps to glEnable/Disable GL_DEPTH_TEST
		///
		/// \li <b>"gl:depthMask" BoolData true</b><br>
		/// Maps to glDepthMask GL_TRUE/GL_FALSE
		///
		/// \par Implementation specific visibility attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"gl:visibility:camera" BoolData true</b><br>
		/// Specifies whether or not objects are visible to the camera.
		/// If a procedural is not visible then it will not be opened
		/// to discover if it's contents might turn visibility back on.
		///
		/// \par Instancing attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"automaticInstancing" BoolData true</b><br>
		/// \li <b>"gl:automaticInstancing" BoolData true</b><br>
		/// Specifies that instances are automatically created if
		/// identical primitives are passed to the renderer
		/// repeatedly. This is currently implemented only for the
		/// mesh, points and curves primitive types.
		////////////////////////////////////////////////////////////
		void setAttribute( const std::string &name, IECore::ConstDataPtr value ) override;
		IECore::ConstDataPtr getAttribute( const std::string &name ) const override;
		/// Supports only shader type "surface" or "gl:surface", looking for "name.vert" and  "name.frag" GLSL source files
		/// in the paths defined by the "searchPath:shader" option. Alternatively if the parameter list contains
		/// "gl:vertexSource" and/or a "gl:fragmentSource" StringData then a new shader is created using the source provided.
		/// For shaders with sampler2D parameters, texture files for these parameters may be specified by passing the filename
		/// to an image as StringData.
		/// \todo Add support for "textureParameterName:filter" and "textureParameterName:wrap" parameters which set wrapping
		/// and filtering on a ShaderStateComponent.
		void shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters ) override;
		void illuminate( const std::string &lightHandle, bool on ) override;

		void motionBegin( const std::set<float> &times ) override;
		void motionEnd() override;

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
		void points( size_t numPoints, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		void disk( float radius, float z, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		/// Supports the following primitive variables :
		///
		/// Vertex V3fVectorData "P"
		/// Constant FloatData "width"
		void curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		void text( const std::string &font, const std::string &text, float kerning = 1.0f, const IECoreScene::PrimitiveVariableMap &primVars=IECoreScene::PrimitiveVariableMap() ) override;
		void sphere( float radius, float zMin, float zMax, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars ) override;
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
		void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		/// All meshes are treated as having interpolation=="linear".
		/// \todo Support normals and st.
		void mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		/// Not implemented
		void nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		/// Not implemented
		void patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		/// Not implemented
		void geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		void procedural( IECoreScene::Renderer::ProceduralPtr proc ) override;

		void instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void instanceEnd() override;
		void instance( const std::string &name ) override;

		/// \par Commands implemented
		///
		/// "removeObject"<br>
		/// Expects a StringData parameter named "name", which specifies the name of an object to remove from the scene.
		/// This only has any effect in deferred mode.
		/// "editBegin"<br>
		/// This parameter-less command marks the start of an edit to an existing scene, and should be called before
		/// any other changes are made when re-using an existing renderer, that has already reached worldEnd.
		/// "editEnd"<br>
		/// This parameter-less command marks the end of an edit to an existing scene, and should be called after
		/// other changes have been made when re-using an existing renderer. Note that if a scene has been drawn
		/// with renderer->scene()->render() prior to this edit, then it is essential that editEnd is called by the
		/// same thread on which drawing was performed, so that GL resources can be released in the appropriate context.
		/// "editQuery"<br>
		/// This parameter-less command returns BoolData( true ) if an edit is in progress, and BoolData( false )
		/// otherwise.
		/// \todo Consider generalising an interface for scene edits and making it a standard part of the documentation
		/// in IECore. Any such interface should take into account support for PRMan's new rerendering API.
		IECore::DataPtr command( const std::string &name, const IECore::CompoundDataMap &parameters ) override;

		/// \todo Implement the existing editing commands in this new form.
		void editBegin( const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void editEnd() override;

		/// Returns the internal ShaderLoader object used to load the shaders for this renderer.
		/// If called before worldBegin it returns 0.
		ShaderLoader *shaderLoader();

		/// Returns the internal TextureLoader object.
		/// If called before worldBegin it returns 0.
		TextureLoader *textureLoader();

		struct MemberData;

	private :

		MemberData *m_data;

};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreGL

#endif // IECOREGL_RENDERER_H
