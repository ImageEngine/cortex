//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_SPLINETOGLTEXTURECONVERTER_H
#define IECOREGL_SPLINETOGLTEXTURECONVERTER_H

#include "IECore/SimpleTypedParameter.h"

#include "IECoreGL/Export.h"
#include "IECoreGL/ToGLConverter.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Texture )

/// Converts IECore::SplineData objects into IECoreGL::Texture objects.
/// The default conversion gives an image of 8x512 and it can return a ColorTexture or a LuminanceTexture.
/// \ingroup conversionGroup
class IECOREGL_API SplineToGLTextureConverter : public ToGLConverter
{

	public :

		typedef IECore::Data InputType;
		typedef IECoreGL::Texture ResultType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::SplineToGLTextureConverter, SplineToGLTextureConverterTypeId, ToGLConverter );

		SplineToGLTextureConverter( IECore::ConstObjectPtr toConvert = 0 );
		virtual ~SplineToGLTextureConverter();

	protected :

		IECore::V2iParameterPtr m_resolutionParameter;
		virtual IECore::RunTimeTypedPtr doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const;
	
	private :
	
		static ConverterDescription<SplineToGLTextureConverter> g_descriptionff;
		static ConverterDescription<SplineToGLTextureConverter> g_descriptionfColor3f;
		static ConverterDescription<SplineToGLTextureConverter> g_descriptionfColor4f;
	
};

IE_CORE_DECLAREPTR( SplineToGLTextureConverter );

} // namespace IECoreGL

#endif // IECOREGL_SPLINETOGLTEXTURECONVERTER_H
