//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_TRANSFORMALGO_H
#define IECOREAPPLESEED_TRANSFORMALGO_H

#include <vector>
#include <set>

#include "OpenEXR/ImathMatrix.h"

#include "renderer/api/utility.h"

namespace IECoreAppleseed
{

namespace TransformAlgo
{

void makeTransform( const Imath::M44f &m, foundation::Transformd &xform );

void makeTransformSequence( const Imath::M44f &m, renderer::TransformSequence &xformSeq );
void makeTransformSequence( const std::set<float> &times, const std::vector<Imath::M44f> &transforms, renderer::TransformSequence &xformSeq );
void makeTransformSequence( const std::vector<float> &times, const std::vector<Imath::M44f> &transforms, renderer::TransformSequence &xformSeq );

} // namespace TransformAlgo

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_TRANSFORMALGO_H
