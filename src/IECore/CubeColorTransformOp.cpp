//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/CubeColorTransformOp.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/Primitive.h"
#include "IECore/Interpolator.h"

using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( CubeColorTransformOp );

CubeColorTransformOp::CubeColorTransformOp() 
	:	ColorTransformOp( "name", "description" ), m_data( 0 )
{
	m_cubeParameter = new CubeColorLookupfParameter(
		"cube",
		"cube description",
		
		new CubeColorLookupfData()
	);

	parameters()->addParameter( m_cubeParameter );			
}

CubeColorTransformOp::~CubeColorTransformOp()
{
}

CubeColorLookupfParameterPtr CubeColorTransformOp::cubeParameter()
{
	return m_cubeParameter;	
}

ConstCubeColorLookupfParameterPtr CubeColorTransformOp::cubeParameter() const
{
	return m_cubeParameter;	
}

void CubeColorTransformOp::begin( ConstCompoundObjectPtr operands )
{
	assert( operands );
	
	m_data = operands->member<CubeColorLookupfData>( "cube" );
}


void CubeColorTransformOp::transform( Imath::Color3f &color ) const
{
	assert( m_data );
	color = m_data->readable().operator()( color );
}
