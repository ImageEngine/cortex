//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundData.h"

using namespace IECore;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( CompoundData );

CompoundData::CompoundData()
{
}

CompoundData::CompoundData( const CompoundDataMap &members )
	:	CompoundDataBase( members )
{
}

void CompoundData::copyFrom( const Object *other, IECore::Object::CopyContext *context )
{
	CompoundDataBase::copyFrom( other, context );
}

void CompoundData::save( IECore::Object::SaveContext *context ) const
{
	CompoundDataBase::save( context );
}

void CompoundData::load( IECore::Object::LoadContextPtr context )
{
	CompoundDataBase::load( context );
}

bool CompoundData::isEqualTo( const Object *other ) const
{
	return CompoundDataBase::isEqualTo( other );
}

void CompoundData::memoryUsage( Object::MemoryAccumulator &a ) const
{
	CompoundDataBase::memoryUsage( a );
}

void CompoundData::hash( MurmurHash &h ) const
{
	CompoundDataBase::hash( h );
}
