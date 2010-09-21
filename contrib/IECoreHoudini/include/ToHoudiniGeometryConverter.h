//////////////////////////////////////////////////////////////////////////
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

#ifndef IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_H
#define IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_H

#include "GB/GB_AttributeRef.h"
#include "GEO/GEO_Vertex.h"
#include "GU/GU_Detail.h"
#include "GU/GU_DetailHandle.h"

#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"

#include "TypeIds.h"
#include "ToHoudiniConverter.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( ToHoudiniGeometryConverter );

/// The ToHoudiniGeometryConverter class forms a base class for all classes able to perform
/// some kind of conversion from an IECore::Primitive to a Houdini GU_Detail.
class ToHoudiniGeometryConverter : public ToHoudiniConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniGeometryConverter, ToHoudiniGeometryConverterTypeId, ToHoudiniConverter );

		/// Converts the IECore::Primitive into the given GU_Detail and returns true if successful
		/// and false otherwise. Implemented to aquire the write lock on the GU_Detail held by the
		/// GU_DetailHandle, call doPrimitiveConversion(), and finally unlock the GU_Detail.
		bool convert( GU_DetailHandle handle ) const;

		/// Creates a converter which will convert the given IECore::Primitive to a Houdini GU_Detail.
		/// Returns 0 if no such converter can be found.
		static ToHoudiniGeometryConverterPtr create( const IECore::Primitive *primitive );

	protected :

		ToHoudiniGeometryConverter( const IECore::Primitive *primitive, const std::string &description );
		
		virtual ~ToHoudiniGeometryConverter();
		
		/// Must be implemented by derived classes to fill the given GU_Detail with data from the IECore::Primitive
		virtual bool doPrimitiveConversion( const IECore::Primitive *primitive, GU_Detail *geo, IECore::ConstCompoundObjectPtr operands ) const = 0;

		typedef ToHoudiniGeometryConverterPtr (*CreatorFn)( const IECore::Primitive *primitive );

		static void registerConverter( IECore::TypeId fromType, CreatorFn creator );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				Description( IECore::TypeId fromType );
			private :
				static ToHoudiniGeometryConverterPtr creator( const IECore::Primitive *primitive );
		};
		
		/// Appends points to the GU_Detail from the given V3fVectorData.
		/// Returns a GEO_PointList containing the newly added points.
		GEO_PointList appendPoints( GU_Detail *geo, const IECore::V3fVectorData *positions ) const;
		
		/// Extracts primitive variables from the IECore::Primitive provided and appends them to the GU_Detail.
		/// In most cases, this is the only transfer function that derived classes will need to use
		void transferAttribs(
			const IECore::Primitive *primitive, GU_Detail *geo,
			GEO_PointList *newPoints = 0, GEO_PrimList *newPrims = 0,
			IECore::PrimitiveVariable::Interpolation vertexInterpolation = IECore::PrimitiveVariable::FaceVarying,
			IECore::PrimitiveVariable::Interpolation primitiveInterpolation = IECore::PrimitiveVariable::Uniform,
			IECore::PrimitiveVariable::Interpolation pointInterpolation = IECore::PrimitiveVariable::Vertex,
			IECore::PrimitiveVariable::Interpolation detailInterpolation = IECore::PrimitiveVariable::Constant
		) const;
		
		typedef void TransferReturnType;
		typedef UT_PtrArray<GEO_Vertex*> VertexList;
		
		/// Utility structs for transfering IECore::Data onto the GU_Detail using DespatchedTypedData
		template <typename Container>
		struct TransferAttrib
		{
			typedef TransferReturnType ReturnType;
			
			TransferAttrib( GU_Detail *geo, Container *container, std::string name, GEO_AttributeOwner owner );
			
			template<typename T>
			TransferReturnType operator()( typename T::ConstPtr data ) const;
			
			GU_Detail *m_geo;
			Container *m_container;
			std::string m_name;
			GEO_AttributeOwner m_owner;
		};
		
		struct TransferDetailAttrib
		{
			typedef TransferReturnType ReturnType;
			
			TransferDetailAttrib( GU_Detail *geo, std::string name );
	
			template<typename T>
			TransferReturnType operator()( typename T::ConstPtr data ) const;
	
			GU_Detail *m_geo;
			std::string m_name;
		};
	
	private :
			
		/// Struct for maintaining the registered derived classes
		struct Types
		{
			Types( IECore::TypeId from );
			IECore::TypeId fromType;
			bool operator < ( const Types &other ) const;
		};

		typedef std::map<Types, CreatorFn> TypesToFnsMap;
		static TypesToFnsMap *typesToFns();

};

} // namespace IECoreHoudini

#include "ToHoudiniGeometryConverter.inl"

#endif // IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_H
