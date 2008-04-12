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

#ifndef IE_COREMAYA_COREMAYA_H
#define IE_COREMAYA_COREMAYA_H

#include "maya/MFnPlugin.h"

namespace IECoreMaya
{

MStatus initialize( MFnPlugin &plugin );
MStatus uninitialize( MFnPlugin &plugin );

}

//! \mainpage
///
/// The IECoreMaya library provides the core C++ framework for all Maya development
/// at Image Engine. Wherever possible, Maya-specific routines should be implemented
/// within this library rather than within their respective tools. Code which is 
/// truly generic and unrelated to Maya should be placed in the main IECore library.
///
/// \section mainPageDependencies Dependencies
///
/// \subsection IECore
///
/// \subsection Maya
///
/// <br>

/// \defgroup environmentgroup Environment variables
///
/// Various aspects of the IECoreMaya library are configured using environment variables.
/// These are listed below.
///
/// <b>IECOREMAYA_DISABLEOUTPUTREDIRECTION</b><br>
/// By default all python output and IECore::MessageHandler output are redirected through
/// the appropriate MGlobal::display*() functions, so that they appear in the script editor.
/// Setting this environment variables disables this redirection, causing the messages to appear
/// in the shell.

#endif // IE_COREMAYA_COREMAYA_H
