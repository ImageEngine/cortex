//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/HitMissTransform.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace std;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( HitMissTransform );

HitMissTransform::HitMissTransform()
	:	ChannelOp( "Performs a hit and miss transformation of an image." )
{
	FloatParameterPtr thresholdParameter = new FloatParameter(
		"threshold",
		"The threshold above which pixels are considered to be part of the foreground.",
		0.5f
	);
	
	parameters()->addParameter( thresholdParameter );
	
	CompoundParameterPtr operationParameter = new CompoundParameter(
		"operation",
		"Parameters which specify the morphological operation applied to the image."
	);
	parameters()->addParameter( operationParameter );
	
	// structuring elements
	
	M33fVectorParameter::PresetsContainer structuringElementsPresets;
	
	std::vector<M33f> thinningElements;
	thinningElements.push_back(
		
		M33f(
			 0.f, 0.f,  0.f,
			-1.f, 1.f, -1.f,
			 1.f, 1.f,  1.f
		)
	
	);
	thinningElements.push_back(
		
		M33f(
			 -1.f, 0.f,  0.f,
			  1.f, 1.f,  0.f,
			 -1.f, 1.f, -1.f
		)
	
	);

	std::vector<M33f> dilatingElements;
	dilatingElements.push_back(
		
		M33f(
			 1.f, -1.f, -1.f,
			-1.f,  0.f, -1.f,
			-1.f, -1.f, -1.f
		)
		
	);
	
	dilatingElements.push_back(
		
		M33f(
			-1.f,  1.f, -1.f,
			-1.f,  0.f, -1.f,
			-1.f, -1.f, -1.f
		)
		
	);
	
	std::vector<M33f> pruningElements;
	pruningElements.push_back(
		
		M33f(
			 0.f,  0.f,  0.f,
			 0.f,  1.f,  0.f,
			 0.f, -1.f, -1.f
		)
		
	);
	
	pruningElements.push_back(
		
		M33f(
			 0.f,  0.f,  0.f,
			 0.f,  1.f,  0.f,
			-1.f, -1.f,  0.f
		)
		
	);
	
	structuringElementsPresets.push_back( M33fVectorParameter::Preset( "thinning", thinningElements ) );
	structuringElementsPresets.push_back( M33fVectorParameter::Preset( "dilation", dilatingElements ) );
	structuringElementsPresets.push_back( M33fVectorParameter::Preset( "pruning", pruningElements ) );
	
	M33fVectorParameterPtr structuringElementsParameter = new M33fVectorParameter(
		
		"structuringElements",
		
		"The structuring elements are 3x3 matrices specifying patterns of "
		"pixels to be detected. Values of 1 specify foreground pixels, values of 0 "
		"specify background pixels and values of -1 specify pixels whose value is irrelevant. "
		"For example the following pattern could be used to detect corner points : \n\n"

		"-1 1 -1"
		"0  1  1"
		"0  0  0",
		
		thinningElements,
		structuringElementsPresets
		
	);
	
	operationParameter->addParameter( structuringElementsParameter );
	
	// rotate structuring elements parameter
	BoolParameter::PresetsContainer rotateElementsPresets;
	rotateElementsPresets.push_back( BoolParameter::Preset( "thinning", true ) );
	rotateElementsPresets.push_back( BoolParameter::Preset( "dilation", true ) );
	rotateElementsPresets.push_back( BoolParameter::Preset( "pruning", true ) );
	
	BoolParameterPtr rotateElementsParameter = new BoolParameter(
		"rotateStructuringElements",
		"When this is true, each structuring element will be duplicated for each of "
		"the possible 90 degree rotations.",
		true,
		rotateElementsPresets
	);
	
	operationParameter->addParameter( rotateElementsParameter );
	
	// value parameter
	
	FloatParameter::PresetsContainer valuePresets;
	valuePresets.push_back( FloatParameter::Preset( "thinning", 0.0f ) );
	valuePresets.push_back( FloatParameter::Preset( "dilation", 1.0f ) );
	valuePresets.push_back( FloatParameter::Preset( "pruning", 0.0f ) );
	
	FloatParameterPtr valueParameter = new FloatParameter(
		"value",
		"The value to set a pixel to if it matches the structuring element.",
		0.0f,
		Imath::limits<float>::min(),
		Imath::limits<float>::max(),
		valuePresets
	);
	
	operationParameter->addParameter( valueParameter );
	
	// border value parameter
	
	FloatParameter::PresetsContainer borderValuePresets;
	borderValuePresets.push_back( FloatParameter::Preset( "thinning", 0.0f ) );
	borderValuePresets.push_back( FloatParameter::Preset( "dilation", 0.0f ) );
	borderValuePresets.push_back( FloatParameter::Preset( "pruning", 0.0f ) );
	
	FloatParameterPtr borderValueParameter = new FloatParameter(
		"borderValue",
		"The that pixels outside of the data window are considered to hold.",
		0.0f,
		Imath::limits<float>::min(),
		Imath::limits<float>::max(),
		borderValuePresets
	);
	
	operationParameter->addParameter( borderValueParameter );
	
	// iterations parameter
	
	IntParameter::PresetsContainer iterationsPresets;
	iterationsPresets.push_back( IntParameter::Preset( "thinning", 0 ) );
	iterationsPresets.push_back( IntParameter::Preset( "dilation", 5 ) );
	iterationsPresets.push_back( IntParameter::Preset( "pruning", 5 ) );
	
	IntParameterPtr iterationsParameter = new IntParameter(
		"iterations",
		"Specifies how many times to apply the transformation to the image. "
		"When left at 0 the process is iterated until it has no further effect.",
		0,
		0,
		Imath::limits<int>::max(),
		iterationsPresets
	);
	
	operationParameter->addParameter( iterationsParameter );
	
	// apply structuring elements alternately parameter
	BoolParameter::PresetsContainer applyElementsAlternatelyPresets;
	applyElementsAlternatelyPresets.push_back( BoolParameter::Preset( "thinning", true ) );
	applyElementsAlternatelyPresets.push_back( BoolParameter::Preset( "dilation", false ) );
	applyElementsAlternatelyPresets.push_back( BoolParameter::Preset( "pruning", false ) );
	
	BoolParameterPtr applyElementsAlternatelyParameter = new BoolParameter(
		"applyElementsAlternately",
		"When this is true, only one element is considered per iteration. When "
		"this is false all elements are considered on every iteration.",
		true,
		applyElementsAlternatelyPresets
	);
	
	operationParameter->addParameter( applyElementsAlternatelyParameter );
}

