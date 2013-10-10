//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Electric Theatre Collective Limited. All rights reserved.
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the following disclaimer.
//
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the following disclaimer in the
//	   documentation and/or other materials provided with the distribution.
//
//	 * Neither the name of Image Engine Design nor the names of any
//	   other contributors to this software may be used to endorse or
//	   promote products derived from this software without specific prior
//	   written permission.
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

#ifndef IECOREMANTRA_RENDERERIMPLEMENTATION_H
#define IECOREMANTRA_RENDERERIMPLEMENTATION_H

#include <list>
#include <stack>

#include "IECore/Camera.h"
#include "IECore/Group.h"
#include "IECoreMantra/Renderer.h"
#include "IECoreMantra/ProceduralPrimitive.h"

namespace IECoreMantra
{

// Mantra doesn't have a single context/api for defining both scenes and 
// procedurals outside of a HIP file.
// 
// For defining a scene the api is IFD script fed to mantra over stdin.
// 
// For defining procedural geometry the api is VRAY_Procedural ..and GU_Detail by 
// association.
// 
// This implementation has to do some strange things in an attempt to be faithful
// to the client interface of IECore::Renderer. It is incomplete.
// 
// There are 3 render modes: Procedural, Render and IfdGen.
// 
// Render and IfdGen modes aren't currently very useful, only a few render 
// methods are implemented, just enough to pass some very simple tests.
//
// Procedural
//     
// This is simple case, during the normal rendering of an IFD mantra has hit the
// bound of a VRAY_ieProcedural.so which then loads a cortex 
// ParameterisedProcedural. It runs the cortex procedural when Mantra asks for 
// geometry.
// 
// VRAY_ieProcedural provides an entry point for users to declare cortex 
// procedurals. It's registred as a SHOP type in the ieCoreMantra.otl and is 
// wrapped in a HDA helper familliar to RI users: 'cortexMantraInject'.
// 
// VRAY_ieProcedural is derived form IECoreMantra::ProceduralPrimitve which has an
// interface that RendererImplementation uses to add VisibleRenderables to mantra.
// RendererImplementation and ProceduralPrimitive are friends and they touch each
// others private data. (Internaly ProceduralPrimitive is derived from the HDK
// class VRAY_Procedural and uses ToHoudiniConverter to create Houdini geometry).
// 
// Render
// 
// An empty constructor starts a live Render context. For IECoreMantra this means
// popen()-ing mantra. Pre and Post world calls are sent as strings over stdin.
// 
// Upon worldBegin() a secret procedural VRAY_ieworld.so is added to the scene
// with the geometry() method. A temporary file name using the pid is stashed in
// the IFD using setOption. ("/tmp/ieworld_${PID}.cob")
// 
// Calls that affect render state are stored in a IECore::Group object m_world.
// 
// Upon worldEnd() the m_world group is saved to the temporary file.
// The IFD commands 'ray_raytrace' and 'ray_quit' are then sent to mantra to   
// signal the end of scene description and start the rendering. The first object
// that mantra encounters will probably be the ieworld procedural. This 
// procedural looks for the stored temporary file of m_world and loads the 
// retained scene.
// 
// IfdGen
// 
// Like Render mode but rather than a pipe to mantra a file stream is used to 
// write an IFD for later rendering. In this case the world cob file is not 
// considered temporary. It uses the same name as the IFD with the suffix 
// 'ieworld.cob'

class RendererImplementation : public IECore::Renderer
{
	public:
		// popen mantra and write strings of ifd commands to it
		RendererImplementation();
		// fopen a file and write strings of ifd commands to it
		RendererImplementation( const std::string &ifdFileName );
		// procedural() method uses this to copy the parent render context.
		RendererImplementation( RendererImplementationPtr parent );
		// A VRAY so derived from  ProceduralPrimitive uses this as an entry point.
		RendererImplementation( ProceduralPrimitive *procedural );
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
		
	private:
		
		enum Mode
		{
			IfdGen,
			Render,
			Procedural
		};
		Mode m_mode;
		std::string m_ifdFileName;

		IECore::GroupPtr m_world;
		std::string m_worldFileName;

        FILE *m_fpipe;

		bool m_preWorld;
	 
	 	IECore::CameraPtr m_camera;
		void outputCamera ( IECore::CameraPtr camera );

		// An object for creating geometry, derived from VRAY_Procedural.
		// This is a raw pointer because mantra owns it. It is only valid in Procedural mode.
		ProceduralPrimitive* m_vrayproc;
		
		typedef std::stack<Imath::M44f> TransformStack;
		TransformStack m_transformStack;
		
		struct AttributeState
		{
			AttributeState();
			AttributeState( const AttributeState &other );
			IECore::CompoundDataPtr attributes;
		};
		typedef std::stack<AttributeState> AttributeStack;
		AttributeStack m_attributeStack;
		
		enum MotionType
		{
			Unknown,
			SetTransform,
			ConcatTransform,
			Geometry,
			Velocity
		};
		MotionType m_motionType;
		std::list<float> m_motionTimes;
		std::list<Imath::M44f> m_motionTransforms;
		int m_motionSize;
		bool m_inMotion;

		// Options and Attributes use the vm: prefix for mantra.
		typedef void (RendererImplementation::*SetOptionHandler)( const std::string &name, IECore::ConstDataPtr d );
		typedef IECore::ConstDataPtr (RendererImplementation::*GetOptionHandler)( const std::string &name ) const;
		typedef std::map<std::string, SetOptionHandler> SetOptionHandlerMap;
		typedef std::map<std::string, GetOptionHandler> GetOptionHandlerMap;
		SetOptionHandlerMap m_setOptionHandlers;
		GetOptionHandlerMap m_getOptionHandlers;
		
		
		typedef void (RendererImplementation::*SetAttributeHandler)( const std::string &name, IECore::ConstDataPtr d );
		typedef IECore::ConstDataPtr (RendererImplementation::*GetAttributeHandler)( const std::string &name ) const; 
		typedef std::map<std::string, SetAttributeHandler> SetAttributeHandlerMap;									
		typedef std::map<std::string, GetAttributeHandler> GetAttributeHandlerMap;									
		SetAttributeHandlerMap m_setAttributeHandlers;																
		GetAttributeHandlerMap m_getAttributeHandlers;
		
		void constructCommon( Mode mode );

		IECore::ConstDataPtr getShutterOption( const std::string &name ) const;
		IECore::ConstDataPtr getResolutionOption( const std::string &name ) const;
		IECore::ConstDataPtr getVelocityBlurAttribute( const std::string &name ) const;
		void setMatteAttribute( const std::string &name, IECore::ConstDataPtr d );

		void ifdString( IECore::ConstDataPtr value, std::string &ifd, std::string &type ); 
		
		friend class IECoreMantra::Renderer;
		friend class IECoreMantra::ProceduralPrimitive;
};

} // namespace IECoreMantra

#endif
 
