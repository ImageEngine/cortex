//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_SETCOLLECTOR_H
#define IECORESCENE_SETCOLLECTOR_H

#include <unordered_map>

#include "IECore/PathMatcherData.h"

#include "IECoreScene/SceneInterface.h"

namespace IECoreScene
{

struct IECORESCENE_API SetCollector
{
	//! add a single path to the named set
	void addPath( const IECore::InternedString &name, const IECoreScene::SceneInterface::Path &path );
	//! add multiple paths using a path matcher to a named set
	void addPaths( const IECore::InternedString &name, const IECore::PathMatcher &paths, const IECoreScene::SceneInterface::Path &prefix );
	//!  Get the paths added ot a particular set
	IECore::PathMatcher paths( const IECore::InternedString &name ) const;
	//! All the set names collected.
	IECoreScene::SceneInterface::NameList names() const;

	typedef std::unordered_map<IECore::InternedString, IECore::PathMatcherDataPtr> TSetCollection;
	TSetCollection setCollection;
};

}

#endif  //IECORESCENE_SETCOLLECTOR_H

