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

#ifndef IECOREAPPLESEED_RENDERERIMPLEMENTATION_H
#define IECOREAPPLESEED_RENDERERIMPLEMENTATION_H

#include "IECoreAppleseed/Renderer.h"
#include "IECoreAppleseed/private/AttributeState.h"
#include "IECoreAppleseed/private/EditBlockHandler.h"
#include "IECoreAppleseed/private/LightHandler.h"
#include "IECoreAppleseed/private/MotionBlockHandler.h"
#include "IECoreAppleseed/private/PrimitiveConverter.h"
#include "IECoreAppleseed/private/TransformStack.h"

#include "IECoreScene/Camera.h"

#include "boost/filesystem/path.hpp"

#include "renderer/api/log.h"
#include "renderer/api/project.h"
#include "renderer/api/scene.h"

#include <memory>
#include <stack>

namespace IECoreAppleseed
{

class RendererImplementation : public IECoreScene::Renderer
{

	public :

		RendererImplementation();
		RendererImplementation( const std::string &fileName );

		~RendererImplementation() override;

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

		renderer::Project *appleseedProject() const;

	private :

		void constructCommon();

		bool isProjectGen() const;
		bool isEditable() const;

		void setCamera( const std::string &name, IECoreScene::CameraPtr cortexCamera,
			foundation::auto_release_ptr<renderer::Camera> &camera );

		std::string currentShaderGroupName();
		std::string currentMaterialName();

		void createAssemblyInstance( const std::string &assemblyName );

		template<class T>
		const T *getOptionAs( const std::string &name ) const
		{
			OptionsMap::const_iterator it( m_optionsMap.find( name ) );

			if( it != m_optionsMap.end() )
			{
				if( const IECore::TypedData<T> *data = IECore::runTimeCast<const IECore::TypedData<T> >( it->second.get() ) )
				{
					return &data->readable();
				}
			}

			return nullptr;
		}

		bool insideMotionBlock() const;
		bool insideEditBlock() const;

		// logging
		foundation::auto_release_ptr<foundation::ILogTarget> m_logTarget;

		// project related
		foundation::auto_release_ptr<renderer::Project> m_project;
		std::string m_fileName;
		boost::filesystem::path m_projectPath;

		typedef std::map<std::string,IECore::ConstDataPtr> OptionsMap;
		OptionsMap m_optionsMap;

		typedef std::stack<AttributeState> AttributeStack;
		AttributeStack m_attributeStack;

		TransformStack m_transformStack;

		renderer::Assembly *m_mainAssembly;
		std::auto_ptr<LightHandler> m_lightHandler;
		std::string m_currentShaderGroupName;
		std::auto_ptr<PrimitiveConverter> m_primitiveConverter;
		std::auto_ptr<MotionBlockHandler> m_motionHandler;
		std::auto_ptr<EditBlockHandler> m_editHandler;

		friend class IECoreAppleseed::Renderer;

};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_RENDERERIMPLEMENTATION_H
