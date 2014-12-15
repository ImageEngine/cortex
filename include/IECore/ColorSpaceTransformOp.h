//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_COLORSPACETRANSFORMOP_H
#define IECORE_COLORSPACETRANSFORMOP_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"
#include "boost/function.hpp"

#include "IECore/Export.h"
#include "IECore/TypedPrimitiveOp.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

/// The ColorSpaceTransformOp allows complex color space operations to performed on images by chaining together simpler ones. e.g.
/// An SRGB->Rec709 op can be constructed from a combination of SRGB->Linear followed by Linear->Rec709. If multiple paths are possible
/// for a requested conversion then the shorted one is used. Static and dynamic registration mechanisms are provided to allow the
/// addition of new transformations.
/// \ingroup imageProcessingGroup
class IECORE_API ColorSpaceTransformOp : public ImagePrimitiveOp
{
	public:

		/// Typedefs make code easier to read
		typedef std::string InputColorSpace;
		typedef std::string OutputColorSpace;

		ColorSpaceTransformOp();
		virtual ~ColorSpaceTransformOp();

		IE_CORE_DECLARERUNTIMETYPED( ColorSpaceTransformOp, ImagePrimitiveOp );

		StringParameter *inputColorSpaceParameter();
		const StringParameter *inputColorSpaceParameter() const;

		StringParameter *outputColorSpaceParameter();
		const StringParameter *outputColorSpaceParameter() const;

		/// The order of the channels listed is important if the conversion is done by a ColorTransformOp.
		/// In that case it is expected to receive Red,Green,Blue channels respectively.
		StringVectorParameter *channelsParameter();
		const StringVectorParameter *channelsParameter() const;

		StringParameter *alphaPrimVarParameter();
		const StringParameter *alphaPrimVarParameter() const;

		BoolParameter *premultipliedParameter();
		const BoolParameter *premultipliedParameter() const;

		/// Definition of a function which can create a Color space converter when given the color spaces.
		/// ModifyOp is the most-derived common base class of ChannelOp and ColorTransformOp
		typedef boost::function<ModifyOpPtr ( const InputColorSpace &, const OutputColorSpace & )> CreatorFn;

		static void registerConversion( const InputColorSpace &, const OutputColorSpace &, const CreatorFn &creator );

		static void inputColorSpaces( std::vector< InputColorSpace > &colorSpaces );
		static void outputColorSpaces( std::vector< OutputColorSpace > &colorSpaces );
		static void colorSpaces( std::vector< std::string > &colorSpaces );

		template<typename T>
		class ColorSpaceDescription
		{
			public:
				ColorSpaceDescription( const InputColorSpace &, const OutputColorSpace & );
			protected :
				static ModifyOpPtr createOp( const InputColorSpace &, const OutputColorSpace & );
		};

	protected :

		typedef std::pair< InputColorSpace, OutputColorSpace > Conversion;
		typedef boost::tuple< CreatorFn *, InputColorSpace, OutputColorSpace > ConversionInfo;

		void findConversion( const InputColorSpace &, const OutputColorSpace &, std::vector< ConversionInfo > &conversions );
		void findConversion(
			const InputColorSpace &,
			const OutputColorSpace &,
			std::set< Conversion > &visitedConversions,
			std::vector< ConversionInfo > &currentConversion,
			std::vector< ConversionInfo > &bestConversion
			);
	 	virtual void modifyTypedPrimitive( ImagePrimitive *image, const CompoundObject *operands );

		StringParameterPtr m_inputColorSpaceParameter;
		StringParameterPtr m_outputColorSpaceParameter;
		StringVectorParameterPtr m_channelsParameter;
		StringParameterPtr m_alphaPrimVarParameter;
		BoolParameterPtr m_premultipliedParameter;

		typedef std::multimap< InputColorSpace, ConversionInfo > ConvertersMap;
		typedef std::map< CreatorFn *, Conversion > ConverterTypesMap;
		typedef std::set< Conversion > ConversionsSet;

		static ConvertersMap &converters();
		static ConverterTypesMap &converterTypes();
		static ConversionsSet &conversionsSet();
};

IE_CORE_DECLAREPTR( ColorSpaceTransformOp );

} // namespace IECore

#include "IECore/ColorSpaceTransformOp.inl"

#endif // IECORE_COLORSPACETRANSFORMOP_H

