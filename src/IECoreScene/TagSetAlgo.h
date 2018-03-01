//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_TAGSETALGO_H
#define IECORESCENE_TAGSETALGO_H

#include "IECoreScene/SceneInterface.h"

namespace IECoreScene
{
	namespace Private
	{
		void loadSetWalk( const IECoreScene::SceneInterface *scene, const IECore::InternedString &setName, IECore::PathMatcher &set, const SceneInterface::Path &path )
		{
			if( scene->hasTag( setName, SceneInterface::LocalTag ) )
			{
				set.addPath( path );
			}

			// Figure out if we need to recurse by querying descendant tags to see if they include
			// anything we're interested in.

			if( !scene->hasTag( setName, SceneInterface::DescendantTag ) )
			{
				return;
			}

			// Recurse to the children.

			SceneInterface::NameList childNames;
			scene->childNames( childNames );
			SceneInterface::Path childPath( path );
			childPath.push_back( IECore::InternedString() ); // room for the child name
			for( SceneInterface::NameList::const_iterator it = childNames.begin(), eIt = childNames.end(); it != eIt; ++it )
			{
				ConstSceneInterfacePtr child = scene->child( *it );
				childPath.back() = *it;
				loadSetWalk( child.get(), setName, set, childPath );
			}
		}
	} // private

} // IECoreScene

#endif //IECORESCENE_TAGSETALGO_H
