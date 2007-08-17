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

#ifndef IE_CORE_RENDERER_H
#define IE_CORE_RENDERER_H

#include "IECore/RunTimeTyped.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CompoundData.h"
#include "IECore/Parameterised.h"

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
/// \todo More primitives.
/// \todo Document some standard options and attributes expected to be implemented
/// by all subclasses. Also document the prefix: naming convention for passing
/// renderer specific options and attributes to subclasses without causing errors
/// in other implementations.
class Renderer : public RunTimeTyped
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
		///////////////////////////////////////////////////////////////////////////
		//@{
		/// Set an option. Must not be called after worldBegin().
		virtual void setOption( const std::string &name, ConstDataPtr value ) = 0;
		/// Get the value of a previously set option.
		virtual ConstDataPtr getOption( const std::string &name ) const = 0;
		//@}

		/// Creates a named camera at the position defined by the current transform.
		/// The last camera specified before worldBegin() is considered to be the
		/// camera for rendering through - other cameras may be used in implementation
		/// specific ways by Renderer subclasses.
		/// \todo The parameters argument should be a const reference.
		virtual void camera( const std::string &name, CompoundDataMap &parameters ) = 0;
		
		/// Specifies an image to be output from the renderer. In the case of file outputs name
		/// specified the filename. type specifies the type of output to create and data specifies
		/// the data to be output, for instance "rgba". parameters provides an implementation specific
		/// set of parameters to control other aspects of the image created. It is only valid to call this
		/// before worldBegin.
		/// \todo The parameters argument should be a const reference.
		virtual void display( const std::string &name, const std::string &type, const std::string &data, CompoundDataMap &parameters ) = 0;

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
		/// Creates a named coordinate system from the current transform.
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
		/// Set the named attribute.
		virtual void setAttribute( const std::string &name, ConstDataPtr value ) = 0;
		/// Return the value of the named attribute.
		virtual ConstDataPtr getAttribute( const std::string &name ) const = 0;
		/// Specifies a shader to be applied to subsequent primitives.
		virtual void shader( const std::string &type, const std::string &name, const CompoundDataMap &parameters ) = 0;
		/// Specifies a light to apply to subsequent primitives.
		virtual void light( const std::string &name, const CompoundDataMap &parameters ) = 0;
		//@}

		//! @name Motion blur
		////////////////////////////////////////////////////////////////////////////
		//@{
		/// Starts a new motion block. You should then make times.size() calls
		/// to one of the primitive or transform functions to specify the motion
		/// for the block.
		virtual void motionBegin( const std::set<float> times ) = 0;
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
		/// \todo Better documentation for the calls below.
		////////////////////////////////////////////////////////////////////////////
		//@{
		/// Renders a set of points.
		virtual void points( size_t numPoints, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders a set of curves.
		virtual void curves( const std::string &interpolation, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars ) = 0;
		/// Returns the extents of the given string (under the current font, text-layout characteristics).
		virtual Imath::Box3f textExtents(const std::string & t, const float width = Imath::limits<float>::max()) = 0;
		/// Renders some text.
		virtual void text(const std::string &t, const float width = Imath::limits<float>::max()) = 0;
		/// Renders an image.
		virtual void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders a mesh.
		virtual void mesh( ConstIntVectorDataPtr vertsPerFace, ConstIntVectorDataPtr vertIds, const std::string &interpolation, const PrimitiveVariableMap &primVars ) = 0;
		/// Renders a nurbs surface.
		virtual void nurbs( int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const PrimitiveVariableMap &primVars ) = 0;
		/// Generic call for specifying renderer specify geometry types.
		virtual void geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars ) = 0;
		//@}
		
		/// The Procedural class defines an interface via which the Renderer can
		/// ask for geometry to be generated in a deferred fashion, at a time
		/// which is convenient to it.
		class Procedural : public Parameterised
		{
			public :
			
				IE_CORE_DECLARERUNTIMETYPED( Procedural, Parameterised );
				
				Procedural( const std::string &name, const std::string &description );
				virtual ~Procedural();
				
				/// Returns a bounding box guaranteed to completely
				/// contain the geometry generated by the render()
				/// method. Implemented to call doBound() - subclasses should
				/// therefore implement that method.
				Imath::Box3f bound() const;
				/// Called when the renderer is ready to receive the procedural
				/// geometry. Any relevant methods of renderer may be called, but
				/// the geometry generated must be contained within the
				/// box returned by bound(). Implemented to call doRender() - subclasses
				/// should therefore implement that method.
				void render( RendererPtr renderer ) const;
				
			protected :
			
				virtual Imath::Box3f doBound( ConstCompoundObjectPtr args ) const = 0;
				virtual void doRender( RendererPtr renderer, ConstCompoundObjectPtr args ) const = 0;
				
		};
		IE_CORE_DECLAREPTR( Procedural );
		
		/// Renders a piece of procedural geometry.
		virtual void procedural( ProceduralPtr proc ) = 0;
		
		/// Generic call for executing arbitrary renderer commands. This is intended to allow
		/// derived classes to support calls such as RiMakeTexture via calls of the form
		/// renderer->command( "ri:makeTexture", ... ).
		/// \todo It would be nice if this had a return value of DataPtr.
		virtual void command( const std::string &name, const CompoundDataMap &parameters ) = 0;

};

}

#endif // IE_CORE_RENDERER_H
