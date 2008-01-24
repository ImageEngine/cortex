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
//	     other contributors to this software may be used to endorse or
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

#ifndef IECORE_TIFFIMAGEWRITER_INL
#define IECORE_TIFFIMAGEWRITER_INL

#include "boost/format.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"

namespace IECore
{

template<typename T>
T * TIFFImageWriter::encodeChannels(ConstImagePrimitivePtr image, std::vector<std::string> & names, const Imath::Box2i & dw)
{
	int width  = 1 + dw.max.x - dw.min.x;
	int height = 1 + dw.max.y - dw.min.y;
	int spp = names.size();

	// build a vector
	T *image_buffer = new T[spp*width*height]();
	
	// compute scaling constant
	int bps = 8 * sizeof(T);
	const float scaler = (1 << bps) - 1;
	
	// stripe in the image data.  we encode RGB channels only, and we clamp the input to [0,1]
	std::vector<std::string>::const_iterator i = names.begin();
	while(i != names.end())
	{
		if(!(*i == "R" || *i == "G" || *i == "B" || *i == "A"))
		{
			msg( Msg::Warning, "TIFFImageWriter::write", boost::format( "Channel \"%s\" was not encoded." ) % *i );
			++i;
			continue;
			//throw Exception("invalid channel for TIFF writer, channel name is: " + *i);
		}
		
		int offset = *i == "R" ? 0 : *i == "G" ? 1 : *i == "B" ? 2 : 3;
		
		// get the image channel
		DataPtr channelp = image->variables.find(i->c_str())->second.data;
		
		switch(channelp->typeId())
		{
			
		case FloatVectorDataTypeId:
		{
			const std::vector<float> &channel = boost::static_pointer_cast<FloatVectorData>(channelp)->readable();

			for(int i = 0; i < width*height; ++i)
			{
				// clamp
				float v = scaler * channel[i] + 0.5f;
				image_buffer[spp*i + offset] = (T) (v > 0.0f ? v < scaler ? v : scaler : 0.0f);
			}
		}
		break;
	
		case UIntVectorDataTypeId:
		{
			const std::vector<unsigned int> &channel = boost::static_pointer_cast<UIntVectorData>(channelp)->readable();
			
			for(int i = 0; i < width*height; ++i)
			{
				float v = (float) ((channel[i] + 1) >> 24) + 0.5f;
				image_buffer[spp*i + offset] = (T) v;
			}
		}
		break;
			
		case HalfVectorDataTypeId:
		{
			const std::vector<half> &channel = boost::static_pointer_cast<HalfVectorData>(channelp)->readable();

			for(int i = 0; i < width*height; ++i)
			{
				// clamp
				float v = scaler * channel[i] + 0.5f;
				//float v = min(scaler, max(0.0f, scaler * (float) channel[i] + 0.5f));
				image_buffer[spp*i + offset] = (T) (v > 0.0f ? v < scaler ? v : scaler : 0.0f);
			}
		}
		break;
			
		default:
			delete [] image_buffer;
			throw InvalidArgumentException( (boost::format( "Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(channelp->typeId()) % *i).str() );
		}
		
		++i;
	}

	return image_buffer;	
}

}


#endif
