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

#include <boost/python.hpp>

#include "IECoreMaya/TypeIds.h"
#include "IECoreMaya/bindings/TypeIdBinding.h"

using namespace boost::python;

namespace IECoreMaya
{

void bindTypeId()
{
	enum_<TypeId>( "TypeId" )
		.value( "FromMayaConverter", FromMayaConverterTypeId )
		.value( "FromMayaObjectConverter", FromMayaObjectConverterTypeId )
		.value( "FromMayaPlugConverter", FromMayaPlugConverterTypeId )
		.value( "FromMayaMeshConverter", FromMayaMeshConverterTypeId )
		.value( "FromMayaCameraConverter", FromMayaCameraConverterTypeId )
		.value( "FromMayaGroupConverter", FromMayaGroupConverterTypeId )
		.value( "FromMayaNumericDataConverter", FromMayaNumericDataConverterTypeId )
		.value( "FromMayaNumericPlugConverter", FromMayaNumericPlugConverterTypeId )
		.value( "FromMayaFluidConverter", FromMayaFluidConverterTypeId )
		.value( "FromMayaStringPlugConverter", FromMayaStringPlugConverterTypeId )
		.value( "FromMayaShapeConverter", FromMayaShapeConverterTypeId )
		.value( "FromMayaCurveConverter", FromMayaCurveConverterTypeId )
		.value( "FromMayaParticleConverter", FromMayaParticleConverterTypeId )
		.value( "FromMayaDagNodeConverter", FromMayaDagNodeConverterTypeId )
		.value( "FromMayaPluginDataPlugConverter", FromMayaPluginDataPlugConverterTypeId )		
		.value( "FromMayaTransformConverter", FromMayaTransformConverterTypeId )				
		.value( "FromMayaImageConverter", FromMayaImageConverterTypeId )						
		.value( "PlaybackFrameList", PlaybackFrameListTypeId )
		.value( "FromMayaUnitPlugConverterf", FromMayaUnitPlugConverterfTypeId )
		.value( "FromMayaUnitPlugConverterd", FromMayaUnitPlugConverterdTypeId )
		.value(	"FromMayaNumericPlugConverterbb", FromMayaNumericPlugConverterbbTypeId )
		.value( "FromMayaNumericPlugConverterbi", FromMayaNumericPlugConverterbiTypeId )
		.value( "FromMayaNumericPlugConverterii", FromMayaNumericPlugConverteriiTypeId )
		.value( "FromMayaNumericPlugConverterif", FromMayaNumericPlugConverterifTypeId )
		.value( "FromMayaNumericPlugConverterid", FromMayaNumericPlugConverteridTypeId )
		.value( "FromMayaNumericPlugConverterfi", FromMayaNumericPlugConverterfiTypeId )
		.value( "FromMayaNumericPlugConverterff", FromMayaNumericPlugConverterffTypeId )
		.value( "FromMayaNumericPlugConverterfd", FromMayaNumericPlugConverterfdTypeId )
		.value( "FromMayaNumericPlugConverterdi", FromMayaNumericPlugConverterdiTypeId )
		.value( "FromMayaNumericPlugConverterdf", FromMayaNumericPlugConverterdfTypeId )
		.value( "FromMayaNumericPlugConverterdd", FromMayaNumericPlugConverterddTypeId )
		.value( "FromMayaArrayDataConverterii", FromMayaArrayDataConverteriiTypeId )
		.value( "FromMayaArrayDataConverterdd", FromMayaArrayDataConverterddTypeId )
		.value( "FromMayaArrayDataConverterdf", FromMayaArrayDataConverterdfTypeId )
		.value( "FromMayaArrayDataConverterss", FromMayaArrayDataConverterssTypeId )
		.value( "FromMayaArrayDataConverterVV3f", FromMayaArrayDataConverterVV3fTypeId )
		.value( "FromMayaArrayDataConverterVV3d", FromMayaArrayDataConverterVV3dTypeId )
		.value( "FromMayaArrayDataConverterVC3f", FromMayaArrayDataConverterVC3fTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV2fV2i", FromMayaCompoundNumericPlugConverterV2fV2iTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV2fV2f", FromMayaCompoundNumericPlugConverterV2fV2fTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV2fV2d", FromMayaCompoundNumericPlugConverterV2fV2dTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV2dV2i", FromMayaCompoundNumericPlugConverterV2dV2iTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV2dV2f", FromMayaCompoundNumericPlugConverterV2dV2fTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV2dV2d", FromMayaCompoundNumericPlugConverterV2dV2dTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3fV3i", FromMayaCompoundNumericPlugConverterV3fV3iTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3fV3f", FromMayaCompoundNumericPlugConverterV3fV3fTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3fV3d", FromMayaCompoundNumericPlugConverterV3fV3dTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3fC3f", FromMayaCompoundNumericPlugConverterV3fC3fTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3dV3i", FromMayaCompoundNumericPlugConverterV3dV3iTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3dV3f", FromMayaCompoundNumericPlugConverterV3dV3fTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3dV3d", FromMayaCompoundNumericPlugConverterV3dV3dTypeId )
		.value( "FromMayaCompoundNumericPlugConverterV3dC3f", FromMayaCompoundNumericPlugConverterV3dC3fTypeId )
		.value( "FromMayaTransformationMatrixfConverter", FromMayaTransformationMatrixfConverterTypeId )	
		.value( "FromMayaTransformationMatrixdConverter", FromMayaTransformationMatrixdConverterTypeId )	
	;
}

}
