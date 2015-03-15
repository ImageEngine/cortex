//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/private/TransformStack.h"

#include <cassert>

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::TransformStack::TransformStack()
{
	clear();
}

void IECoreAppleseed::TransformStack::push( const asr::TransformSequence &m )
{
	m_stack.push(m);
}

void IECoreAppleseed::TransformStack::push_identity()
{
	push(asr::TransformSequence());
}

void IECoreAppleseed::TransformStack::clear()
{
	while ( size() )
		pop();

	m_stack.push(asr::TransformSequence());
}

void IECoreAppleseed::TransformStack::pop()
{
	m_stack.pop();
}

std::size_t IECoreAppleseed::TransformStack::size() const
{
	return m_stack.size();
}

const asr::TransformSequence& IECoreAppleseed::TransformStack::top() const
{
	return m_stack.top();
}

asr::TransformSequence& IECoreAppleseed::TransformStack::top()
{
	return m_stack.top();
}

void IECoreAppleseed::TransformStack::setTransform( const Imath::M44f &m )
{
	asf::Transformd xform;
	makeTransform( m, xform );
	m_stack.top().set_transform( 0.0, xform );
}

void IECoreAppleseed::TransformStack::setTransform( const std::set<float> &times,
	const std::vector<Imath::M44f> &transforms )
{
	makeTransformSequence( times, transforms, m_stack.top() );
}

void IECoreAppleseed::TransformStack::concatTransform( const Imath::M44f &m )
{
	asf::Transformd xform;
	makeTransform( m, xform );
	asr::TransformSequence seq;
	seq.set_transform( 0.0, xform );
	m_stack.top() = seq * m_stack.top();
}

void IECoreAppleseed::TransformStack::concatTransform( const std::set<float> &times,
	const std::vector<Imath::M44f> &transforms )
{
	asr::TransformSequence seq;
	makeTransformSequence( times, transforms, seq );
	m_stack.top() = seq * m_stack.top();
}

void IECoreAppleseed::TransformStack::makeTransform( const Imath::M44f &m, asf::Transformd &xform ) const
{
	Imath::M44d md( m );
	xform.set_local_to_parent( asf::Matrix4d( md ) );
	md.invert();
	xform.set_parent_to_local( asf::Matrix4d( md ) );
}

void IECoreAppleseed::TransformStack::makeTransformSequence( const std::set<float> &times,
	const std::vector<Imath::M44f> &transforms,
	renderer::TransformSequence &xformSeq ) const
{
	assert( times.size() == transforms.size() );

	asf::Transformd xform;

	size_t j = 0;
	for( std::set<float>::const_iterator it( times.begin() ), e( times.end() ); it != e; ++it )
	{
		makeTransform( transforms[j++], xform );
		xformSeq.set_transform( *it, xform );
	}
}
