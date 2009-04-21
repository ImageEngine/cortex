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

#include "boost/python.hpp"

#include "IECore/bindings/ParameterBinding.h"
#include "IECore/bindings/Wrapper.h"
#include "IECore/TypedParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"
#include "IECore/SplineParameter.h"
#include "IECore/CubeColorLookupParameter.h"
#include "IECore/DateTimeParameter.h"
#include "IECore/TimeDurationParameter.h"
#include "IECore/TimePeriodParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace Imath;

namespace IECore
{

template<typename T>
class TypedParameterWrap : public TypedParameter<T>, public Wrapper< TypedParameter<T> >
{
	public:
	
		IE_CORE_DECLAREMEMBERPTR( TypedParameterWrap<T> );
	
	protected:

		static typename TypedData<T>::Ptr makeDefault( object defaultValue )
		{
			typename TypedData<T>::Ptr defaultData;
			extract<T> de( defaultValue );
			if( de.check() )
			{
				defaultData = new TypedData<T>( de() );
			}
			else
			{
				defaultData = extract<TypedData<T> *>( defaultValue )();
			}
			return defaultData;
		}

	public :

		TypedParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0 )	
			:	TypedParameter<T>( n, d, makeDefault( dv ), parameterPresets<typename TypedParameter<T>::ObjectPresetsContainer>( p ), po, ud ), Wrapper< TypedParameter<T> >( self, this ) {};

		IE_COREPYTHON_PARAMETERWRAPPERFNS( TypedParameter<T> );
};

template<typename T>
static void bindTypedParameter( const char *name )
{
	using boost::python::arg;
	
	typedef class_< TypedParameter<T>, typename TypedParameterWrap< T >::Ptr, boost::noncopyable, bases<Parameter> > TypedParameterPyClass;
	TypedParameterPyClass( name, no_init )
		.def( 
			init< const std::string &, const std::string &, object, boost::python::optional<const object &, bool, CompoundObjectPtr > >
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
		.def( "getTypedValue", (const T &(TypedParameter<T>::* )() const)&TypedParameter<T>::getTypedValue, return_value_policy<copy_const_reference>() )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( TypedParameter<T> )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( TypedParameter<T> )
	;

	WrapperToPython< typename TypedParameter<T>::Ptr >();

	INTRUSIVE_PTR_PATCH( TypedParameter<T>, typename TypedParameterPyClass );
	implicitly_convertible<typename TypedParameter<T>::Ptr, ParameterPtr>();

}

void bindTypedParameter()
{
	/// \todo Split some of these off into separate files to speed up compilation
	bindTypedParameter<bool>( "BoolParameter" );
	bindTypedParameter<V2i>( "V2iParameter" );
	bindTypedParameter<V3i>( "V3iParameter" );
	bindTypedParameter<V2f>( "V2fParameter" );
	bindTypedParameter<V3f>( "V3fParameter" );
	bindTypedParameter<V2d>( "V2dParameter" );
	bindTypedParameter<V3d>( "V3dParameter" );
	bindTypedParameter<Color3f>( "Color3fParameter" );
	bindTypedParameter<Color4f>( "Color4fParameter" );
	bindTypedParameter<Box2i>( "Box2iParameter" );
	bindTypedParameter<Box3i>( "Box3iParameter" );
	bindTypedParameter<Box2f>( "Box2fParameter" );
	bindTypedParameter<Box3f>( "Box3fParameter" );
	bindTypedParameter<Box2d>( "Box2dParameter" );
	bindTypedParameter<Box3d>( "Box3dParameter" );
	bindTypedParameter<M44f>( "M44fParameter" );
	bindTypedParameter<M44d>( "M44dParameter" );
	bindTypedParameter<string>( "StringParameter" );	
	bindTypedParameter<Splineff>( "SplineffParameter" );
	bindTypedParameter<Splinedd>( "SplineddParameter" );
	bindTypedParameter<SplinefColor3f>( "SplinefColor3fParameter" );
	bindTypedParameter<SplinefColor4f>( "SplinefColor4fParameter" );
	bindTypedParameter<CubeColorLookupf>( "CubeColorLookupfParameter" );
	bindTypedParameter<CubeColorLookupd>( "CubeColorLookupdParameter" );	
	bindTypedParameter<boost::posix_time::ptime>( "DateTimeParameter" );	
	bindTypedParameter<boost::posix_time::time_duration>( "TimeDurationParameter" );	

	bindTypedParameter<vector<bool> >( "BoolVectorParameter" );
	bindTypedParameter<vector<int> >( "IntVectorParameter" );
	bindTypedParameter<vector<float> >( "FloatVectorParameter" );
	bindTypedParameter<vector<double> >( "DoubleVectorParameter" );
	bindTypedParameter<vector<string> >( "StringVectorParameter" );
	bindTypedParameter<vector<V2f> >( "V2fVectorParameter" );
	bindTypedParameter<vector<V3f> >( "V3fVectorParameter" );
	bindTypedParameter<vector<V2d> >( "V2dVectorParameter" );
	bindTypedParameter<vector<V3d> >( "V3dVectorParameter" );
	bindTypedParameter<vector<Box3f> >( "Box3fVectorParameter" );
	bindTypedParameter<vector<Box3d> >( "Box3dVectorParameter" );
	bindTypedParameter<vector<M33f> >( "M33fVectorParameter" );
	bindTypedParameter<vector<M44f> >( "M44fVectorParameter" );
	bindTypedParameter<vector<M33d> >( "M33dVectorParameter" );
	bindTypedParameter<vector<M44d> >( "M44dVectorParameter" );
	bindTypedParameter<vector<Quatf> >( "QuatfVectorParameter" );
	bindTypedParameter<vector<Quatd> >( "QuatdVectorParameter" );
	bindTypedParameter<vector<Color3f> >( "Color3fVectorParameter" );
	bindTypedParameter<vector<Color4f> >( "Color4fVectorParameter" );
}

} // namespace IECore
