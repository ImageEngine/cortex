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

#ifndef IECORERI_RENDERERIMPLEMENTATION_H
#define IECORERI_RENDERERIMPLEMENTATION_H

#include "IECore/Renderer.h"
#include "IECore/CachedReader.h"
#include "IECore/Camera.h"

#include "ri.h"

#include <stack>

namespace IECoreRI
{

IE_CORE_FORWARDDECLARE( RendererImplementation );

class RendererImplementation : public IECore::Renderer
{

	public :

		RendererImplementation();
		RendererImplementation( const std::string &name );
		
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
		virtual void light( const std::string &name, const IECore::CompoundDataMap &parameters );

		virtual void motionBegin( const std::set<float> times );
		virtual void motionEnd();

		virtual void points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars );
		
		virtual void curves( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars );

		virtual Imath::Box3f textExtents(const std::string & t, const float width = Imath::limits<float>::max() );
		virtual void text(const std::string &t, const float width = Imath::limits<float>::max() );

		virtual void image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars );
		virtual void mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars );
		
		virtual void nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars );

		virtual void geometry( const std::string &type, const IECore::CompoundDataMap &topology, const IECore::PrimitiveVariableMap &primVars );

		virtual void procedural( IECore::Renderer::ProceduralPtr proc );
		
		virtual IECore::DataPtr command( const std::string &name, const IECore::CompoundDataMap &parameters );

	private :
	
		// Does things common to both constructors
		void constructCommon();
	
		RtContextHandle m_context;
	
		typedef void (RendererImplementation::*SetOptionHandler)( const std::string &name, IECore::ConstDataPtr d );
		typedef IECore::ConstDataPtr (RendererImplementation::*GetOptionHandler)( const std::string &name ) const;
		typedef std::map<std::string, SetOptionHandler> SetOptionHandlerMap;
		typedef std::map<std::string, GetOptionHandler> GetOptionHandlerMap;
		SetOptionHandlerMap m_setOptionHandlers;
		GetOptionHandlerMap m_getOptionHandlers;
		
		void setShaderSearchPathOption( const std::string &name, IECore::ConstDataPtr d );
		void setPixelSamplesOption( const std::string &name, IECore::ConstDataPtr d );
		IECore::ConstDataPtr getShutterOption( const std::string &name ) const;
		IECore::ConstDataPtr getResolutionOption( const std::string &name ) const;
		
		IECore::CameraPtr m_camera;
		
		struct AttributeState
		{
			AttributeState();
			AttributeState( const AttributeState &other );
			std::map<std::string, std::string> primVarTypeHints;
		};
		std::stack<AttributeState> m_attributeStack;
		static const unsigned int g_shaderCacheSize;
		IECore::CachedReaderPtr m_shaderCache;
		
		typedef void (RendererImplementation::*SetAttributeHandler)( const std::string &name, IECore::ConstDataPtr d );
		typedef IECore::ConstDataPtr (RendererImplementation::*GetAttributeHandler)( const std::string &name ) const;
		typedef std::map<std::string, SetAttributeHandler> SetAttributeHandlerMap;
		typedef std::map<std::string, GetAttributeHandler> GetAttributeHandlerMap;
		SetAttributeHandlerMap m_setAttributeHandlers;
		GetAttributeHandlerMap m_getAttributeHandlers;
		
		void setShadingRateAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setMatteAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setColorAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setOpacityAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setSidesAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setDoubleSidedAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setRightHandedOrientationAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setGeometricApproximationAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setNameAttribute( const std::string &name, IECore::ConstDataPtr d );

		IECore::ConstDataPtr getShadingRateAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getMatteAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getDoubleSidedAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getRightHandedOrientationAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getNameAttribute( const std::string &name ) const;
		
		struct ProcData
		{
			IECore::Renderer::ConstProceduralPtr proc;
			RendererImplementationPtr that;
		};
		static void procSubdivide( void *data, float detail );
		static void procFree( void *data );
		
		typedef IECore::DataPtr ( RendererImplementation::*CommandHandler)( const std::string &name, const IECore::CompoundDataMap &parameters );
		typedef std::map<std::string, CommandHandler> CommandHandlerMap;
		CommandHandlerMap m_commandHandlers;
		
		IECore::DataPtr  readArchiveCommand( const std::string &name, const IECore::CompoundDataMap &parameters );
		
		typedef std::map<std::string, const void *> ObjectHandleMap;
		ObjectHandleMap m_objectHandles;
		IECore::DataPtr objectBeginCommand( const std::string &name, const IECore::CompoundDataMap &parameters );
		IECore::DataPtr objectEndCommand( const std::string &name, const IECore::CompoundDataMap &parameters );
		IECore::DataPtr objectInstanceCommand( const std::string &name, const IECore::CompoundDataMap &parameters );
		IECore::DataPtr archiveRecordCommand( const std::string &name, const IECore::CompoundDataMap &parameters );

		static std::vector<int> g_nLoops;
};

} // namespace IECoreRI

#endif // IECORERI_RENDERERIMPLEMENTATION_H
