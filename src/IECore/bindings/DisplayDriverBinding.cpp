//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DisplayDriver.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h" 
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/WrapperToPython.h"

using namespace boost;
using namespace boost::python;

namespace IECore {

class DisplayDriverCreatorWrap : public DisplayDriver::DisplayDriverCreator, public Wrapper<DisplayDriver::DisplayDriverCreator>
{
	public :
		
		DisplayDriverCreatorWrap( PyObject *self ) : DisplayDriverCreator(), Wrapper<DisplayDriver::DisplayDriverCreator>( self, this )
		{
		};
		
		virtual DisplayDriverPtr create( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters )
		{
			override c = this->get_override( "create" );
			if( c )
			{
				list channelList;
				std::vector<std::string>::const_iterator iterX = channelNames.begin();
				while ( iterX != channelNames.end() )
				{
					channelList.append( *iterX );
					iterX++;
				}
				
				DisplayDriverPtr r = c( displayWindow, dataWindow, channelList, 
					const_pointer_cast<CompoundData>( parameters ) );
				if( !r )
				{
					throw Exception( "create() python method didn't return a DisplayDriver." );
				}
				return r;
			}
			else
			{
				throw Exception( "create() python method not defined" );
			}
		};
};
IE_CORE_DECLAREPTR( DisplayDriverCreatorWrap );

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

template< typename T >
std::vector< T > listToVector( const boost::python::list &names )
{
	std::vector< T > n;
	boost::python::container_utils::extend_container( n, names );
	return n;
}

static DisplayDriverPtr displayDriverCreate( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const boost::python::list &channelNames, CompoundDataPtr parameters )
{
	return DisplayDriver::create( displayWindow, dataWindow, listToVector<std::string>(channelNames), parameters ); 
}

void bindDisplayDriver()
{
	typedef class_< DisplayDriver, DisplayDriverPtr, boost::noncopyable, bases<RunTimeTyped> > DisplayDriverPyClass;
	scope displayDriverScope = DisplayDriverPyClass( "DisplayDriver", no_init )
		.def( "imageData", &displayDriverImageData )
		.def( "imageClose", &displayDriverImageClose )
		.def( "scanLineOrderOnly", &displayDriverScanLineOrderOnly )
		.def( "displayWindow", &DisplayDriver::displayWindow )
		.def( "dataWindow", &DisplayDriver::dataWindow )
		.def( "channelNames", &channelNames )
		.def( "create", &displayDriverCreate ).staticmethod("create")
		.def( "registerFactory", &DisplayDriver::registerFactory ).staticmethod("registerFactory")
		.def( "unregisterFactory", &DisplayDriver::unregisterFactory ).staticmethod("unregisterFactory")
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(DisplayDriver)		
	;
	INTRUSIVE_PTR_PATCH( DisplayDriver, DisplayDriverPyClass );
	implicitly_convertible<DisplayDriverPtr, RunTimeTypedPtr>();

	typedef class_< DisplayDriver::DisplayDriverCreator, DisplayDriverCreatorWrapPtr, boost::noncopyable, bases<RunTimeTyped> > DisplayDriverCreatorPyClass;
	DisplayDriverCreatorPyClass( "DisplayDriverCreator" )
		.def( "create", &DisplayDriverCreatorWrap::create )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( DisplayDriver::DisplayDriverCreator )		
	;
	
	WrapperToPython<DisplayDriver::DisplayDriverCreatorPtr>();
	INTRUSIVE_PTR_PATCH( DisplayDriver::DisplayDriverCreator, DisplayDriverCreatorPyClass );
	implicitly_convertible< DisplayDriver::DisplayDriverCreatorPtr, RunTimeTypedPtr>();

}

} // namespace IECore
