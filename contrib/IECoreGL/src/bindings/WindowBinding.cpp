//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Window.h"
#include "IECoreGL/bindings/WindowBinding.h"

#include "IECore/MessageHandler.h"
#include "IECore/bindings/RefCountedBinding.h"
#include "IECore/bindings/Wrapper.h"

using namespace boost::python;

namespace IECoreGL
{

class WindowWrap : public Window, public IECore::Wrapper<Window>
{

	public :

		WindowWrap( PyObject *self, const std::string &title )
			:	Window( title ), IECore::Wrapper<Window>( self, this )
		{
		}

		virtual void display()
		{
			try
			{
				override o = this->get_override( "display" );
				if( o )
				{
					o();
				}
				else
				{
					Window::display();
				}
			}
			catch( error_already_set )
			{
				PyErr_Print();
			}
			catch( const std::exception &e )
			{
				IECore::msg( IECore::Msg::Error, "WindowWrap::display", e.what() );
			}
			catch( ... )
			{
				IECore::msg( IECore::Msg::Error, "WindowWrap::display", "Caught unknown exception" );
			}
		}

		virtual void reshape( int width, int height )
		{
			try
			{
				override o = this->get_override( "reshape" );
				if( o )
				{
					o( width, height );
				}
				else
				{
					Window::reshape( width, height );
				}
			}
			catch( error_already_set )
			{
				PyErr_Print();
			}
			catch( const std::exception &e )
			{
				IECore::msg( IECore::Msg::Error, "WindowWrap::reshape", e.what() );
			}
			catch( ... )
			{
				IECore::msg( IECore::Msg::Error, "WindowWrap::reshape", "Caught unknown exception" );
			}
		}


};

IE_CORE_DECLAREPTR( WindowWrap );

void bindWindow()
{
	IECore::RefCountedClass<Window, IECore::RefCounted, WindowWrapPtr>( "Window" )
		.def( init<std::string>() )
		.def( "setTitle", &Window::setTitle )
		.def( "getTitle", &Window::getTitle, return_value_policy<copy_const_reference>() )
		.def( "setVisibility", &Window::setVisibility )
		.def( "getVisibility", &Window::getVisibility )
		.def( "start", &Window::start ).staticmethod( "start" )
	;
}

} // namespace IECoreGL
