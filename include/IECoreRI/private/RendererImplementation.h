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

#ifndef IECORERI_RENDERERIMPLEMENTATION_H
#define IECORERI_RENDERERIMPLEMENTATION_H

#include <stack>

#include "tbb/queuing_rw_mutex.h"
#include "tbb/recursive_mutex.h"
#include "tbb/mutex.h"

#include "ri.h"

#include "IECore/CachedReader.h"
#include "IECore/Camera.h"
#include "IECore/Font.h"
#include "IECore/Primitive.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/DiskPrimitive.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/SpherePrimitive.h"
#include "IECore/NURBSPrimitive.h"
#include "IECore/PatchMeshPrimitive.h"

#include "IECoreRI/Renderer.h"

namespace IECoreRI
{

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

		virtual void editBegin( const std::string &name, const IECore::CompoundDataMap &parameters );
		virtual void editEnd();

	private :

		// Does things common to both constructors
		void constructCommon();
	
		// When a procedural is processed, we make a new RendererImplementation
		// for it to talk to. Some member data should be unique to the new RendererImplementation
		// and other member data should be shared with the parent RendererImplementation. The
		// shared data is stored in this SharedData class.
		struct SharedData : IECore::RefCounted
		{
			IE_CORE_DECLAREMEMBERPTR( SharedData );
			// A map from instance names as given to us, and those used by renderman itself.
			// We use this because if RiObjectBeginV doesn't exist we don't get to choose the names.
			// It is part of the shared data so that procedurals may share instances.
			typedef std::map<std::string, const void *> ObjectHandleMap;
			ObjectHandleMap objectHandles;	
			// We need to mutex around access to objectHandles because it will be
			// accessed from multiple threads when running threaded procedurals
			typedef tbb::recursive_mutex ObjectHandlesMutex;
			ObjectHandlesMutex objectHandlesMutex;
		};
				
		RtContextHandle m_context;
		SharedData::Ptr m_sharedData;
		
		// All RendererImplementation instances associated with the same render must have the same SharedData
		// object, otherwise we won't be able to share object instance handles between them. This is confounded
		// when we call RendererImplementation() with no arguments, as it has no information about the render
		// that called it, and hence no idea what SharedData to use.
		
		// We address this using s_contextToSharedDataMap. Whenever a RendererImplementation is created, it adds
		// an entry associating the current context with a SharedData instance, so if the argument free constructor
		// is called later on in the same context, it can query the map and grab the correct SharedData. This
		// is a multimap, as multiple RendererImplementation instances can be created in a given context,
		// and we want to be able to clean up by removing entries in ~RendererImplementation().
		
		typedef tbb::mutex ContextToSharedDataMapMutex;
		typedef std::multimap< RtContextHandle, SharedData::Ptr > ContextToSharedDataMap;
		static ContextToSharedDataMapMutex s_contextToSharedDataMapMutex;
		static ContextToSharedDataMap s_contextToSharedDataMap;
		
		RtContextHandle m_contextToSharedDataMapKey;
		
		typedef void (RendererImplementation::*SetOptionHandler)( const std::string &name, IECore::ConstDataPtr d );
		typedef IECore::ConstDataPtr (RendererImplementation::*GetOptionHandler)( const std::string &name ) const;
		typedef std::map<std::string, SetOptionHandler> SetOptionHandlerMap;
		typedef std::map<std::string, GetOptionHandler> GetOptionHandlerMap;
		SetOptionHandlerMap m_setOptionHandlers;
		GetOptionHandlerMap m_getOptionHandlers;

		void setFontSearchPathOption( const std::string &name, IECore::ConstDataPtr d );
		void setShaderSearchPathOption( const std::string &name, IECore::ConstDataPtr d );
		void setPixelSamplesOption( const std::string &name, IECore::ConstDataPtr d );
		IECore::ConstDataPtr getFontSearchPathOption( const std::string &name ) const;
		IECore::ConstDataPtr getShutterOption( const std::string &name ) const;
		IECore::ConstDataPtr getResolutionOption( const std::string &name ) const;
		IECore::ConstDataPtr getRxOption( const char *name ) const;

		IECore::CompoundDataPtr m_options;
		IECore::CameraPtr m_camera;
		size_t m_numDisplays;
		bool m_inWorld;
		void outputCamera( IECore::CameraPtr camera );

		struct AttributeState
		{
			AttributeState();
			AttributeState( const AttributeState &other );
			std::map<std::string, std::string> primVarTypeHints;
		};
		std::stack<AttributeState> m_attributeStack;
		static const unsigned int g_shaderCacheSize;
		IECore::CachedReaderPtr m_shaderCache;
		static IECore::CachedReaderPtr defaultShaderCache();

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
		void setSubsurfaceAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setDetailAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setDetailRangeAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setTextureCoordinatesAttribute( const std::string &name, IECore::ConstDataPtr d );
		void setAutomaticInstancingAttribute( const std::string &name, IECore::ConstDataPtr d );

