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

#include "boost/tokenizer.hpp"
#include "boost/format.hpp"

#include "IECoreAlembic/AlembicScene.h"

#include "IECore/NullObject.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace IECoreAlembic;
using namespace Imath;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( AlembicScene )

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

// Register the .scc extension in the factory function for AlembicScene on Read and Write modes
static SceneInterface::FileFormatDescription<AlembicScene> registrar(".abc", IndexedIO::Read );

//////////////////////////////////////////////////////////////////////////
// AlembicScene
//////////////////////////////////////////////////////////////////////////

AlembicScene::AlembicScene( const std::string &fileName, IndexedIO::OpenMode mode ) : m_input( new AlembicInput( fileName ) )
{
	m_fileNameHash.append( fileName );
	m_rootInput = m_input;
	if( mode != IndexedIO::Read )
	{
		throw Exception( "AlembicScene only supports Read mode" );
	}
}

AlembicScene::AlembicScene( AlembicInputPtr input, AlembicInputPtr rootInput, MurmurHash fileNameHash ) : m_input( input ), m_rootInput( rootInput ), m_fileNameHash( fileNameHash )
{
}

AlembicScene::~AlembicScene()
{
}

std::string AlembicScene::fileName() const
{
	return "";
}

void AlembicScene::path( AlembicScene::Path &p ) const
{
	std::string fullName = m_input->fullName();

	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	Tokenizer tokens(fullName, boost::char_separator<char>("/"));
	Tokenizer::iterator t = tokens.begin();
	p.clear();
	for ( ; t != tokens.end(); t++ )
	{
		p.push_back( *t );
	}

}

AlembicScene::Name AlembicScene::name() const
{
	return m_input->name();
}

size_t AlembicScene::numBoundSamples() const
{
	return m_input->numSamples();
}

double AlembicScene::boundSampleTime( size_t sampleIndex ) const
{
	return m_input->timeAtSample( sampleIndex );
}

double AlembicScene::boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return m_input->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

Imath::Box3d AlembicScene::readBoundAtSample( size_t sampleIndex ) const
{
	return m_input->boundAtSample( sampleIndex );
}

Imath::Box3d AlembicScene::readBound( double time ) const
{
	return m_input->boundAtTime( time );
}

void AlembicScene::writeBound( const Imath::Box3d &bound, double time )
{
	throw IECore::Exception( "AlembicScene::writeBound not supported" );
}

size_t AlembicScene::numTransformSamples() const
{
	return m_input->numSamples();
}

double AlembicScene::transformSampleTime( size_t sampleIndex ) const
{
	return m_input->timeAtSample( sampleIndex );
}

double AlembicScene::transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return m_input->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

ConstDataPtr AlembicScene::readTransformAtSample( size_t sampleIndex ) const
{
	return new IECore::M44dData( readTransformAsMatrixAtSample( sampleIndex ) );
}

Imath::M44d AlembicScene::readTransformAsMatrixAtSample( size_t sampleIndex ) const
{
	return m_input->transformAtSample( sampleIndex );
}

ConstDataPtr AlembicScene::readTransform( double time ) const
{
	return new IECore::M44dData( readTransformAsMatrix( time ) );
}

Imath::M44d AlembicScene::readTransformAsMatrix( double time ) const
{
	return m_input->transformAtTime( time );
}

void AlembicScene::writeTransform( const Data *transform, double time )
{
	throw IECore::Exception( "AlembicScene::writeTransform not supported" );
}

bool AlembicScene::hasAttribute( const Name &name ) const
{
	return false;
}

void AlembicScene::attributeNames( NameList &attrs ) const
{
	attrs.clear();
}

size_t AlembicScene::numAttributeSamples( const Name &name ) const
{
	return m_input->numSamples();
}

double AlembicScene::attributeSampleTime( const Name &name, size_t sampleIndex ) const
{
	return m_input->timeAtSample( sampleIndex );
}

double AlembicScene::attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return m_input->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

ConstObjectPtr AlembicScene::readAttributeAtSample( const Name &name, size_t sampleIndex ) const
{
	return IECore::NullObject::defaultNullObject();
}

ConstObjectPtr AlembicScene::readAttribute( const Name &name, double time ) const
{
	return IECore::NullObject::defaultNullObject();
}

void AlembicScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw IECore::Exception( "AlembicScene::writeAttribute not supported" );
}

bool AlembicScene::hasTag( const Name &name, int filter ) const
{
	return false;
}

void AlembicScene::readTags( NameList &tags, int filter ) const
{
	tags.clear();
}

void AlembicScene::writeTags( const NameList &tags )
{
	throw IECore::Exception( "AlembicScene::writeTags not supported" );
}

bool AlembicScene::hasObject() const
{
	return m_input->converter( IECore::RenderableTypeId );
}

