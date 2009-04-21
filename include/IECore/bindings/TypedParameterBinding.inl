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

#ifndef IE_COREPYTHON_TYPEDPARAMETERBINDING_INL
#define IE_COREPYTHON_TYPEDPARAMETERBINDING_INL

#include "IECore/TypedParameter.h"
#include "IECore/CompoundObject.h"

#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/ParameterBinding.h"

namespace IECore
{

template<typename T>
class TypedParameterWrap : public TypedParameter<T>, public Wrapper< TypedParameter<T> >
{
	public:
	
		IE_CORE_DECLAREMEMBERPTR( TypedParameterWrap<T> );
	
	protected:

		static typename TypedData<T>::Ptr makeDefault( boost::python::object defaultValue )
		{
			typename TypedData<T>::Ptr defaultData;
			boost::python::extract<T> de( defaultValue );
			if( de.check() )
			{
				defaultData = new TypedData<T>( de() );
			}
			else
			{
				defaultData = boost::python::extract<TypedData<T> *>( defaultValue )();
			}
			return defaultData;
		}

	public :

		TypedParameterWrap( PyObject *self, const std::string &n, const std::string &d, boost::python::object dv, const boost::python::object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0 )	
			:	TypedParameter<T>( n, d, makeDefault( dv ), parameterPresets<typename TypedParameter<T>::ObjectPresetsContainer>( p ), po, ud ), Wrapper< TypedParameter<T> >( self, this ) {};

		IE_COREPYTHON_PARAMETERWRAPPERFNS( TypedParameter<T> );
};

template<typename T>
void bindTypedParameter( const char *name )
{
	using boost::python::arg;
	
	typedef boost::python::class_< TypedParameter<T>, typename TypedParameterWrap< T >::Ptr, boost::noncopyable, boost::python::bases<Parameter> > TypedParameterPyClass;
	TypedParameterPyClass( name, boost::python::no_init )
		.def( 
			boost::python::init< const std::string &, const std::string &, boost::python::object, boost::python::optional<const boost::python::object &, bool, CompoundObjectPtr > >
			( 
				( 
					arg( "name" ), 
					arg( "description" ), 
					arg( "defaultValue" ),
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false , 
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				) 
			)
		) 
		.def( "setTypedValue", &TypedParameter<T>::setTypedValue )
		.def( "getTypedValue", (const T &(TypedParameter<T>::* )() const)&TypedParameter<T>::getTypedValue, boost::python::return_value_policy<boost::python::copy_const_reference>() )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( TypedParameter<T> )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( TypedParameter<T> )
	;

	WrapperToPython< typename TypedParameter<T>::Ptr >();

	INTRUSIVE_PTR_PATCH( TypedParameter<T>, typename TypedParameterPyClass );
	boost::python::implicitly_convertible<typename TypedParameter<T>::Ptr, ParameterPtr>();

}

}

#endif // IE_COREPYTHON_TYPEDPARAMETERBINDING_INL
