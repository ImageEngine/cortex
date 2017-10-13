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

#ifndef IECORERI_RENDERER_H
#define IECORERI_RENDERER_H

#include "IECoreRI/Export.h"
#include "IECoreRI/TypeIds.h"

#include "IECore/Renderer.h"

namespace IECoreRI
{

IE_CORE_FORWARDDECLARE( RendererImplementation );

/// An IECore::Renderer subclass which renders through the renderman interface.
/// \threading Scenes should be described to this class from a single thread.
/// However, when rendering live (with a Renderer constructed with Renderer( "" )), procedurals may
/// be expanded concurrently in multiple threads, and in this case each procedural will see a separate
/// Renderer instance to provide thread safety.
/// \ingroup renderingGroup
class IECORERI_API Renderer : public IECore::Renderer
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

		~Renderer() override;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreRI::Renderer, RendererTypeId, IECore::Renderer );

		/// \par Standard options supported :
		///
		/// \li <b>"searchPath:font"</b>
		///
		/// \par Implementation specific options :
		///
		/// \li <b>"ri:searchpath:shader" StringData()</b><br>
		/// A colon separated list of paths to search for shaders on.
		///
		/// \li <b>"ri:pixelSamples" V2iData()</b><br>
		/// Passed to an RiPixelSamples call.
		///
		/// \li <b>"ri:hider" StringData()</b></br>
		/// \li <b>"ri:hider:*" StringData()</b></br>
		/// Passed to an RiHider call.
		///
		/// \li <b>"ri:frame" IntData()</b></br>
		/// Specifies the frame number for RiFrameBegin. If not specified,
		/// then no frame block will be output.
		///
		/// \li <b>"ri:*:*"</b><br>
		/// Passed to an RiOption call.
		void setOption( const std::string &name, IECore::ConstDataPtr value ) override;
		/// Currently supported options :
		///
		/// "camera:resolution" V2iData
		/// "camera:shutter"
		///	"shutter"	V2fData()
		/// "user:*"
		/// "ri:*" Queries of this form use the Rx API and therefore only work
		/// for options supported by that API.
		IECore::ConstDataPtr getOption( const std::string &name ) const override;

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
		///	\li <b>"projection:*"</b><br>
		/// All parameters matching this naming convention are passed to the RiProjection call.
		void camera( const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters ) override;

		void worldBegin() override;
		void worldEnd() override;

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
		/// \par Implementation specific attributes :
		////////////////////////////////////////////////////////////////////////////
		///
		/// \li <b>"ri:*:*" Data</b><br>
		/// Passed to an RiAttribute call.
		///
		/// \li <b>"ri:*" CompoundData</b><br>
		/// Passed to an RiAttribute call. This allows multiple attribute values to be
		/// specified in one setAttribute() call.
		///
		/// \li <b>"ri:shadingRate" FloatData</b><br>
		/// Passed to RiShadingRate.
		///
		/// \li <b>"ri:matte" BoolData</b><br>
		/// Passed to RiMatte.
		///
		/// \li <b>"ri:geometricApproximation:motionFactor" and ri:geometricApproximation:focusFactor" FloatData</b><br>
		///	Passed to RiGeometricApproximation.
		///
		/// \li <b>"ri:subsurface" CompoundData</b><br>
		/// 3delight gets upset if subsurface attributes aren't all specified as a group and in the right order. This
		/// is a problem as we can't specify order in the AttributeState or Group objects (see todo items there). We therefore
		/// support their specification as a single CompoundData, and ensure that they are specified in an appropriate order.
		/// \todo Do the todo items in IECore::Group and IECore::AttributeState and deprecate the ri:subsurface call.
		///
		/// \li <b>"ri:detail" Box3fData</b><br>
		/// Passed to RiDetail.
		///
		/// \li <b>"ri:detailRange" FloatVectorData</b><br>
		/// Passed to RiDetailRange. The FloatVectorData must have length 4.
		/// \todo Consider whether or not we should have a standard LOD mechanism defined in IECore.
		///
		/// \li <b>"ri:textureCoordinates" FloatVectorData( [ s1, t1, s2, t2, s3, t3, s4, t4 ] )</b><br>
		/// Passed to RiTextureCoordinates.
		/// \li <b>"ri:automaticInstancing" BoolData</b><br>
		/// When this is true, all primitives are rendered as instances, and if a previously rendered
		/// primitive is encountered, the instance will simply be reused.
		void setAttribute( const std::string &name, IECore::ConstDataPtr value ) override;
		/// \par Currently supported attributes :
		///
		/// \li <b>"doubleSided"</b>
		/// \li <b>"rightHandedOrientation"</b>
		/// \li <b>"name"</b>
		/// \li <b>"user:*"</b>
		/// \li <b>"ri:shadingRate"</b><br>
		/// \li <b>"ri:matte"</b><br>
		/// \li <b>"ri:textureCoordinates"</b><br>
		/// \li <b>"ri:*:*"</b><br>
		/// Supports all attributes for which the RxAttribute query works.
		IECore::ConstDataPtr getAttribute( const std::string &name ) const override;
		/// If type is "surface" or "ri:surface" then calls RiSurfaceV.
		/// If type is "displacement" or "ri:displacement" then calls RiDisplacementV.
		/// If type is "atmosphere" or "ri:atmosphere" then calls RiDisplacementV.
		/// If type is "interior" or "ri:interior" then calls RiInteriorV.
		/// If type is "exterior" or "ri:exterior" then calls RiExteriorV.
		/// If type is "deformation" or "ri:deformation" then calls RiDeformationV.
		/// If type is "shader" or "ri:shader" then calls RiShader. In this case you must specify a handle
		/// as a StringData parameter named "__handle".
		void shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters ) override;

		/// By default, calls RiLightSource
		/// If the "ri:areaLight" parameter is true, calls RiAreaLightSource instead
		void light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters ) override;
		void illuminate( const std::string &lightHandle, bool on ) override;

		void motionBegin( const std::set<float> &times ) override;
		void motionEnd() override;

		void points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars ) override;
		void disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars ) override;

		void curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars ) override;

		void text( const std::string &font, const std::string &text, float kerning = 1.0f, const IECore::PrimitiveVariableMap &primVars=IECore::PrimitiveVariableMap() ) override;
		void sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars ) override;

		void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars ) override;
		/// Renders interpolation types of "linear" as RiPointsGeneralPolygons and "catmullClark" as RiSubdivisionMesh.
		/// Supports an optional "tags" primitive variable of type CompoundData and interpolation Constant, which is used to specify tags
		/// for the RiSubdivisionMesh call. This should contain the following children :
		///
		///		StringVectorData "names"
		///		IntVectorData "nArgs"
		///		FloatVectorData "floats"
		///		IntVectorData "integers"
		///
		/// \todo Consider how we might standardise a means of storing tags explicitly on the mesh rather than as primitive variables.
		void mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars ) override;

		void nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars ) override;

		void patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECore::PrimitiveVariableMap &primVars ) override;

		void geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars ) override;

		/// ExternalProcedurals are treated as DelayedReadArchives if their filename ends with ".rib"
		/// and as DynamicLoad procedurals otherwise. Because RenderMan has very poor support for passing
		/// parameters to DynamicLoad procedurals (you can only pass a single string), the arbitrary parameters
		/// of the ExternalProcedural are treated as follows :
		///
		/// - If an "ri:data" StringData parameter exists, it is passed verbatim to the procedural. This
		/// allows procedurals which require data in a specific format to be supported.
		/// - All other parameters are serialised in a command line "--name value" style and concatenated.
		/// This allows the convenience of arbitrary typed parameters provided that the procedural itself
		/// uses a command line style parser.
		void procedural( IECore::Renderer::ProceduralPtr proc ) override;

		void instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void instanceEnd() override;
		void instance( const std::string &name ) override;

		///
		/// Supports the following commands :
		///
		/// "clippingPlane"
		/// Makes an RiClippingPlane. This implementation is currently limited to making
		/// clipping planes only after all cameras have been specified.
		///
		///	"ri:readArchive"
		/// Expects a single StringData parameter called "name", and calls RiReadArchive with it.
		/// \todo Make a VisibleRenderable subclass to encapsulate a call to this.
		///
		/// "ri:archiveRecord"
		/// Makes a call to RiArchiveRecord(). Expects StringData parameters called "type" and
		/// "record".
		///
		IECore::DataPtr command( const std::string &name, const IECore::CompoundDataMap &parameters ) override;

		void editBegin( const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void editEnd() override;

	private :

		friend class RendererImplementation;
		// Constructor used by RendererImplementation when rendering procedurals.
		Renderer( RendererImplementationPtr implementation );

		RendererImplementationPtr m_implementation;

};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreRI

#endif // IECORERI_RENDERER_H