HitMissTransform::~HitMissTransform()
{
}

FloatParameterPtr HitMissTransform::thresholdParameter()
{
	return parameters()->parameter<FloatParameter>( "threshold" );
}

ConstFloatParameterPtr HitMissTransform::thresholdParameter() const
{
	return parameters()->parameter<FloatParameter>( "threshold" );
}

CompoundParameterPtr HitMissTransform::operationParameter()
{
	return parameters()->parameter<CompoundParameter>( "operation" );
}

ConstCompoundParameterPtr HitMissTransform::operationParameter() const
{
	return parameters()->parameter<CompoundParameter>( "operation" );
}

M33fVectorParameterPtr HitMissTransform::structuringElementsParameter()
{
	return operationParameter()->parameter<M33fVectorParameter>( "structuringElements" );
}

ConstM33fVectorParameterPtr HitMissTransform::structuringElementsParameter() const
{
	return operationParameter()->parameter<M33fVectorParameter>( "structuringElements" );
}

FloatParameterPtr HitMissTransform::valueParameter()
{
	return operationParameter()->parameter<FloatParameter>( "value" );
}

ConstFloatParameterPtr HitMissTransform::valueParameter() const
{
	return operationParameter()->parameter<FloatParameter>( "value" );
}

FloatParameterPtr HitMissTransform::borderValueParameter()
{
	return operationParameter()->parameter<FloatParameter>( "borderValue" );
}

