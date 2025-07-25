//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/ClippingPlane.h"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( ClippingPlane );

ClippingPlane::ClippingPlane()
{
}

ClippingPlane::~ClippingPlane()
{
}

void ClippingPlane::copyFrom( const Object *other, CopyContext *context )
{
	Renderable::copyFrom( other, context );
}

void ClippingPlane::save( SaveContext *context ) const
{
	Renderable::save( context );
}

void ClippingPlane::load( LoadContextPtr context )
{
	Renderable::load( context );
}

bool ClippingPlane::isEqualTo( const Object *other ) const
{
	return Renderable::isEqualTo( other );
}

void ClippingPlane::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Renderable::memoryUsage( a );
}

void ClippingPlane::hash( MurmurHash &h ) const
{
	Renderable::hash( h );
}
