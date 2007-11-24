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

#ifndef IE_COREMAYA_FROMMAYARENDERABLECONVERTERUTIL_H
#define IE_COREMAYA_FROMMAYARENDERABLECONVERTERUTIL_H

#include "IECoreMaya/Converter.h"

#include "IECore/CompoundObject.h"
#include "IECore/TypedParameter.h"
#include "IECore/Renderable.h"

#include "maya/MObject.h"

namespace IECoreMaya
{

/// The FromMayaRenderableConverterUtil provides utility functions shared by converters to Renderable derived classes.
// Converts attributes to blindData dictionary and also the maya full node name.
// \todo Create a base class for FromMayaGroupConverter and FromMayaMeshConverter with this functionality.
class FromMayaRenderableConverterUtil : public Converter
{

	public :

		FromMayaRenderableConverterUtil();

		/// Adds blindData keys based on the blindDataAttrPrefix and also add the full maya node name (converting '|' to '.').
		static void addBlindDataAttributes( IECore::ConstCompoundObjectPtr operands, const MObject &object, IECore::RenderablePtr renderable );
	
	protected :

		IECore::StringParameterPtr m_blindDataAttrPrefix;
		IECore::BoolParameterPtr m_removeNamespace;
		 
};

IE_CORE_DECLAREPTR( FromMayaRenderableConverterUtil );

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYARENDERABLECONVERTERUTIL_H
