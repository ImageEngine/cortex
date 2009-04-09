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

#include "boost/python.hpp"

#include "maya/MGlobal.h"

#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/Exception.h"

#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/ViewportPostProcessCallback.h"

#include "IECoreMaya/bindings/ViewportPostProcessCallbackBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECoreMaya
{

struct ViewportPostProcessCallbackHelper
{
	static void registerCallback( const std::string &panelName, ViewportPostProcessPtr postProcess )
	{
		int isModelPanel = 0;
                MStatus s = MGlobal::executeCommand( MString( "modelPanel -q -exists " ) + panelName.c_str(), isModelPanel, false );
                if ( !s )
                {
                        throw InvalidArgumentException( "ViewportPostProcessCallback: Invalid modelPane name specified" );
                }
                
                if ( !isModelPanel )
                {
                        throw InvalidArgumentException( "ViewportPostProcessCallback: modelPanel '" + panelName + "' does not exist" );
                }

		s = ViewportPostProcessCallback::registerCallback( panelName.c_str(), postProcess );
                IECoreMaya::StatusException::throwIfError( s );
	}
	
	static void deregisterCallback( const std::string &panelName )
	{
		MStatus s = ViewportPostProcessCallback::deregisterCallback( panelName.c_str() );
                IECoreMaya::StatusException::throwIfError( s );

	}
};

void bindViewportPostProcessCallback()
{
	typedef class_< ViewportPostProcessCallback, bases< RefCounted >, boost::noncopyable > ViewportPostProcessCallbackPyClass;
	
	ViewportPostProcessCallbackPyClass( "ViewportPostProcessCallback", no_init )				
		.def( "registerCallback", &ViewportPostProcessCallbackHelper::registerCallback ).staticmethod( "registerCallback" )
		.def( "deregisterCallback", &ViewportPostProcessCallbackHelper::deregisterCallback ).staticmethod( "deregisterCallback" )
	;
	
	INTRUSIVE_PTR_PATCH( ViewportPostProcessCallback, ViewportPostProcessCallbackPyClass );
	implicitly_convertible< ViewportPostProcessCallback::Ptr, RefCountedPtr >();
}

}
