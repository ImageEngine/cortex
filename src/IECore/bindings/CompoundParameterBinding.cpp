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
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/ParameterBinding.h"
#include "IECore/bindings/Wrapper.h"

using namespace boost;
using namespace boost::python;

namespace IECore
{

class CompoundParameterWrap : public CompoundParameter, public Wrapper< CompoundParameter >
{
	public:

		IE_CORE_DECLAREMEMBERPTR( CompoundParameterWrap );

	protected:

		static std::vector<ParameterPtr> getMembers( const object &members )
		{
			std::vector<ParameterPtr> m;
			for( int i=0; i<members.attr("__len__")(); i++ )
			{
				object o = members[i];
				Parameter &p = extract<Parameter &>( o );
				m.push_back( &p );
			}
			return m;
		}

		void addParametersFromMembers( const object &members )
		{
			for( int i=0; i<members.attr("__len__")(); i++ )
			{
				object o = members[i];
				Parameter &p = extract<Parameter &>( o );
				this->addParameter( &p );
			}
		}

	public :

		CompoundParameterWrap( PyObject *self, const std::string &name = "", const std::string &description = "", const list &members = list(), CompoundObjectPtr userData = 0 )
			:	CompoundParameter( name, description, userData ), Wrapper< CompoundParameter >( self, this )
		{
			addParametersFromMembers( members );
		}

		IE_COREPYTHON_PARAMETERWRAPPERFNS( CompoundParameter );
};

static unsigned int compoundParameterLen( const CompoundParameter &o )
{
	return o.parameters().size();
}

static ParameterPtr compoundParameterGetItem( CompoundParameter &o, const char *n )
{
	const CompoundParameter::ParameterMap &p = o.parameters();
	CompoundParameter::ParameterMap::const_iterator it = p.find( n );
	if( it!=p.end() )
	{
		return it->second;
	}

	throw Exception( std::string("Bad index: ") + n );
}

static ParameterPtr compoundParameterGetAttr( CompoundParameter &o, const char *n )
{
	if( PyErr_WarnEx( PyExc_DeprecationWarning, "Access to CompoundParameter children as attributes is deprecated - please use item style access instead.", 1 ) )
	{
		// warning converted to exception;
		throw error_already_set();
	}
	return compoundParameterGetItem( o, n );
}

static bool compoundParameterContains( const CompoundParameter &o, const std::string &n )
{
	return o.parameter<const Parameter>( n );
}

static boost::python::list compoundParameterKeys( const CompoundParameter &o )
{
	boost::python::list result;
	CompoundParameter::ParameterVector::const_iterator it;
	for( it = o.orderedParameters().begin(); it!=o.orderedParameters().end(); it++ )
	{
		result.append( (*it)->name() );
	}
	return result;
}

static boost::python::list compoundParameterValues( const CompoundParameter &o )
{
	boost::python::list result;
	CompoundParameter::ParameterVector::const_iterator it;
	for( it = o.orderedParameters().begin(); it!=o.orderedParameters().end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

static void compoundParameterAddParameters( CompoundParameter &o, const boost::python::list &p )
{
	std::vector<ParameterPtr> pp;
	boost::python::container_utils::extend_container( pp, p );
	o.addParameters( pp.begin(), pp.end() );
}

static boost::python::list parameterPath( CompoundParameter &o, ConstParameterPtr child )
{
	std::vector<std::string> p;
	o.parameterPath( child, p );
	boost::python::list result;
	for( std::vector<std::string>::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

void bindCompoundParameter()
{
	using boost::python::arg ;

	RunTimeTypedClass<CompoundParameter, CompoundParameterWrap::Ptr>()
		.def(
			init< const std::string &, const std::string &, boost::python::optional<const list &, CompoundObjectPtr > >
			(
				(
					arg( "name" ) = std::string(""),
					arg( "description" ) = std::string(""),
					arg( "members" ) = list(),
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.def( "__len__", &compoundParameterLen )
		.def( "__getitem__", &compoundParameterGetItem )
		/// \todo Remove attribute style access in major version 5.
		.def( "__getattr__", &compoundParameterGetAttr )
		.def( "__contains__", &compoundParameterContains )
		.def( "keys", &compoundParameterKeys )
		.def( "values", &compoundParameterValues )
		.def( "has_key", &compoundParameterContains )
		.def( "addParameter", &CompoundParameter::addParameter )
		.def( "addParameters", &compoundParameterAddParameters )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( CompoundParameter )
		.def( "insertParameter", &CompoundParameter::insertParameter )
		.def( "removeParameter", (void (CompoundParameter::*)(ParameterPtr)) &CompoundParameter::removeParameter )
		.def( "removeParameter", (void (CompoundParameter::*)(const std::string&)) &CompoundParameter::removeParameter )
		.def( "parameter", (ParameterPtr (CompoundParameter::*)(const std::string&)) &CompoundParameter::parameter<Parameter> )
		.def( "parameterPath", &parameterPath )
	;

}

} // namespace IECore
