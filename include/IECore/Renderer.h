//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_RENDERER_H
#define IE_CORE_RENDERER_H

#include "IECore/Export.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CompoundData.h"
#include "IECore/CubicBasis.h"
#include "IECore/MurmurHash.h"

#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathBox.h"

#include <set>

namespace IECore
{

IE_CORE_FORWARDDECLARE( Renderer );

/// The Renderer class provides a means of describing scenes for rendering. Its
/// interface is modelled closely on OpenGL/Renderman with an attribute and
/// transform stack etc. Renderer implementations should flag errors and warnings
/// using the MessageHandler class rather than by throwing Exceptions - it's often
/// more useful to have an incomplete image for diagnosis of the problem than to
/// have an Exception thrown.
///
/// \par Naming conventions
///
/// Many of the calls in the Renderer interface associate a name with a piece of
/// data. Both the setOption() and setAttribute() calls take a name to specify what
/// is being modified and a DataPtr to specify the new value. Many other calls
/// accept either a CompoundDataMap or a PrimitiveVariableMap, both of which may
/// contain many named pieces of Data.
///
/// A naming convention exists to specify that particular data is intended only
/// for a particular Renderer implementation. This allows rendering to be
/// customised for a particular implementation without causing other implementations
/// to error due to unsupported features. The convention for each name is as
/// follows :
///
///	\li <b>"name"</b><br>
/// Should be supported by all Renderer implementations. For instance, the "doubleSided"
/// attribute should be supported by all Renderers. A warning message should be
/// output if the name is not recognised and supported.
///
/// \li <b>"prefix:name"</b><br>
/// Used to specify data intended only for a particular implementation. Implementations
/// silently ignore all data destined for other implementations. For instance, the
/// "gl:primitive:wireframe" attribute is used by the GL renderer implementation but
/// silently ignored by other implementations.
///
/// \li <b>"user:name"</b><br>
/// Used to specify data for the purposes of users. The renderer should store the value
/// and make it available for query, but otherwise it should have no effect. This applies
/// mostly to the attribute and option calls.
/// \ingroup renderingGroup
class IECORE_API Renderer : public RunTimeTyped
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( Renderer, RunTimeTyped );

		Renderer();
		virtual ~Renderer();

		//! @name Options
		/// Options are named items of data which control some global aspect of the
		/// render. These functions allow their setting and getting. All options must
		/// be set before a call to worldBegin() - it is invalid to change an option
		/// after worldBegin().
		///
		/// \par Standard SearchPath Options
		/// <br>
		/// \li <b>"searchPath:font" StringData</b><br>
		/// A colon separated list of paths to search for fonts on - these are used
		/// by the text() primitive. The default value should come from the
		/// IECORE_FONT_PATHS environment variable if set.
		///
		/// \par Rerendering Options
		/// <br>
		/// \li <b>"editable" BoolData</b><br>
		/// This option must be set to enable the use of the interactive rerendering
		/// methods defined below.
		///////////////////////////////////////////////////////////////////////////
		//@{
		/// Set an option. Must not be called after worldBegin().
		virtual void setOption( const std::string &name, ConstDataPtr value ) = 0;
		/// Get the value of a previously set option.
		virtual ConstDataPtr getOption( const std::string &name ) const = 0;
		//@}

