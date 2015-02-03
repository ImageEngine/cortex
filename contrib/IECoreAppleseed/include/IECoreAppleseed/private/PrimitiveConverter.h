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

#include "boost/noncopyable.hpp"
#include "boost/filesystem/path.hpp"

#include "renderer/api/scene.h"
#include "renderer/api/object.h"

#include "IECore/MurmurHash.h"
#include "IECore/Primitive.h"

#include "IECoreAppleseed/private/PrimitiveConverter.h"
#include "IECoreAppleseed/private/AttributeState.h"

namespace IECoreAppleseed
{

/// A class for managing the conversion of a series of IECore::Primitives to
/// appleseed entities, automatically creating instances when a previously
/// converted primitive is processed again.
class PrimitiveConverter : boost::noncopyable
{

	public :

		typedef enum
		{
			BinaryMeshFormat,
			ObjFormat
		} MeshFileFormat;

		explicit PrimitiveConverter( const boost::filesystem::path &projectPath );

		void setMeshFileFormat( MeshFileFormat format );

		const renderer::Assembly *convertPrimitive( IECore::PrimitivePtr primitive, const AttributeState &attrState, const std::string &materialName, renderer::Assembly &parentAssembly, const foundation::SearchPaths &searchPaths );

	private :

		foundation::auto_release_ptr<renderer::Object> convertAndWriteMeshPrimitive( IECore::PrimitivePtr primitive, const IECore::MurmurHash &meshHash );

		void createObjectInstance( renderer::Assembly &assembly, const renderer::Object *obj, const std::string &objSourceName, const std::string &materialName );

		typedef std::map<IECore::MurmurHash, const renderer::Assembly*> InstanceMapType;

		boost::filesystem::path m_projectPath;
		std::string m_meshGeomExtension;
		InstanceMapType m_instanceMap;
		bool m_interactive;

};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_PRIMITIVECONVERTER_H
