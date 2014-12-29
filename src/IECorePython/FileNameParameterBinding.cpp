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
#include "IECore/FileNameParameter.h"
#include "IECore/CompoundObject.h"
#include "IECorePython/FileNameParameterBinding.h"
#include "IECorePython/Wrapper.h"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

class FileNameParameterWrap : public FileNameParameter, public Wrapper<FileNameParameter>
{
	public :

		FileNameParameterWrap( PyObject *self, const std::string &n, const std::string &d, const std::string &e, const std::string &dv, bool ae,
			PathParameter::CheckType c, const object &p, bool po, CompoundObjectPtr ud )
			:	FileNameParameter( n, d, e, dv, ae, c, parameterPresets<PathParameter::PresetsContainer>( p ), po, ud ), Wrapper<FileNameParameter>( self, this ) {};

		IECOREPYTHON_PARAMETERWRAPPERFNS( FileNameParameter );

};

static boost::python::list fileNameParameterExtensions( const FileNameParameter &that )
{
	boost::python::list r;
	const vector<string> &e = that.extensions();
	for( vector<string>::const_iterator it=e.begin(); it!=e.end(); it++ )
	{
		r.append( *it );
	}
	return r;
}

void bindFileNameParameter()
{
	using boost::python::arg;

	RunTimeTypedClass<FileNameParameter, FileNameParameterWrap>()
		.def(
			init<const std::string &, const std::string &, const std::string &, const std::string &, bool, PathParameter::CheckType, const object &, bool, CompoundObjectPtr>
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "extensions" ) = std::string( "" ),
					arg( "defaultValue" ) = std::string( "" ),
					arg( "allowEmptyString" ) = true,
					arg( "check" ) = PathParameter::DontCare,
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.add_property( "extensions", &fileNameParameterExtensions )
		.IECOREPYTHON_DEFPARAMETERWRAPPERFNS( FileNameParameter )
	;

}

} // namespace IECorePython
