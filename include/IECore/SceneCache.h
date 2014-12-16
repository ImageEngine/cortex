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

#ifndef IECORE_SCENECACHE_H
#define IECORE_SCENECACHE_H

#include "IECore/Export.h"
#include "IECore/SampledSceneInterface.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( SceneCache );

/// A simple means of saving and loading hierarchical descriptions of animated scene, with
/// the ability to traverse the scene and perform partial loading on demand.
/// When saving, it's important to keep the initial root SceneCache object alive until the very end.
/// The destruction of the root scene will trigger the recursive computation of the bounding boxes for all the
/// locations that no bounds were written. It will also store (without duplication) all the
/// sample times used by objects, transforms, bounds and attributes.
/// \ingroup ioGroup
class IECORE_API SceneCache : public SampledSceneInterface
{
	public :

		typedef SceneInterface::Name Name;
		typedef SceneInterface::NameList NameList;
		typedef SceneInterface::Path Path;

		IE_CORE_DECLARERUNTIMETYPED( SceneCache, SampledSceneInterface );

		/// Opens the cache, using the specified open mode, and setting the
		/// current object path to "/". Depending on what mode is chosen,
		/// different subsets of the methods below are available. When the
		/// open mode is Read, only the const methods may be used and
		/// when the open mode is Write, the non-const methods
		/// may be used in addition. Append mode is currently not supported.
		SceneCache( const std::string &fileName, IndexedIO::OpenMode mode );
		/// Constructor which uses an already-opened IndexedIO, this
		/// can be used if you wish to use an alternative IndexedIO
		/// implementation for the backend. The given IndexedIO should be 
		/// pointing to the root location on the file. The open mode will 
		/// be the same from the given IndexedIO object. Append mode is not
		/// supported.
		SceneCache( IECore::IndexedIOPtr indexedIO );

		~SceneCache();

		/*
		 * virtual functions defined in SceneInterface.
		 */

		virtual std::string fileName() const;

		virtual Name name() const;
		virtual void path( Path &p ) const;

		virtual size_t numBoundSamples() const;
		virtual double boundSampleTime( size_t sampleIndex ) const;
		virtual double boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const;
		virtual Imath::Box3d readBoundAtSample( size_t sampleIndex ) const;
		virtual Imath::Box3d readBound( double time ) const;
		virtual void writeBound( const Imath::Box3d &bound, double time );

		virtual size_t numTransformSamples() const;
		virtual double transformSampleTime( size_t sampleIndex ) const;
		virtual double transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const;
		virtual ConstDataPtr readTransformAtSample( size_t sampleIndex ) const;
		virtual Imath::M44d readTransformAsMatrixAtSample( size_t sampleIndex ) const;
		virtual ConstDataPtr readTransform( double time ) const;
		virtual Imath::M44d readTransformAsMatrix( double time ) const;
		virtual void writeTransform( const Data *transform, double time );

		virtual bool hasAttribute( const Name &name ) const;
		virtual void attributeNames( NameList &attrs ) const;
		virtual size_t numAttributeSamples( const Name &name ) const;
		virtual double attributeSampleTime( const Name &name, size_t sampleIndex ) const;
		virtual double attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const;
		virtual ConstObjectPtr readAttributeAtSample( const Name &name, size_t sampleIndex ) const;
		virtual ConstObjectPtr readAttribute( const Name &name, double time ) const;
		virtual void writeAttribute( const Name &name, const Object *attribute, double time );

		virtual bool hasTag( const Name &name, int filter = SceneInterface::LocalTag ) const;
		virtual void readTags( NameList &tags, int filter = SceneInterface::LocalTag ) const;
		virtual void writeTags( const NameList &tags );

		virtual bool hasObject() const;
		virtual size_t numObjectSamples() const;
		virtual double objectSampleTime( size_t sampleIndex ) const;
		virtual double objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const;
		virtual ConstObjectPtr readObjectAtSample( size_t sampleIndex ) const;
		virtual ConstObjectPtr readObject( double time ) const;
		virtual PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const;
		virtual void writeObject( const Object *object, double time );

		virtual bool hasChild( const Name &name ) const;
		virtual void childNames( NameList &childNames ) const;
		virtual SceneInterfacePtr child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing );
		virtual ConstSceneInterfacePtr child( const Name &name, SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const;
		virtual SceneInterfacePtr createChild( const Name &name );
		virtual SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = ThrowIfMissing );
		virtual ConstSceneInterfacePtr scene( const Path &path, SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const;
		
		virtual void hash( HashType hashType, double time, MurmurHash &h ) const;

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

} // namespace IECore

#endif // IECORE_SCENECACHE_H
