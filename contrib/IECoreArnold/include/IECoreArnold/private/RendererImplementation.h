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

#ifndef IECOREARNOLD_RENDERERIMPLEMENTATION_H
#define IECOREARNOLD_RENDERERIMPLEMENTATION_H

#include <stack>

#include "ai.h"

#include "IECoreArnold/Renderer.h"
#include "IECoreArnold/UniverseBlock.h"
#include "IECoreArnold/InstancingConverter.h"

namespace IECoreArnold
{

class RendererImplementation : public IECore::Renderer
{

	public :

		RendererImplementation();
		RendererImplementation( const std::string &assFileName );
		// Used to create an appropriate context for procedurals specified by procedural() to
		// run in.
		RendererImplementation( const RendererImplementation &other );
		// Initialises transform and attribute stacks from proceduralNode - used to
		// create an appropriate context for the cortex procedural DSO.
		RendererImplementation( const AtNode *proceduralNode );

		virtual ~RendererImplementation();

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

	private :

		enum Mode
		{
			AssGen,
			Render,
			Procedural
		};

		void constructCommon( Mode mode );

		void addPrimitive( const IECore::Primitive *primitive, const std::string &attributePrefix );
		void addShape( AtNode *shape );
		void applyTransformToNode( AtNode *node );
		void applyVisibilityToNode( AtNode *node );
		void addNode( AtNode *node );

		// this is what we use in the userptr for procedurals. it contains
		// the procedural we wish to render, and a renderer which contains the
		// state at the point the procedural was emitted.
		struct ProceduralData
		{
			IECore::Renderer::ProceduralPtr procedural;
			RendererPtr renderer;
		};

		static int procLoader( AtProcVtable *vTable );
		static int procInit( AtNode *node, void **userPtr );
		static int procCleanup( void *userPtr );
		static int procNumNodes( void *userPtr );
		static AtNode *procGetNode( void *userPtr, int i );

		boost::shared_ptr<UniverseBlock> m_universe;
		InstancingConverterPtr m_instancingConverter;

		Mode m_mode;
		std::string m_assFileName;

		// created in constructor.
		AtNode *m_defaultFilter;

		// built by the display() method, and passed to the arnold global options in worldBegin().
		std::vector<std::string> m_outputDescriptions;

		// transform stack stuff
		typedef std::stack<Imath::M44f> TransformStack;
		TransformStack m_transformStack;

		// attribute stack stuff
		class AttributeState
		{

			public :

				AttributeState();
				AttributeState( const AttributeState &other );

				AtNode *surfaceShader;
				AtNode *displacementShader;

				// shaders specified using "shader" or "ai:shader" type.
				// these are used as input connections to other shaders.
				typedef std::map<std::string, AtNode *> ShaderMap;
				ShaderMap shaders;

				IECore::CompoundDataPtr attributes;

		};

		typedef std::stack<AttributeState> AttributeStack;
		AttributeStack m_attributeStack;

		// list of nodes that have been output so far. we have
		// to collect this so we can support dripfeeding nodes to
		// arnold one by one in procedurals.
		std::vector<AtNode *> m_nodes;

		friend class IECoreArnold::Renderer;

};

} // namespace IECoreArnold

#endif // IECOREARNOLD_RENDERERIMPLEMENTATION_H
