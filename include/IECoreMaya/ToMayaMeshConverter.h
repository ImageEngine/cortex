//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_TOMAYAMESHCONVERTER_H
#define IE_COREMAYA_TOMAYAMESHCONVERTER_H

#include "maya/MFnMesh.h"

#include "IECoreScene/MeshPrimitive.h"

#include "IECoreMaya/ToMayaObjectConverter.h"

namespace IECoreMaya
{

class ToMayaMeshConverter;
IE_CORE_DECLAREPTR( ToMayaMeshConverter );

/// This class converts IECore::MeshPrimitives to maya mesh objects.
/// \ingroup conversionGroup
class IECOREMAYA_API ToMayaMeshConverter : public ToMayaObjectConverter
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToMayaMeshConverter, ToMayaMeshConverterTypeId, ToMayaObjectConverter );

		ToMayaMeshConverter( IECore::ConstObjectPtr object );

		/// creates the standard attribute ieMeshInterpolation plug in the given Mesh object (it expects to be a MFnMesh bindable object).
		/// \param defaultInterpolation - only accept values listed in the presets (keys or values) of FromMayaMeshConverter.interpolationParameter().
		static bool setMeshInterpolationAttribute( MObject &object, std::string interpolation = "linear" );

	protected:

		virtual bool doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const;

		typedef ToMayaObjectConverterDescription<ToMayaMeshConverter> Description;
		static Description g_meshDataDescription;
		static Description g_meshDescription;

	private:

		void assignDefaultShadingGroup( MObject &shape ) const;
		void addUVSet( MFnMesh &fnMesh, const MIntArray &polygonCounts, const IECoreScene::MeshPrimitive *mesh, IECoreScene::PrimitiveVariableMap::const_iterator &uvIt ) const;

};

}

#endif // IE_COREMAYA_TOMAYAMESHCONVERTER_H
