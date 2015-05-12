//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "tbb/mutex.h"

#include "IECoreRI/SLOReader.h"

#include "IECore/Shader.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundObject.h"

#include "slo.h"

#ifdef PRMANEXPORT
typedef POINT3D SLO_POINT;
#endif

using namespace IECore;
using namespace IECoreRI;
using namespace boost;
using namespace std;
using namespace Imath;

const Reader::ReaderDescription<SLOReader> SLOReader::m_readerDescription( "sdl" );
static tbb::mutex g_mutex;

SLOReader::SLOReader()
	:	Reader( "Reads compiled renderman shaders.", new ObjectParameter( "result", "The loaded shader", new NullObject, Shader::staticTypeId() ) )
{
}

SLOReader::SLOReader( const std::string &fileName )
	:	Reader( "Reads compiled renderman shaders.", new ObjectParameter( "result", "The loaded shader", new NullObject, Shader::staticTypeId() ) )
{
	m_fileNameParameter->setTypedValue( fileName );
}

SLOReader::~SLOReader()
{
}

bool SLOReader::canRead( const std::string &fileName )
{
	// Avoid seg faults from 3delight if we pass arbitrary files.
	if ( fileName.size() < 4 || fileName.substr( fileName.size() - 4, 4 ) != ".sdl" )
	{
		return false;
	}

	tbb::mutex::scoped_lock lock( g_mutex );
	
	if( Slo_SetShader( (char *)fileName.c_str() ) )
	{
		return false;
	}
	Slo_EndShader();
	return true;
}

