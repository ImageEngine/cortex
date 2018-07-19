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

#ifndef IECOREALEMBIC_ALEMBICSCENE_H
#define IECOREALEMBIC_ALEMBICSCENE_H

#include "IECoreAlembic/Export.h"
#include "IECoreAlembic/TypeIds.h"

#include "IECoreScene/SampledSceneInterface.h"

namespace IECoreAlembic
{

class IECOREALEMBIC_API AlembicScene : public IECoreScene::SampledSceneInterface
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( AlembicScene, IECoreAlembic::AlembicSceneTypeId, IECoreScene::SampledSceneInterface );

		AlembicScene( const std::string &fileName, IECore::IndexedIO::OpenMode mode );

		~AlembicScene() override;

		std::string fileName() const override;

		Name name() const override;
		void path( Path &p ) const override;

		bool hasBound() const override;
		size_t numBoundSamples() const override;
		double boundSampleTime( size_t sampleIndex ) const override;
		double boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const override;
		Imath::Box3d readBoundAtSample( size_t sampleIndex ) const override;
		void writeBound( const Imath::Box3d &bound, double time ) override;

		size_t numTransformSamples() const override;
		double transformSampleTime( size_t sampleIndex ) const override;
		double transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const override;
		IECore::ConstDataPtr readTransformAtSample( size_t sampleIndex ) const override;
		Imath::M44d readTransformAsMatrixAtSample( size_t sampleIndex ) const override;
		IECore::ConstDataPtr readTransform( double time ) const override;
		Imath::M44d readTransformAsMatrix( double time ) const override;
		void writeTransform( const IECore::Data *transform, double time ) override;

		bool hasAttribute( const Name &name ) const override;
		void attributeNames( NameList &attrs ) const override;
		size_t numAttributeSamples( const Name &name ) const override;
		double attributeSampleTime( const Name &name, size_t sampleIndex ) const override;
		double attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const override;
		IECore::ConstObjectPtr readAttributeAtSample( const Name &name, size_t sampleIndex ) const override;
		void writeAttribute( const Name &name, const IECore::Object *attribute, double time ) override;

		bool hasTag( const Name &name, int filter = SceneInterface::LocalTag ) const override;
		void readTags( NameList &tags, int filter = SceneInterface::LocalTag ) const override;
		void writeTags( const NameList &tags ) override;

		NameList setNames( bool includeDescendantSets = true ) const override;
		IECore::PathMatcher readSet( const Name &name, bool includeDescendantSets = true ) const override;
		void writeSet( const Name &name, const IECore::PathMatcher &set ) override;
		void hashSet( const Name& setName, IECore::MurmurHash &h ) const override;

		bool hasObject() const override;
		size_t numObjectSamples() const override;
		double objectSampleTime( size_t sampleIndex ) const override;
		double objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const override;
		IECore::ConstObjectPtr readObjectAtSample( size_t sampleIndex ) const override;
		IECoreScene::PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const override;
		void writeObject( const IECore::Object *object, double time ) override;

		bool hasChild( const Name &name ) const override;
		void childNames( NameList &childNames ) const override;
		IECoreScene::SceneInterfacePtr child( const Name &name, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) override;
		IECoreScene::ConstSceneInterfacePtr child( const Name &name, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const override;
		IECoreScene::SceneInterfacePtr createChild( const Name &name ) override;
		IECoreScene::SceneInterfacePtr scene( const Path &path, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) override;
		IECoreScene::ConstSceneInterfacePtr scene( const Path &path, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const override;

		void hash( HashType hashType, double time, IECore::MurmurHash &h ) const override;

	private :

		IE_CORE_FORWARDDECLARE( AlembicIO );
		IE_CORE_FORWARDDECLARE( AlembicReader );
		IE_CORE_FORWARDDECLARE( AlembicWriter );

		AlembicScene( const AlembicIOPtr &root, const AlembicIOPtr &io );

		const AlembicReader *reader() const;
		AlembicWriter *writer();

		AlembicIOPtr m_root;
		AlembicIOPtr m_io;

};

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_ALEMBICSCENE_H
