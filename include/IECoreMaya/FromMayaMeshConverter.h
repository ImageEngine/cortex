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
#include "IECore/Primitive.h"

#include "maya/MString.h"

class MFnMesh;

namespace IECoreMaya
{

/// The FromMayaMeshConverter converts types compatible with
/// MFnMesh into IECore::MeshPrimitive objects.
/// \todo Vertex color support. Blind data support?
/// \ingroup conversionGroup
class FromMayaMeshConverter : public FromMayaShapeConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaMeshConverter, FromMayaMeshConverterTypeId, FromMayaShapeConverter );

		FromMayaMeshConverter( const MObject &object );
		FromMayaMeshConverter( const MDagPath &dagPath );

		virtual ~FromMayaMeshConverter();

		/// Returns just the points for the mesh.
		/// \todo It would be nice if this was virtual somewhere and implemented for other
		/// converters for which it makes sense. I'm not sure it belongs in the FromMayaShapeConverter though.
		IECore::V3fVectorDataPtr points() const;
		/// Returns just the normals for the mesh.
		IECore::V3fVectorDataPtr normals() const;
		/// Returns just the s for the mesh.
		IECore::FloatVectorDataPtr s( const MString &uvSet="" ) const;
		/// Returns just the t for the mesh.
		IECore::FloatVectorDataPtr t( const MString &uvSet="" ) const;
		/// Returns the facevarying uv indexes for the specified uv set,
		/// or the current set if not specified. This can be necessary for
		/// some algorithms as the facevarying uv data returned from the above
		/// functions has thrown away the concept of two faces sharing the same
		/// logical uv at a vertex. By comparing the uv indices this information can be
		/// obtained.
		IECore::IntVectorDataPtr stIndices( const MString &uvSet="" ) const;
		/// Returns just the face-varying colors for the mesh.
		/// @param forceRgb will do appropriate conversions to return rgb colors.
		IECore::DataPtr colors( const MString &colorSet="", bool forceRgb = false ) const;
		/// Returns just the face-varying colors for the mesh.
		template< class T >	typename IECore::TypedData< T >::Ptr colors( const MString &colorSet="" ) const
		{
			return boost::dynamic_pointer_cast< T >( colors( colorSet, false  ) );
		}

		//! @name Parameter accessors
		//////////////////////////////////////////////////////////
		//@{
		IECore::StringParameterPtr interpolationParameter();
		IECore::StringParameterPtr interpolationParameter() const;

		IECore::BoolParameterPtr pointsParameter();
		IECore::BoolParameterPtr pointsParameter() const;

		IECore::BoolParameterPtr normalsParameter();
		IECore::BoolParameterPtr normalsParameter() const;

		IECore::BoolParameterPtr stParameter();
		IECore::BoolParameterPtr stParameter() const;

		IECore::BoolParameterPtr extraSTParameter();
		IECore::BoolParameterPtr extraSTParameter() const;

		IECore::BoolParameterPtr colorsParameter();
		IECore::ConstBoolParameterPtr colorsParameter() const;

		IECore::BoolParameterPtr extraColorsParameter();
		IECore::ConstBoolParameterPtr extraColorsParameter() const;

		//@}

	protected :

		virtual IECore::PrimitivePtr doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;
		virtual IECore::PrimitivePtr doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const;

	private :

		void constructCommon();
		
		IECore::IntVectorDataPtr getStIndices( const MString &uvSet, IECore::ConstIntVectorDataPtr verticesPerFaceData ) const;
		
		void sAndT( const MString &uvSet, IECore::ConstIntVectorDataPtr stIndicesData, IECore::FloatVectorDataPtr& s, IECore::FloatVectorDataPtr& t ) const;

		IECore::PrimitivePtr doPrimitiveConversion( MFnMesh &fnMesh ) const;

		IECore::StringParameterPtr m_interpolation;
		IECore::BoolParameterPtr m_points;
		IECore::BoolParameterPtr m_normals;
		IECore::BoolParameterPtr m_st;
		IECore::BoolParameterPtr m_extraST;
		IECore::StringParameterPtr m_primVarAttrPrefix;

		static Description<FromMayaMeshConverter> m_description;
		static Description<FromMayaMeshConverter> m_dataDescription;

};

IE_CORE_DECLAREPTR( FromMayaMeshConverter );

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYAMESHCONVERTER_H
