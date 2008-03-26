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

#include "IECore/PrimitiveEvaluator.h"

#include "IECore/MeshPrimitiveEvaluator.h"
#include "IECore/SpherePrimitiveEvaluator.h"
#include "IECore/ImagePrimitiveEvaluator.h"

using namespace IECore;

PrimitiveEvaluator::CreatorMap &PrimitiveEvaluator::getCreateFns()
{
	static CreatorMap *g_createFns = new CreatorMap();
	assert( g_createFns );
	return *g_createFns;
}

void PrimitiveEvaluator::registerCreator( TypeId id, CreatorFn f )
{
	assert( f );
	assert( f != &PrimitiveEvaluator::create );
	
        CreatorMap &createFns = getCreateFns();
        
        assert( createFns.find( id ) == createFns.end() );
        
        createFns.insert( CreatorMap::value_type( id, f) );
}

PrimitiveEvaluatorPtr PrimitiveEvaluator::create( ConstPrimitivePtr primitive )
{
	assert( primitive );
	
        const CreatorMap &createFns = getCreateFns();
        
        CreatorMap::const_iterator it = createFns.find( primitive->typeId() );
        if (it == createFns.end())
        {
                return 0;
        }
        
        return (it->second)(primitive);
}

PrimitiveEvaluator::~PrimitiveEvaluator()
{
}

PrimitiveEvaluator::Result::~Result()
{
}

void PrimitiveEvaluator::validateResult( const ResultPtr &result ) const
{
	if (dynamic_cast< const MeshPrimitiveEvaluator * >( this ) )
	{
		if (! boost::dynamic_pointer_cast< MeshPrimitiveEvaluator::Result >( result ) )
		{
			throw InvalidArgumentException("Invalid PrimitiveEvaulator result type");
		}
	}
	else if (dynamic_cast< const SpherePrimitiveEvaluator * >( this ) )
	{
		if (! boost::dynamic_pointer_cast< SpherePrimitiveEvaluator::Result >( result ) )
		{
			throw InvalidArgumentException("Invalid PrimitiveEvaulator result type");
		}
	}
	else if (dynamic_cast< const ImagePrimitiveEvaluator * >( this ) )
	{
		if (! boost::dynamic_pointer_cast< ImagePrimitiveEvaluator::Result >( result ) )
		{
			throw InvalidArgumentException("Invalid PrimitiveEvaulator result type");
		}
	}
}
