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

#include "IECore/bindings/RefCountedBinding.h"
#include "IECore/bindings/Wrapper.h"

#include "IECore/Exception.h"

#include "IECoreMaya/StatusException.h"

#include "IECoreMaya/bindings/ImageViewportPostProcessBinding.h"
#include "IECoreMaya/ImageViewportPostProcess.h"

using namespace IECore;
using namespace boost::python;

namespace IECoreMaya
{

struct ImageViewportPostProcessWrapper : public ImageViewportPostProcess, Wrapper< ImageViewportPostProcess >
{

	ImageViewportPostProcessWrapper(PyObject *self ) : ImageViewportPostProcess(), Wrapper<ImageViewportPostProcess>( self, this )
        {
        }
	
	virtual ~ImageViewportPostProcessWrapper()
	{
	}
	
	virtual bool needsDepth () const 
	{  
		override o = this->get_override( "needsDepth" );
                if( o )
                {
			try
			{
                        	return o();
			}
			catch ( error_already_set )
			{
				PyErr_Print();
				return false;
			}
                }
                else
                {
                        return ImageViewportPostProcess::needsDepth();
                }	
	}

	virtual void preRender( const std::string &panelName )
	{  
		override o = this->get_override( "preRender" );
                if( o )
                {
			try
			{
                        	o( panelName );
			}
			catch ( error_already_set )
			{
				PyErr_Print();
			}
                }
		else
		{
			ImageViewportPostProcess::preRender( panelName );
		}	
	}
	
	virtual void postRender( const std::string &panelName, IECore::ImagePrimitivePtr image )
	{
		override o = this->get_override( "postRender" );
                if( o )
                {
                        try
			{
                        	o( panelName, image );
			}
			catch ( error_already_set )
			{
				PyErr_Print();
			}
                }
                else
                {
			/// Maya would crash if we were to throw an exception here
			MGlobal::displayError( "ImageViewportPostProcess: postRender() python method not defined" );
                }
	
	}
};
IE_CORE_DECLAREPTR( ImageViewportPostProcessWrapper );

void bindImageViewportPostProcess()
{
	typedef class_< ImageViewportPostProcess, ImageViewportPostProcessWrapperPtr, bases< ViewportPostProcess >, boost::noncopyable > ImageViewportPostProcessPyClass;
	
	RefCountedClass<ImageViewportPostProcess, ViewportPostProcess, ImageViewportPostProcessWrapperPtr>( "ImageViewportPostProcess" )
		.def( init<>() )
		.def( "needsDepth", &ImageViewportPostProcessWrapper::needsDepth )
		.def( "preRender", &ImageViewportPostProcessWrapper::preRender )
		.def( "postRender", pure_virtual( &ImageViewportPostProcessWrapper::postRender ) )
	;
}

}
