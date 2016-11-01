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


#include "IECoreAppleseed/TransformAlgo.h"

using namespace IECoreAppleseed;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

void doMakeTransform( const Imath::M44f &m, asf::Transformd &xform )
{
	Imath::M44d md( m );
	xform.set_local_to_parent( asf::Matrix4d( md ) );
	md.invert();
	xform.set_parent_to_local( asf::Matrix4d( md ) );
}

template <class TimeContainer>
void doMakeTransformSequence( const TimeContainer &times, const std::vector<Imath::M44f> &transforms, asr::TransformSequence &xformSeq )
{
	assert( times.size() == transforms.size() );

	xformSeq.clear();
	asf::Transformd xform;

	size_t j = 0;
	for( typename TimeContainer::const_iterator it( times.begin() ), e( times.end() ); it != e; ++it )
	{
		doMakeTransform( transforms[j++], xform );
		xformSeq.set_transform( *it, xform );
	}
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementation of public API.
//////////////////////////////////////////////////////////////////////////

namespace IECoreAppleseed
{

namespace TransformAlgo
{

void makeTransform( const Imath::M44f &m, asf::Transformd &xform )
{
	doMakeTransform( m, xform );
}

void makeTransformSequence( const Imath::M44f &m, asr::TransformSequence &xformSeq )
{
	asf::Transformd xform;
	makeTransform( m, xform );
	xformSeq.clear();
	xformSeq.set_transform( 0.0f, xform );
}

void makeTransformSequence( const std::set<float> &times, const std::vector<Imath::M44f> &transforms, asr::TransformSequence &xformSeq )
{
	doMakeTransformSequence( times, transforms, xformSeq );
}

void makeTransformSequence( const std::vector<float> &times, const std::vector<Imath::M44f> &transforms, asr::TransformSequence &xformSeq )
{
	doMakeTransformSequence( times, transforms, xformSeq );
}

} // namespace TransformAlgo

} // namespace IECoreAppleseed
