//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_SXRENDERERIMPLEMENTATION_H
#define IECORERI_SXRENDERERIMPLEMENTATION_H

#include <stack>

#include "sx.h"

#include "IECoreRI/Export.h"
#include "IECoreRI/SXRenderer.h"
#include "IECoreRI/SXExecutor.h"

namespace IECoreRI
{

IE_CORE_FORWARDDECLARE( SXRendererImplementation );

class IECORERI_API SXRendererImplementation : public IECoreScene::Renderer
{

	public :

		SXRendererImplementation( IECoreRI::SXRenderer *parent );

		~SXRendererImplementation() override;

		void setOption( const std::string &name, IECore::ConstDataPtr value ) override;
		IECore::ConstDataPtr getOption( const std::string &name ) const override;

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

		IECore::CompoundDataPtr shade( const IECore::CompoundData *points ) const;
		IECore::CompoundDataPtr shade( const IECore::CompoundData *points, const Imath::V2i &gridSize ) const;
		IECore::CompoundDataPtr shadePlane( const Imath::V2i &resolution ) const;
		IECoreImage::ImagePrimitivePtr shadePlaneToImage( const Imath::V2i &resolution ) const;

	private :

		SxShader createShader( const char *name, const char *handle, const IECore::CompoundDataMap &parameters ) const;

		SXRenderer *m_parent;

		typedef boost::shared_ptr<void> SxContextPtr;

		struct State
		{
			State();
			State( const State &other, bool deepCopy );
			~State();
			IECore::CompoundDataPtr attributes;
			SxContextPtr context;
			SxShader displacementShader;
			SxShader surfaceShader;
			SxShader atmosphereShader;
			SxShader imagerShader;
			SXExecutor::ShaderVector coshaders;
			SXExecutor::ShaderVector lights;

			Imath::M44f transform;
		};
		typedef std::stack<State> StateStack;

		bool m_inWorld;
		StateStack m_stateStack;

};

} // namespace IECoreRI

#endif // IECORERI_SXRENDERERIMPLEMENTATION_H
