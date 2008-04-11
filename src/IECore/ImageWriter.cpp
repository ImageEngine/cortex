//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ImageWriter.h"
#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/TypedParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"

using namespace std;
using namespace IECore;
using namespace Imath;

ImageWriter::ImageWriter( const std::string &name, const std::string &description ) :
		Writer(name, description, ImagePrimitiveTypeId)
{
	m_channelsParameter = new StringVectorParameter("channels", "The list of channels to write.  No list causes all channels to be written." );
	parameters()->addParameter( m_channelsParameter );
}

bool ImageWriter::canWrite( ConstObjectPtr image, const string &fileName )
{
	return runTimeCast<const ImagePrimitive>( image );
}

/// get the user-requested channel names
void ImageWriter::imageChannels( vector<string> &names ) const
{	
	ConstImagePrimitivePtr image = getImage();
	assert( image );
	
	vector<string> allNames;
	image->channelNames( allNames );

	const vector<string> &d = m_channelsParameter->getTypedValue();

	// give all channels when no list is provided
	if ( !d.size() )
	{
		names = allNames;
		return;
	}

	// otherwise, copy in the requested names from the parameter set.
	// this is intersection(A, D)
	names.clear();
	for ( vector<string>::const_iterator it = d.begin(); it != d.end(); it++ )
	{
		if ( find( allNames.begin(), allNames.end(), *it ) != allNames.end() &&
			find( names.begin(), names.end(), *it ) == names.end() )
		{
			names.push_back( *it );
		}
	}
}

ConstImagePrimitivePtr ImageWriter::getImage() const
{
	/// \todo This case isn't good until we're making the input parameter accept only ImagePrimitive instances
	return boost::static_pointer_cast<const ImagePrimitive>(object());
}

void ImageWriter::doWrite()
{
	// write the image channel data
	vector<string> channels;
	imageChannels(channels);

	ConstImagePrimitivePtr image = getImage();
	assert( image );

	if ( !image->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "ImageWriter: Invalid primitive variables on image" );
	}

	Box2i dataWindow = image->getDataWindow();

	writeImage( channels, image, dataWindow );
}
