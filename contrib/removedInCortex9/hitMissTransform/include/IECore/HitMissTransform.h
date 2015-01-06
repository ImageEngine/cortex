//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_HITMISSTRANSFORM_H
#define IECORE_HITMISSTRANSFORM_H

#include "IECore/Export.h"
#include "IECore/ChannelOp.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// Implements the hit and miss transform, which can be used for thinning and
/// thickening and various other operations on binary images. Note that the ImageThinner
/// provides a much more refined thinning than this class. See the following
/// webpage for a nice friendly description.
///
/// http://homepages.inf.ed.ac.uk/rbf/HIPR2/hitmiss.htm
/// \ingroup imageProcessingGroup
class IECORE_API HitMissTransform : public ChannelOp
{
	public:

		HitMissTransform();
		virtual ~HitMissTransform();

		IE_CORE_DECLARERUNTIMETYPED( HitMissTransform, ChannelOp );

		/// The transform considers images to be binary, whereby each pixel is
		/// either a member of the foreground or background. Pixels with a value
		/// below the threshold are considered to be background, and those above
		/// the threshold are considered to be foreground.
		FloatParameterPtr thresholdParameter();
		ConstFloatParameterPtr thresholdParameter() const;

		/// This parameter is the parent of all the parameters below. It contains a number
		/// of named presets which simplify the specification of common operations.
		CompoundParameterPtr operationParameter();
		ConstCompoundParameterPtr operationParameter() const;

		/// The structuring elements are 3x3 matrices specifying patterns of
		/// pixels to be detected. Values of 1 specify foreground pixels, values of 0
		/// specify background pixels and values of -1 specify pixels whose value is irrelevant.
		///
		/// For example the following pattern could be used to detect corner points :
		///
		/// -1 1 -1
		/// 0  1  1
		/// 0  0  0
		///
		/// This parameter contains named presets for common operations such as thinning.
		M33fVectorParameterPtr structuringElementsParameter();
		ConstM33fVectorParameterPtr structuringElementsParameter() const;

		/// When this parameter is set to true, the structuring elements will be
		/// duplicated for each of the possible 90 degree rotations. This is just
		/// a convenience to simplify the specification of the elements.
		BoolParameterPtr rotateStructuringElementsParameter();
		ConstBoolParameterPtr rotateStructuringElementsParameter() const;
				
		/// The value that pixels are set to if they match the structuring element.
		FloatParameterPtr valueParameter();
		ConstFloatParameterPtr valueParameter() const;
		
		/// The value that pixels outside of the data window are considered to have.
		FloatParameterPtr borderValueParameter();
		ConstFloatParameterPtr borderValueParameter() const;

		/// Specifies how many times to apply the transformation to the image. When left at 0
		/// the process is iterated until it has no further effect.
		IntParameterPtr iterationsParameter();
		ConstParameterPtr iterationsParameter() const;

		/// When this parameter is true, only one structuring element is applied
		/// per iteration. When it is false, all elements are considered during each iteration.
		BoolParameterPtr applyElementsAlternatelyParameter();
		ConstBoolParameterPtr applyElementsAlternatelyParameter() const;

	protected :

		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels );

	private :

		void processMatrix( const Imath::M33f &m, int &mask, int &elements ) const;

		struct Thresholder;
		struct Copyer;

};

IE_CORE_DECLAREPTR( HitMissTransform );

} // namespace IECore

#endif // IECORE_HITMISSTRANSFORM_H

