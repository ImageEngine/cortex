//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ScopedGILRelease.h"

#include "IECoreImage/DisplayDriver.h"

#include "IECoreImageBindings/DisplayDriverBinding.h"

using namespace boost;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreImage;

namespace
{

static boost::python::list channelNames( DisplayDriverPtr dd )
{
	boost::python::list newList;
	std::vector<std::string> names = dd->channelNames();

	std::vector<std::string>::const_iterator iterX = names.begin();
	while ( iterX != names.end() )
	{
		newList.append( *iterX );
		iterX++;
	}
	return newList;
}

static void displayDriverImageData( DisplayDriverPtr dd, const Imath::Box2i &box, FloatVectorDataPtr data )
{
	ScopedGILRelease gilRelease;
	dd->imageData( box, &(data->readable()[0]), data->readable().size() );
}

static void displayDriverImageClose( DisplayDriverPtr dd )
{
	ScopedGILRelease gilRelease;
	dd->imageClose();
}

static bool displayDriverScanLineOrderOnly( DisplayDriverPtr dd )
{
	return dd->scanLineOrderOnly();
}

static DisplayDriverPtr displayDriverCreate( const std::string &typeName, const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const boost::python::list &channelNames, CompoundDataPtr parameters )
{
	std::vector<std::string> names;
	boost::python::container_utils::extend_container( names, channelNames );
	ScopedGILRelease gilRelease;
	DisplayDriverPtr res = DisplayDriver::create( typeName, displayWindow, dataWindow, names, parameters );
	return res;
}

} // namespace

namespace IECoreImageBindings
{

void bindDisplayDriver()
{
	RunTimeTypedClass<DisplayDriver>()
		.def( "imageData", &displayDriverImageData )
		.def( "imageClose", &displayDriverImageClose )
		.def( "scanLineOrderOnly", &displayDriverScanLineOrderOnly )
		.def( "acceptsRepeatedData", &DisplayDriver::acceptsRepeatedData )
		.def( "displayWindow", &DisplayDriver::displayWindow )
		.def( "dataWindow", &DisplayDriver::dataWindow )
		.def( "channelNames", &channelNames )
		.def( "create", &displayDriverCreate ).staticmethod("create")
	;
}

} // namespace IECoreImageBindings
