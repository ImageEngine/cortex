//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_FROMMAYAIMAGECONVERTER_H
#define IE_COREMAYA_FROMMAYAIMAGECONVERTER_H

#include <string>
#include <vector>

#include "IECoreMaya/FromMayaConverter.h"

#include "IECore/Object.h"
#include "IECore/ImagePrimitive.h"

#include "maya/MImage.h"


namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( FromMayaImageConverter );

/// The FromMayaImageConverter class allows conversion from MImages to an IECore::ImagePrimitive
class FromMayaImageConverter : public FromMayaConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaImageConverter, FromMayaImageConverterTypeId, FromMayaConverter );
		
		FromMayaImageConverter( MImage &image );
	
		/// The MImage which will be converted by the convert() function.
		const MImage &image() const;				
		
	protected :
				
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;
		
	private :

		/// This is mutable here because Maya isn't const-correct. It's not a copy because it appears that the default assignment/copy operators for
		/// MImage can create two instances which reference the same data. When one of them dies, the other is then in an invalid state which can
		/// crash Maya.
		MImage &m_image;
		
		template<typename T> 
		void writeChannels( IECore::ImagePrimitivePtr target, const std::vector< std::string > &channelNames ) const;

};

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYAIMAGECONVERTER_H
