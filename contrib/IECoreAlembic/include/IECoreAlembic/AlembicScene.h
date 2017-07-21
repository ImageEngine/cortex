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

#include "IECore/SampledSceneInterface.h"

#include "IECoreAlembic/Export.h"
#include "IECoreAlembic/TypeIds.h"

namespace IECoreAlembic
{

class IECOREALEMBIC_API AlembicScene : public IECore::SampledSceneInterface
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( AlembicScene, IECoreAlembic::AlembicSceneTypeId, IECore::SampledSceneInterface );

		AlembicScene( const std::string &fileName, IECore::IndexedIO::OpenMode mode );

		virtual ~AlembicScene();

		virtual std::string fileName() const;

		virtual Name name() const;
		virtual void path( Path &p ) const;

		virtual bool hasBound() const;
		virtual size_t numBoundSamples() const;
		virtual double boundSampleTime( size_t sampleIndex ) const;
		virtual double boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const;
		virtual Imath::Box3d readBoundAtSample( size_t sampleIndex ) const;
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

	private :

		class AlembicIO;
		class AlembicReader;
		typedef std::unique_ptr<AlembicIO> AlembicIOPtr;

		AlembicScene( AlembicIOPtr io );

		const AlembicReader *reader() const;

		AlembicIOPtr m_io;

};

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_ALEMBICSCENE_H
