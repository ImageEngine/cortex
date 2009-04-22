//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/LuminanceOp.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/Primitive.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( LuminanceOp );

LuminanceOp::LuminanceOp()
	:	PrimitiveOp( staticTypeName(), "Calculates luminance and adds it as a primitive variable." )
{
	
	m_colorPrimVarParameter = new StringParameter(
		"colorPrimVar", 
		"The name of the primitive variable which holds colour data. This "
		"can have data of type Color3fData or Color3fVectorData.",
		"Cs"
	);
	
	m_redPrimVarParameter = new StringParameter(
		"redPrimVar", 
		"The name of the primitive variable which holds the red channel of the colour data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this primvar must match the type of the other colour component primvars.",
		"R"
	);
	
	m_greenPrimVarParameter = new StringParameter(
		"greenPrimVar", 
		"The name of the primitive variable which holds the green channel of the colour data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this primvar must match the type of the other colour component primvars.",
		"G"
	);
	
	m_bluePrimVarParameter = new StringParameter(
		"bluePrimVar", 
		"The name of the primitive variable which holds the blue channel of the colour data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this primvar must match the type of the other colour component primvars.",
		"B"
	);
	
	m_weightsParameter = new Color3fParameter(
		"weights",
		"The weights used in averaging the rgb values to produce luminance.",
		Color3f( 0.2125, 0.7154, 0.0721 )
	);
	
	m_luminancePrimVarParameter = new StringParameter(
		"luminancePrimVar", 
		"The name of the primitive variable to hold the resulting luminance data.",
		"Y"
	);
	
	m_removeColorPrimVarsParameter = new BoolParameter(
		"removeColorPrimVars", 
		"When this is true, the input primitive variables are removed after luminance is calculated.",
		true
	);
	
	parameters()->addParameter( m_colorPrimVarParameter );
	parameters()->addParameter( m_redPrimVarParameter );
	parameters()->addParameter( m_greenPrimVarParameter );
	parameters()->addParameter( m_bluePrimVarParameter );
	parameters()->addParameter( m_weightsParameter );
	parameters()->addParameter( m_luminancePrimVarParameter );
	parameters()->addParameter( m_removeColorPrimVarsParameter );
	
}

LuminanceOp::~LuminanceOp()
{
}

StringParameterPtr LuminanceOp::colorPrimVarParameter()
{
	return m_colorPrimVarParameter;
}

ConstStringParameterPtr LuminanceOp::colorPrimVarParameter() const
{
	return m_colorPrimVarParameter;
}

StringParameterPtr LuminanceOp::redPrimVarParameter()
{
	return m_redPrimVarParameter;
}

ConstStringParameterPtr LuminanceOp::redPrimVarParameter() const
{
	return m_redPrimVarParameter;
}

StringParameterPtr LuminanceOp::greenPrimVarParameter()
{
	return m_greenPrimVarParameter;
}

ConstStringParameterPtr LuminanceOp::greenPrimVarParameter() const
{
	return m_greenPrimVarParameter;
}

StringParameterPtr LuminanceOp::bluePrimVarParameter()
{
	return m_bluePrimVarParameter;
}

ConstStringParameterPtr LuminanceOp::bluePrimVarParameter() const
{
	return m_bluePrimVarParameter;
}

Color3fParameterPtr LuminanceOp::weightsParameter()
{
	return m_weightsParameter;
}

ConstColor3fParameterPtr LuminanceOp::weightsParameter() const
{
	return m_weightsParameter;
}


StringParameterPtr LuminanceOp::luminancePrimVarParameter()
{
	return m_luminancePrimVarParameter;
}

ConstStringParameterPtr LuminanceOp::luminancePrimVarParameter() const
{
	return m_luminancePrimVarParameter;
}

BoolParameterPtr LuminanceOp::removeColorPrimVarsParameter()
{
	return m_removeColorPrimVarsParameter;
}

ConstBoolParameterPtr LuminanceOp::removeColorPrimVarsParameter() const
{
	return m_removeColorPrimVarsParameter;
}
		
template<typename T>
void LuminanceOp::calculate( const T *r, const T *g, const T *b, int steps[3], int size, T *y )
{
	Color3f weights = m_weightsParameter->getTypedValue();
	for( int i=0; i<size; i++ )
	{
		*y++ = weights[0] * *r + weights[1] * *g + weights[2] * *b;
		r += steps[0];
		g += steps[1];
		b += steps[2];
	}
}

