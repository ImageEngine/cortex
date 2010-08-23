//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREHOUDINI_FROMHOUDINISOPCONVERTER_H
#define IE_COREHOUDINI_FROMHOUDINISOPCONVERTER_H

#include "GB/GB_AttributeRef.h"
#include "GU/GU_Detail.h"
#include "SOP/SOP_Node.h"

#include "IECore/Primitive.h"

#include "TypeIds.h"
#include "FromHoudiniNodeConverter.h"

namespace IECoreHoudini
{
	
/// The FromHoudiniSopConverter class forms a base class for all classes able to perform
/// some kind of conversion from a Houdini SOP_Node to an IECore::Primitive.
class FromHoudiniSopConverter : public IECoreHoudini::FromHoudiniNodeConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromHoudiniSopConverter, FromHoudiniSopConverterTypeId, IECore::ToCoreConverter );

		typedef SOP_Node FromType;
		
	protected :

		FromHoudiniSopConverter( const SOP_Node *sop, const std::string &description );

		virtual ~FromHoudiniSopConverter();

		/// Implemented to call doPrimitiveConversion() Derived classes need not reimplement this function,
		/// but should instead implement doPrimitiveConversion().
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;
		/// Must be implemented by derived classes to return a Primitive created to represent the specified GU_Detail.
		virtual IECore::PrimitivePtr doPrimitiveConversion( const GU_Detail *geo, IECore::ConstCompoundObjectPtr operands ) const = 0;
		
		/// Retrieves the SOP_Node held by the converter
		SOP_Node *sop() const;
		
		/// Extracts position and attribs from Houdini and stores them as primitive variables on the IECore::Primitive provided.
		/// In most cases, this is the only transfer function that derived classes will need to use
		void transferAttribs(
			const GU_Detail *geo, IECore::Primitive *result,
			IECore::PrimitiveVariable::Interpolation vertexInterpolation = IECore::PrimitiveVariable::FaceVarying,
			IECore::PrimitiveVariable::Interpolation primitiveInterpolation = IECore::PrimitiveVariable::Uniform,
			IECore::PrimitiveVariable::Interpolation pointInterpolation = IECore::PrimitiveVariable::Vertex,
			IECore::PrimitiveVariable::Interpolation detailInterpolation = IECore::PrimitiveVariable::Constant
		) const;
		
		typedef UT_PtrArray<const GEO_Vertex*> VertexList;
		
		/// Utility functions for transfering each attrib type from Houdini onto the IECore::Primitive provided
		void transferDetailAttribs( const GU_Detail *geo, IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation ) const;
		void transferPointAttribs( const GU_Detail *geo, IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation, const GEO_PointList &points ) const;
		void transferPrimitiveAttribs( const GU_Detail *geo, IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation, const GEO_PrimList &primitives ) const;
		void transferVertexAttribs( const GU_Detail *geo, IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation, const VertexList &vertices ) const;
		
		template <typename Container>
		void transferAttribData(
			const Container &container, IECore::Primitive *result,
			IECore::PrimitiveVariable::Interpolation interpolation,
			const GB_Attribute *attr, const GB_AttributeRef &attrRef
		) const;
		
		/// Utility functions for extracting attrib data from Houdini and storing it as a DataPtr of type T
		template <typename T, typename Container>
		IECore::DataPtr extractData( const Container &container, const GB_AttributeRef &attrRef ) const;
		
		template <typename T>
		IECore::DataPtr extractData( const GB_AttributeTable &attribs, const GB_AttributeRef &attrRef ) const;

	private :

		static FromHoudiniNodeConverter::Description<FromHoudiniSopConverter> m_description;
};

// register our converter
IE_CORE_DECLAREPTR( FromHoudiniSopConverter );

}

#include "FromHoudiniSopConverter.inl"

#endif // IE_COREHOUDINI_FROMHOUDINISOPCONVERTER_H
