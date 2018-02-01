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

#ifndef IE_COREMAYA_FROMMAYAMESHCONVERTER_H
#define IE_COREMAYA_FROMMAYAMESHCONVERTER_H

#include "IECoreMaya/FromMayaShapeConverter.h"

#include "IECore/VectorTypedData.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECoreScene/Primitive.h"

#include "maya/MString.h"

class MFnMesh;

namespace IECoreMaya
{

/// The FromMayaMeshConverter converts types compatible with
/// MFnMesh into IECore::MeshPrimitive objects.
/// \todo Vertex color support. Blind data support?
/// \ingroup conversionGroup
class IECOREMAYA_API FromMayaMeshConverter : public FromMayaShapeConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaMeshConverter, FromMayaMeshConverterTypeId, FromMayaShapeConverter );

		FromMayaMeshConverter( const MObject &object );
		FromMayaMeshConverter( const MDagPath &dagPath );

		virtual ~FromMayaMeshConverter();

		//! @name Parameter accessors
		//////////////////////////////////////////////////////////
		//@{
		IECore::StringParameter *interpolationParameter();
		const IECore::StringParameter *interpolationParameter() const;

		IECore::BoolParameter *colorsParameter();
		const IECore::BoolParameter *colorsParameter() const;

		IECore::BoolParameter *extraColorsParameter();
		const IECore::BoolParameter *extraColorsParameter() const;

		IECore::BoolParameter *normalsParameter();
		const IECore::BoolParameter *normalsParameter() const;

		IECore::BoolParameter *uvParameter();
		const IECore::BoolParameter *uvParameter() const;

		//@}

	protected :

		virtual IECoreScene::PrimitivePtr doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;
		virtual IECoreScene::PrimitivePtr doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const;

	private :

		void constructCommon();

		IECoreScene::PrimitiveVariable points() const;
		IECoreScene::PrimitiveVariable normals() const;
		IECoreScene::PrimitiveVariable uvs( const MString &uvSet, const std::vector<int> &vertsPerFace ) const;
		IECoreScene::PrimitiveVariable colors( const MString &colorSet="", bool forceRgb = false ) const;

		IECoreScene::PrimitivePtr doPrimitiveConversion( MFnMesh &fnMesh ) const;

		static Description<FromMayaMeshConverter> m_description;
		static Description<FromMayaMeshConverter> m_dataDescription;

};

IE_CORE_DECLAREPTR( FromMayaMeshConverter );

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYAMESHCONVERTER_H
