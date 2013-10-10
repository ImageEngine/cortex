//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_TOGLTEXTURECONVERTER_H
#define IECOREGL_TOGLTEXTURECONVERTER_H

#include "IECoreGL/ToGLConverter.h"

#include "IECore/CompoundData.h"

namespace IECore
{
	IE_CORE_FORWARDDECLARE( ImagePrimitive );
}

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Texture )

/// Converts IECore::ImagePrimitive objects (or their CompoundData representation) into IECoreGL::Texture objects.
/// Because its not possible to pass ImagePrimitives through calls that only accept types derived from IECore::Data.
/// The converter also supports an IECore::CompoundData object as a representation of the image. This should have, at
/// minimum:
///
///    'dataWindow' : Box2iData()
///    'displayWindow' : Box2iData()
///    'channels' : CompoundData()
///
/// The channels data object should contain FloatVectorData in keys corresponding to the original image channel names.
/// Other data types in images are not currently supported.
/// 
/// Such an object can easily be constructed in python like so:
///
/// \code
///	   import IECore
///	   compoundData = IECore.CompoundData()
///	   compoundData["displayWindow"] = IECore.Box2iData( image.displayWindow )
///	   compoundData["dataWindow"] = IECore.Box2iData( image.dataWindow )
///
///	   channelData = IECore.CompoundData()
///	   for channel in image.channelNames() :
///	       channelData[ channel ] = image[ channel ].data
///	
///	   compoundData["channels"] = channelData
/// \endcode
/// \ingroup conversionGroup
class ToGLTextureConverter : public ToGLConverter
{

	public :

		typedef IECore::ImagePrimitive InputType;
		typedef IECoreGL::Texture ResultType;
		
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::ToGLTextureConverter, ToGLTextureConverterTypeId, ToGLConverter );

		ToGLTextureConverter( IECore::ConstObjectPtr toConvert = 0, bool createMissingRGBChannels = false );
		virtual ~ToGLTextureConverter();

	protected :

		virtual IECore::RunTimeTypedPtr doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const;
	
	private :

		IECore::ImagePrimitivePtr createMissingChannels( const IECore::ImagePrimitive *image ) const;
		IECore::ImagePrimitivePtr imageFromCompoundData( IECore::CompoundData::ConstPtr data ) const;
		bool m_createMissingRGBChannels;

		static ConverterDescription<ToGLTextureConverter> g_description;
		static ConverterDescription<ToGLTextureConverter> g_compoundDataDescription;
	
};

IE_CORE_DECLAREPTR( ToGLTextureConverter );

} // namespace IECoreGL

#endif // IECOREGL_TOGLTEXTURECONVERTER_H
