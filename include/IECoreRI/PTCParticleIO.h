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

#ifndef IE_CORERI_PTCPARTICLEIO_H
#define IE_CORERI_PTCPARTICLEIO_H

#define PTC_HEADER_BBOX_FLOATS			6
#define PTC_HEADER_MATRIX_FLOATS		16
#define PTC_HEADER_FORMAT_FLOATS		3
#define PTC_MAX_VARIABLES				1024


namespace IECoreRI
{

namespace PTCParticleIO
{

	enum VarType
	{
		Color = 0,
		Point,
		Normal,
		Vector,
		Float,
		Matrix,
		VarTypeCount,
	};

	struct Record
	{
		VarType type;
		int position;
	};

	struct PTCHeader
	{
		bool valid;
		int nPoints;
		bool hasBbox, hasWorld2eye, hasWorld2ndc, hasFormat;
		float bbox[ PTC_HEADER_BBOX_FLOATS ];
		int datasize;
		float world2eye[ PTC_HEADER_MATRIX_FLOATS ];
		float world2ndc[ PTC_HEADER_MATRIX_FLOATS ];
		float format[ PTC_HEADER_FORMAT_FLOATS ];
		int nvars;
		char const *varnames[ PTC_MAX_VARIABLES ];
		char const *vartypes[ PTC_MAX_VARIABLES ];
		std::map<std::string, Record > attributes;
	};

	struct PTCType
	{
		std::string name;
		int nFloats;
	};

	extern PTCType ptcVariableTypes[];

	extern void checkPTCParticleIO();

} // namespace PTCParticleIO

} // namespace IECoreRI

#endif // IE_CORERI_PTCPARTICLEIO_H
