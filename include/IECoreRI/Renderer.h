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

		/// \par Standard parameters supported :
		///
		/// \li <b>"resolution"</b>
		/// \li <b>"screenWindow"</b>
		///	\li <b>"cropWindow"</b>
		/// \li <b>"projection"</b>
		/// \li <b>"projection:fov"</b>
		///	\li <b>"clippingPlanes"</b>
		/// \li <b>"shutter"</b>
		///
		/// \par Implementation specific parameters supported :
		///
		/// \li <b>"transform" M44fData()</b><br>
		/// This overrides the transform specified via the transform*() calls below. It's provided
		/// to work around a bug that prevents RxTransform() from working when in RIB output mode.
		/// \deprecated The "transform" parameter should no longer be used as the bug in 3delight
		/// which required it has now been fixed.
		///
		///	\li <b>"projection:*"</b><br>
		/// All parameters matching this naming convention are passed to the RiProjection call.
		///
		/// \li <b>"ri:hider" StringData()</b><br>
		///
		/// \li <b>"ri:hider:*"</b><br>
		/// All parameters matching this naming convention are passed to an RiHider call.
		///
		/// \todo Support moving cameras.
		virtual void camera( const std::string &name, const IECore::CompoundDataMap &parameters );
		virtual void display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters );

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
		/// \par Standard attributes supported :
		////////////////////////////////////////////////////////////////////////////
		///
		/// \li <b>"color"</b><br>
		/// Mapped to an RiColor call.
		///
		/// \li <b>"opacity"</b><br>
		/// Mapped to an RiOpacity call.
		///
		/// \li <b>"doubleSided"</b><br>
		/// Mapped to an RiSides call.
		///
		/// \li <b>"rightHandedOrientation"</b><br>
		/// Implemented via RiOrientation.
		///
		/// \li <b>"name"</b><br>
		/// Mapped to an RiAttribute "identifier" "name" call.
		///
		/// \li <b>"user:*"</b><br>
		///
		/// \par Implementation specific attributes :
		////////////////////////////////////////////////////////////////////////////
		///
		/// \li <b>"ri:*:*"</b><br>
		/// Passed to an RiAttribute call.
		///
		/// \li <b>"ri:shadingRate" FloatData</b><br>
		/// Passed to RiShadingRate.
		///
		/// \li <b>"ri:matte" BoolData</b><br>
		/// Passed to RiMatte.
		///
		/// \li <b>"ri:color" Color3fData</b><br>
		/// \deprecated Use "color" in preference to "ri:color"
		///
		/// \li <b>"ri:opacity" Color3fData</b><br>
		/// \deprecated Use "opacity" in preference to "ri:opacity"
		/// 
		/// \li <b>"ri:sides" IntData</b><br>
		/// Passed to RiSides
		/// \deprecated Use "doubleSided" in preference to "ri:sides"
		///
		/// \li <b>"ri:geometricApproximation:motionFactor" and ri:geometricApproximation:focusFactor" FloatData</b><br>
		///	Passed to RiGeometricApproximation.
		virtual void setAttribute( const std::string &name, IECore::ConstDataPtr value );
		/// \par Currently supported attributes :
		///
		/// \li <b>"doubleSided"</b>
		/// \li <b>"rightHandedOrientation"</b>
		/// \li <b>"name"</b>
		/// \li <b>"user:*"</b>
		/// \li <b>"ri:shadingRate"</b><br>
		/// \li <b>"ri:matte"</b><br>
		/// \li <b>"ri:*:*"</b><br>
		/// Supports all attributes for which the RxAttribute query works.
		virtual IECore::ConstDataPtr getAttribute( const std::string &name ) const;
		virtual void shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters );
		virtual void light( const std::string &name, const IECore::CompoundDataMap &parameters );

		virtual void motionBegin( const std::set<float> times );
		virtual void motionEnd();

		virtual void points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars );
		virtual void disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars );
		
		virtual void curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars );

		virtual Imath::Box3f textExtents(const std::string & t, const float width = Imath::limits<float>::max() );
		virtual void text(const std::string &t, const float width = Imath::limits<float>::max() );
		virtual void sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars );

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
		virtual IECore::DataPtr command( const std::string &name, const IECore::CompoundDataMap &parameters );
		
	private :
	
		IECore::RendererPtr m_implementation;

};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreRI

#endif // IECORERI_RENDERER_H
