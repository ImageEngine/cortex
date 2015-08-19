//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SampledSceneInterface.h"

#include "IECoreAlembic/TypeIds.h"
#include "IECoreAlembic/Export.h"
#include "IECoreAlembic/AlembicInput.h"

namespace IECoreAlembic
{

IE_CORE_FORWARDDECLARE( AlembicScene );

/// A scene interface for reading/writing Alembic files
/// \ingroup ioGroup
class IECOREALEMBIC_API AlembicScene : public IECore::SampledSceneInterface
{
	public :

		typedef IECore::SceneInterface::Name Name;
		typedef IECore::SceneInterface::NameList NameList;
		typedef IECore::SceneInterface::Path Path;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( AlembicScene, AlembicSceneTypeId, IECore::SampledSceneInterface );

		/// Opens the cache, using the specified open mode, and setting the
		/// current object path to "/". Depending on what mode is chosen,
		/// different subsets of the methods below are available. When the
		/// open mode is Read, only the const methods may be used and
		/// when the open mode is Write, the non-const methods
		/// may be used in addition. Append mode is currently not supported.
		AlembicScene( const std::string &fileName, IECore::IndexedIO::OpenMode mode );

		~AlembicScene();

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
		virtual IECore::ConstDataPtr readTransformAtSample( size_t sampleIndex ) const;
		virtual Imath::M44d readTransformAsMatrixAtSample( size_t sampleIndex ) const;
		virtual IECore::ConstDataPtr readTransform( double time ) const;
		virtual Imath::M44d readTransformAsMatrix( double time ) const;
		virtual void writeTransform( const IECore::Data *transform, double time );

		virtual bool hasAttribute( const Name &name ) const;
		virtual void attributeNames( NameList &attrs ) const;
		virtual size_t numAttributeSamples( const Name &name ) const;
		virtual double attributeSampleTime( const Name &name, size_t sampleIndex ) const;
		virtual double attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const;
		virtual IECore::ConstObjectPtr readAttributeAtSample( const Name &name, size_t sampleIndex ) const;
		virtual IECore::ConstObjectPtr readAttribute( const Name &name, double time ) const;
		virtual void writeAttribute( const Name &name, const IECore::Object *attribute, double time );

		virtual bool hasTag( const Name &name, int filter = SceneInterface::LocalTag ) const;
		virtual void readTags( NameList &tags, int filter = SceneInterface::LocalTag ) const;
		virtual void writeTags( const NameList &tags );

		virtual bool hasObject() const;
		virtual size_t numObjectSamples() const;
		virtual double objectSampleTime( size_t sampleIndex ) const;
		virtual double objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const;
		virtual IECore::ConstObjectPtr readObjectAtSample( size_t sampleIndex ) const;
		virtual IECore::ConstObjectPtr readObject( double time ) const;
		virtual IECore::PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const;
		virtual void writeObject( const IECore::Object *object, double time );

		virtual bool hasChild( const Name &name ) const;
		virtual void childNames( NameList &childNames ) const;
		virtual IECore::SceneInterfacePtr child( const Name &name, IECore::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing );
		virtual IECore::ConstSceneInterfacePtr child( const Name &name, IECore::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const;
		virtual IECore::SceneInterfacePtr createChild( const Name &name );
		virtual IECore::SceneInterfacePtr scene( const Path &path, IECore::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing );
		virtual IECore::ConstSceneInterfacePtr scene( const Path &path, IECore::SceneInterface::MissingBehaviour missingBehaviour = ThrowIfMissing ) const;
		
		virtual void hash( HashType hashType, double time, IECore::MurmurHash &h ) const;

	private:

		AlembicScene( AlembicInputPtr input, AlembicInputPtr rootInput, IECore::MurmurHash fileNameHash );

		AlembicInputPtr m_input;
		AlembicInputPtr m_rootInput;
		IECore::MurmurHash m_fileNameHash;


};

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_ALEMBICSCENE_H
