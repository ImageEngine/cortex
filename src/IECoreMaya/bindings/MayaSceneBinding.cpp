//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/MayaScene.h"
#include "IECoreMaya/bindings/MayaSceneBinding.h"

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/SceneInterfaceBinding.h"

using namespace IECoreMaya;
using namespace boost::python;

class CustomTagReader
{
	public :
		CustomTagReader( object hasFn, object readFn ) : m_has( hasFn ), m_read( readFn )
		{
		}

		bool operator() ( const MDagPath &dagPath, const IECore::SceneInterface::Name &tag )
		{
			MString p = dagPath.fullPathName();
			IECorePython::ScopedGILLock gilLock;
			return m_has( p.asChar(), tag );
		}
		
		void operator() ( const MDagPath &dagPath, IECore::SceneInterface::NameList &tags, bool includeChildren )
		{
			MString p = dagPath.fullPathName();
			IECorePython::ScopedGILLock gilLock;
			object o = m_read( p.asChar(), includeChildren );
			extract<list> l( o );
			if ( !l.check() )
			{
				throw IECore::InvalidArgumentException( std::string( "Invalid value! Expecting a list of strings." ) );
			}
			
			IECorePython::listToSceneInterfaceNameList( l(), tags );
		}

		object m_has;
		object m_read;
};

void registerCustomTags( object hasFn, object readFn )
{
	CustomTagReader reader( hasFn, readFn );
	MayaScene::registerCustomTags( reader, reader );
}

void IECoreMaya::bindMayaScene()
{
	IECorePython::RunTimeTypedClass<MayaScene>()
		.def( init<>() )
		.def( "registerCustomTags", registerCustomTags ).staticmethod( "registerCustomTags" )
	;
}