ConstFloatParameterPtr HitMissTransform::borderValueParameter() const
{
	return operationParameter()->parameter<FloatParameter>( "borderValue" );
}

BoolParameterPtr HitMissTransform::rotateStructuringElementsParameter()
{
	return operationParameter()->parameter<BoolParameter>( "rotateStructuringElements" );
}

ConstBoolParameterPtr HitMissTransform::rotateStructuringElementsParameter() const
{
	return operationParameter()->parameter<BoolParameter>( "rotateStructuringElements" );
}
		
IntParameterPtr HitMissTransform::iterationsParameter()
{
	return operationParameter()->parameter<IntParameter>( "iterations" );
}

ConstParameterPtr HitMissTransform::iterationsParameter() const
{
	return operationParameter()->parameter<IntParameter>( "iterations" );
}

BoolParameterPtr HitMissTransform::applyElementsAlternatelyParameter()
{
	return operationParameter()->parameter<BoolParameter>( "applyElementsAlternately" );
}

ConstBoolParameterPtr HitMissTransform::applyElementsAlternatelyParameter() const
{
	return operationParameter()->parameter<BoolParameter>( "applyElementsAlternately" );
}

struct HitMissTransform::Thresholder
{
	typedef void ReturnType;

	Thresholder( float threshold, char borderValue, const V2i &size, std::vector<char> &output )
		:	m_threshold( threshold ), m_borderValue( borderValue ), m_size( size ), m_output( output )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::ConstPtr data )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::value_type V;

		const Container &input = data->readable();
		
		V2i paddedSize = m_size + V2i( 2 ); // padding with the border value makes the iteration easier later
		
		m_output.resize( paddedSize.x * paddedSize.y );

		V threshold = (V)m_threshold;

		std::fill( m_output.begin(), m_output.begin() + paddedSize.x, m_borderValue ); // 1 scanline padding at top
		int ii = 0;
		int oi = paddedSize.x;
		for( int y=0; y<m_size.y; y++ )
		{
			m_output[oi++] = m_borderValue; // 1 pixel padding at left
			for( int x=0; x<m_size.x; x++ )
			{
				m_output[oi++] = input[ii++] > threshold ? 1 : 0;
			}
			m_output[oi++] = m_borderValue; // 1 pixel padding at right
		}
		std::fill( m_output.begin() + oi, m_output.begin() + oi + paddedSize.x, m_borderValue ); // 1 scanline padding at bottom 
	}

	private :

		float m_threshold;
		char m_borderValue;
		const V2i &m_size;
		std::vector<char> &m_output;

};

struct HitMissTransform::Copyer
{
	typedef void ReturnType;

	Copyer( const V2i &size, const std::vector<char> &input )
		:	m_size( size ), m_input( input )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::value_type V;

		Container &output = data->writable();
		
		V2i paddedSize = m_size + V2i( 2 );

		V *op = &(output[0]);
		const char *ip = &(m_input[0]) + paddedSize.x + 1;
		for( int y=0; y<m_size.y; y++ )
		{
			for( int x=0; x<m_size.x; x++ )
			{
				*op++ = *ip++;
			}
			ip +=2; // skip padding on right and left of next row
		}
	}

	private :

		const V2i &m_size;
		const std::vector<char> &m_input;

};

void HitMissTransform::processMatrix( const Imath::M33f &m, int &mask, int &elements ) const
{
	mask = elements = 0;
	const float *x = m.getValue();
	for( unsigned i=0; i<9; i++)
	{
		if( *x >= 0.0f )
		{
			mask |= 3 << (i*2);
			if( *x > 0.0f )
			{
				elements |= 1 << (i*2);
			}
		}
		
		x++;
	}
}

