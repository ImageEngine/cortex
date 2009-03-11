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

#ifndef IE_COREPYTHON_IECOREBINDING_H
#define IE_COREPYTHON_IECOREBINDING_H

#include <string>

namespace IECore
{

/// Many bindings need to define the __str__ function,
/// and in doing so they frequently might require to use
/// the __str__ function for child objects. So we define
/// a template str function here which other headers can
/// specialise, thus making it available for use in other
/// str implementations. The motivation for the
/// str() implementations being as they are currently
/// (ie returning very simple representations with no
/// type information) is that they are used to display
/// the results of the "do" script, and as such need to
/// be as simple as possible to parse.
/// \todo Reference to T should be const.
/// \todo If we used a class instead, then partial specialisations would allow us to more
///       easily bind templates without the need for macros to instantiate all the str variations.
template<typename T>
std::string str( T &x );

/// Same as above, except for __repr__. repr()
/// implementations should be in a richer form than
/// the str() implementations - the motivation here is
/// to return as much useful information as possible to
/// a python programmer. Ideally repr() should return a
/// string which, when passed to eval() in python, will
/// recreate the object. Names in this string should be
/// fully qualified with the module prefix - e.g. "IECore.V2f( 1, 2 )".
/// \todo Ensure all implementations follow this convention.
/// \todo Reference to T should be const.
/// \todo If we used a class instead, then partial specialisations would allow us to more
///       easily bind templates without the need for macros to instantiate all the repr variations.
template<typename T>
std::string repr( T &x );

}

#endif // IE_COREPYTHON_IECOREBINDING_H
