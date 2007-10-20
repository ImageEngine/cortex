//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>

#include "IECore/bindings/ParameterBinding.h"
#include "IECore/bindings/Wrapper.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

#include "IECore/Renderable.h"
#include "IECore/StateRenderable.h"
#include "IECore/AttributeState.h"
#include "IECore/Shader.h"
#include "IECore/Transform.h"
#include "IECore/MatrixMotionTransform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/Group.h"
#include "IECore/MotionPrimitive.h"
#include "IECore/Primitive.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/PointsPrimitive.h"

using namespace std;
using namespace boost;
using namespace boost::python;

namespace IECore
{

template<typename T>
class TypedObjectParameterWrap : public TypedObjectParameter<T>, public Wrapper< TypedObjectParameter<T> >
{
	protected:

		static typename TypedObjectParameter<T>::ObjectPresetsMap makePresets( const dict &d )
		{
			typename TypedObjectParameter<T>::ObjectPresetsMap p;
			boost::python::list keys = d.keys();
			boost::python::list values = d.values();
			for( int i = 0; i<keys.attr( "__len__" )(); i++ )
			{
				extract<typename T::Ptr> e( values[i] );
				p.insert( typename TypedObjectParameter<T>::ObjectPresetsMap::value_type( extract<string>( keys[i] )(), e() ) );			
			}
			return p;
		}

	public :

		TypedObjectParameterWrap( PyObject *self, const std::string &n, const std::string &d, typename T::Ptr dv, const dict &p = dict(), bool po = false, CompoundObjectPtr ud = 0 )	
			:	TypedObjectParameter<T>( n, d, dv, makePresets( p ), po, ud ), Wrapper< TypedObjectParameter<T> >( self, this ) {};
		
		TypedObjectParameterWrap( PyObject *self, const std::string &n, const std::string &d, typename T::Ptr dv, CompoundObjectPtr ud )	
			:	TypedObjectParameter<T>( n, d, dv, typename TypedObjectParameter<T>::ObjectPresetsMap(), false, ud ), Wrapper< TypedObjectParameter<T> >( self, this ) {};

		IE_COREPYTHON_PARAMETERWRAPPERFNS( TypedObjectParameter<T> );
};

template<typename T>
static void bindTypedObjectParameter( const char *name )
{
	typedef class_< TypedObjectParameter<T>, intrusive_ptr< TypedObjectParameterWrap< T > >, boost::noncopyable, bases<ObjectParameter> > TypedObjectParameterPyClass;
	TypedObjectParameterPyClass( name, no_init )
		.def( init< const std::string &, const std::string &, typename T::Ptr, optional<const dict &, bool, CompoundObjectPtr > >( args( "name", "description", "defaultValue", "presets", "presetsOnly", "userData") ) )
		.def( init< const std::string &, const std::string &, typename T::Ptr, CompoundObjectPtr >( args( "name", "description", "defaultValue", "userData") ) )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( TypedObjectParameter<T> )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( TypedObjectParameter<T> )
	;

	WrapperToPython< intrusive_ptr<TypedObjectParameter<T> > >();

	INTRUSIVE_PTR_PATCH( TypedObjectParameter<T>, typename TypedObjectParameterPyClass );
	implicitly_convertible<intrusive_ptr<TypedObjectParameter<T> >, ObjectParameterPtr>();

}

void bindTypedObjectParameter()
{
	bindTypedObjectParameter<Renderable>( "RenderableParameter" );
	bindTypedObjectParameter<StateRenderable>( "StateRenderableParameter" );
	bindTypedObjectParameter<AttributeState>( "AttributeStateParameter" );
	bindTypedObjectParameter<Shader>( "ShaderParameter" );
	bindTypedObjectParameter<Transform>( "TransformParameter" );
	bindTypedObjectParameter<MatrixMotionTransform>( "MatrixMotionTransformParameter" );
	bindTypedObjectParameter<MatrixTransform>( "MatrixTransformParameter" );
	bindTypedObjectParameter<VisibleRenderable>( "VisibleRenderableParameter" );
	bindTypedObjectParameter<Group>( "GroupParameter" );
	bindTypedObjectParameter<MotionPrimitive>( "MotionPrimitiveParameter" );
	bindTypedObjectParameter<Primitive>( "PrimitiveParameter" );
	bindTypedObjectParameter<ImagePrimitive>( "ImagePrimitiveParameter" );
	bindTypedObjectParameter<MeshPrimitive>( "MeshPrimitiveParameter" );
	bindTypedObjectParameter<PointsPrimitive>( "PointsPrimitiveParameter" );	
}

} // namespace IECore
