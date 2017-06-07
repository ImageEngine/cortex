//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/ParameterBinding.h"
#include "IECore/NumericParameter.h"
#include "IECore/CompoundObject.h"

#include "IECorePython/NumericParameterBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;

namespace
{

template<typename T>
class NumericParameterWrapper : public ParameterWrapper<NumericParameter<T> >
{
	public :

		NumericParameterWrapper(
			PyObject *self, const std::string &n, const std::string &d, T v = T(), T minValue = Imath::limits<T>::min(),
			T maxValue = Imath::limits<T>::max(), const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0
		)
			: ParameterWrapper<NumericParameter<T> >( self, n, d, v, minValue, maxValue, parameterPresets<typename NumericParameter<T>::PresetsContainer>( p ), po, ud )
		{
		};

};

} //namespace

namespace IECorePython
{

template<typename T>
static void bindNumericParameter()
{
	using boost::python::arg;

	ParameterClass<NumericParameter<T>, NumericParameterWrapper<T> >()
		.def(
			init<const std::string &, const std::string &, boost::python::optional< T, T, T, const object &, bool, CompoundObjectPtr> >
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ) = T(),
					arg( "minValue" ) = Imath::limits<T>::min(),
					arg( "maxValue" ) = Imath::limits<T>::max(),
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.add_property( "numericDefaultValue", &NumericParameter<T>::numericDefaultValue  )
		.def( "getNumericValue", &NumericParameter<T>::getNumericValue  )
		.def( "setNumericValue", &NumericParameter<T>::setNumericValue  )
		.def( "getTypedValue", &NumericParameter<T>::getNumericValue )		// added for consistency
		.def( "setTypedValue", &NumericParameter<T>::setNumericValue )		// added for consistency
		.def( "hasMinValue", &NumericParameter<T>::hasMinValue )
		.def( "hasMaxValue", &NumericParameter<T>::hasMaxValue )
		.add_property( "minValue", &NumericParameter<T>::minValue )
		.add_property( "maxValue", &NumericParameter<T>::maxValue )
	;
}

void bindNumericParameter()
{
	bindNumericParameter<int>();
	bindNumericParameter<float>();
	bindNumericParameter<double>();
}

} // namespace IECorePython
