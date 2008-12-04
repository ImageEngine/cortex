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

//! \file CompositeAlgo.h
/// This file contains rsl implementations of the C++
/// compositing operations defined in IECore/CompositeAlgo.h

#ifndef IECORERI_COMPOSITEALGO_H
#define IECORERI_COMPOSITEALGO_H

color ieCompositeOver( color aVal; color aAlpha; color bVal; color bAlpha )
{
	return aVal + bVal * ( color( 1 ) - aAlpha );
}

color ieCompositeMax( color aVal; color aAlpha; color bVal; color bAlpha )
{
	return max( aVal, bVal );
}

color ieCompositeMin( color aVal; color aAlpha; color bVal; color bAlpha )
{
	return min( aVal, bVal );
}

color ieCompositeMultiply( color aVal; color aAlpha; color bVal; color bAlpha )
{
	return aVal * bVal;
}

#define IECORE_COMPOSITE_INVALID 0
#define IECORE_COMPOSITE_OVER 1
#define IECORE_COMPOSITE_MAX 2
#define IECORE_COMPOSITE_MIN 3
#define IECORE_COMPOSITE_MULTIPLY 4

color ieComposite( float operation; color aVal; color aAlpha; color bVal; color bAlpha )
{
	if( operation==IECORE_COMPOSITE_OVER )
	{
		return ieCompositeOver( aVal, aAlpha, bVal, bAlpha );
	}
	else if( operation==IECORE_COMPOSITE_MAX )
	{
		return ieCompositeMax( aVal, aAlpha, bVal, bAlpha );
	}
	else if( operation==IECORE_COMPOSITE_MIN )
	{
		return ieCompositeMin( aVal, aAlpha, bVal, bAlpha );
	}
	else if( operation==IECORE_COMPOSITE_MULTIPLY )
	{
		return ieCompositeMultiply( aVal, aAlpha, bVal, bAlpha );
	}
	return 0;
}

#endif // IECORERI_COMPOSITEALGO_H
