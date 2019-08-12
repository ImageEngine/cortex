//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/PDCParticleWriter.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECore/ByteOrder.h"
#include "IECore/DataCastOp.h"
#include "IECore/FileNameParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"

#include "boost/format.hpp"

#include <fstream>

using namespace IECore;
using namespace IECoreScene;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( PDCParticleWriter )

const Writer::WriterDescription<PDCParticleWriter> PDCParticleWriter::m_writerDescription( "pdc" );

PDCParticleWriter::PDCParticleWriter( )
	:	ParticleWriter( "Creates files in maya pdc format" )
{
}

PDCParticleWriter::PDCParticleWriter( ObjectPtr object, const std::string &fileName )
	:	ParticleWriter( "Creates files in maya pdc format" )
{
	m_objectParameter->setValue( object );
	m_fileNameParameter->setTypedValue( fileName );
}

template<class T, class E, unsigned int n>
static void writeAttr( ofstream &oStream, const T *attr )
{
	if( bigEndian() )
	{
		const typename T::ValueType &v = attr->readable();
		oStream.write( (const char *)&v[0], sizeof( E ) * n * v.size() );
	}
	else
	{
		typename T::Ptr attrCopy = attr->copy();
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
static void writeSimpleAttr( ofstream &oStream, const T *attr )
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

void PDCParticleWriter::doWrite( const CompoundObject *operands )
{
	// write the header
	int numParticles = particleCount();

	ofstream oStream( fileName().c_str(), std::ios_base::binary | std::ios_base::out );
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
		const IECore::TypeId t = attr->typeId();
		if( t==DoubleVectorDataTypeId || t==IntVectorDataTypeId || t==V3dVectorDataTypeId ||
			t==DoubleDataTypeId || t==IntDataTypeId || t==V3dDataTypeId ||
			t==FloatVectorDataTypeId || t==V3fVectorDataTypeId || t==FloatDataTypeId || t==V3fDataTypeId ||
			t==Color3fDataTypeId || t==Color3fVectorDataTypeId )
		{
			checkedAttrNames.push_back( *it );
		}
		else
		{
			msg( Msg::Warning, "PDCParticleWriter::write", format( "Attribute \"%s\" is of unsupported type \"%s\"." ) % *it % attr->typeName() );
		}
	}

	// write out the attributes
	DataCastOp castOp;
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
					IntVectorDataPtr d = boost::static_pointer_cast<IntVectorData>( attr );
					writeAttr<IntVectorData, int, 1>( oStream, d.get() );
				}
				break;
			// casting FloatVectorData to DoubleVectorData since PDCs don't handle floats
			case FloatVectorDataTypeId :
				{
					castOp.objectParameter()->setValue( attr );
					castOp.targetTypeParameter()->setNumericValue( DoubleVectorDataTypeId );
					attr = boost::static_pointer_cast<DoubleVectorData>( castOp.operate() );
				} // intentionally falling through
			case DoubleVectorDataTypeId :
				{
					int type = 3; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					DoubleVectorDataPtr d = boost::static_pointer_cast<DoubleVectorData>( attr );
					writeAttr<DoubleVectorData, double, 1>( oStream, d.get() );
				}
				break;
			// casting V3fVectorData and Color3fVectorData to V3dVectorData since PDCs don't handle floats
			case V3fVectorDataTypeId :
			case Color3fVectorDataTypeId :
				{
					castOp.objectParameter()->setValue( attr );
					castOp.targetTypeParameter()->setNumericValue( V3dVectorDataTypeId );
					attr = boost::static_pointer_cast<V3dVectorData>( castOp.operate() );
				} // intentionally falling through
			case V3dVectorDataTypeId :
				{
					int type = 5; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					V3dVectorDataPtr d = boost::static_pointer_cast<V3dVectorData>( attr );
					writeAttr<V3dVectorData, double, 3>( oStream, d.get() );
				}
				break;
			case IntDataTypeId :
				{
					int type = 0; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					IntDataPtr d = boost::static_pointer_cast<IntData>( attr );
					writeSimpleAttr<IntData, int, 1>( oStream, d.get() );
				}
				break;
			// casting FloatData to DoubleData since PDCs don't handle floats
			case FloatDataTypeId :
				{
					castOp.objectParameter()->setValue( attr );
					castOp.targetTypeParameter()->setNumericValue( DoubleDataTypeId );
					attr = boost::static_pointer_cast<DoubleData>( castOp.operate() );
				} // intentionally falling through
			case DoubleDataTypeId :
				{
					int type = 2; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					DoubleDataPtr d = boost::static_pointer_cast<DoubleData>( attr );
					writeSimpleAttr<DoubleData, double, 1>( oStream, d.get() );
				}
				break;
			// casting V3fData and Color3fData to V3dData since PDCs don't handle floats
			case V3fDataTypeId :
			case Color3fDataTypeId :
				{
					castOp.objectParameter()->setValue( attr );
					castOp.targetTypeParameter()->setNumericValue( V3dDataTypeId );
					attr = boost::static_pointer_cast<V3dData>( castOp.operate() );
				} // intentionally falling through
			case V3dDataTypeId :
				{
					int type = 4; type = asBigEndian( type );
					oStream.write( (const char *)&type, sizeof( type ) );
					V3dDataPtr d = boost::static_pointer_cast<V3dData>( attr );
					writeSimpleAttr<V3dData, double, 3>( oStream, d.get() );
				}
				break;

			default :
				// we should never get here because we checked the types above
				assert( 0 );
		}
	}
}
