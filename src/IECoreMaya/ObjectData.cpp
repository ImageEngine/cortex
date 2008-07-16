//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/type_traits/is_integral.hpp"

#include "maya/MGlobal.h"

#include "IECore/VectorTypedData.h"
#include "IECore/MemoryIndexedIO.h"
#include "IECore/HexConversion.h"

#include "IECoreMaya/ObjectData.h"
#include "IECoreMaya/MayaTypeIds.h"

using namespace IECore;
using namespace IECoreMaya;

const MString ObjectData::typeName( "ieObjectData" );
const MTypeId ObjectData::id( ObjectDataId );

ObjectData::ObjectData() : m_object( 0 )
{
}

ObjectData::~ObjectData()
{
}

void *ObjectData::creator()
{
	return new ObjectData;
}

MStatus ObjectData::readASCII( const MArgList &argList, unsigned int &endOfTheLastParsedElement )
{
	m_object = 0;

	if ( argList.length() ==  1 )
	{
		MStatus s;
		std::string str = argList.asString( endOfTheLastParsedElement++, &s ).asChar();

		if ( !s )
		{
			return s;
		}
		CharVectorDataPtr buf = new CharVectorData();

		for ( unsigned i = 0; i < str.size(); i +=2 )
		{
			buf->writable().push_back( hexToDec<char>( std::string( str, i, 2 ) ) );
		}
		assert( 2 * buf->readable().size() == str.length() );

		try
		{
			MemoryIndexedIOPtr io = new MemoryIndexedIO( buf, "/", IndexedIO::Exclusive | IndexedIO::Read );
			setObject( Object::load( io, "object" ) );
			return MS::kSuccess;
		}
		catch ( std::exception &e )
		{
			MGlobal::displayError( e.what() );
			return MS::kFailure;
		}
	}
	else if ( argList.length() == 0 )
	{

		return MS::kSuccess;
	}
	else
	{
		return MS::kFailure;
	}
}

MStatus ObjectData::readBinary( istream& in, unsigned length )
{	
	CharVectorDataPtr buf = new CharVectorData( );
	buf->writable().resize( length );
	CharVectorData::ValueType &data = buf->writable();

	in.read( &data[0], length );
	
	try
	{
		MemoryIndexedIOPtr io = new MemoryIndexedIO( buf, "/", IndexedIO::Exclusive | IndexedIO::Read );
		setObject( Object::load( io, "object" ) );
		return MS::kSuccess;
	}
	catch ( std::exception &e )
	{
		MGlobal::displayError( e.what() );
		return MS::kFailure;
	}
}

MStatus ObjectData::writeASCII( ostream& out )
{
	if ( m_object )
	{
		try
		{
			MemoryIndexedIOPtr io = new MemoryIndexedIO( ConstCharVectorDataPtr(), "/", IndexedIO::Exclusive | IndexedIO::Write );

			getObject()->save( io, "object" );

			ConstCharVectorDataPtr buf = io->buffer();
			const CharVectorData::ValueType &data = buf->readable();

			out << "\"";
			for ( CharVectorData::ValueType::const_iterator it = data.begin(); it != data.end(); ++it )
			{
				out << decToHex( *it );
			}
			out << "\"";
		}
		catch ( std::exception &e )
		{
			MGlobal::displayError( e.what() );
			return MS::kFailure;
		}
	}

	return MS::kSuccess;
}

MStatus ObjectData::writeBinary( ostream& out )
{
	if ( m_object )
	{
		try
		{
			MemoryIndexedIOPtr io = new MemoryIndexedIO( ConstCharVectorDataPtr(), "/", IndexedIO::Exclusive | IndexedIO::Write );

			getObject()->save( io, "object" );

			ConstCharVectorDataPtr buf = io->buffer();
			const CharVectorData::ValueType &data = buf->readable();

			int sz = data.size();
			out.write( &data[0], sz );			
		}
		catch ( std::exception &e )
		{
			MGlobal::displayError( e.what() );
			return MS::kFailure;
		}
	}

	return MS::kSuccess;
}

void ObjectData::copy( const MPxData &other )
{
	const ObjectData *otherData = dynamic_cast<const ObjectData *>( &other );
	assert( otherData );

	setObject( otherData->getObject() );
}

MTypeId ObjectData::typeId() const
{
	return id;
}

MString ObjectData::name() const
{
	return typeName;
}

ObjectPtr ObjectData::getObject()
{
	return m_object;
}

ConstObjectPtr ObjectData::getObject() const
{
	return m_object;
}

void ObjectData::setObject( ConstObjectPtr otherObject )
{
	if ( otherObject )
	{
		m_object = otherObject->copy();
	}
	else
	{
		m_object = 0;
	}
}
