//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IMG/IMG_DeepShadow.h"

#include "RixDeepTexture.h"

#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/NumericParameter.h"

#include "IECore/RatToDtexOp.h"

using namespace IECore;

RatToDtexOp::RatToDtexOp()
	:	Op( "Converts a Houdini Deep Image (rat) to a PRMan Deep Image (dtex).", new IntParameter( "result", "result status", 0 ) )
{
	m_ratParameter = new FileNameParameter(
		"ratFile",
		"The rat file to convert",
		"rat",
		"",
		false,
		PathParameter::MustExist
	);
	
	m_dtexParameter = new FileNameParameter(
		"dtexFile",
		"The dtex file to write",
		"dtex",
		"",
		false
	);
	
	parameters()->addParameter( m_ratParameter );
	parameters()->addParameter( m_dtexParameter );
}

RatToDtexOp::~RatToDtexOp()
{
}

ObjectPtr RatToDtexOp::doOperation( const CompoundObject *operands )
{
	std::string rat = m_ratParameter->getTypedValue();
	std::string dtex = m_dtexParameter->getTypedValue();
	
	IMG_DeepShadow ratImage;
	
	if ( !ratImage.open( rat.c_str() ) )
	{
		IECore::msg( IECore::MessageHandler::Error, "RatToDtexOp", "Cannot open deep image for reading: \""+ rat +"\"" );
	}
	
	/// \todo: what are these used for? should they come from the rat options?
	float NP[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 };
	float NL[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 };
	
	/// \todo: is this a good default for tile width and height?
	int xRes, yRes, tileWidth=16, tileHeight=16;
	ratImage.resolution( xRes, yRes );
	int numChannels = ratImage.getChannelCount();
	
	RixContext *context = RixGetContext();
	RixDeepTexture *dtexInterface = (RixDeepTexture *)context->GetRixInterface( k_RixDeepTexture );
	RixDeepTexture::DeepFile *dtexFile;
	RixDeepTexture::DeepCache *dtexCache = dtexInterface->CreateCache( xRes / tileWidth );
	int status = dtexInterface->CreateFile( dtex.c_str(), dtexCache, &dtexFile );
	if ( status != RixDeepTexture::k_ErrNOERR )
	{
		dtexInterface->DestroyCache( dtexCache );
		return new IntData( status );
	}
	
	const IMG_DeepShadowChannel *channel = 0;
	const IMG_DeepShadowChannel *depthChannel = 0;
	const IMG_DeepShadowChannel *opacityChannel = 0;
	const IMG_DeepShadowChannel *colorChannel = 0;
	
	/// \todo: should we support abitrary channels? nuke can't read them at the moment...
	for ( int c=0; c < numChannels; c++ )
	{
		channel = ratImage.getChannel( c );
		std::string name = channel->getName();
		
		if ( name == "Pz" )
		{
			depthChannel = channel;
		}
		else if ( name == "Of" )
		{
			opacityChannel = channel;
		}
		else if ( name == "C" )
		{
			colorChannel = channel;
		}
	}
	
	if ( !depthChannel || !opacityChannel || !colorChannel )
	{
		IECore::msg( IECore::MessageHandler::Error, "RatToDtexOp", "Rat missing required channels: \""+ rat +"\"" );
	}
	
	int dataSize = colorChannel->getTupleSize();
	
	RixDeepTexture::DeepImage *dtexImage;
	/// \todo: should compression style be a parameter? should we determine data type from the rat or always assume float?
	status = dtexFile->AddImage( "main.rgba", dataSize, xRes, yRes, tileWidth, tileHeight, &NP[0], &NL[0], RixDeepTexture::k_CmpLZW, RixDeepTexture::k_TypeFLOAT, &dtexImage );
	if ( status != RixDeepTexture::k_ErrNOERR )
	{
		dtexInterface->DestroyFile( dtexFile );
		dtexInterface->DestroyCache( dtexCache );
		return new IntData( status );
	}
	
	IMG_DeepPixelReader ratPixel( ratImage );
	RixDeepTexture::DeepPixel *dtexPixel = dtexInterface->CreatePixel( dataSize );
	
	for ( int y=0; y < yRes; y++ )
	{
		for ( int x=0; x < xRes; x++ )
		{
			if ( !ratPixel.open( x, y ) )
			{
				IECore::msg( IECore::MessageHandler::Warning, "RatToDtexOp", ( boost::format( "Unable to open pixel %d, %d" ) % x % y ).str() );
				continue;
			}
			
			int depth = ratPixel.getDepth();
			if ( !depth )
			{
				continue;
			}
			
			ratPixel.uncomposite( *depthChannel, *opacityChannel );
			dtexPixel->Clear( dataSize );
			
			for ( int d=0; d < depth; d++ )
			{
				// only using Pz and C here as nuke can't read anything else
				dtexPixel->Append(
					*ratPixel.getData( *depthChannel, d ),
					const_cast<float*>( ratPixel.getData( *colorChannel, d ) ),
					0
				);
			}
			
			dtexPixel->Finish();
			dtexImage->SetPixel( x, yRes - y - 1, dtexPixel );
			
			ratPixel.close();
		}
	}
	
	dtexInterface->DestroyPixel( dtexPixel );
	status = dtexFile->Close();
	dtexInterface->DestroyCache( dtexCache );
	
	return new IntData( status );
}