void HitMissTransform::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	
	// process the structuring elements, including making rotated versions if requested.
	
	bool rotateElements = rotateStructuringElementsParameter()->getTypedValue();
	
	vector<int> masks;
	vector<int> elements;
	const std::vector<M33f> &matrices = structuringElementsParameter()->getTypedValue();
	for( unsigned i=0; i<matrices.size(); i++ )
	{
		int mask = 0;
		int element = 0;
		processMatrix( matrices[i], mask, element );
		masks.push_back( mask );
		elements.push_back( element );
		if( rotateElements )
		{
			M33f m = matrices[i];
			for( unsigned r=0; r<3; r++ )
			{
				M33f m2( m[0][2], m[1][2], m[2][2], m[0][1], m[1][1], m[2][1], m[0][0], m[1][0], m[2][0] );
				processMatrix( m2, mask, element );
				masks.push_back( mask );
				elements.push_back( element );
				m = m2;
			}
		}
	}
		
	// apply the operation to each channel
	
	char value = valueParameter()->getNumericValue() > thresholdParameter()->getNumericValue() ? 1 : 0;
	char borderValue = borderValueParameter()->getNumericValue() > thresholdParameter()->getNumericValue() ? 1 : 0;
	bool applyAlternately = applyElementsAlternatelyParameter()->getTypedValue();
	int numIterations = iterationsParameter()->getNumericValue();
	if( applyAlternately )
	{
		numIterations *= elements.size();
	}
	V2i size = dataWindow.size() + V2i( 1 );
	V2i paddedSize = size + V2i( 2 );
	
	std::vector<char> pixels;
	std::vector<char> pixels2; 
	for( unsigned i=0; i<channels.size(); i++ )
	{
		// threshold the image into a temporary pixels structure
		Thresholder thresholder( thresholdParameter()->getNumericValue(), borderValue, size, pixels );
		despatchTypedData<Thresholder, TypeTraits::IsNumericVectorTypedData>( channels[i].get(), thresholder );
		pixels2.clear(); pixels2.resize( pixels.size(), borderValue );
		
		// do the work
		unsigned iterationsSinceChange = 0;
		for( int n=0; n<numIterations || numIterations==0; n++ )
		{
			iterationsSinceChange++;
			for( int y=0; y<size.y; y++ )
			{
				const char *r0 = &(pixels[y * paddedSize.x]) + 1;
				const char *r1 = r0 + paddedSize.x;
				const char *r2 = r1 + paddedSize.x;
				char *ro = &(pixels2[(y+1) * paddedSize.x]) + 1;
				
				for( int x=0; x<size.x; x++ )
				{
					if( *r1==value )
					{
						// no point doing the work if the existing value is the one we'd change it to anyway
						*ro = value;
					}
					else
					{
						int v = r0[-1] 			| 	r0[0] << 2 		| 	r0[1] << 4 		|
								r1[-1] << 6 	| 	r1[0] << 8 		| 	r1[1] << 10 	|
								r2[-1] << 12 	| 	r2[0] << 14 	| 	r2[1] << 16;

						bool matches = false;
						if( applyAlternately )
						{
							size_t e = n % elements.size();
							if( (v & masks[e])==elements[e] )
							{
								matches = true;
							}				
						}
						else
						{
							for( size_t e=0; e<elements.size(); e++ )
							{
								if( (v & masks[e])==elements[e] )
								{
									matches = true;
									break;
								}
							}
						}

						if( matches )
						{
							*ro = value;
							iterationsSinceChange = 0;
						}
						else
						{
							*ro = *r1;
						}
					}
					
					ro++;
					r0++;
					r1++;
					r2++;
				}
			}
			pixels.swap( pixels2 );
			
			if( (applyAlternately && iterationsSinceChange==elements.size()) || ( !applyAlternately && iterationsSinceChange ) )
			{
				break;
			}
			
		}
		
		// and copy back into the original structure
		Copyer copyer( size, pixels );
		despatchTypedData<Copyer, TypeTraits::IsNumericVectorTypedData>( channels[i].get(), copyer );
	}
}

