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

#include "IECore/Renderer.h"

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
class Renderer : public IECore::Renderer
{

    public :

        /// Makes a renderer which will perfom an actual appleseed render at worldEnd().
        Renderer();

        /// Makes a renderer which will generate an appleseed project rather than
        /// produce images.
        Renderer( const std::string &fileName );

        virtual ~Renderer();

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreAppleseed::Renderer, RendererTypeId, IECore::Renderer );

        /// \par Implementation specific options :
        ///
        /// \li <b>"as:searchpath" StringData()</b><br>
        /// Appends a searchpath to the project searchpaths.
        ///
        /// \li <b>"as:mesh_file_format" StringData()</b><br>
        /// File format used to save meshes when creating a project file.
        /// Can be "obj" or "binarymesh".
        ///
        /// \li <b>"as:environment_edf" StringData()</b><br>
        /// Name of the environment light used when rendering.
        ///
        /// \li <b>"as:environment_edf_background" BoolData()</b><br>
        /// True if the environment is visible in the background.
        ///
        /// \li <b>"as:cfg:*"</b><br>
        /// Passed to appleseed Configuration.
        virtual void setOption( const std::string &name, IECore::ConstDataPtr value );
        virtual IECore::ConstDataPtr getOption( const std::string &name ) const;

        /// \par Standard parameters supported :
        ///
        /// \li <b>"resolution"</b>
        ///	\li <b>"cropWindow"</b>
        /// \li <b>"projection"</b>
        /// \li <b>"projection:fov"</b>
        /// \li <b>"shutter"</b>
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
        /// \li <b>"name"</b><br>
        /// Sets the name of the object being specified.
        ///
        ////////////////////////////////////////////////////////////////////////////
        ///
        /// \li <b>"as:shading_samples" FloatData</b><br>
        /// Number of shading samples to use for the current material.
        ///
        /// \li <b>"as:alpha_map" StringData</b><br>
        /// Specifies a texture to use as an alpha map.
        virtual void setAttribute( const std::string &name, IECore::ConstDataPtr value );
        virtual IECore::ConstDataPtr getAttribute( const std::string &name ) const;

        virtual void shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters );
        virtual void light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters );
        virtual void illuminate( const std::string &lightHandle, bool on );

        virtual void motionBegin( const std::set<float> &times );
        virtual void motionEnd();

        virtual void points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars );
        virtual void disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars );

        virtual void curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars );

        virtual void text( const std::string &font, const std::string &text, float kerning = 1.0f, const IECore::PrimitiveVariableMap &primVars=IECore::PrimitiveVariableMap() );
        virtual void sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars );

        virtual void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars );

        virtual void mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars );

        virtual void nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars );

        virtual void patchMesh( const IECore::CubicBasisf &uBasis, const IECore::CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const IECore::PrimitiveVariableMap &primVars );

        virtual void geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars );

        virtual void procedural( IECore::Renderer::ProceduralPtr proc );

        virtual void instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters );
        virtual void instanceEnd();
        virtual void instance( const std::string &name );

        virtual IECore::DataPtr command( const std::string &name, const IECore::CompoundDataMap &parameters );

        virtual void editBegin( const std::string &editType, const IECore::CompoundDataMap &parameters );
        virtual void editEnd();

        /// Returns a pointer to the appleseed Project held by
        /// the renderer. This is used in unit tests.
        renderer::Project *appleseedProject() const;

    private :

        friend class RendererImplementation;

        RendererImplementationPtr m_implementation;

};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_RENDERER_H
