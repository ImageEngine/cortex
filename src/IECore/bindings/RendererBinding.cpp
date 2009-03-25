//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings 
// regarding redefinition of _POSIX_C_SOURCE
#include <boost/python.hpp>
#include <boost/python/suite/indexing/container_utils.hpp>

#include "IECore/Renderer.h"
#include "IECore/CompoundObject.h"
#include "IECore/MessageHandler.h"
#include "IECore/bindings/RendererBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace boost;
using namespace std;

namespace IECore
{

class ProceduralWrap : public Renderer::Procedural, public Wrapper<Renderer::Procedural>
{
	public :
		ProceduralWrap( PyObject *self, const std::string &name, const std::string &description ) : Renderer::Procedural( name, description ), Wrapper<Renderer::Procedural>( self, this ) {};
		virtual Imath::Box3f doBound( ConstCompoundObjectPtr args ) const
		{
			try
			{
				override o = this->get_override( "doBound" );
				if( o )
				{
					return o( const_pointer_cast<CompoundObject>( args ) );
				}
				else
				{
					msg( Msg::Error, "ProceduralWrap::doBound", "doBound() python method not defined" );
				}
			}
			catch( error_already_set )
			{
				PyErr_Print();
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ProceduralWrap::doRender", e.what() );
			}
			catch( ... )
			{
				msg( Msg::Error, "ProceduralWrap::doRender", "Caught unknown exception" );
			}
			return Imath::Box3f(); // empty
		}
		virtual void doRender( RendererPtr r, ConstCompoundObjectPtr args ) const
		{
			// ideally we might not do any exception handling here, and always leave it to the host.
			// but in our case the host is mainly 3delight and that does no exception handling at all.
			try
			{
				override o = this->get_override( "doRender" );
				if( o )
				{
					//// \todo We may want to call operands->copy() here instead of casting away the constness. If the Python code being called
					/// here actually attempts to change the CompoundObject, then any C++ calling code might get confused when a suposedly const value
					/// changes unexpectedly. Check any performance overhead of the copy.
					o( r, const_pointer_cast<CompoundObject>( args ) );
				}
				else
				{
					msg( Msg::Error, "ProceduralWrap::doRender", "doRender() python method not defined" );
				}
			}
			catch( error_already_set )
			{
				PyErr_Print();
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ProceduralWrap::doRender", e.what() );
			}
			catch( ... )
			{
				msg( Msg::Error, "ProceduralWrap::doRender", "Caught unknown exception" );
			}
		}

};
IE_CORE_DECLAREPTR( ProceduralWrap );

static void fillCompoundDataMap( CompoundDataMap &m, const dict &d )
{
	boost::python::list keys = d.keys();
	for( unsigned i=0; i<keys.attr( "__len__" )(); i++ )
	{
		m[extract<string>( keys[i] )] = extract<DataPtr>( d[keys[i]] );
	}
}

static void fillPrimitiveVariableMap( PrimitiveVariableMap &m, const dict &d )
{
	boost::python::list keys = d.keys();
	for( unsigned i=0; i<keys.attr( "__len__" )(); i++ )
	{
		m[extract<string>( keys[i] )] = extract<PrimitiveVariable>( d[keys[i]] );
	}
}

static DataPtr getOption( Renderer &r, const std::string &name )
{
	ConstDataPtr d = r.getOption( name );
	if( d )
	{
		return d->copy();
	}
	return 0;
}

static void camera( Renderer &r, const std::string &name, const dict &parameters )
{
	CompoundDataMap p;
	fillCompoundDataMap( p, parameters );
	r.camera( name, p );
}

static void display( Renderer &r, const std::string &name, const std::string &type, const std::string &data, const dict &parameters )
{
	CompoundDataMap p;
	fillCompoundDataMap( p, parameters );
	r.display( name, type, data, p );
}

static DataPtr getAttribute( Renderer &r, const std::string &name )
{
	ConstDataPtr d = r.getAttribute( name );
	if( d )
	{
		return d->copy();
	}
	return 0;
}

static void shader( Renderer &r, const std::string &type, const std::string &name, const dict &parameters )
{
	CompoundDataMap p;
	fillCompoundDataMap( p, parameters );
	r.shader( type, name, p );
}

static void light( Renderer &r, const std::string &name, const dict &parameters )
{
	CompoundDataMap p;
	fillCompoundDataMap( p, parameters );
	r.light( name, p );
}

static void motionBegin( Renderer &r, const boost::python::list &times )
{
	std::set<float> t;
	for( unsigned i=0; i<times.attr( "__len__" )(); i++ )
	{
		t.insert( extract<float>( times[i] ) );
	}
	r.motionBegin( t );
}

static void points( Renderer &r, size_t numPoints, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.points( numPoints, p );
}

static void disk( Renderer &r, float radius, float z, float thetaMax, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.disk( radius, z, thetaMax, p );
}

static void curves( Renderer &r, const CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.curves( basis, periodic, numVertices, p );
}

static void text( Renderer &r, const std::string &font, const std::string &text, float kerning, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.text( font, text, kerning, p );
}

static void sphere( Renderer &r, float radius, float zMin, float zMax, float thetaMax, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.sphere( radius, zMin, zMax, thetaMax, p );
}

static void image( Renderer &r, const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.image( dataWindow, displayWindow, p );
}

static void mesh( Renderer &r, ConstIntVectorDataPtr vertsPerFace, ConstIntVectorDataPtr vertIds, const std::string &interpolation, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.mesh( vertsPerFace, vertIds, interpolation, p );
}

static void nurbs( Renderer &r, int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.nurbs( uOrder, uKnot, uMin, uMax, vOrder, vKnot, vMin, vMax, p );
}

static void patchMesh( Renderer &r, const CubicBasisf &uBasis, const CubicBasisf &vBasis, const std::string &type, int nu, bool uPeriodic, int nv, bool vPeriodic, const dict &primVars )
{
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.patchMesh( uBasis, vBasis, type, nu, uPeriodic, nv, vPeriodic, p );
}

static void geometry( Renderer &r, const std::string &type, const dict &topology, const dict &primVars )
{
	CompoundDataMap t;
	fillCompoundDataMap( t, topology );
	
	PrimitiveVariableMap p;
	fillPrimitiveVariableMap( p, primVars );
	r.geometry( type, t, p );
}

static void instanceBegin( Renderer &r, const std::string &name, const dict &parameters )
{
	CompoundDataMap p;
	fillCompoundDataMap( p, parameters );
	r.instanceBegin( name, p );
}

static void command( Renderer &r, const std::string &name, const dict &parameters )
{
	CompoundDataMap p;
	fillCompoundDataMap( p, parameters );
	r.command( name, p );
}

void bindRenderer()
{
	typedef class_< Renderer, boost::noncopyable, RendererPtr, bases< RunTimeTyped > > RendererPyClass;
	scope rendererScope = RendererPyClass("Renderer", "An abstract class to define a renderer", no_init)
		.def("setOption", &Renderer::setOption)
		.def("getOption", &getOption, "Returns a copy of the internal option data." )

		.def("camera", &camera)
		.def("display", &display)

		.def("worldBegin", &Renderer::worldBegin)
		.def("worldEnd", &Renderer::worldEnd)

		.def("transformBegin", &Renderer::transformBegin)
		.def("transformEnd", &Renderer::transformEnd)
		.def("setTransform", (void (Renderer::*)( const Imath::M44f &))&Renderer::setTransform)
		.def("setTransform", (void (Renderer::*)( const std::string &))&Renderer::setTransform)
		.def("getTransform", (Imath::M44f (Renderer::*)() const )&Renderer::getTransform)
		.def("getTransform", (Imath::M44f (Renderer::*)( const std::string &) const ) &Renderer::getTransform)
		.def("concatTransform", &Renderer::concatTransform)
		.def("coordinateSystem", &Renderer::coordinateSystem)
		
		.def("attributeBegin", &Renderer::attributeBegin)
		.def("attributeEnd", &Renderer::attributeEnd)
		.def("setAttribute", &Renderer::setAttribute)
		.def("getAttribute", &getAttribute, "Returns a copy of the internal attribute data.")

		.def("shader", &shader)
		.def("light", &light)
		
		.def("motionBegin", &motionBegin)
		.def("motionEnd", &Renderer::motionEnd)

		.def("points", &points)
		.def("disk", &disk)
		.def("curves", &curves)
		.def("text", &text)
		.def("sphere", &sphere)
		.def("image", &image)
		.def("mesh", &mesh)
		.def("nurbs", &nurbs)
		.def("patchMesh", &patchMesh)
		.def("geometry", &geometry)
		
		.def("procedural", &Renderer::procedural)

		.def("instanceBegin", &instanceBegin)
		.def("instanceEnd", &Renderer::instanceEnd)
		.def("instance", &Renderer::instance)

		.def("command", &command)
		
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(Renderer)		
	;

	INTRUSIVE_PTR_PATCH( Renderer, RendererPyClass );
	implicitly_convertible<RendererPtr, RunTimeTypedPtr>();
	
	typedef class_< Renderer::Procedural, ProceduralWrapPtr, boost::noncopyable, bases<Parameterised> > ProceduralPyClass;
	ProceduralPyClass( "Procedural", no_init )
		.def( init<const std::string &, const std::string &>() )
		.def( "bound", &Renderer::Procedural::bound )
		.def( "render", &Renderer::Procedural::render )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( Renderer::Procedural )		
	;
	
	WrapperToPython<Renderer::ProceduralPtr>();

	INTRUSIVE_PTR_PATCH( Renderer::Procedural, ProceduralPyClass );
	
	implicitly_convertible<Renderer::ProceduralPtr, ParameterisedPtr>();
	implicitly_convertible<Renderer::ProceduralPtr, Renderer::ConstProceduralPtr>();
	implicitly_convertible<ProceduralWrapPtr, Renderer::ProceduralPtr>();
	
}
	
} // namespace IECore
