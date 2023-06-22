//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2022, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORENUKE_LIVESCENE_H
#define IECORENUKE_LIVESCENE_H

#include "DDImage/GeoOp.h"
#include "DDImage/GeometryList.h"

#include "IECore/PathMatcher.h"

#include "IECoreScene/SceneInterface.h"

#include "IECoreNuke/Export.h"
#include "IECoreNuke/TypeIds.h"

namespace IECoreNuke
{

IE_CORE_FORWARDDECLARE( LiveScene );

/// A read-only class for representing a live Nuke scene as an IECore::SceneInterface
class IECORENUKE_API LiveScene : public IECoreScene::SceneInterface
{
	public :

		static const std::string& nameAttribute;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( LiveScene, LiveSceneTypeId, IECoreScene::SceneInterface );

		LiveScene();
		LiveScene( DD::Image::GeoOp *op, const IECoreScene::SceneInterface::Path& rootPath=IECoreScene::SceneInterface::rootPath );

		~LiveScene() override;

		std::string fileName() const override;

		Name name() const override;
		void path( Path &p ) const override;

		Imath::Box3d readBound( double time ) const override;
		void writeBound( const Imath::Box3d &bound, double time );

		IECore::ConstDataPtr readTransform( double time ) const override;
		Imath::M44d readTransformAsMatrix( double time ) const override;
		IECore::ConstDataPtr readWorldTransform( double time ) const;
		Imath::M44d readWorldTransformAsMatrix( double time ) const;
		void writeTransform( const IECore::Data *transform, double time ) override;

		bool hasAttribute( const Name &name ) const override;
		void attributeNames( NameList &attrs ) const override;
		IECore::ConstObjectPtr readAttribute( const Name &name, double time ) const override;
		void writeAttribute( const Name &name, const IECore::Object *attribute, double time ) override;

		bool hasTag( const Name &name, int filter = SceneInterface::LocalTag ) const override;
		void readTags( NameList &tags, int filter = SceneInterface::LocalTag ) const override;
		void writeTags( const NameList &tags ) override;

		NameList setNames( bool includeDescendantSets = true ) const override;
		IECore::PathMatcher readSet( const Name &name, bool includeDescendantSets = true, const IECore::Canceller *canceller = nullptr ) const override;
		void writeSet( const Name &name, const IECore::PathMatcher &set ) override;
		void hashSet( const Name& setName, IECore::MurmurHash &h ) const override;

		bool hasObject() const override;
		IECore::ConstObjectPtr readObject( double time, const IECore::Canceller *canceller = nullptr  ) const override;
		IECoreScene::PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const override;
		void writeObject( const IECore::Object *object, double time ) override;

		void childNames( NameList &childNames ) const override;
		bool hasChild( const Name &name ) const override;
		IECoreScene::SceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) override;
		IECoreScene::ConstSceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const override;
		IECoreScene::SceneInterfacePtr createChild( const Name &name ) override;

		IECoreScene::SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) override;
		IECoreScene::ConstSceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const override;

		void hash( HashType hashType, double time, IECore::MurmurHash &h ) const override;

		static double timeToFrame( const double& time );
		static double frameToTime( const int& frame );

		typedef std::map<double, DD::Image::GeometryList> FrameGeometryCache;
		typedef std::map<DD::Image::Hash, FrameGeometryCache> OpGeometryCache;
		typedef std::map<const LiveScene*, OpGeometryCache> LiveSceneGeometryCache;

		void setOp( DD::Image::GeoOp* op );
		const DD::Image::GeoOp *getOp() const;

		private:

		DD::Image::GeometryList geometryList( const double& frame ) const;
		DD::Image::GeometryList geometryList( DD::Image::Op* op, const double& frame ) const;
		unsigned objectNum( const double* time=nullptr ) const;
		DD::Image::GeoInfo* object( const unsigned& index, const double* time=nullptr ) const;

		std::string geoInfoPath( const int& index ) const;

		DD::Image::GeoOp *m_op;
		//std::string m_rootPath;
		IECoreScene::SceneInterface::Path m_rootPath;
		IECore::PathMatcher m_pathMatcher;
		typedef std::map<unsigned, std::string> objectPathMap;
		mutable objectPathMap m_objectPathMap;

		void cacheGeometryList( const double& frame ) const;
};

IE_CORE_DECLAREPTR( LiveScene );

} // namespace IECoreNuke

#endif // IECORENUKE_LIVESCENE_H
