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

#ifndef IECORE_LINKEDSCENE_H
#define IECORE_LINKEDSCENE_H

#include "IECore/SampledSceneInterface.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( LinkedScene );
//IE_CORE_FORWARDDECLARE( StringData );
//IE_CORE_FORWARDDECLARE( InternedStringVectorData );

/// Implements a scene that have references (links) to external scenes.
/// Links can be created at any location in a scene. When a link is created in a given location,
/// the object, bounds and children will be loaded from the linked scene (with time remapping). The transform, attributes
/// are still loaded from the main scene. Tags defined in the link location will be applied (when read) to all the child transforms from the linked scene.
/// This class wraps another SceneInterface object that is responsible for actually storing the data
/// (we call it the "main scene"). Links are represented as an attribute in the main scene called "SceneInterface:link".
/// When created for reading, this class provides seamless access to the hierarchy inside the linked scenes, 
/// concatenating the two hierarchies in a single path that uniquely identify that location. The time is also 
/// transparently translated. Tags that were saved in the linked scene are propagated to the main scene, 
/// to keep consistent behavior.
/// When writing, there's no access to the contents of the indexed scene. Instead, it creates the links by either
/// (1) calls to the function writeLink() or 
/// (2) calls to the function writeAttribute( LinkedScene::linkSceneAttribute, LinkedScene::linkAttributeData(), ... ). 
/// Note that the link can be animated, allowing for time remapped animations.
class LinkedScene : public  SampledSceneInterface
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( LinkedScene, SampledSceneInterface );

		/// Equals to "SceneInterface:link" and it's the name given to the link attribute that is recognized
		// by this class when expanding linked scenes.
		static const Name &linkAttribute;
		
		static const Name &fileNameLinkAttribute;
		static const Name &rootLinkAttribute;
		static const Name &timeLinkAttribute;

		/// When the open mode is Read it expands the links and only the const methods may be used and the
		/// when the open mode is Write, only the non-const methods may be used and 
		/// Append mode is not supported.
		LinkedScene( const std::string &fileName, IndexedIO::OpenMode mode );

		/// Constructor for wrapping the given read-only scene and expanding its links. If the scene is not sampled
		/// then the sampled-specific functions will raise exceptions.
		LinkedScene( ConstSceneInterfacePtr mainScene );

		virtual ~LinkedScene();

		/// Creates an attribute on the current location of this scene that represents a link to the given scene (no time remapping).
		/// This function should only be used once in a given scene location. For more control (and time remapping), 
		/// use writeAttribute in combination with linkAttributeData.
		void writeLink( const SceneInterface *scene );

		/// Returns the data that should be stored in a link attribute if we want to map it to the given scene (no time remapping).
		static IECore::CompoundDataPtr linkAttributeData( const SceneInterface *scene );

		/// Returns the data that should be stored in a link attribute if we want to map it to the given scene (with time remapping).
		/// \param time Specifies the time that should be used to query the given scene
		static IECore::CompoundDataPtr linkAttributeData( const SceneInterface *scene, double time );

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

	private :

		LinkedScene( SceneInterface *mainScene, const SceneInterface *linkedScene, int rootLinkDepth, bool readOnly, bool atLink, bool timeRemapped );

		ConstSceneInterfacePtr expandLink( const StringData *fileName, const InternedStringVectorData *root, int &linkDepth );

		// uses the mainScene to ask what is the time the link is remapped to. Should only be called when the linkAttribute is available.
		double remappedLinkTime( double time ) const;
		double remappedLinkTimeAtSample( size_t sampleIndex ) const;

		SceneInterfacePtr m_mainScene;
		ConstSceneInterfacePtr m_linkedScene;
		unsigned int m_rootLinkDepth;
		bool m_readOnly;
		bool m_atLink;
		bool m_sampled;
		bool m_timeRemapped;
		// \todo: std::map< Path, LinkedScenes > for quick scene calls... built by scene... dies with the instance (usually only root uses it).

		static const InternedString g_fileName;
		static const InternedString g_root;
		static const InternedString g_time;
};

} // namespace IECore

#endif  // IECORE_LINKEDSCENE_H
