//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PDCParticleWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/FileNameParameter.h"
#include "IECore/PointsPrimitive.h"

#include "boost/format.hpp"

#include <fstream>

using namespace IECore;
using namespace std;
using namespace boost;

const Writer::WriterDescription<PDCParticleWriter> PDCParticleWriter::m_writerDescription( "pdc" );

PDCParticleWriter::PDCParticleWriter( )
	:	ParticleWriter( "PDCParticleWriter", "Creates files in maya pdc format" )
{
}

PDCParticleWriter::PDCParticleWriter( ObjectPtr object, const std::string &fileName )
	:	ParticleWriter( "PDCParticleWriter", "Creates files in maya pdc format" )
{
	m_objectParameter->setValue( object );
	m_fileNameParameter->setTypedValue( fileName );
}

template<class T, class E, unsigned int n>
static void writeAttr( ofstream &oStream, intrusive_ptr<T> attr )
{
	if( bigEndian() )
	{
		const typename T::ValueType &v = attr->readable();
		oStream.write( (const char *)&v[0], sizeof( E ) * n * v.size() );
	}
	else
	{
		intrusive_ptr<T> attrCopy = attr->copy();
		typename T::ValueType &v = attrCopy->writable();
		E *data = (E *)&v[0];
		for( size_t i=0; i<v.size() * n; i++ )
		{
			*data = reverseBytes( *data );
			data++;
		}
		oStream.write( (const char *)&v[0], sizeof( E ) * n * v.size() );
	}
}

template<class T, class E, unsigned int n>
static void writeSimpleAttr( ofstream &oStream, intrusive_ptr<T> attr )
{
	if( bigEndian() )
	{
		oStream.write( (const char *)&attr->readable(), sizeof( E ) * n );
	}
	else
	{
		typename T::ValueType v = attr->readable();
		E *data = (E *)&v;
		for( size_t i=0; i<n; i++ )
		{
			*data = reverseBytes( *data );
			data++;
		}
		oStream.write( (const char *)&v, sizeof( E ) * n );
	}
}

void PDCParticleWriter::doWrite()
{	
	// write the header
	int numParticles = particleCount();
	
	ofstream oStream( fileName().c_str() );
	if( !oStream.is_open() )
	{
		throw IOException( ( format( "Unable to open file \%s\"." ) % fileName() ).str() );
	}
	
	char pdc[4] = { 'P', 'D', 'C', ' ' };
	oStream.write( pdc, 4 );
	
	int fileVersion = 1; fileVersion = asBigEndian( fileVersion );
	oStream.write( (const char *)&fileVersion, sizeof( fileVersion ) );
	
	int one = 1; one = asBigEndian( one );
	oStream.write( (const char *)&one, sizeof( one ) );
	
	int unused = 0; unused = asBigEndian( unused );
	oStream.write( (const char *)&unused, sizeof( unused ) );
	oStream.write( (const char *)&unused, sizeof( unused ) );

	int numParticlesReversed = asBigEndian( numParticles );
	oStream.write( (const char *)&numParticlesReversed, sizeof( numParticlesReversed ) );
	
	// check each attribute is of an appropriate type
	const PrimitiveVariableMap &pv = particleObject()->variables;
	vector<string> attrNames;
	particleAttributes( attrNames );
	vector<string> checkedAttrNames;
	for( vector<string>::const_iterator it = attrNames.begin(); it!=attrNames.end(); it++ )
	{
		DataPtr attr = pv.find( *it )->second.data;
		TypeId t = attr->typeId();
		if( t==DoubleVectorDataTypeId || t==IntVectorDataTypeId || t==V3dVectorDataTypeId ||
			t==DoubleDataTypeId || t==IntDataTypeId || t==V3dDataTypeId )
		{
			checkedAttrNames.push_back( *it );
		}
		else
		{
			msg( Msg::Warning, "PDCParticleWriter::write", format( "Attribute \"%s\" is of unsupported type \"%s\"." ) % *it % attr->typeName() );
		}
	}
		
	// write out the attributes
	int numAttrs = checkedAttrNames.size();
	int numAttrsReversed = asBigEndian( numAttrs );
	oStream.write( (const char *)&numAttrsReversed, sizeof( numAttrsReversed ) );
	for( vector<string>::const_iterator it = checkedAttrNames.begin(); it!=checkedAttrNames.end(); it++ )
	{
		int nameLength = it->size();
		int nameLengthReversed = asBigEndian( nameLength );
		oStream.write( (const char *)&nameLengthReversed, sizeof( nameLengthReversed ) );
		oStream.write( it->c_str(), nameLength );
		
		DataPtr attr = pv.find( *it )->second.data;
		switch( attr->typeId() )
		{
			case IntVectorDataTypeId :
				{
					int type = 1; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					IntVectorDataPtr d = static_pointer_cast<IntVectorData>( attr );
					writeAttr<IntVectorData, int, 1>( oStream, d );
				}
				break;
			case DoubleVectorDataTypeId :
				{
					int type = 3; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					DoubleVectorDataPtr d = static_pointer_cast<DoubleVectorData>( attr );
					writeAttr<DoubleVectorData, double, 1>( oStream, d );
				}
				break;
				
			case V3dVectorDataTypeId :
				{
					int type = 5; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					V3dVectorDataPtr d = static_pointer_cast<V3dVectorData>( attr );
					writeAttr<V3dVectorData, double, 3>( oStream, d );
				}
				break;
			case IntDataTypeId :
				{
					int type = 0; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					IntDataPtr d = static_pointer_cast<IntData>( attr );
					writeSimpleAttr<IntData, int, 1>( oStream, d );
				}
				break;
			case DoubleDataTypeId :
				{
					int type = 2; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					DoubleDataPtr d = static_pointer_cast<DoubleData>( attr );
					writeSimpleAttr<DoubleData, double, 1>( oStream, d );
				}
				break;
			case V3dDataTypeId :
				{
					int type = 4; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					V3dDataPtr d = static_pointer_cast<V3dData>( attr );
					writeSimpleAttr<V3dData, double, 3>( oStream, d );
				}
				break;	
				
			default :
				// we should never get here because we checked the types above
				assert( 0 );
		}
	}
}
