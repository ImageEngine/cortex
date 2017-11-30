//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_SCENECACHE_H
#define IECORESCENE_SCENECACHE_H

#include "IECoreScene/Export.h"
#include "IECoreScene/SampledSceneInterface.h"

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( SceneCache );

/// A simple means of saving and loading hierarchical descriptions of animated scene, with
/// the ability to traverse the scene and perform partial loading on demand.
/// When saving, it's important to keep the initial root SceneCache object alive until the very end.
/// The destruction of the root scene will trigger the recursive computation of the bounding boxes for all the
/// locations that no bounds were written. It will also store (without duplication) all the
/// sample times used by objects, transforms, bounds and attributes.
/// \ingroup ioGroup
class IECORESCENE_API SceneCache : public SampledSceneInterface
{
	public :

		typedef SceneInterface::Name Name;
		typedef SceneInterface::NameList NameList;
		typedef SceneInterface::Path Path;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( SceneCache, SceneCacheTypeId, SampledSceneInterface );

		/// Opens the cache, using the specified open mode, and setting the
		/// current object path to "/". Depending on what mode is chosen,
		/// different subsets of the methods below are available. When the
		/// open mode is Read, only the const methods may be used and
		/// when the open mode is Write, the non-const methods
		/// may be used in addition. Append mode is currently not supported.
		SceneCache( const std::string &fileName, IECore::IndexedIO::OpenMode mode );
		/// Constructor which uses an already-opened IndexedIO, this
		/// can be used if you wish to use an alternative IndexedIO
		/// implementation for the backend. The given IndexedIO should be
		/// pointing to the root location on the file. The open mode will
		/// be the same from the given IndexedIO object. Append mode is not
		/// supported.
		SceneCache( IECore::IndexedIOPtr indexedIO );

		~SceneCache() override;

		/*
		 * virtual functions defined in SceneInterface.
		 */

		std::string fileName() const override;

		Name name() const override;
		void path( Path &p ) const override;

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

		bool hasObject() const override;
		size_t numObjectSamples() const override;
		double objectSampleTime( size_t sampleIndex ) const override;
		double objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const override;
		IECore::ConstObjectPtr readObjectAtSample( size_t sampleIndex ) const override;
		PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const override;
		void writeObject( const IECore::Object *object, double time ) override;

		bool hasChild( const Name &name ) const override;
		void childNames( NameList &childNames ) const override;
		SceneInterfacePtr child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) override;
		ConstSceneInterfacePtr child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const override;
		SceneInterfacePtr createChild( const Name &name ) override;
		SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = ThrowIfMissing ) override;
		ConstSceneInterfacePtr scene( const Path &path, SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const override;

		void hash( HashType hashType, double time, IECore::MurmurHash &h ) const override;

		/// tells you if this scene cache is read only or writable:
		bool readOnly() const;

		// The attribute names used to mark animated topology and primitive variables
		// when SceneCache objects are Primitives.
		static const Name &animatedObjectTopologyAttribute;
		static const Name &animatedObjectPrimVarsAttribute;

	protected:

		IE_CORE_FORWARDDECLARE( Implementation );
		virtual SceneCachePtr duplicate( ImplementationPtr& implementation ) const;
		SceneCache( ImplementationPtr& implementation );

		/// LinkedScene need to specify whether the tag is supposed to be saved
		/// as a local tag or a tag that was artificially inherited from the child transforms.
		void writeTags( const NameList &tags,  bool descendentTags );

		friend class LinkedScene;

	private :

		ImplementationPtr m_implementation;

		class ReaderImplementation;
		class WriterImplementation;

};

} // namespace IECoreScene

#endif // IECORESCENE_SCENECACHE_H
