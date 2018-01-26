//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREIMAGE_LENSDISTORT_H
#define IECOREIMAGE_LENSDISTORT_H

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"
#include "IECoreImage/WarpOp.h"

#include "IECore/LensModel.h"
#include "IECore/ObjectParameter.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/VectorTypedData.h"

namespace IECoreImage
{

/// Distorts an ImagePrimitive using a parametric lens model.
/// This Op expects a CompoundObject which contains the lens model's parameters.
/// \ingroup imageProcessingGroup
class IECOREIMAGE_API LensDistortOp : public WarpOp
{
	public:

		LensDistortOp();
		~LensDistortOp() override;

		IECore::ObjectParameter * lensParameter();
		const IECore::ObjectParameter * lensParameter() const;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( LensDistortOp, LensDistortOpTypeId, WarpOp );

	protected :

		void begin( const IECore::CompoundObject * operands ) override;
		Imath::Box2i warpedDataWindow( const Imath::Box2i &dataWindow ) const override;
		Imath::V2f warp( const Imath::V2f &p ) const override;
		void end() override;

	private :

		enum Mode
		{
			kUndistort = 0,
			kDistort = 1
		};

		int m_mode;
		IECore::LensModelPtr m_lensModel;
		IECore::ObjectParameterPtr m_lensParameter;
		IECore::IntParameterPtr m_modeParameter;
		Imath::Box2i m_distortedDataWindow;
		IECore::FloatVectorDataPtr m_cachePtr;
};

IE_CORE_DECLAREPTR( LensDistortOp );

} // namespace IECoreImage

#endif // IECOREIMAGE_LENSDISTORT_H

