//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>
#include <boost/python/suite/indexing/container_utils.hpp>

#include "IECore/ImageDisplayDriver.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h" 
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost;
using namespace boost::python;

namespace IECore {

template< typename T >
std::vector< T > listToVector( const boost::python::list &names )
{
	std::vector< T > n;
	boost::python::container_utils::extend_container( n, names );
	return n;
}

class ImageDisplayDriverWrap : public ImageDisplayDriver, public Wrapper<ImageDisplayDriver>
{
	public :
		
		ImageDisplayDriverWrap( PyObject *self, const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const list &channelNames, CompoundDataPtr parameters ) :
 				ImageDisplayDriver( displayWindow, dataWindow, listToVector<std::string>(channelNames), parameters ), 
				Wrapper<ImageDisplayDriver>( self, this )
		{
		}

		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize )
		{
			ImageDisplayDriver::imageData( box, data, dataSize );
			override c = this->get_override( "imageData" );
			if( c )
			{
				c( const_cast<Imath::Box2i &>(box), FloatVectorDataPtr(new FloatVectorData(std::vector<float>(data, data + dataSize))) );
			}
		}

		virtual void imageClose()
		{
			ImageDisplayDriver::imageClose();
			override c = this->get_override( "imageClose" );
			if( c )
			{
				c();
			}
		}

		virtual bool scanLineOrderOnly() const
		{
			override c = this->get_override( "scanLineOrderOnly" );
			if( c )
			{
				return c();
			}
			else
			{
				return ImageDisplayDriver::scanLineOrderOnly();
			}
		}

		ImagePrimitivePtr image() const
		{
			return ImageDisplayDriver::image()->copy();
		}

};
IE_CORE_DECLAREPTR( ImageDisplayDriverWrap );

static void displayDriverImageData( DisplayDriverPtr dd, const Imath::Box2i &box, FloatVectorDataPtr data )
{
	dd->imageData( box, &(data->readable()[0]), data->readable().size() );
}

static void displayDriverImageClose( DisplayDriverPtr dd )
{
	dd->imageClose();
}

static bool displayDriverScanLineOrderOnly( DisplayDriverPtr dd )
{
	return dd->scanLineOrderOnly();
}

void bindImageDisplayDriver()
{
	typedef class_< ImageDisplayDriver, ImageDisplayDriverWrapPtr, boost::noncopyable, bases<DisplayDriver> > ImageDisplayDriverPyClass;
	ImageDisplayDriverPyClass( "ImageDisplayDriver", no_init )
		.def( init< const Imath::Box2i &, const Imath::Box2i &, const list &, CompoundDataPtr >( args( "displayWindow", "dataWindow", "channelNames", "parameters" ) ) )
		.def( "imageData", &displayDriverImageData )
		.def( "imageClose", &displayDriverImageClose )
		.def( "scanLineOrderOnly", &displayDriverScanLineOrderOnly )
		.def( "image", &ImageDisplayDriverWrap::image )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(ImageDisplayDriver)
	;
	WrapperToPython<ImageDisplayDriverPtr>();
	INTRUSIVE_PTR_PATCH( ImageDisplayDriver, ImageDisplayDriverPyClass );
	implicitly_convertible<ImageDisplayDriverPtr, DisplayDriverPtr>();
}

} // namespace IECore
