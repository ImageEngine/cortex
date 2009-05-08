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

#include "IECore/SimpleTypedData.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/MArrayTraits.h"
#include "IECoreMaya/ToMayaMatrixDataConverter.h"

#include "maya/MFnMatrixData.h"

using namespace IECoreMaya;

template<typename F>
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaMatrixDataConverter<F> > ToMayaMatrixDataConverter<F>::g_description( F::staticTypeId(), MFn::kMatrixData );

template<typename F>
ToMayaMatrixDataConverter<F>::ToMayaMatrixDataConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "ToMayaMatrixDataConverter", "Converts IECore::M44*Data objects to a Maya object.", object )
{
}

template<typename F>
bool ToMayaMatrixDataConverter<F>::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;

	typename F::ConstPtr data = IECore::runTimeCast<const F>( from );
	if( !data )
	{
		return false;
	}

	const typename F::ValueType &coreMatrix = data->readable();
	MMatrix mayaMatrix = IECore::convert<MMatrix>( coreMatrix );

	MFnMatrixData fnMD;
	to = fnMD.create( mayaMatrix, &s );

	return s;
}

template class ToMayaMatrixDataConverter<IECore::M44fData>;
template class ToMayaMatrixDataConverter<IECore::M44dData>;