size_t AlembicScene::numObjectSamples() const
{
	return m_input->numSamples();
}

double AlembicScene::objectSampleTime( size_t sampleIndex ) const
{
	return m_input->timeAtSample( sampleIndex );
}

double AlembicScene::objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return m_input->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

ConstObjectPtr AlembicScene::readObjectAtSample( size_t sampleIndex ) const
{
	ConstObjectPtr o = m_input->objectAtSample( sampleIndex, IECore::RenderableTypeId );
	if( o )
	{
		return o;
	}
	return IECore::NullObject::defaultNullObject();
}

ConstObjectPtr AlembicScene::readObject( double time ) const
{
	ConstObjectPtr o = m_input->objectAtTime( time, IECore::RenderableTypeId );
	if( o )
	{
		return o;
	}
	return IECore::NullObject::defaultNullObject();
}

PrimitiveVariableMap AlembicScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	return PrimitiveVariableMap();
}

void AlembicScene::writeObject( const Object *object, double time )
{
	throw IECore::Exception( "AlembicScene::writeObject not supported" );
}

void AlembicScene::childNames( NameList &childNames ) const
{
	childNames.clear();
	IECore::StringVectorDataPtr names = m_input->childNames();
	for( size_t i=0; i < names->readable().size(); ++i )
	{
		childNames.push_back( names->readable()[i] );
	}
}

SceneInterfacePtr AlembicScene::child( const Name &name, AlembicScene::MissingBehaviour missingBehaviour )
{
	AlembicInputPtr c = m_input->child( name );
	if( c )
	{
		return new AlembicScene( c, m_rootInput, m_fileNameHash );
	}
	
	if( missingBehaviour == NullIfMissing )
	{
		return 0;
	}
	
	throw Exception( ( boost::format( "AlembicScene::child: no child called %s at %s" ) % name.value() % m_input->fullName() ).str() );
}

ConstSceneInterfacePtr AlembicScene::child( const Name &name, AlembicScene::MissingBehaviour missingBehaviour ) const
{
	return const_cast<AlembicScene*>( this )->child( name, missingBehaviour );
}

bool AlembicScene::hasChild( const Name &name ) const
{
	return false;	
}

SceneInterfacePtr AlembicScene::createChild( const Name &name )
{
	throw Exception( "AlembicScene::createChild: Not supported" );
}

SceneInterfacePtr AlembicScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	AlembicInputPtr s = m_rootInput;
	for( size_t i=0; i < path.size(); ++i )
	{
		s = s->child( path[i] );
		if( !s )
		{
			if( missingBehaviour == NullIfMissing )
			{
				return 0;
			}
			std::string currentPathStr = "/";
			for( size_t j=0; j < i; ++j )
			{
				currentPathStr += path[j].value() + "/";
			}
			throw Exception( ( boost::format( "AlembicScene::scene: no child called %s at %s" ) % path[i].value() % currentPathStr ).str() );
		}
	}
	return new AlembicScene( s, m_rootInput, m_fileNameHash );
}

ConstSceneInterfacePtr AlembicScene::scene( const Path &path, AlembicScene::MissingBehaviour missingBehaviour ) const
{
	return const_cast<AlembicScene*>( this )->scene( path, missingBehaviour );
}

void AlembicScene::hash( HashType hashType, double time, MurmurHash &h ) const
{
	SceneInterface::hash( hashType, time, h );
	h.append( int(hashType) );
	switch( hashType )
	{
		case TransformHash:
		{
			h.append( readTransformAsMatrix( time ) );
			break;
		}
		case BoundHash:
		{
			if( m_input->hasStoredBound() )
			{
				// this read will be quick as the bound has been stored, so we can use it in the hash:
				h.append( readBound( time ) );
			}
			else
			{
				// fall back and just hash in a bunch of stuff:
				h.append( m_fileNameHash );
				h.append( m_input->fullName() );
				if( numBoundSamples() > 1 )
				{
					h.append( time );
				}
			}
			break;
		}
		case ObjectHash:
		{
			// try and get the stored object hash from Alembic:
			if( !m_input->objectHash( time, h ) )
			{
				// fallback: 
				h.append( m_fileNameHash );
				h.append( m_input->fullName() );
				if( numObjectSamples() > 1 )
				{
					h.append( time );
				}
			}
			break;
		}
		case ChildNamesHash:
		{
			IECore::StringVectorDataPtr names = m_input->childNames();
			h.append( names->readable().data(), names->readable().size() );
			break;
		}
		case HierarchyHash:
		{
			// \todo: use inbuilt hierarchy hashes
			h.append( m_fileNameHash );
			h.append( m_input->fullName() );
			h.append( time );
			break;
		}
		case AttributesHash: break;
	}
}
