//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_RENDERER_H
#define IECOREAPPLESEED_RENDERER_H

#include "IECoreScene/Renderer.h"

#include "IECoreAppleseed/Export.h"
#include "IECoreAppleseed/TypeIds.h"

namespace renderer
{
	class Project;
}

namespace IECoreAppleseed
{

IE_CORE_FORWARDDECLARE( RendererImplementation )

/// An IECore::Renderer subclass which renders to appleseed.
/// \ingroup renderingGroup
class IECOREAPPLESEED_API Renderer : public IECoreScene::Renderer
{

	public :

		/// Makes a renderer which will perfom an actual appleseed render at worldEnd().
		Renderer();

		/// Makes a renderer which will generate an appleseed project rather than
		/// produce images.
		Renderer( const std::string &fileName );

		~Renderer() override;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreAppleseed::Renderer, RendererTypeId, IECoreScene::Renderer );

		/// \par Implementation specific options :
		///
		/// \li <b>"as:searchpath" StringData()</b><br>
		/// Appends a searchpath to the project searchpaths.
		///
		/// \li <b>"as:mesh_file_format" StringData()</b><br>
		/// File format used to save meshes when creating a project file.
		/// Can be "obj" or "binarymesh".
		///
		/// \li <b>"as:automatic_instancing" BoolData()</b><br>
		/// Enables or disables automatic instancing.
		///
		/// \li <b>"as:environment_edf" StringData()</b><br>
		/// Name of the environment light used when rendering.
		///
		/// \li <b>"as:environment_edf_background" BoolData()</b><br>
		/// True if the environment is visible in the background.
		///
		/// \li <b>"as:cfg:*"</b><br>
		/// Passed to appleseed configuration (render settings).
		void setOption( const std::string &name, IECore::ConstDataPtr value ) override;
		IECore::ConstDataPtr getOption( const std::string &name ) const override;

		/// \par Standard parameters supported :
		///
		/// \li <b>"resolution"</b>
		///	\li <b>"cropWindow"</b>
		/// \li <b>"projection"</b>
		/// \li <b>"projection:fov"</b>
		/// \li <b>"shutter"</b>
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
		/// \li <b>"name"</b><br>
		/// Sets the name of the object being specified.
		///
		////////////////////////////////////////////////////////////////////////////
		///
		/// \li <b>"as:visibility:*" BoolData</b><br>
		/// Visibility flags.
		///
		/// \li <b>"as:shading_samples" FloatData</b><br>
		/// Number of shading samples to use for the current shader.
		///
		/// \li <b>"as:alpha_map" StringData</b><br>
		/// Specifies a texture to use as an alpha map.
		///
		/// \li <b>"as:photon_target" BoolData</b><br>
		/// Specifies that an object is an important target for photons.
		void setAttribute( const std::string &name, IECore::ConstDataPtr value ) override;
		IECore::ConstDataPtr getAttribute( const std::string &name ) const override;

		void shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters ) override;
		void illuminate( const std::string &lightHandle, bool on ) override;

		void motionBegin( const std::set<float> &times ) override;
		void motionEnd() override;

		void points( size_t numPoints, const IECoreScene::PrimitiveVariableMap &primVars ) override;
		void disk( float radius, float z, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void text( const std::string &font, const std::string &text, float kerning = 1.0f, const IECoreScene::PrimitiveVariableMap &primVars=IECoreScene::PrimitiveVariableMap() ) override;
		void sphere( float radius, float zMin, float zMax, float thetaMax, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECoreScene::PrimitiveVariableMap &primVars ) override;

		void procedural( IECoreScene::Renderer::ProceduralPtr proc ) override;

		void instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters ) override;
		void instanceEnd() override;
		void instance( const std::string &name ) override;

		IECore::DataPtr command( const std::string &name, const IECore::CompoundDataMap &parameters ) override;

		void editBegin( const std::string &editType, const IECore::CompoundDataMap &parameters ) override;
		void editEnd() override;

		/// Returns a pointer to the appleseed Project held by the renderer.
		/// It is used in IECoreAppleseed unit tests.
		renderer::Project *appleseedProject() const;

	private :

		friend class RendererImplementation;

		RendererImplementationPtr m_implementation;

};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_RENDERER_H
