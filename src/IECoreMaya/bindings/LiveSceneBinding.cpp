//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "maya/MString.h"

#include "boost/python.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECoreMaya/LiveScene.h"
#include "IECoreMaya/bindings/LiveSceneBinding.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILLock.h"

using namespace IECoreMaya;
using namespace boost::python;

namespace
{

class CustomTagReader
{
	public :
		CustomTagReader( object hasFn, object readFn ) : m_has( hasFn ), m_read( readFn )
		{
		}

		bool operator() ( const MDagPath &dagPath, const IECoreScene::SceneInterface::Name &tag, int filter )
		{
			MString p = dagPath.fullPathName();
			IECorePython::ScopedGILLock gilLock;
			try
			{
				return m_has( p.asChar(), tag, filter );
			}
			catch ( error_already_set )
			{
				PyErr_Print();
				throw IECore::Exception( std::string( "Python exception while checking IECoreMaya::LiveScene tag " + tag.string() ) );
			}
		}

		void operator() ( const MDagPath &dagPath, IECoreScene::SceneInterface::NameList &tags, int filter )
		{
			MString p = dagPath.fullPathName();
			IECorePython::ScopedGILLock gilLock;
			object o;
			try
			{
				o = m_read( p.asChar(), filter );
			}
			catch ( error_already_set )
			{
				PyErr_Print();
				throw IECore::Exception( std::string( "Python exception while evaluating IECoreMaya::LiveScene tags" ) );
			}
			extract<list> l( o );
			if ( !l.check() )
			{
				throw IECore::InvalidArgumentException( std::string( "Invalid value! Expecting a list of strings." ) );
			}

			container_utils::extend_container( tags, l() );
		}

		object m_has;
		object m_read;
};

void registerCustomTags( object hasFn, object readFn )
{
	CustomTagReader reader( hasFn, readFn );
	LiveScene::registerCustomTags( reader, reader );
}

class CustomAttributeReader
{
	public :
		CustomAttributeReader( object namesFn, object readFn ) : m_names( namesFn ), m_read( readFn )
		{
		}

		IECore::ConstObjectPtr operator() ( const MDagPath &dagPath, const IECoreScene::SceneInterface::Name &attr )
		{
			MString p = dagPath.fullPathName();
			IECorePython::ScopedGILLock gilLock;
			try
			{
				return extract<IECore::ConstObjectPtr>(m_read( p.asChar(), attr ));
			}
			catch ( error_already_set )
			{
				PyErr_Print();
				throw IECore::Exception( std::string( "Python exception while evaluating IECoreMaya::LiveScene attribute " + attr.string() ) );
			}
		}

		void operator() ( const MDagPath &dagPath, IECoreScene::SceneInterface::NameList &attributes )
		{
			MString p = dagPath.fullPathName();
			IECorePython::ScopedGILLock gilLock;
			object o;
			try
			{
				o = m_names( p.asChar() );
			}
			catch ( error_already_set )
			{
				PyErr_Print();
				throw IECore::Exception( std::string( "Python exception while evaluating attribute names for IECoreMaya::LiveScene." ) );
			}

			extract<list> l( o );
			if ( !l.check() )
			{
				throw IECore::InvalidArgumentException( std::string( "Invalid value! Expecting a list of strings." ) );
			}

			container_utils::extend_container( attributes, l() );
		}

		object m_names;
		object m_read;
};

class CustomAttributeReaderMightHave
{
	public :
		CustomAttributeReaderMightHave( object mightHaveFn ) : m_mightHave( mightHaveFn )
		{
		}

		bool operator() ( const MDagPath &dagPath, const IECoreScene::SceneInterface::Name &attr )
		{
			// Use with care when registering a Python function.
			// MDagPath::fullPathName() is a slow API. We allow registering the "might have" callback from Python
			// for compatibility with IECoreMaya C++ API but registering a Python "might have" function may spoil the efficiency.
			MString p = dagPath.fullPathName();
			IECorePython::ScopedGILLock gilLock;
			try
			{
				return m_mightHave( p.asChar(), attr );
			}
			catch ( error_already_set )
			{
				PyErr_Print();
				throw IECore::Exception( std::string( "Python exception while evaluating IECoreMaya::LiveScene attribute  " + attr.string() ) );
			}
		}

		object m_mightHave;
};

void registerCustomAttributes( object namesFn, object readFn, object mightHaveFn )
{
	CustomAttributeReader reader( namesFn, readFn );

	if ( mightHaveFn == object() )
	{
		LiveScene::registerCustomAttributes( reader, reader );
	}
	else
	{
		CustomAttributeReaderMightHave readerm( mightHaveFn );
		LiveScene::registerCustomAttributes( reader, reader, readerm );
	}
}

} // namespace

void IECoreMaya::bindLiveScene()
{
	IECorePython::RunTimeTypedClass<LiveScene>()
		.def( init<>() )
		.def( "registerCustomTags", registerCustomTags, ( arg_( "hasFn" ), arg_( "readFn" ) ) ).staticmethod( "registerCustomTags" )
		.def( "registerCustomAttributes", registerCustomAttributes, (  arg_( "namesFn" ), arg_( "readFn" ), arg_( "mightHaveFn" ) = object() ) ).staticmethod( "registerCustomAttributes" )
		.def( "toMayaAttributeName", LiveScene::toMayaAttributeName ).staticmethod( "toMayaAttributeName" )
		.def( "fromMayaAttributeName", LiveScene::fromMayaAttributeName ).staticmethod( "fromMayaAttributeName" )
		.def_readonly("visibilityOverrideName", LiveScene::visibilityOverrideName )
	;
}
