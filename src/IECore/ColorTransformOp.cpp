//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ColorTransformOp.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/Primitive.h"

using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( ColorTransformOp );

ColorTransformOp::ColorTransformOp( const std::string &description )
	:	PrimitiveOp( description )
{

	m_colorPrimVarParameter = new StringParameter(
		"colorPrimVar",
		"The name of the primitive variable which holds color data. This "
		"can have data of type Color3fData, Color3dData, Color3fVectorData, or Color3dVectorData.",
		"Cs"
	);

	m_redPrimVarParameter = new StringParameter(
		"redPrimVar",
		"The name of the primitive variable which holds the red channel of the color data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this primvar must match the type of the other color component primvars.",
		"R"
	);

	m_greenPrimVarParameter = new StringParameter(
		"greenPrimVar",
		"The name of the primitive variable which holds the green channel of the color data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this primvar must match the type of the other color component primvars.",
		"G"
	);

	m_bluePrimVarParameter = new StringParameter(
		"bluePrimVar",
		"The name of the primitive variable which holds the blue channel of the color data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this primvar must match the type of the other color component primvars.",
		"B"
	);

	m_alphaPrimVarParameter = new StringParameter(
		"alphaPrimVar",
		"The name of the primitive variable which holds the alpha channel. This is only used "
		"if the premultiplied parameter is on. The type must match the type of the color channels.",
		"A"
	);

	m_premultipliedParameter = new BoolParameter(
		"premultiplied",
		"If this is on, then the colors are divided by alpha before transformation and "
		"premultiplied again afterwards.",
		true
	);

	parameters()->addParameter( m_colorPrimVarParameter );
	parameters()->addParameter( m_redPrimVarParameter );
	parameters()->addParameter( m_greenPrimVarParameter );
	parameters()->addParameter( m_bluePrimVarParameter );
	parameters()->addParameter( m_alphaPrimVarParameter );
	parameters()->addParameter( m_premultipliedParameter );

}

ColorTransformOp::~ColorTransformOp()
{
}

StringParameter * ColorTransformOp::colorPrimVarParameter()
{
	return m_colorPrimVarParameter.get();
}

const StringParameter * ColorTransformOp::colorPrimVarParameter() const
{
	return m_colorPrimVarParameter.get();
}

StringParameter * ColorTransformOp::redPrimVarParameter()
{
	return m_redPrimVarParameter.get();
}

const StringParameter * ColorTransformOp::redPrimVarParameter() const
{
	return m_redPrimVarParameter.get();
}

StringParameter * ColorTransformOp::greenPrimVarParameter()
{
	return m_greenPrimVarParameter.get();
}

const StringParameter * ColorTransformOp::greenPrimVarParameter() const
{
	return m_greenPrimVarParameter.get();
}

StringParameter * ColorTransformOp::bluePrimVarParameter()
{
	return m_bluePrimVarParameter.get();
}

const StringParameter * ColorTransformOp::bluePrimVarParameter() const
{
	return m_bluePrimVarParameter.get();
}

StringParameter * ColorTransformOp::alphaPrimVarParameter()
{
	return m_alphaPrimVarParameter.get();
}

const StringParameter * ColorTransformOp::alphaPrimVarParameter() const
{
	return m_alphaPrimVarParameter.get();
}


BoolParameter * ColorTransformOp::premultipliedParameter()
{
	return m_premultipliedParameter.get();
}

const BoolParameter * ColorTransformOp::premultipliedParameter() const
{
	return m_premultipliedParameter.get();
}

template<typename T>
const typename T::BaseType *ColorTransformOp::alphaData( Primitive * primitive, size_t requiredElements )
{
	if( m_premultipliedParameter->getTypedValue()==false )
	{
		return 0;
	}

	PrimitiveVariableMap::const_iterator it = primitive->variables.find( m_alphaPrimVarParameter->getTypedValue() );
	if( it==primitive->variables.end() )
	{
		return 0;
	}
	if( !it->second.data )
	{
		return 0;
	}
	if( it->second.data->typeId()!=T::staticTypeId() )
	{
		throw Exception( "Alpha data type does not match color data type." );
	}

	T *d = static_cast<T *>( it->second.data.get() );
	if( d->baseSize()!=requiredElements )
	{
		throw Exception( "Alpha data has incorrect number of elements." );
	}

	return d->baseReadable();
}

template <typename T>
void ColorTransformOp::transformSeparate( Primitive * primitive, const CompoundObject * operands, T * r, T * g, T * b )
{
	size_t n = r->baseSize();
	const typename T::BaseType *alpha = alphaData<T>( primitive, n );

	typename T::BaseType *rw = r->baseWritable();
	typename T::BaseType *gw = g->baseWritable();
	typename T::BaseType *bw = b->baseWritable();

	begin( operands );

	try
	{
		for( size_t i=0; i<n; i++ )
		{
			Color3f c( *rw, *gw, *bw );
			if( alpha && *alpha > 0 )
			{
				c /= *alpha;
			}
			transform( c );
			if( alpha )
			{
				c *= *alpha++;
			}
			
			*rw++ = c[0];
			*gw++ = c[1];
			*bw++ = c[2];
		}
	}
	catch ( ... )
	{
		end();
		throw;
	}

	end();
}

