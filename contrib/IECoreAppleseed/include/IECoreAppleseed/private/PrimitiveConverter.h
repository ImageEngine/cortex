//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_PRIMITIVECONVERTER_H
#define IECOREAPPLESEED_PRIMITIVECONVERTER_H

#include <map>
#include <set>

#include "boost/noncopyable.hpp"

#include "renderer/api/scene.h"
#include "renderer/api/object.h"

#include "IECore/MurmurHash.h"
#include "IECore/Primitive.h"

#include "IECoreAppleseed/private/AttributeState.h"

namespace IECoreAppleseed
{

/// An abstract base class for managing the conversion of a series of IECore::Primitives to
/// appleseed entities, automatically creating instances when a previously
/// converted primitive is processed again.
class PrimitiveConverter : boost::noncopyable
{

	public :

		explicit PrimitiveConverter( const foundation::SearchPaths &searchPaths );

		virtual ~PrimitiveConverter();

		virtual void setOption( const std::string &name, IECore::ConstDataPtr value );

		const renderer::Assembly *convertPrimitive( IECore::PrimitivePtr primitive, const AttributeState &attrState, const std::string &materialName, renderer::Assembly &parentAssembly );

		const renderer::Assembly *convertPrimitive( const std::set<float> &times,
			const std::vector<IECore::PrimitivePtr> &primitives, const AttributeState &attrState,
			const std::string &materialName, renderer::Assembly &parentAssembly );

	private :

		void createObjectInstance( renderer::Assembly &assembly, const renderer::Object *obj,
			const std::string &objSourceName, const AttributeState &attrState, const std::string &materialName );

		virtual foundation::auto_release_ptr<renderer::Object> doConvertPrimitive( IECore::PrimitivePtr primitive, const std::string &name ) = 0;

		typedef std::map<IECore::MurmurHash, const renderer::Assembly*> InstanceMapType;

		const foundation::SearchPaths &m_searchPaths;
		InstanceMapType m_instanceMap;
		bool m_autoInstancing;

};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_PRIMITIVECONVERTER_H