ObjectPtr SLOReader::doOperation( const CompoundObject * operands )
{
	tbb::mutex::scoped_lock lock( g_mutex );
	
	if( Slo_SetShader( (char *)fileName().c_str() ) )
	{
		throw Exception( boost::str( boost::format( "Unable to set shader to \"%s\"" ) % fileName() ) );
	}

	string name = Slo_GetName();
	string type = Slo_TypetoStr( Slo_GetType() );
	ShaderPtr result = new Shader( name, type );

	CompoundDataPtr typeHints = new CompoundData;
	result->blindData()->writable().insert( pair<string, DataPtr>( "ri:parameterTypeHints", typeHints ) );

	// we lose the ordering of parameter names when we put them in result->parameters(),
	// so we stick the correct order in the blind data as a workaround for anyone interested
	// in the true ordering.
	StringVectorDataPtr orderedParameterNames = new StringVectorData;
	result->blindData()->writable().insert( pair<string, DataPtr>( "ri:orderedParameterNames", orderedParameterNames ) );	

	// we don't have a way of communicating which parameters are outputs in the Shader::parametersData(),
	// so we work around that using the blind data too.
	StringVectorDataPtr outputParameterNames = new StringVectorData;
	result->blindData()->writable().insert( pair<string, DataPtr>( "ri:outputParameterNames", outputParameterNames ) );	

	int numArgs = Slo_GetNArgs();
	for( int i=1; i<=numArgs; i++ )
	{
		DataPtr data = 0;

		SLO_VISSYMDEF *arg = Slo_GetArgById( i );
		switch( arg->svd_type )
		{
			case SLO_TYPE_POINT :
			case SLO_TYPE_VECTOR :
			case SLO_TYPE_NORMAL :
				{
					GeometricData::Interpretation interpretation;
					switch( arg->svd_type )
					{
						case SLO_TYPE_POINT : 
							interpretation = GeometricData::Point;
							break;
						case SLO_TYPE_VECTOR :
							interpretation = GeometricData::Vector;
							break;
						case SLO_TYPE_NORMAL :
							interpretation = GeometricData::Normal;
							break;
						default:
							interpretation = GeometricData::Numeric;
							break;
					}
					
					if( arg->svd_arraylen==0 )
					{
						const SLO_POINT *p = arg->svd_default.pointval;
						if( p )
						{
							data = new V3fData( V3f( p->xval, p->yval, p->zval ), interpretation );
						}
						else
						{
							// 0 length and null value signifies a variable length array
							V3fVectorDataPtr vData = new V3fVectorData;
							vData->setInterpretation( interpretation );
							data = vData;
						}
					}
					else
					{
						V3fVectorDataPtr vData = new V3fVectorData;
						for( int j=0; j<arg->svd_arraylen; j++ )
						{
							SLO_VISSYMDEF *a = Slo_GetArrayArgElement( arg, j );
							const SLO_POINT *p = a->svd_default.pointval;
							vData->writable().push_back( V3f( p->xval, p->yval, p->zval ) );
						}
						vData->setInterpretation( interpretation );
						data = vData;
					}

					typeHints->writable().insert( pair<string, DataPtr>( arg->svd_name, new StringData( Slo_TypetoStr( arg->svd_type ) ) ) );
					break;
				}

			case SLO_TYPE_COLOR :
				{
					if( arg->svd_arraylen==0 )
					{
						const SLO_POINT *p = arg->svd_default.pointval;
						if( p )
						{
							data = new Color3fData( Color3f( p->xval, p->yval, p->zval ) );
						}
						else
						{
							// 0 length and null value signifies a variable length array
							data = new Color3fVectorData();
						}
					}
					else
					{
						Color3fVectorDataPtr vData = new Color3fVectorData();
						data = vData;
						for( int j=0; j<arg->svd_arraylen; j++ )
						{
							SLO_VISSYMDEF *a = Slo_GetArrayArgElement( arg, j );
							const SLO_POINT *p = a->svd_default.pointval;
							vData->writable().push_back( Color3f( p->xval, p->yval, p->zval ) );
						}
					}
				}
				break;

			case SLO_TYPE_SCALAR :
				{
					if( arg->svd_arraylen==0 )
					{
						const float *value = arg->svd_default.scalarval;
						if( value )
						{
							data = new FloatData( *value );
						}
						else
						{
							// 0 length and null value signifies a variable length array
							data = new FloatVectorData();
						}
					}
					else
					{
						FloatVectorDataPtr vData = new FloatVectorData();
						data = vData;
						for( int j=0; j<arg->svd_arraylen; j++ )
						{
							SLO_VISSYMDEF *a = Slo_GetArrayArgElement( arg, j );
							vData->writable().push_back( *(a->svd_default.scalarval) );
						}
						if( arg->svd_arraylen==3 )
						{
							// allow V3fData and V3fVectorData to be mapped to float[3] parameters.
							typeHints->writable().insert( pair<string, DataPtr>( arg->svd_name, new StringData( "float[3]" ) ) );
						}
					}
				}
				break;

			case SLO_TYPE_STRING :
				{
					if( arg->svd_arraylen==0 )
					{
						const char *defaultValue = arg->svd_default.stringval;
						if( defaultValue )
						{
							data = new StringData( defaultValue );
						}
						else
						{
							// 0 length and null value signifies a variable length array
							data = new StringVectorData();
						}
					}
					else
					{
						StringVectorDataPtr vData = new StringVectorData();
						data = vData;
						for( int j=0; j<arg->svd_arraylen; j++ )
						{
							SLO_VISSYMDEF *a = Slo_GetArrayArgElement( arg, j );
							// sometimes the default value for an element of a string array can be a null pointer.
							// i'm not sure what the meaning of this is. the 3delight shaderinfo utility reports such values
							// as "(null)", so that's what we do too.
							const char *defaultValue = a->svd_default.stringval;
							vData->writable().push_back( defaultValue ? defaultValue : "(null)" );
						}
					}
				}
				break;

			case SLO_TYPE_MATRIX :
				{
					if( arg->svd_arraylen==0 )
					{
						const float *m = arg->svd_default.matrixval;
						if( m )
						{
							M44f mm(	m[0], m[1], m[2], m[3],
										m[4], m[5], m[6], m[7],
										m[8], m[9], m[10], m[11],
										m[12], m[13], m[14], m[15] 	);
							data = new M44fData( mm );
						}
						else
						{
							// 0 length and null value signifies a variable length array
							data = new M44fVectorData();
						}
					}
					else
					{
						M44fVectorDataPtr vData = new M44fVectorData();
						data = vData;
						for( int j=0; j<arg->svd_arraylen; j++ )
						{
							SLO_VISSYMDEF *a = Slo_GetArrayArgElement( arg, j );
							const float *m = a->svd_default.matrixval;
							M44f mm(	m[0], m[1], m[2], m[3],
										m[4], m[5], m[6], m[7],
										m[8], m[9], m[10], m[11],
										m[12], m[13], m[14], m[15] 	);
							vData->writable().push_back( mm );
						}
					}
				}
				break;
				
			case SLO_TYPE_SHADER :
				{
					if( arg->svd_arraylen==0 )
					{
						if( !arg->svd_valisvalid )
						{
							// variable length array
							data = new StringVectorData();
						}
						else
						{
							data = new StringData();
						}
					}
					else
					{
						StringVectorDataPtr sData = new StringVectorData();
						data = sData;
						sData->writable().resize( arg->svd_arraylen );
					}
					typeHints->writable().insert( pair<string, DataPtr>( arg->svd_name, new StringData( Slo_TypetoStr( arg->svd_type ) ) ) );
				}
				break;

			default :

				msg( Msg::Warning, "SLOReader::read", format( "Parameter \"%s\" has unsupported type." ) % arg->svd_name );
		}

		if( data )
		{
			orderedParameterNames->writable().push_back( arg->svd_name );
			result->parameters().insert( CompoundDataMap::value_type( arg->svd_name, data ) );
			if( arg->svd_storage == SLO_STOR_OUTPUTPARAMETER )
			{
				outputParameterNames->writable().push_back( arg->svd_name );
			}
		}

	}

	// shader annotations
	
	CompoundDataPtr annotations = new CompoundData;
	result->blindData()->writable().insert( pair<string, DataPtr>( "ri:annotations", annotations ) );
	
#ifndef PRMANEXPORT
	for( int i=1, n=Slo_GetNAnnotations(); i <= n; i++ )
	{
		const char *key = Slo_GetAnnotationKeyById( i );
		annotations->writable()[key] = new StringData( Slo_GetAnnotationByKey( key ) );
	}
#endif

	Slo_EndShader();
	return result;
}