void LuminanceOp::modifyPrimitive( PrimitivePtr primitive, ConstCompoundObjectPtr operands )
{
	DataPtr luminanceData = 0;
	PrimitiveVariable::Interpolation interpolation = PrimitiveVariable::Invalid;
	int steps[3] = { 1, 1, 1 };

	PrimitiveVariableMap::iterator colorIt = primitive->variables.find( m_colorPrimVarParameter->getTypedValue() );
	if( colorIt!=primitive->variables.end() && colorIt->second.data )
	{
		// RGB in a single channel
		switch( colorIt->second.data->typeId() )
		{
			case Color3fDataTypeId :
				{
					FloatDataPtr l = new FloatData;
					const float *d = boost::static_pointer_cast<Color3fData>( colorIt->second.data )->baseReadable();
					calculate( d, d + 1, d + 2, steps, 1, l->baseWritable() );
					luminanceData = l;
				}
				break;
			case Color3fVectorDataTypeId :
				{
					FloatVectorDataPtr l = new FloatVectorData;
					Color3fVectorDataPtr d = boost::static_pointer_cast<Color3fVectorData>( colorIt->second.data );
					l->writable().resize( d->readable().size() );
					const float *dd = d->baseReadable();
					steps[0] = steps[1] = steps[2] = 3;
					calculate( dd, dd + 1, dd + 2, steps, d->readable().size(), l->baseWritable() );
					luminanceData = l;
				}
				break;
			default :
				throw Exception( "PrimitiveVariable has unsupported type." );
				break;
		}
		interpolation = colorIt->second.interpolation;
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
		size_t rSize = despatchTypedData<TypedDataSize>( rIt->second.data );
		size_t gSize = despatchTypedData<TypedDataSize>( gIt->second.data );
		size_t bSize = despatchTypedData<TypedDataSize>( bIt->second.data );
		if( rSize!=gSize || rSize!=bSize )
		{
			throw Exception( "PrimitiveVariable sizes do not match." );
		}
		switch( type )
		{
			case HalfDataTypeId :
				{
					HalfDataPtr l = new HalfData;
					calculate(
						boost::static_pointer_cast<HalfData>( rIt->second.data )->baseReadable(),
						boost::static_pointer_cast<HalfData>( gIt->second.data )->baseReadable(),
						boost::static_pointer_cast<HalfData>( bIt->second.data )->baseReadable(),
						steps,
						rSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;
			case HalfVectorDataTypeId :
				{
					HalfVectorDataPtr l = new HalfVectorData;
					l->writable().resize( rSize );
					calculate(
						boost::static_pointer_cast<HalfVectorData>( rIt->second.data )->baseReadable(),
						boost::static_pointer_cast<HalfVectorData>( gIt->second.data )->baseReadable(),
						boost::static_pointer_cast<HalfVectorData>( bIt->second.data )->baseReadable(),
						steps,
						rSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;
			case FloatDataTypeId :
				{
					FloatDataPtr l = new FloatData;
					calculate(
						boost::static_pointer_cast<FloatData>( rIt->second.data )->baseReadable(),
						boost::static_pointer_cast<FloatData>( gIt->second.data )->baseReadable(),
						boost::static_pointer_cast<FloatData>( bIt->second.data )->baseReadable(),
						steps,
						rSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;
			case FloatVectorDataTypeId :
				{
					FloatVectorDataPtr l = new FloatVectorData;
					l->writable().resize( rSize );
					calculate(
						boost::static_pointer_cast<FloatVectorData>( rIt->second.data )->baseReadable(),
						boost::static_pointer_cast<FloatVectorData>( gIt->second.data )->baseReadable(),
						boost::static_pointer_cast<FloatVectorData>( bIt->second.data )->baseReadable(),
						steps,
						rSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;	
			default :
				throw Exception( "PrimitiveVariables have unsupported type." );
				break;
		}
		interpolation = rIt->second.interpolation;
	}
	
	assert( interpolation != PrimitiveVariable::Invalid );
	assert( luminanceData );
	
	primitive->variables[luminancePrimVarParameter()->getTypedValue()] = PrimitiveVariable( interpolation, luminanceData );
	
	if( removeColorPrimVarsParameter()->getTypedValue() )
	{
		primitive->variables.erase( colorPrimVarParameter()->getTypedValue() );
		primitive->variables.erase( redPrimVarParameter()->getTypedValue() );
		primitive->variables.erase( greenPrimVarParameter()->getTypedValue() );
		primitive->variables.erase( bluePrimVarParameter()->getTypedValue() );
	}
}
