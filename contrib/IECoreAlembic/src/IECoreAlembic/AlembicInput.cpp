//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "Alembic/AbcCoreHDF5/ReadWrite.h"
#include "Alembic/Abc/IArchive.h"
#include "Alembic/Abc/IObject.h"
#include "Alembic/AbcGeom/IGeomBase.h"
#include "Alembic/AbcGeom/ArchiveBounds.h"
#include "Alembic/AbcGeom/IXform.h"

#include "IECoreAlembic/AlembicInput.h"
#include "IECoreAlembic/FromAlembicConverter.h"

#include "IECore/SimpleTypedData.h"

using namespace Imath;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace IECoreAlembic;
using namespace IECore;

struct AlembicInput::DataMembers
{
	DataMembers()
		: boundValid( false )
	{
	}
	
	boost::shared_ptr<IArchive> archive;
	IObject object;
	bool boundValid;
	Box3d bound;
};

AlembicInput::AlembicInput( const std::string &fileName )
{
	m_data = boost::shared_ptr<DataMembers>( new DataMembers );
	m_data->archive = boost::shared_ptr<IArchive>( new IArchive( ::Alembic::AbcCoreHDF5::ReadArchive(), fileName ) );
	m_data->object = m_data->archive->getTop();
}

AlembicInput::AlembicInput()
{
}

AlembicInput::~AlembicInput()
{
}

const std::string &AlembicInput::name() const
{
	return m_data->object.getName();
}

const std::string &AlembicInput::fullName() const
{
	return m_data->object.getFullName();
}

IECore::CompoundDataPtr AlembicInput::metaData() const
{
	const MetaData &md = m_data->object.getMetaData();
	CompoundDataPtr resultData = new CompoundData;
	CompoundDataMap &resultMap = resultData->writable();
	for( MetaData::const_iterator it = md.begin(), eIt = md.end(); it!=eIt; it++ )
	{
		resultMap[it->first] = new StringData( it->second );
	}
	
	return resultData;
}

Imath::Box3d AlembicInput::bound() const
{
	if( m_data->boundValid )
	{
		return m_data->bound;
	}
	
	const MetaData &md = m_data->object.getMetaData();

	if( !m_data->object.getParent() )
	{
		// top of archive
		m_data->bound = GetIArchiveBounds( *(m_data->archive) ).getValue();
	}
	else if( IXform::matches( md ) )
	{
		// intermediate transform. pray that we've been given the
		// optional child bounds. pray that one day they stop being
		// optional.
		IXform iXForm( m_data->object, kWrapExisting );
		IXformSchema &iXFormSchema = iXForm.getSchema();
		XformSample sample;
		iXFormSchema.get( sample );
		Box3d childBounds = sample.getChildBounds();
		if( childBounds.isEmpty() )
		{
			// the child bounds weren't stored at write time. instead
			// we have to compute them over and over at read time.
			for( size_t i=0, n=numChildren(); i<n; i++ )
			{
				AlembicInputPtr c = child( i );
				childBounds.extendBy( c->bound() );
			}
		}
		m_data->bound = transform( childBounds, sample.getMatrix() );
	}
	else
	{
		IGeomBaseObject geomBase( m_data->object, kWrapExisting );
		m_data->bound = geomBase.getSchema().getValue().getSelfBounds();
	}
	
	m_data->boundValid = true;

	return m_data->bound;
}

IECore::ObjectPtr AlembicInput::convert( IECore::TypeId resultType ) const
{
	FromAlembicConverterPtr converter = FromAlembicConverter::create( m_data->object, resultType );
	if( converter )
	{
		return converter->convert();
	}
	return 0;
}

size_t AlembicInput::numChildren() const
{
	return m_data->object.getNumChildren();
}

AlembicInputPtr AlembicInput::child( size_t index ) const
{
	AlembicInputPtr result = new AlembicInput();
	result->m_data = boost::shared_ptr<DataMembers>( new DataMembers );
	result->m_data->archive = this->m_data->archive;
	/// \todo this is documented as not being the best way of doing things in
	/// the alembic documentation. I'm not sure what would be better though,
	/// and it appears to work fine so far.
	result->m_data->object = this->m_data->object.getChild( index );
	return result;
}

IECore::StringVectorDataPtr AlembicInput::childNames() const
{
	StringVectorDataPtr resultData = new StringVectorData;
	std::vector<std::string> &resultVector = resultData->writable();
	size_t numChildren = m_data->object.getNumChildren();
	for( size_t i=0; i<numChildren; i++ )
	{
		resultVector.push_back( m_data->object.getChildHeader( i ).getName() );
	}
	return resultData;
}

AlembicInputPtr AlembicInput::child( const std::string &name ) const
{
	IObject c = m_data->object.getChild( name );
	if( !c )
	{
		throw InvalidArgumentException( name );
	}
	
	AlembicInputPtr result = new AlembicInput();
	result->m_data = boost::shared_ptr<DataMembers>( new DataMembers );
	result->m_data->archive = m_data->archive;
	result->m_data->object = c;
	return result;
}
