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

#ifndef IECOREGL_UNIFORMFUNCTIONS_H
#define IECOREGL_UNIFORMFUNCTIONS_H

#include "IECoreGL/GL.h"

#include <vector>

namespace IECoreGL
{

/// The type of the glUniformNfv functions.
typedef void (*UniformFloatFunction)( GLint, GLsizei count, const GLfloat * );
/// Returns a vector containing functions pointers to glUniform1fv, glUniform2fv ... glUniform4fv.
/// This can be useful to avoid switching on your datasize when setting shader parameters - you
/// can instead index straight into this vector to retrieve the appropriate function to call.
const std::vector<UniformFloatFunction> &uniformFloatFunctions();

/// The type of the glUniformNiv functions.
typedef void (*UniformIntFunction)( GLint, GLsizei count, const GLint * );
/// Returns a vector containing functions pointers to glUniform1iv, glUniform2iv ... glUniform4iv.
/// This can be useful to avoid switching on your datasize when setting shader parameters - you
/// can instead index straight into this vector to retrieve the appropriate function to call.
const std::vector<void (*)( GLint, GLsizei count, const GLint * )> &uniformIntFunctions();

} // namespace IECoreGL

#endif // IECOREGL_UNIFORMFUNCTIONS_H