		/// Creates a named camera at the position defined by the current transform. The camera
		/// looks down negative Z, with screen space left to right being positive X and
		/// screen space top to bottom being positive Y. The last camera specified before
		/// worldBegin() is considered to be the camera for rendering through - other cameras
		/// may be used in implementation specific ways by Renderer subclasses.
		///
		/// \par Standard Parameters
		/// <br>
		/// The following standard parameters should be supported by all implementations
		/// wherever possible - they are largely based on the RenderMan specification :
		///
		///	\li <b>"resolution"	V2iData</b><br>
		/// The resolution of any output images. Should default to 640x480 if not specified.
		///
		/// \li <b>"pixelAspectRatio" FloatData</b><br>
		/// The xSize/ySize aspect ratio for a pixel.
		///
		/// \li <b>"screenWindow" Box2fData</b><br>
		/// The region in screen space which is mapped to the output resolution. If unspecified
		/// then this should default to -1,1 in the smallest image dimension and the other
		/// dimension should be sized appropriately to preserve pixel aspect ratio.
		///
		/// \li <b>"cropWindow" Box2fData</b><br>
		/// The region in raster space which should actually be rendered - this allows just
		/// a section of the full resolution to be rendered. Note that raster space runs from
		/// 0,0 at the top left to 1,1 at the bottom right. Defaults to 0,0 1,1 if not specified.
		///
		/// \li <b>"projection" StringData</b><br>
		/// The projection that determines how camera coordinates are converted to screen space
		/// coordinates. Implementations should support "perspective" and "orthographic", with
		/// orthographic being the default if not specified.
		///
		/// \li <b>"projection:fov" FloatData</b><br>
		/// In the case of the "projection" parameter specifying a perspective projection, this
		/// specifies the field of view (in degrees) which is visible between -1 and 1 in screen
		/// space. Defaults to 90 degrees if unspecified.
		///
		/// \li <b>"clippingPlanes" V2fData</b><br>
		/// The near and far clipping planes. Defaults to 0.01, 100000 if unspecified.
		///
		/// \li <b>"shutter" V2fData</b><br>
		/// The time interval for which the shutter is open - this is used in conjunction with the
		/// times passed to motionBegin() to specify motion blur. Defaults to 0,0 if unspecified.
		virtual void camera( const std::string &name, const CompoundDataMap &parameters ) = 0;

		/// Specifies an image to be output from the renderer. In the case of file outputs name
		/// specified the filename. type specifies the type of output to create and data specifies
		/// the data to be output, for instance "rgba". parameters provides an implementation specific
		/// set of parameters to control other aspects of the image created. It is only valid to call this
		/// before worldBegin.
		virtual void display( const std::string &name, const std::string &type, const std::string &data, const CompoundDataMap &parameters ) = 0;

		//! @name World block
		/// Once all options, cameras and displays are specified, a world block
		/// is created in which the visible elements of the scene are described.
		///////////////////////////////////////////////////////////////////////////
		//@{
		/// Starts the world block and resets the current transform to the
		/// identity.
		virtual void worldBegin() = 0;
		/// Finishes the world block.
		virtual void worldEnd() = 0;
		//@}

		//! @name Transforms
		/// The Renderer manages a hierarchical set of transforms, applying the
		/// current transform to all cameras and primitives that are created.
		///////////////////////////////////////////////////////////////////////////
		//@{
		/// Push a new transform state identical to the current one. Modifications
		/// can then be made locally before calling transformEnd() to return to the
		/// previous transform state.
		virtual void transformBegin() = 0;
		/// Returns to the transform state saved by the last transformBegin() call.
		virtual void transformEnd() = 0;
		/// Sets the current transform.
		virtual void setTransform( const Imath::M44f &m ) = 0;
		/// Sets the current transform to a coordinate system previously
		/// created with a call to coordinateSystem().
		virtual void setTransform( const std::string &coordinateSystem ) = 0;
		/// Returns the current transform.
		virtual Imath::M44f getTransform() const = 0;
		/// Returns the transform associated with the named coordinate system.
		virtual Imath::M44f getTransform( const std::string &coordinateSystem ) const = 0;
		/// Concatenates the specified transform onto the current transform.
		virtual void concatTransform( const Imath::M44f &m ) = 0;
		/// Creates a named coordinate system from the current transform. Coordinate systems
		/// are scoped by attributeBegin/attributeEnd blocks.
		virtual void coordinateSystem( const std::string &name ) = 0;
		//@}

