//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREIMAGE_SPLINETOIMAGE_H
#define IECOREIMAGE_SPLINETOIMAGE_H

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

#include "IECore/ObjectParameter.h"
#include "IECore/Op.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

}

namespace IECoreImage
{

/// This Op creates ImagePrimitives from SplineData.
/// \todo Different projections would be nice.
/// \ingroup imageProcessingGroup
class IECOREIMAGE_API SplineToImage : public IECore::Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( SplineToImage, SplineToImageTypeId, IECore::Op );

		SplineToImage();
		~SplineToImage() override;

		IECore::ObjectParameter *splineParameter();
		const IECore::ObjectParameter *splineParameter() const;

		IECore::V2iParameter *resolutionParameter();
		const IECore::V2iParameter *resolutionParameter() const;

	protected :

		IECore::ObjectPtr doOperation( const IECore::CompoundObject *operands ) override;

	private :

		struct CreateImage;

		IECore::ObjectParameterPtr m_splineParameter;
		IECore::V2iParameterPtr m_resolutionParameter;

};

IE_CORE_DECLAREPTR( SplineToImage );

} // namespace IECoreImage

#endif // IECOREIMAGE_SPLINETOIMAGE_H
