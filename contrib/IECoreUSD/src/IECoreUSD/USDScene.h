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

#ifndef IECOREUSD_USDSCENE_H
#define IECOREUSD_USDSCENE_H

#include "TypeIds.h"

#include "IECoreScene/SceneInterface.h"

#include "IECore/PathMatcherData.h"

namespace IECoreUSD
{

class USDScene : public IECoreScene::SceneInterface
{
	public:
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( USDScene, IECoreUSD::USDSceneTypeId, IECoreScene::SceneInterface )
		USDScene( const std::string &path, IECore::IndexedIO::OpenMode &mode );

		~USDScene() override;

		std::string fileName() const override;

		Imath::Box3d readBound( double time ) const override;
		IECore::ConstDataPtr readTransform( double time ) const override;
		Imath::M44d readTransformAsMatrix( double time ) const override;
		IECore::ConstObjectPtr readAttribute( const Name &name, double time ) const override;
		IECore::ConstObjectPtr readObject( double time ) const override;

		Name name() const override;
		void path( Path &p ) const override;
		bool hasBound() const override;
		void writeBound( const Imath::Box3d &bound, double time ) override;
		void writeTransform( const IECore::Data *transform, double time ) override;
		bool hasAttribute( const Name &name ) const override;
		void attributeNames( NameList &attrs ) const override;
		void writeAttribute( const Name &name, const IECore::Object *attribute, double time ) override;

		bool hasTag( const Name &name, int filter ) const override;
		void readTags( NameList &tags, int filter ) const override;
		void writeTags( const NameList &tags ) override;

		NameList setNames( bool includeDescendantSets = true ) const override;
		IECore::PathMatcher readSet( const Name &name, bool includeDescendantSets = true ) const override;
		void writeSet( const Name &name, const IECore::PathMatcher &set ) override;
		void hashSet( const Name &name, IECore::MurmurHash &h ) const override;

		bool hasObject() const override;
		IECoreScene::PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const override;
		void writeObject( const IECore::Object *object, double time ) override;
		bool hasChild( const Name &name ) const override;
		void childNames( NameList &childNames ) const override;
		IECoreScene::SceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour ) override;
		IECoreScene::ConstSceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour ) const override;
		IECoreScene::SceneInterfacePtr createChild( const Name &name ) override;
		IECoreScene::SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour ) override;
		IECoreScene::ConstSceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour ) const override;
		void hash( HashType hashType, double time, IECore::MurmurHash &h ) const override;

	private:
		IE_CORE_FORWARDDECLARE( IO );
		IE_CORE_FORWARDDECLARE( Reader );
		IE_CORE_FORWARDDECLARE( Writer );
		IE_CORE_FORWARDDECLARE( Location );

		USDScene( IOPtr root, LocationPtr location);

		void boundHash( double time, IECore::MurmurHash &h ) const;
		void transformHash( double time, IECore::MurmurHash &h ) const;
		void attributeHash ( double time, IECore::MurmurHash &h) const;
		void objectHash( double time, IECore::MurmurHash &h ) const;
		void childNamesHash( double time, IECore::MurmurHash &h ) const;
		void hierarchyHash( double time, IECore::MurmurHash &h ) const;

		void recurseReadSet( const Path &prefix, const Name &name, IECore::PathMatcher &pathMatcher, bool includeDescendantSets ) const;
		IECore::PathMatcherDataPtr readLocalSet( const Name &name ) const;

		IOPtr m_root;
		LocationPtr m_location;
};

IE_CORE_DECLAREPTR( USDScene )

}

#endif // IECOREUSD_USDSCENE_H
