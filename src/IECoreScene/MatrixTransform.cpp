//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MurmurHash.h"
#include "IECoreScene/MatrixTransform.h"
#include "IECoreScene/Renderer.h"

using namespace IECore;
using namespace IECoreScene;
using namespace boost;

static IndexedIO::EntryID g_matrixEntry("matrix");
const unsigned int MatrixTransform::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(MatrixTransform);

MatrixTransform::MatrixTransform( const Imath::M44f &m )
	:	matrix( m )
{
}

MatrixTransform::~MatrixTransform()
{
}

void MatrixTransform::render( Renderer *renderer ) const
{
	renderer->concatTransform( matrix );
}

Imath::M44f MatrixTransform::transform( float time ) const
{
	return matrix;
}

void MatrixTransform::copyFrom( const Object *other, CopyContext *context )
{
	Transform::copyFrom( other, context );
	const MatrixTransform *t = static_cast<const MatrixTransform *>( other );
	matrix = t->matrix;
}

void MatrixTransform::save( SaveContext *context ) const
{
	Transform::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( g_matrixEntry, matrix.getValue(), 16 );
}

void MatrixTransform::load( LoadContextPtr context )
{
	Transform::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	float *f = matrix.getValue();
	container->read( g_matrixEntry, f, 16 );
}

bool MatrixTransform::isEqualTo( const Object *other ) const
{
	if( !Transform::isEqualTo( other ) )
	{
		return false;
	}
	const MatrixTransform *t = static_cast<const MatrixTransform *>( other );
	return matrix == t->matrix;
}

void MatrixTransform::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Transform::memoryUsage( a );
	a.accumulate( sizeof( matrix ) );
}

void MatrixTransform::hash( MurmurHash &h ) const
{
	Transform::hash( h );
	h.append( matrix );
}
