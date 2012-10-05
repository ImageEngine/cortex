//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathBoxAlgo.h"

#include "IECore/ModelCache.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/HeaderGenerator.h"
#include "IECore/VisibleRenderable.h"

using namespace IECore;
using namespace Imath;

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

// Register FileIndexedIO as the handler for .mdc files so that people
// can use the filename constructor as a convenience over the IndexedIO
// constructor.
static IndexedIOInterface::Description<FileIndexedIO> extensionDescription( ".mdc" );

class ModelCache::Implementation : public RefCounted
{

	public :
	
		Implementation( IndexedIOInterfacePtr indexedIO )
			:	m_indexedIO( indexedIO ), m_path( "/" ), m_explicitBound( false )
		{
			if( m_indexedIO->openMode() & IndexedIO::Append )
			{
				throw InvalidArgumentException( "Append mode not supported" );
			}
			
			if( m_indexedIO->openMode() & IndexedIO::Write )
			{
				ObjectPtr header = HeaderGenerator::header();
				header->save( m_indexedIO, "header" );
				m_indexedIO->mkdir( "root" );
			}
			m_indexedIO->chdir( "root" );
			m_indexedIO = m_indexedIO->resetRoot();
		}
		
		virtual ~Implementation()
		{
			// write out the bound if necessary.
			if( m_indexedIO->openMode() & ( IndexedIO::Write | IndexedIO::Append ) )
			{
				m_indexedIO->write( "bound", m_bound.min.getValue(), 6 );			
				// propagate the bound to our parent.
				if( m_parent && !m_parent->m_explicitBound )
				{
					Box3d transformedBound = transform( m_bound, m_transform );
					m_parent->m_bound.extendBy( transformedBound );
				}
			}
			
		}
	
		const std::string &path() const
		{
			return m_path;
		}
		
		Imath::Box3d readBound() const
		{
			Box3d result;
			double *resultAddress = result.min.getValue();
			m_indexedIO->read( "bound", resultAddress, 6 );
			return result;
		}
		
		void writeBound( const Imath::Box3d &bound )
		{
			m_bound = bound;
			m_explicitBound = true;
		}
	
		Imath::M44d readTransform() const
		{
			M44d result;
			double *resultAddress = result.getValue();
			try
			{
				m_indexedIO->read( "transform", resultAddress, 16 );
			}
			catch( const IOException &e )
			{
				// we only write non-identity transforms, so it's fine
				// if we don't find one.
			}
			return result;
		}
		
		void writeTransform( const Imath::M44d &transform )
		{
			m_transform = transform;
			if( transform != M44d() )
			{
				m_indexedIO->write( "transform", transform.getValue(), 16 );
			}
		}
		
		ObjectPtr readObject() const
		{
			ObjectPtr result = 0;
			try
			{
				result = Object::load( m_indexedIO, "object" );
			}
			catch( const IOException &e )
			{
				// we only write non-null objects, so it's fine
				// if we don't find one.
			}
			return result;
		}
		
		void writeObject( const IECore::Object *object )
		{
			object->save( m_indexedIO, "object" );
			const VisibleRenderable *renderable = runTimeCast<const VisibleRenderable>( object );
			if( renderable && !m_explicitBound )
			{
				Box3f bf = renderable->bound();
				Box3d bd(
					V3d( bf.min.x, bf.min.y, bf.min.z ),
					V3f( bf.max.x, bf.max.y, bf.max.z )
				);
				m_bound.extendBy( bd );
			}
		}
	
		void childNames( std::vector<std::string> &childNames ) const
		{
			try
			{
				m_indexedIO->chdir( "children" );
			}
			catch( const IOException &e  )
			{
				// it's ok for an entry to not have children
				return;
			}
			
			IndexedIO::EntryList entries;
			entries = m_indexedIO->ls();
			m_indexedIO->chdir( ".." );
			
			for( IndexedIO::EntryList::const_iterator it = entries.begin(); it!=entries.end(); it++ )
			{
				childNames.push_back( it->id() );
			}
		}
		
		ModelCachePtr writableChild( const std::string &childName )
		{
			std::string childPath = m_path;
			if( childPath != "/" )
			{
				childPath += "/";
			}
			childPath += childName;
			
			std::string dirName = "children/" + childName;
			m_indexedIO->mkdir( dirName );
			m_indexedIO->chdir( dirName );
			ModelCachePtr result = new ModelCache(
				new Implementation(
					m_indexedIO->resetRoot(),
					childPath,
					this
				)
			);
			m_indexedIO->chdir( "../.." );
		
			return result;
		}
		
		ConstModelCachePtr readableChild( const std::string &childName ) const
		{
			std::string childPath = m_path;
			if( childPath != "/" )
			{
				childPath += "/";
			}
			childPath += childName;
			
			std::string dirName = "children/" + childName;
			m_indexedIO->chdir( dirName );
			ModelCachePtr result = new ModelCache(
				new Implementation(
					m_indexedIO->resetRoot(),
					childPath,
					0 // read only so no need for parent for bounds propagation
				)
			);
			m_indexedIO->chdir( "../.." );
		
			return result;
		}

	private :
	
		Implementation( IndexedIOInterfacePtr indexedIO, const std::string &path, ImplementationPtr parent )
			:	m_indexedIO( indexedIO ), m_path( path ), m_explicitBound( false ), m_parent( parent )
		{
		}
	
		IndexedIOInterfacePtr m_indexedIO;
		std::string m_path;
		// accumulated into during writing, and written out in the destructor
		Box3d m_bound;
		bool m_explicitBound; // bound was specified by writeBound(), and overrides implicit bounds
		M44d m_transform;
		// stored during writing so we can propagate bounds
		ImplementationPtr m_parent;

};

//////////////////////////////////////////////////////////////////////////
// ModelCache
//////////////////////////////////////////////////////////////////////////

ModelCache::ModelCache( const std::string &fileName, IndexedIO::OpenMode mode )
{
	IndexedIOInterfacePtr indexedIO = IndexedIOInterface::create( fileName, "/", mode );
	m_implementation = new Implementation( indexedIO );
}

ModelCache::ModelCache( IECore::IndexedIOInterfacePtr indexedIO )
{
	m_implementation = new Implementation( indexedIO );
}

ModelCache::ModelCache( ImplementationPtr implementation )
	:	m_implementation( implementation )
{
}

ModelCache::~ModelCache()
{
}

const std::string &ModelCache::path() const
{
	return m_implementation->path();
}

Imath::Box3d ModelCache::readBound() const
{
	return m_implementation->readBound();
}

void ModelCache::writeBound( const Imath::Box3d &bound )
{
	m_implementation->writeBound( bound );
}

Imath::M44d ModelCache::readTransform() const
{
	return m_implementation->readTransform();
}

void ModelCache::writeTransform( const Imath::M44d &transform )
{
	m_implementation->writeTransform( transform );
}

ObjectPtr ModelCache::readObject() const
{
	return m_implementation->readObject();
}

void ModelCache::writeObject( const IECore::Object *object )
{
	m_implementation->writeObject( object );
}

void ModelCache::childNames( std::vector<std::string> &childNames ) const
{
	m_implementation->childNames( childNames );
}

ModelCachePtr ModelCache::writableChild( const std::string &childName )
{
	return m_implementation->writableChild( childName );
}

ConstModelCachePtr ModelCache::readableChild( const std::string &childName ) const
{
	return m_implementation->readableChild( childName );
}
