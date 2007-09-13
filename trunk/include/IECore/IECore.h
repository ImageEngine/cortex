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

#ifndef IE_CORE_CORE_H
#define IE_CORE_CORE_H

#include <string>

/// This namespace contains all components of the core library.
namespace IECore
{

int majorVersion();
int minorVersion();
int patchVersion();
const std::string &versionString();

}

//! \mainpage
///
/// The IECore library provides the core C++ framework for all development
/// at Image Engine. Wherever possible code should be implemented as well
/// designed and documented reusable elements in this library, rather than
/// code specific to a 3rd party application or plugin. This avoids code
/// duplication and ensures that all developers are collaborating together on
/// a common codebase.
///
/// \section mainPageDependencies Dependencies
///
/// The IECore library is built on top of a few carefully chosen libraries.
/// You should think very carefully before introducing more dependencies.
///
/// \subsection Boost
///
/// The <a HREF="http://www.boost.org/libs/libraries.htm">boost libraries</a> provide a broad chunk of useful well tested cross
/// platform functionality including parsing, filesystem access, regular expressions,
/// and python access.
///
/// \subsection OpenEXR
///
/// The <a HREF="http://www.openexr.org">OpenEXR libraries</a> provide a useful low level templated maths library
/// and a library for loading and saving the industry standard exr image file format.
///
/// \subsection SQLite
///
/// The <a HREF="http://www.sqlite.org/">SQLite library</a> is used to implement a flexible random access serialisation
/// and caching architecture embodied in the SQLiteIndexedIO class.
///
/// \subsection libTIFF
///
/// <a href="http://www.libtiff.org/">This software</a> provides support for the Tag Image File Format (TIFF), a widely used format for storing image data. 
///
/// \subsection libJPEG
///
/// The <a href="http://www.ijg.org/">libJPEG</a> library provides routines for
/// reading and writing JPEG format images.
/// <br>

#endif // IE_CORE_CORE_H