template<typename T>
void ColorTransformOp::transformInterleaved( Primitive * primitive, const CompoundObject * operands, T * colors )
{
	assert( colors->baseSize() %3 == 0 );
	size_t numElements = colors->baseSize() / 3;

	const typename T::BaseType *alpha = alphaData<TypedData<std::vector<typename T::BaseType> > >( primitive, numElements );

	begin( operands );
	try
	{

		size_t i = 0;
		typename T::BaseType *data = colors->baseWritable();
		for( i=0; i<numElements; i++ )
		{
			Color3f c( data[0], data[1], data[2] );
			if( alpha && *alpha > 0 )
			{
				c /= *alpha;
			}
			transform( c );
			if( alpha )
			{
				c *= *alpha++;
			}
			*data++ = c[0];
			*data++ = c[1];
			*data++ = c[2];
		}
	}
	catch ( ... )
	{
		end();
		throw;
	}

	end();
}

void ColorTransformOp::modifyPrimitive( Primitive * primitive, const CompoundObject * operands )
{
	PrimitiveVariableMap::iterator colorIt = primitive->variables.find( m_colorPrimVarParameter->getTypedValue() );
	if( colorIt!=primitive->variables.end() && colorIt->second.data )
	{
		// RGB in a single channel
		switch( colorIt->second.data->typeId() )
		{
			case Color3fDataTypeId :
				transformInterleaved<Color3fData>( primitive, operands, static_cast<Color3fData *>( colorIt->second.data.get() ) );
				break;
			case Color3fVectorDataTypeId :
				transformInterleaved<Color3fVectorData>( primitive, operands, static_cast<Color3fVectorData *>( colorIt->second.data.get() ) );
				break;
			case Color3dDataTypeId :
				transformInterleaved<Color3dData>( primitive, operands, static_cast<Color3dData *>( colorIt->second.data.get() ) );
				break;
			case Color3dVectorDataTypeId :
				transformInterleaved<Color3dVectorData>( primitive, operands, static_cast<Color3dVectorData *>( colorIt->second.data.get() ) );
				break;
			default :
				throw Exception( "PrimitiveVariable has unsupported type." );
				break;
		}
	}
	else
	{
		// separate RGB channels?
		PrimitiveVariableMap::iterator rIt = primitive->variables.find( m_redPrimVarParameter->getTypedValue() );
		PrimitiveVariableMap::iterator gIt = primitive->variables.find( m_greenPrimVarParameter->getTypedValue() );
		PrimitiveVariableMap::iterator bIt = primitive->variables.find( m_bluePrimVarParameter->getTypedValue() );
		if( rIt==primitive->variables.end() || gIt==primitive->variables.end() || bIt==primitive->variables.end() )
		{
			throw Exception( "Primitive does not have appropriately named PrimitiveVariables." );
		}
		TypeId type = rIt->second.data->typeId();
		if( gIt->second.data->typeId() != type || bIt->second.data->typeId() != type )
		{
			throw Exception( "PrimitiveVariable types do not match." );
		}
		switch( type )
		{
			case HalfDataTypeId :
				transformSeparate<HalfData>(
					primitive,
					operands,
					static_cast<HalfData *>( rIt->second.data.get() ),
					static_cast<HalfData *>( gIt->second.data.get() ),
					static_cast<HalfData *>( bIt->second.data.get() )
				);
				break;
			case HalfVectorDataTypeId :
				transformSeparate<HalfVectorData>(
					primitive,
					operands,
					static_cast<HalfVectorData *>( rIt->second.data.get() ),
					static_cast<HalfVectorData *>( gIt->second.data.get() ),
					static_cast<HalfVectorData *>( bIt->second.data.get() )
				);
				break;
			case FloatDataTypeId :
				transformSeparate<FloatData>(
					primitive,
					operands,
					static_cast<FloatData *>( rIt->second.data.get() ),
					static_cast<FloatData *>( gIt->second.data.get() ),
					static_cast<FloatData *>( bIt->second.data.get() )
				);
				break;
			case FloatVectorDataTypeId :
				transformSeparate<FloatVectorData>(
					primitive,
					operands,
					static_cast<FloatVectorData *>( rIt->second.data.get() ),
					static_cast<FloatVectorData *>( gIt->second.data.get() ),
					static_cast<FloatVectorData *>( bIt->second.data.get() )
				);
				break;
			default :
				throw Exception( "PrimitiveVariables have unsupported type." );
				break;
		}
	}
}


void ColorTransformOp::begin( const CompoundObject * operands )
{
}

void ColorTransformOp::end()
{
}
