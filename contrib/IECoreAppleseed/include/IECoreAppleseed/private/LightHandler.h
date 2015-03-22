//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_LIGHTHANDLER_H
#define IECOREAPPLESEED_LIGHTHANDLER_H

#include <map>
#include <string>

#include "boost/noncopyable.hpp"

#include "foundation/utility/searchpaths.h"

#include "renderer/api/scene.h"
#include "renderer/api/utility.h"

#include "IECore/CompoundData.h"

namespace IECoreAppleseed
{

/// The LightHandler class manages the list of lights in an
/// appleseed project, creating, editing and deleting them as needed.
class LightHandler : public boost::noncopyable
{
	public:

		LightHandler( renderer::Scene &scene, const foundation::SearchPaths &searchPaths );

		void environment( const std::string &name, const std::string &handle,
			bool visible, const IECore::CompoundDataMap &parameters );

		void light( const std::string &name, const std::string &handle,
			const foundation::Transformd &transform, const IECore::CompoundDataMap &parameters );

		void illuminate( const std::string &lightHandle, bool on );

	private:

		renderer::Scene &m_scene;
		const foundation::SearchPaths &m_searchPaths;
		renderer::Assembly *m_mainAssembly;

		// environment light
		std::string m_environmentHandle;
		std::string m_environmentModel;
		renderer::ParamArray m_environmentParams;
		bool m_environmentVisible;

		// singular lights
		struct LightEntry
		{
			std::string model;
			renderer::ParamArray parameters;
			foundation::Transformd transform;
		};

		typedef std::map<std::string, LightEntry> LightMap;
		LightMap m_lightMap;

		renderer::ParamArray convertParams( const std::string &handle,
			const IECore::CompoundDataMap &parameters, bool isEnvironment ) const;

		void createOrUpdateLight( const std::string &handle, const LightEntry &lightEntry );
		void createOrUpdateEnvironment();
};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_LIGHTHANDLER_H
