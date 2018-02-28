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

#ifndef IECOREHOUDINI_DETAILSPLITTER_H
#define IECOREHOUDINI_DETAILSPLITTER_H

#include "IECoreScene/SceneInterface.h"

#include "IECore/PathMatcherData.h"
#include "IECore/RefCounted.h"

#include "GU/GU_DetailHandle.h"

#include <map>
#include <string>
#include <unordered_map>

namespace IECoreHoudini
{



/// DetailSplitter is a convenience class for extracting select bits of geometry
/// from a GU_Detail. It is intended to improve performance when making multiple
/// calls to split the same detail. The default use is splitting based on the name
/// attribute, but any primitive string attribute could be used.
class DetailSplitter : public IECore::RefCounted
{
	public :

		typedef std::vector<std::string> Names;

		IE_CORE_DECLAREMEMBERPTR( DetailSplitter );

		/// Create a DetailSplitter which will split the handle by the given key.
		/// @param key The name of a primitive string attribute on the GU_Detail.
		DetailSplitter( const GU_DetailHandle &handle, const std::string &key = "name", bool cortexSegment = false );

		virtual ~DetailSplitter();

		/// Creates and returns a handle to a new GU_Detail which contains only
		/// the primitives that match the value requested.
		const GU_DetailHandle split( const std::string &value );

		/// Retrieve the locally split object if possible
		/// Can be null and split which returns the detail handle
		/// should be used to convert the geometry.
		IECore::ObjectPtr splitObject( const std::string& value );

		/// Fills the result vector with all valid values in the GU_Detail
		void values( std::vector<std::string> &result );

		/// Returns the handle held by the splitter
		const GU_DetailHandle &handle() const;

		// Returns the child names for a given path
		Names getNames(const std::vector<IECore::InternedString>& path);


		bool hasPath( const IECoreScene::SceneInterface::Path& path, bool isExplicit = true );
	private :

		bool validate();


		typedef std::map<std::string, GU_DetailHandle> Cache;

		int m_lastMetaCount;
		const std::string m_key;
		const GU_DetailHandle m_handle;
		Cache m_cache;
		IECore::PathMatcherDataPtr m_pathMatcher;

		std::unordered_map<std::string, IECore::ObjectPtr> m_segmentMap;
		std::vector<std::string> m_names;
		bool m_cortexSegment;
};

IE_CORE_DECLAREPTR( DetailSplitter );

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_DETAILSPLITTER_H
