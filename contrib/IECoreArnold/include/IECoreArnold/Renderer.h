//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IECOREARNOLD_RENDERER_H
#define IECOREARNOLD_RENDERER_H

#include "IECoreArnold/TypeIds.h"

#include "IECore/Renderer.h"

struct AtNode;

namespace IECoreArnold
{

IE_CORE_FORWARDDECLARE( RendererImplementation )

/// An IECore::Renderer subclass which renders to Arnold through the AI interface.
/// \ingroup renderingGroup
class Renderer : public IECore::Renderer
{

	public :

		/// Makes a renderer which will perfom an actual Arnold render at worldEnd().
		Renderer();
		/// Makes a renderer which will generate an ass file rather than
		/// produce images.
		Renderer( const std::string &assFileName );
		/// Makes a renderer which can be used for expanding the procedural
		/// passed as an argument.
		Renderer( const AtNode *proceduralNode );
		
		virtual ~Renderer();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreArnold::Renderer, RendererTypeId, IECore::Renderer );

		/// Anything matching "ai:*" is used to set parameters on the global arnold options node.
		virtual void setOption( const std::string &name, IECore::ConstDataPtr value );
		virtual IECore::ConstDataPtr getOption( const std::string &name ) const;

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
		/// \par Implementation specific attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"ai:visibility:camera" BoolData( true )</b>
		/// \li <b>"ai:visibility:shadow" BoolData( true )</b>
		/// \li <b>"ai:visibility:reflected" BoolData( true )</b>
		/// \li <b>"ai:visibility:refracted" BoolData( true )</b>
		/// \li <b>"ai:visibility:diffuse" BoolData( true )</b>
		/// \li <b>"ai:visibility:glossy" BoolData( true )</b>
		///
		/// \li <b>"ai:*:*" Data</b><br>
		/// Mapped to shape node parameters, such that "ai:nodeType:parameterName"
		/// entries will set a parameter called parameterName on all shapes of type
		/// nodeType.
		///
		/// \par Instancing attributes :
		////////////////////////////////////////////////////////////
		///
		/// \li <b>"automaticInstancing" BoolData true</b><br>
		/// \li <b>"ai:automaticInstancing" BoolData true</b><br>
		/// Specifies that instances are automatically created if
		/// identical primitives are passed to the renderer
		/// repeatedly.
		////////////////////////////////////////////////////////////
		virtual void setAttribute( const std::string &name, IECore::ConstDataPtr value );
		virtual IECore::ConstDataPtr getAttribute( const std::string &name ) const;
		/// Supports types "surface", "ai:surface", "displacement", "ai:displacement",
		/// "shader" and "ai:shader". The "shader" types require the parameter list to
		/// contain an __handle parameter, specifying a string handle - this can then
		/// be used in the parameter lists for subsequent shaders to create connections,
		/// by providing a parameter value of "link:handle". In addition to loading shaders
		/// by name, names of the form "reference:nodeName" will reference an already existing
		/// Arnold shader node of the specified name.
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
		
		/// May be called when this renderer is being used to run a procedural, to return the number of
		/// Arnold nodes created by the procedural.
		size_t numProceduralNodes() const;
		/// May be called when this renderer is being used from a procedural, to return the index'th node
		/// created by the procedural.
		void *proceduralNode( size_t index );

	private :

		friend class RendererImplementation;
		
		Renderer( RendererImplementationPtr implementation );

		RendererImplementationPtr m_implementation;

};

IE_CORE_DECLAREPTR( Renderer );

} // namespace IECoreArnold

#endif // IECOREARNOLD_RENDERER_H
