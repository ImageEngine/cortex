//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PRIMITIVEIMPLICITSURFACEFUNCTION_H
#define IE_CORE_PRIMITIVEIMPLICITSURFACEFUNCTION_H

#include "OpenEXR/ImathVec.h"

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECore/PrimitiveEvaluator.h"


namespace IECore
{

/// A model of ImplicitSurfaceFunction for creating a signed distance field with respect to a Primitive. This will
/// only work on continuous surfaces, as it expects the signed distance to be computable from only the closest point
/// and closest normal.
class PrimitiveImplicitSurfaceFunction : public ImplicitSurfaceFunction< Imath::V3f, float >
{
        public:
		IE_CORE_DECLAREMEMBERPTR( PrimitiveImplicitSurfaceFunction );

		/// Construct the function from the given Primitive. If a PrimitiveEvaluator cannot be created from it an
		/// InvalidArgumentException is thrown.
		PrimitiveImplicitSurfaceFunction( PrimitivePtr primitive );

		virtual ~PrimitiveImplicitSurfaceFunction();

		// Retrieve the signed distance from the primitive at the given point
                Value operator()( const Point &p );

		// Retrieve the signed distance from the primitive at the given point
		virtual Value getValue( const Point &p );


	protected:

		PrimitiveImplicitSurfaceFunction();

		PrimitiveEvaluatorPtr m_evaluator;
};

IE_CORE_DECLAREPTR( PrimitiveImplicitSurfaceFunction );

}

#endif // IE_CORE_PRIMITIVEIMPLICITSURFACEFUNCTION_H
