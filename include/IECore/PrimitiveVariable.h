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

#ifndef IE_CORE_PRIMITIVEVARIABLE_H
#define IE_CORE_PRIMITIVEVARIABLE_H

#include "IECore/Data.h"

namespace IECore
{

/// The PrimitiveVariable defines a simple
/// structure to store primitive variables to
/// be used by the Renderer and Primitive classes.
struct PrimitiveVariable {
	/// The Interpolation enum is used to describe how the
	/// values of a Variable are to be interpolated
	/// across the surface of a Primitive. The types are
	/// essentially those defined in the RenderMan standard.
	enum Interpolation {
		Invalid,
		Constant,
		Uniform,
		Vertex,
		Varying,
		FaceVarying
	};
	/// Constructs a PrimitiveVariable with Interpolation type Invalid
	/// and a null data pointer. This allows the [] operator to work
	/// in the PrimitiveVariableMap, but you must be careful to use it
	/// only for assignment or reading of entries you /know/ exist,
	/// otherwise you're inadvertently populating the map with invalid
	/// PrimitiveVariables. This is a problem with the map [] operator
	/// generally but it's worth noting as it's likely to appear here.
	PrimitiveVariable();
	/// Constructor - Data is not copied but referenced directly.
	PrimitiveVariable( Interpolation i, DataPtr d );
	/// Shallow copy constructor - data is not copied just rereferenced
	PrimitiveVariable( const PrimitiveVariable &other );
	bool operator==( const PrimitiveVariable &other ) const;
	/// The interpolation type for this PrimitiveVariable.
	Interpolation interpolation;
	/// The Data for this PrimitiveVariable. Unless Interpolation is Constant,
	/// Variable data is expected to be one of the types defined in VectorTypedData.h.
	/// Constant interpolated data can be represented by any type of Data.
	DataPtr data;
};
		
/// A simple type to hold named PrimitiveVariables.
typedef std::map<std::string, PrimitiveVariable> PrimitiveVariableMap;

} // namespace IECore
		
#endif // IE_CORE_PRIMITIVEVARIABLE_H