		//! @name Attributes
		/// Attributes are named items of data which control some per-object aspect
		/// of the render. Attributes may be set both before and after worldBegin(),
		/// and are scoped between attributeBegin(), attributeEnd() calls.
		////////////////////////////////////////////////////////////////////////////
		//@{
		/// Push a new attribute state onto the attribute stack. This is identical
		/// to the current state, but subsequent calls to setAttribute() will be
		/// discarded by the matching attributeEnd().
		virtual void attributeBegin() = 0;
		/// Return to the attribute state saved by the last call to attributeBegin().
		virtual void attributeEnd() = 0;
		/// Sets the named attribute to the specified value.
		///
		/// \par Standard Attributes
		/// <br>
		/// The following standard attributes should be supported by all implementations
		/// wherever possible :
		///
		///	\li <b>"color"	Color3fData</b><br>
		/// The color of primitives. Defaults to 1, 1, 1 if not specified.
		///
		///	\li <b>"opacity" Color3fData</b><br>
		/// The opacity of primitives. Defaults to 1, 1, 1 if not specified.
		///
		/// \li <b>"doubleSided" BoolData</b><br>
		/// When true both sides of a primitive are rendered, when false
		/// only one side is. Defaults to true.
		///
		/// \li <b>"rightHandedOrientation" BoolData</b><br>
		/// Controls which side of a primitive
		/// is forward facing. This attribute starts with a value of true,
		/// because the camera is specified in a right handed coordinate system -
		/// a value of false specifies that the current orientation is left handed.
		/// The renderer will automatically toggle the value whenever a transformation
		/// flips the sense of the current object space coordinate system (this
		/// happens when a transform has a negative determinant - typically a
		/// negative scaling in some axis). You can change the interpretation of
		/// "forward facing" by flipping the attribute value at any time - see
		/// the documentation for the various primitives below for how orientation
		/// affects the way their front face is defined.
		///
		/// \li <b>"name" StringData "unnamed"\li </b>
		/// A descriptive name for the object.
		///
		/// \li <b>"user:*"</b><br>
		/// Should be stored for later query, but have no other effect upon the
		/// rendering process.
		virtual void setAttribute( const std::string &name, ConstDataPtr value ) = 0;
		/// Return the value of the named attribute.
		virtual ConstDataPtr getAttribute( const std::string &name ) const = 0;
		/// Specifies a shader to be applied to subsequent primitives.
		virtual void shader( const std::string &type, const std::string &name, const CompoundDataMap &parameters ) = 0;
		/// Specifies a light to apply to subsequent primitives.
		virtual void light( const std::string &name, const std::string &handle, const CompoundDataMap &parameters ) = 0;
		/// Turns the specified light on or off for the current attribute state.
		virtual void illuminate( const std::string &lightHandle, bool on ) = 0;
		//@}

		//! @name Motion blur
		////////////////////////////////////////////////////////////////////////////
		//@{
		/// Starts a new motion block. You should then make times.size() calls
		/// to one of the primitive or transform functions to specify the motion
		/// for the block.
		virtual void motionBegin( const std::set<float> &times ) = 0;
		/// Ends a motion block. Should be called when times.size() calls to an
		/// appropriate primitive or transform function have been made following
		/// a motionBegin() call.
		virtual void motionEnd() = 0;
		//@}

