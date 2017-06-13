//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECore/ObjectParameter.h"
#include "IECore/Object.h"
#include "IECore/CompoundObject.h"
#include "IECorePython/ObjectParameterBinding.h"
#include "IECorePython/ParameterBinding.h"

#include <vector>
#include <algorithm>

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;

namespace
{

class ObjectParameterWrapper : public ParameterWrapper<ObjectParameter>
{

	protected:

		/// Allow construction from either a string, StringData, or a FrameList
		static ObjectParameter::TypeIdSet makeTypes( object types )
		{
			vector<TypeId> tv;
			boost::python::container_utils::extend_container( tv, types );
			ObjectParameter::TypeIdSet t;
			copy( tv.begin(), tv.end(), insert_iterator<ObjectParameter::TypeIdSet>( t, t.begin() ) );
			return t;
		}

	public :

		ObjectParameterWrapper(
			PyObject *self, const std::string &n, const std::string &d, ObjectPtr dv, TypeId t,
			const object &p = boost::python::tuple(), bool po = false, IECore::CompoundObjectPtr ud = 0
		)
			: ParameterWrapper<ObjectParameter>( self, n, d, dv, t, parameterPresets<Parameter::PresetsContainer>( p ), po, ud )
		{
		};

		ObjectParameterWrapper(
			PyObject *self, const std::string &n, const std::string &d, ObjectPtr dv, const boost::python::list &ts,
			const object &p = boost::python::tuple(), bool po = false, IECore::CompoundObjectPtr ud = 0
		)
			: ParameterWrapper<ObjectParameter>( self, n, d, dv, makeTypes( ts ), parameterPresets<Parameter::PresetsContainer>( p ), po, ud )
		{
		};

};

static boost::python::list validTypes( ObjectParameter &o )
{
	boost::python::list result;
	for( ObjectParameter::TypeIdSet::const_iterator it=o.validTypes().begin(); it!=o.validTypes().end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

} // namespace

namespace IECorePython
{

void bindObjectParameter()
{
	using boost::python::arg;

	ParameterClass<ObjectParameter, ObjectParameterWrapper>()
		.def(
			init< const std::string &, const std::string &, ObjectPtr, const TypeId &, boost::python::optional<const object &, bool, CompoundObjectPtr > >
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ),
					arg( "type" ),
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false ,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.def(
			init< const std::string &, const std::string &, ObjectPtr, const boost::python::list &, boost::python::optional<const object &, bool, CompoundObjectPtr > >
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ),
					arg( "types" ),
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false ,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.def( "validTypes", &validTypes )
	;

}

} // namespace IECorePython
