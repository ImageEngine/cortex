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

#ifndef IECORERI_RENDERER_H
#define IECORERI_RENDERER_H

#include "IECoreRI/TypeIds.h"

#include "IECore/Renderer.h"

namespace IECoreRI
{

/// An IECore::Renderer subclass which renders through the renderman interface.
class Renderer : public IECore::Renderer
{

	public :

		/// Creates a Renderer that will always render to the RiContext which
		/// is active at the time a given function is called.
		Renderer();
		/// Creates a Renderer that will create a new RiContext with a call
		/// to RiBegin( name.c_str() ) and will subsequently always render to that
		/// context. If the empty string is passed then RiBegin( 0 ) is called to actually
		/// render the scene rather than create a rib.
		/// \bug Due to an apparent bug in lib3delight, the above is not true. Instead
		/// a new context is made with RiBegin and it is finished with RiEnd in the destructor,
		/// but no context save/restore is made during Renderer calls - therefore it's only safe
		/// to have one of these Renderer objects in existence at any given time.
		/// \todo Fix the context save/restore bug - when doing this check that all appropriate
		/// functions are actually calling contextBegin()/end(). 
		Renderer( const std::string &name );
		
		virtual ~Renderer();
		
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreRI::Renderer, RendererTypeId, IECore::Renderer );

		/// Implementation specific options :
		///
		/// "ri:searchpath:shader" StringData()
		/// A colon separated list of paths to search for shaders on.
		///
		/// "ri:pixelSamples" V2iData()
		/// Passed to an RiPixelSamples call.
		///
		/// "ri:*:*"
		/// Passed to an RiOption call.
		virtual void setOption( const std::string &name, IECore::ConstDataPtr value );
		/// Currently supported options :
		///
		/// "camera:resolution" V2iData
		/// "camera:shutter"
		///	"shutter"	V2fData()
		/// "user:*"
		virtual IECore::ConstDataPtr getOption( const std::string &name ) const;

		/// Currently supported parameters :
		///
		/// "transform"			M44fData()
		/// This overrides the transform specified via the transform*() calls below. It's provided
		/// to work around a bug that prevents RxTransform() from working when in RIB output mode.
		///
		/// "resolution"		V2iData()
		/// Specifies the image resolution.
		///
		/// "screenWindow"		Box2fData()
		///
		///	"cropWindow"		Box2fData()
		///
		///	"clippingPlanes"	V2fData()
		///
		/// "projection"		StringData()
		///
		///	"projection:*"
		/// All parameters matching this naming convention are passed to the RiProjection call.
		///
		/// "hider"				StringData()
		///
		/// "hider:*"
		/// All parameters matching this naming convention are passed to an RiHider call.
		///
		/// "shutter"			V2fData()
		/// \todo Support moving cameras.
		/// \todo Move the definitions of common parameters into the core library.
		virtual void camera( const std::string &name, IECore::CompoundDataMap &parameters );
		virtual void display( const std::string &name, const std::string &type, const std::string &data, IECore::CompoundDataMap &parameters );

		virtual void worldBegin();
		virtual void worldEnd();

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
		/// Currently supported attributes :
		///
		/// "ri:*:*"
		/// Passed to an RiAttribute call.
		///
		/// "user:*"
		/// Passed to an RiAttribute( "user", "*", ... ) call.
		///
		/// "ri:shadingRate" FloatData
		/// Passed to RiShadingRate.
		///
		/// "ri:matte" BoolData
		/// Passed to RiMatte.
		///
		/// "color"
		/// "ri:color" Color3fData
		/// Passed to RiColor
		///
		/// "opacity"
		/// "ri:opacity" Color3fData
		/// Passed to RiOpacity
		/// 
		/// "ri:sides" IntData
		/// Passed to RiSides
		/// \deprecated Use "doubleSided" in preference to "ri:sides"
		///
		/// "doubleSided" BoolData
		/// When true both sides of a primitive are rendered, when false
		/// only one side is. Defaults to true.
		///
		/// "ri:geometricApproximation:motionFactor"
		/// "ri:geometricApproximation:focusFactor"	FloatData
		///	Passed to RiGeometricApproximation.
		virtual void setAttribute( const std::string &name, IECore::ConstDataPtr value );
		/// Currently supported attributes :
		///
		/// "user:*"
		/// "doubleSided"
		virtual IECore::ConstDataPtr getAttribute( const std::string &name ) const;
		virtual void shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters );
		virtual void light( const std::string &name, const IECore::CompoundDataMap &parameters );

		virtual void motionBegin( const std::set<float> times );
		virtual void motionEnd();

		virtual void points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars );
		
		virtual void curves( const std::string &interpolation, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars );

		virtual Imath::Box3f textExtents(const std::string & t, const float width = Imath::limits<float>::max() );
		virtual void text(const std::string &t, const float width = Imath::limits<float>::max() );

		virtual void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars );
		/// Renders interpolation types of "linear" as RiPointsGeneralPolygons and "catmullClark" as RiSubdivisionMesh.
		virtual void mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars );
		
		virtual void nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars );

		virtual void geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars );

		virtual void procedural( IECore::Renderer::ProceduralPtr proc );
		///
		/// Supports the following commands :
		///
		///	"ri:readArchive"
		/// Expects a single StringData parameter called "name", and calls RiReadArchive with it.
		/// \todo Make a VisibleRenderable subclass to encapsulate a call to this.
		///
		/// "objectBegin"
		/// "ri:objectBegin"
		/// Calls RiObjectBegin. Expects a single StringData parameter called "name", which
		/// can be used in a later call to command( "ri:objectInstance" ) to instance the object.
		///
		/// "objectEnd"
		/// "ri:objectEnd"
		/// Calls RiObjectEnd.
		///
		/// "objectInstance"
		/// "ri:objectInstance"
		/// Calls RiObjectInstance. Expects a single StringData parameter called "name", which
		/// refers to a name previously passed to command( "ri:objectBegin" ).
		///
		/// "ri:archiveRecord"
		/// Makes a call to RiArchiveRecord(). Expects StringData parameters called "type" and
		/// "record".
		///
		/// \todo Implement instancing support through specific calls we add to the IECore::Renderer
		/// interface definition.
		virtual void command( const std::string &name, const IECore::CompoundDataMap &parameters );
		
	private :
	
		IECore::RendererPtr m_implementation;

};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreRI

#endif // IECORERI_RENDERER_H
