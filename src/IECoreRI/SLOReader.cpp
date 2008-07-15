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

#include "IECoreRI/SLOReader.h"

#include "IECore/Shader.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundObject.h"

#include "slo.h"

using namespace IECore;
using namespace IECoreRI;
using namespace boost;
using namespace std;
using namespace Imath;

const Reader::ReaderDescription<SLOReader> SLOReader::m_readerDescription( "sdl" );

SLOReader::SLOReader()
	:	Reader( "SLOReader", "Reads compiled renderman shaders.", new ObjectParameter( "result", "The loaded shader", new NullObject, Shader::staticTypeId() ) )
{	
}

SLOReader::SLOReader( const std::string &fileName )
	:	Reader( "SLOReader", "Reads compiled renderman shaders.", new ObjectParameter( "result", "The loaded shader", new NullObject, Shader::staticTypeId() ) )
{
	m_fileNameParameter->setTypedValue( fileName );
}

SLOReader::~SLOReader()
{
}

bool SLOReader::canRead( const std::string &fileName )
{
	if( Slo_SetShader( fileName.c_str() ) )
	{
		return false;
	}
	Slo_EndShader();
	return true;
}

ObjectPtr SLOReader::doOperation( ConstCompoundObjectPtr operands )
{
	if( Slo_SetShader( fileName().c_str() ) )
	{
		return 0;
	}

	string name = Slo_GetName();
	string type = Slo_TypetoStr( Slo_GetType() );
	ShaderPtr result = new Shader( name, type );
	
	CompoundDataPtr typeHints = new CompoundData;
	result->blindData()->writable().insert( pair<string, DataPtr>( "ri:parameterTypeHints", typeHints ) );
	
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
					if( arg->svd_arraylen==0 )
					{
						const SLO_POINT *p = arg->svd_default.pointval;
						data = new V3fData( V3f( p->xval, p->yval, p->zval ) );
					}
					else
					{
						V3fVectorDataPtr vData = new V3fVectorData();
						data = vData;
						for( int j=0; j<arg->svd_arraylen; j++ )
						{
							SLO_VISSYMDEF *a = Slo_GetArrayArgElement( arg, j );
							const SLO_POINT *p = a->svd_default.pointval;
							vData->writable().push_back( V3f( p->xval, p->yval, p->zval ) );
						}
					}
					typeHints->writable().insert( pair<string, DataPtr>( arg->svd_name, new StringData( Slo_TypetoStr( arg->svd_type ) ) ) );
					break;
				}
				
			case SLO_TYPE_COLOR :
				{
					if( arg->svd_arraylen==0 )
					{
						const SLO_POINT *p = arg->svd_default.pointval;
						data = new Color3fData( Color3f( p->xval, p->yval, p->zval ) );
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
						data = new FloatData( *(arg->svd_default.scalarval) );
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
					}
				}	
				break;
		
			case SLO_TYPE_STRING :
				{
					if( arg->svd_arraylen==0 )
					{
						data = new StringData( arg->svd_default.stringval );
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
						M44f mm(	m[0], m[1], m[2], m[3],
									m[4], m[5], m[6], m[7],
									m[8], m[9], m[10], m[11],
									m[12], m[13], m[14], m[15] 	);
						data = new M44fData( mm );
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
	
			default :
			
				msg( Msg::Warning, "SLOReader::read", format( "Parameter \"%s\" has unsupported type." ) % arg->svd_name );
		}
		
		if( data )
		{
			result->parameters().insert( CompoundDataMap::value_type( arg->svd_name, data ) );
		}
	
	}

	return result;
}

