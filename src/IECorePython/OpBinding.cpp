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

#include "IECore/Op.h"
#include "IECore/Parameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/CompoundObject.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/Wrapper.h"
#include "IECorePython/ScopedGILRelease.h"
#include "IECorePython/ScopedGILLock.h"

using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

class OpWrap : public Op, public Wrapper<Op>
{
	public :

		OpWrap( PyObject *self, const std::string &description, ParameterPtr resultParameter ) : Op( description, resultParameter ), Wrapper<Op>( self, this ) {};

		OpWrap( PyObject *self, const std::string &description, CompoundParameterPtr compoundParameter, ParameterPtr resultParameter ) : Op( description, compoundParameter, resultParameter ), Wrapper<Op>( self, this ) {};

		virtual ObjectPtr doOperation( const CompoundObject * operands )
		{
			ScopedGILLock gilLock;
			override o = this->get_override( "doOperation" );
			if( o )
			{
				ObjectPtr r = o( CompoundObjectPtr( const_cast<CompoundObject *>( operands ) ) );
				if( !r )
				{
					throw Exception( "doOperation() python method didn't return an Object." );
				}
				return r;
			}
			else
			{
				throw Exception( "doOperation() python method not defined" );
			}
		};

		IECOREPYTHON_RUNTIMETYPEDWRAPPERFNS( Op );
		
};

static ParameterPtr resultParameter( const Op &o )
{
	return const_cast<Parameter *>( o.resultParameter() );
}

static ObjectPtr operate( Op &op )
{
	ScopedGILRelease gilRelease;
	ObjectPtr result = op.operate();
	return result;
}

static ObjectPtr operateWithArgs( Op &op, const CompoundObject *args )
{
	ScopedGILRelease gilRelease;
	ObjectPtr result = op.operate(args);
	return result;
}

void bindOp()
{
	using boost::python::arg;

	RunTimeTypedClass<Op, OpWrap>()
		.def( init< const std::string &, ParameterPtr >( ( arg( "description" ), arg( "resultParameter") ) ) )
		.def( init< const std::string &, CompoundParameterPtr, ParameterPtr >( ( arg( "description" ), arg( "compoundParameter" ), arg( "resultParameter") ) ) )
		.def( "resultParameter", &resultParameter )
		.def( "operate", &operate )
		.def( "operate", &operateWithArgs )
		.def( "__call__", &operate )
		.def( "__call__", &operateWithArgs )
	;

}

} // namespace IECorePython