		IECore::ConstDataPtr getShadingRateAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getMatteAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getDoubleSidedAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getRightHandedOrientationAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getNameAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getTextureCoordinatesAttribute( const std::string &name ) const;
		IECore::ConstDataPtr getAutomaticInstancingAttribute( const std::string &name ) const;
		
		/// ProceduralData used to contain a smart pointer to the RendererImplementation which created it.
		/// This normally works fine, as 3delight typically calls procFree() immediately after procSubdivide(),
		/// meaning there are no extra references to the top level RendererImplementation lying around and it dies
		/// when it's supposed to. Unfortunately, when "ri:hider:editable" is enabled in later versions of 3delight
		/// (for progressive ipr rendering), calls to procFree() get deferred until RiEnd(). As RiEnd() 
		/// only gets called when the top level RendererImplementation dies, this makes it impossible to
		/// stop the progressive render. We get round this by using smart pointers to the shared data and options
		/// instead.
		struct ProceduralData
		{
			IECore::Renderer::ProceduralPtr procedural;
			SharedData::Ptr sharedData;
			IECore::CompoundDataPtr options;
		};
		
		// This constructor is used to create a child renderer in procSubdivide()
		RendererImplementation( SharedData::Ptr sharedData, IECore::CompoundDataPtr options );
		
		static void procSubdivide( void *data, float detail );
		static void procFree( void *data );

		typedef IECore::DataPtr ( RendererImplementation::*CommandHandler)( const std::string &name, const IECore::CompoundDataMap &parameters );
		typedef std::map<std::string, CommandHandler> CommandHandlerMap;
		CommandHandlerMap m_commandHandlers;

		IECore::DataPtr readArchiveCommand( const std::string &name, const IECore::CompoundDataMap &parameters );

		IECore::DataPtr archiveRecordCommand( const std::string &name, const IECore::CompoundDataMap &parameters );
		IECore::DataPtr illuminateCommand( const std::string &name, const IECore::CompoundDataMap &parameters );

		IECore::SearchPath m_fontSearchPath;
#ifdef IECORE_WITH_FREETYPE
		typedef std::map<std::string, IECore::FontPtr> FontMap;
		FontMap m_fonts;
#endif

		static tbb::queuing_rw_mutex g_nLoopsMutex;
		static std::vector<int> g_nLoops;

		bool automaticInstancingEnabled() const; // as for getAutomaticInstancingAttribute but doesn't need to allocate heap memory for the result	
		
		void addPrimitive( IECore::ConstPrimitivePtr primitive );
		
		void emitPrimitiveAttributes( const IECore::Primitive *primitive );
		void emitCurvesPrimitiveAttributes( const IECore::CurvesPrimitive *primitive );
		void emitPatchMeshPrimitiveAttributes( const IECore::PatchMeshPrimitive *primitive );
		
		void emitPrimitive( const IECore::Primitive *primitive );
		void emitPointsPrimitive( const IECore::PointsPrimitive *primitive );
		void emitDiskPrimitive( const IECore::DiskPrimitive *primitive );
		void emitCurvesPrimitive( const IECore::CurvesPrimitive *primitive );
		void emitMeshPrimitive( const IECore::MeshPrimitive *primitive );
		void emitSpherePrimitive( const IECore::SpherePrimitive *primitive );
		void emitNURBSPrimitive( const IECore::NURBSPrimitive *primitive );
		void emitPatchMeshPrimitive( const IECore::PatchMeshPrimitive *primitive );		
		
		/// Renderman treats curve basis as an attribute, whereas we want to treat it as
		/// part of the topology of primitives. It makes no sense as an attribute, as it changes the
		/// size of primitive variables - an attribute which makes a primitive invalid is dumb. This
		/// difference is fine, except it means we have to implement curves() as a call to RiBasis followed
		/// by RiCurves. Which is fine too, until we do that inside a motion block - at this point the context
		/// is invalid for the basis call - we should just be emitting the RiCurves call. We work around
		/// this by delaying all calls to motionBegin until the primitive or transform calls have had a chance
		/// to emit something first. This is what this function is all about - all interface functions which
		/// may be called from within a motion block must call delayedMotionBegin(). This makes for an ugly
		/// implementation but a better interface for the client. delayedMotionBegin() assumes that the correct
		/// RiContext will have been made current already.
		void delayedMotionBegin();
		/// True when an RiMotionBegin call has been emitted but we have not yet emitted the matching RiMotionEnd.
		bool m_inMotion;
		/// The times we'll emit in delayedMotionBegin.
		std::vector<float> m_delayedMotionTimes;
		/// Renderman doesn't accept instances inside motion blocks, but it does accept motion blocks inside
		/// instances. So when auto-instancing is on, we queue up primitives in here, and turn them into an instance
		/// at motionEnd().
		std::vector<IECore::ConstPrimitivePtr> m_motionPrimitives;

};

} // namespace IECoreRI

#endif // IECORERI_RENDERERIMPLEMENTATION_H