		//! @name Primitives
		/// Primitives specify actual visible objects to be rendered. Each primitive
		/// has a topology which is usually specified in a manner unique to that type,
		/// and a set of PrimitiveVariables which is specified in the same manner
		/// for all primitives - these specify data to vary over the surface of the
		/// primitive.
		/// \todo Better documentation for the calls below, particularly in relation
		/// to the rightHandedOrientation attribute.
		////////////////////////////////////////////////////////////////////////////
		//@{
		/// Renders a set of points.
		virtual void points( size_t numPoints, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders a disk of the specified radius on the xy plane, at the specified z value.
		/// If the "rightHandedOrientation" attribute is true then the normal faces down
		/// positive z, otherwise it faces down negative z.
		virtual void disk( float radius, float z, float thetaMax, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders a set of curves.
		virtual void curves( const CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars ) = 0;
		/// Renders some text.
		virtual void text( const std::string &font, const std::string &text, float kerning = 1.0f, const PrimitiveVariableMap &primVars=PrimitiveVariableMap() ) = 0;
		/// Renders a sphere of the specified radius. zMin and zMax are measured as a proportion of the radius - so no matter
		/// what the radius, the default values will always give a full sphere. If the "rightHandedOrientation"
		/// attribute is true then the normals point outwards, otherwise they point inwards.
		/// \todo Give this the default values it talks about.
		virtual void sphere( float radius, float zMin, float zMax, float thetaMax, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders an image.
		/// \todo Clarify the intended use of dataWindow and displayWindow.
		virtual void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders a mesh. The geometric normal of a face will be facing camera if the winding order of its vertices is anticlockwise from
		/// the point of view of the camera and the "rightHandedOrientation" attribute is true. If the "rightHandedOrientation" attribute
		/// is false then faces whose vertices wind /clockwise/ with respect to the camera are considered forward facing instead.
		virtual void mesh( ConstIntVectorDataPtr vertsPerFace, ConstIntVectorDataPtr vertIds, const std::string &interpolation, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders a nurbs surface.
		virtual void nurbs( int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const PrimitiveVariableMap &primVars ) = 0;
		/// Render a patch mesh.
		virtual void patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars ) = 0;
		/// Generic call for specifying renderer specify geometry types.
		virtual void geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars ) = 0;
		//@}

		/// The Procedural class defines an interface via which the Renderer can
		/// ask for geometry to be generated in a deferred fashion, at a time
		/// which is convenient to it.
		class IECORE_API Procedural : public RefCounted
		{
			public :

				IE_CORE_DECLAREMEMBERPTR( Procedural );

				Procedural();
				virtual ~Procedural();

				/// Returns a bounding box guaranteed to completely
				/// contain the geometry generated by the render()
				/// method.
				virtual Imath::Box3f bound() const = 0;
				/// Called when the renderer is ready to receive the procedural
				/// geometry. Any relevant methods of renderer may be called, but
				/// the geometry generated must be contained within the
				/// box returned by bound().
				virtual void render( Renderer *renderer ) const = 0;
				/// Implement this to return a hash for procedural level instancing.
				/// Procedurals with the same hash will be reused by renderers that
				/// support this feature. If computing a sensible hash is difficult
				/// or impossible, return IECore::MurmurHash() from this function
				/// and this feature will be disabled.
				virtual MurmurHash hash() const = 0;

		};
		IE_CORE_DECLAREPTR( Procedural );

		/// A placeholder for specifying a procedural which the Renderer
		/// must load from a file on disk.
		class IECORE_API ExternalProcedural : public Procedural
		{
			public :

				IE_CORE_DECLAREMEMBERPTR( ExternalProcedural )

				ExternalProcedural( const std::string &fileName, const Imath::Box3f &bound, const CompoundDataMap &parameters );
				virtual ~ExternalProcedural();

				const std::string &fileName() const;
				const CompoundDataMap &parameters() const;

				virtual Imath::Box3f bound() const;
				virtual void render( Renderer *renderer ) const;
				virtual MurmurHash hash() const;

			private :

				std::string m_fileName;
				Imath::Box3f m_bound;
				CompoundDataMap m_parameters;

		};
		IE_CORE_DECLAREPTR( ExternalProcedural );

		/// Renders a piece of procedural geometry.
		virtual void procedural( ProceduralPtr proc ) = 0;

		//! @name Instancing
		/// These methods provide a means of describing a portion of a scene once and reusing
		/// it many times.
		///////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Starts the description of a portion of a scene to be instanced.
		virtual void instanceBegin( const std::string &name, const CompoundDataMap &parameters ) = 0;
		/// Ends the description of an instance.
		virtual void instanceEnd() = 0;
		/// Instantiates a previously described instance at the current transform position, and
		/// using the current attribute state.
		virtual void instance( const std::string &name ) = 0;
		//@}

		/// Generic call for executing arbitrary renderer commands. This is intended to allow
		/// derived classes to support calls such as RiMakeTexture via calls of the form
		/// renderer->command( "ri:makeTexture", ... ).
		///
		/// Renderers supporting arbitrary clipping planes should implement a "clippingPlane"
		/// command which must be issued before worldBegin(), and which makes a clipping plane
		/// using the current transformation.
		///
		/// \todo Make a dedicated virtual clippingPlane() method for the next major version.
		virtual DataPtr command( const std::string &name, const CompoundDataMap &parameters ) = 0;

		//! @name Interactive rerendering
		/// These methods provide a means of upating the scene following a call to worldEnd(),
		/// causing the renderer to interactively rerender the scene.
		///////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Starts a new scene edit of the specified type. The standard functions above for
		/// declaring the scene can then be used to perform the edit.
		/// \todo Better define the semantics of this function as we implement it for different
		/// renderers.
		virtual void editBegin( const std::string &editType, const CompoundDataMap &parameters ) = 0;
		/// Ends the current scene edit.
		virtual void editEnd() = 0;
		//@}
		
};

}

#endif // IE_CORE_RENDERER_H
