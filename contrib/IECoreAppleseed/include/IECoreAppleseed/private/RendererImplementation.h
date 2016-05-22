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

#include <memory>
#include <stack>

#include "boost/filesystem/path.hpp"

#include "renderer/api/log.h"
#include "renderer/api/scene.h"
#include "renderer/api/project.h"

#include "IECore/Camera.h"

#include "IECoreAppleseed/Renderer.h"
#include "IECoreAppleseed/private/AttributeState.h"
#include "IECoreAppleseed/private/EditBlockHandler.h"
#include "IECoreAppleseed/private/LightHandler.h"
#include "IECoreAppleseed/private/MotionBlockHandler.h"
#include "IECoreAppleseed/private/TransformStack.h"
#include "IECoreAppleseed/private/PrimitiveConverter.h"

namespace IECoreAppleseed
{

class RendererImplementation : public IECore::Renderer
{

	public :

		RendererImplementation();
		RendererImplementation( const std::string &fileName );

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

		renderer::Project *appleseedProject() const;

	private :

		void constructCommon();

		bool isProjectGen() const;
		bool isEditable() const;

		void setCamera( const std::string &name, IECore::CameraPtr cortexCamera,
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

			return 0;
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
