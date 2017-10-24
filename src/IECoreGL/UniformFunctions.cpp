//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/UniformFunctions.h"

const std::vector<IECoreGL::UniformFloatFunction> &IECoreGL::uniformFloatFunctions()
{
	static std::vector<IECoreGL::UniformFloatFunction> t;
	if( !t.size() )
	{
		t.push_back( nullptr ); // so that index 1 is glUniform1fv
		t.push_back( glUniform1fv );
		t.push_back( glUniform2fv );
		t.push_back( glUniform3fv );
		t.push_back( glUniform4fv );
	}
	return t;
}

const std::vector<IECoreGL::UniformIntFunction> &IECoreGL::uniformIntFunctions()
{
	static std::vector<IECoreGL::UniformIntFunction> t;
	if( !t.size() )
	{
		t.push_back( nullptr );
		t.push_back( glUniform1iv );
		t.push_back( glUniform2iv );
		t.push_back( glUniform3iv );
		t.push_back( glUniform4iv );
	}
	return t;
}

const std::vector<std::vector<IECoreGL::UniformMatrixFunction> > &IECoreGL::uniformMatrixFunctions()
{
	static std::vector<std::vector<IECoreGL::UniformMatrixFunction> > t;
	if( !t.size() )
	{
		// empty entries for 0 to 1.
		t.push_back( std::vector<IECoreGL::UniformMatrixFunction>() );
		t.push_back( std::vector<IECoreGL::UniformMatrixFunction>() );
		// [2][0-4]
		t.push_back( std::vector<IECoreGL::UniformMatrixFunction>() );
		t[2].push_back( nullptr );
		t[2].push_back( nullptr );
		t[2].push_back( glUniformMatrix2fv );
		t[2].push_back( glUniformMatrix2x3fv );
		t[2].push_back( glUniformMatrix2x4fv );
		// [3][0-4]
		t.push_back( std::vector<IECoreGL::UniformMatrixFunction>() );
		t[3].push_back( nullptr );
		t[3].push_back( nullptr );
		t[3].push_back( glUniformMatrix3x2fv );
		t[3].push_back( glUniformMatrix3fv );
		t[3].push_back( glUniformMatrix3x4fv );
		// [4][0-4]
		t.push_back( std::vector<IECoreGL::UniformMatrixFunction>() );
		t[4].push_back( nullptr );
		t[4].push_back( nullptr );
		t[4].push_back( glUniformMatrix4x2fv );
		t[4].push_back( glUniformMatrix4x3fv );
		t[4].push_back( glUniformMatrix4fv );
	}
	return t;
}
