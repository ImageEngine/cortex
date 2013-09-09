//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_H
#define IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_H

#include "GA/GA_AttributeRef.h"
#include "GU/GU_Detail.h"
#include "GU/GU_DetailHandle.h"
#include "SOP/SOP_Node.h"
#include "UT/UT_StringMMPattern.h"

#include "IECore/Primitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedData.h"

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/FromHoudiniConverter.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( FromHoudiniGeometryConverter );

/// The FromHoudiniGeometryConverter class forms a base class for all classes able to perform
/// some kind of conversion from a Houdini GU_Detail to an IECore::Primitive.
class FromHoudiniGeometryConverter : public FromHoudiniConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromHoudiniGeometryConverter, FromHoudiniGeometryConverterTypeId, IECore::ToCoreConverter );

		//! @name Factory
		//////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter which will convert the given Houdini GU_Detail to an IECore::Primitive.
		/// If resultType is specified then only converters which create objects of that type will
		/// be returned - the default value allows any suitable converter to be created. If no
		/// matching converters exist then returns 0. If a null handle is provided, any suitable
		/// converter will be returned. This may be useful to access parameters of a derived
		/// converter before the geometry exists. See SOP_ParameterisedHolder for an example.
		static FromHoudiniGeometryConverterPtr create( const GU_DetailHandle &handle, IECore::TypeId resultType=IECore::InvalidTypeId );
		static FromHoudiniGeometryConverterPtr create( const GU_DetailHandle &handle, const std::set<IECore::TypeId> &resultTypes );
		//@}
		
		/// Fills the passed vector with all the IECore::TypeIds for which
		/// a FromHoudiniGeometryConverter is available.
		static void supportedTypes( std::set<IECore::TypeId> &types );
		
		/// Convenience function to extract the named shapes from the given GU_Detail. This can be used before
		/// calling the factory create mechanism, when only the named portion of the detail is of interest.
		static GU_DetailHandle extract( const GU_Detail *geo, const UT_StringMMPattern &nameFilter );
		
		enum Convertability
		{
			Inapplicable = 0,
			Ideal,
			Suitable,
			Admissible,
			InvalidValue,
		};
		
	protected :

		FromHoudiniGeometryConverter( const GU_DetailHandle &handle, const std::string &description );
		FromHoudiniGeometryConverter( const SOP_Node *sop, const std::string &description );

		virtual ~FromHoudiniGeometryConverter();
		
		/// Implemented to aquire the read lock on the GU_Detail held by the GU_DetailHandle,
		/// call doPrimitiveConversion(), and finally unlock the GU_Detail. Derived classes
		/// need not reimplement this function, but should instead implement doPrimitiveConversion().
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;
		/// Must be implemented by derived classes to return a IECore::Primitive created to represent the specified GU_Detail.
		virtual IECore::PrimitivePtr doPrimitiveConversion( const GU_Detail *geo, const IECore::CompoundObject *operands ) const = 0;
		
		typedef FromHoudiniGeometryConverterPtr (*CreatorFn)( const GU_DetailHandle &handle );
		typedef Convertability (*ConvertabilityFn)( const GU_DetailHandle &handle );

		static void registerConverter( IECore::TypeId resultType, CreatorFn creator, ConvertabilityFn canConvert );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				Description( IECore::TypeId resultType );
			private :
				static FromHoudiniGeometryConverterPtr creator( const GU_DetailHandle &handle );
				static Convertability canConvert( const GU_DetailHandle &handle );
		};
		
		/// returns a reference to the GU_DetailHandle
		const GU_DetailHandle &handle() const;
		/// extracts the GU_DetailHandle from a SOP_Node
		static const GU_DetailHandle handle( const SOP_Node *sop );
		
		/// Extracts position and attribs from the GU_Detail and stores them as primitive variables on the IECore::Primitive provided.
		/// In most cases, this is the only transfer function that derived classes will need to use
		void transferAttribs(
			const GU_Detail *geo, IECore::Primitive *result, const IECore::CompoundObject *operands,
			IECore::PrimitiveVariable::Interpolation vertexInterpolation = IECore::PrimitiveVariable::FaceVarying,
			IECore::PrimitiveVariable::Interpolation primitiveInterpolation = IECore::PrimitiveVariable::Uniform,
			IECore::PrimitiveVariable::Interpolation pointInterpolation = IECore::PrimitiveVariable::Vertex,
			IECore::PrimitiveVariable::Interpolation detailInterpolation = IECore::PrimitiveVariable::Constant
		) const;
		
		/// This simple class is used to describe the destination mapping for point or primitive
		/// attributes that have been remapped using the 'attribute' sop.
		struct RemapInfo
		{
			std::string name;
			IECore::TypeId type;
			IECore::PrimitiveVariable::Interpolation interpolation;
			int elementIndex;
		};

		/// Attribute remapping
		typedef std::map<std::string, std::vector<RemapInfo> > AttributeMap;
		void remapAttributes( const GU_Detail *geo, AttributeMap &pointAttributeMap, AttributeMap &primitiveAttributeMap ) const;

		/// Utility functions for transfering each attrib type from Houdini onto the IECore::Primitive provided
		void transferDetailAttribs(
			const GU_Detail *geo, const UT_StringMMPattern &attribFilter,
			IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation
		) const;
		void transferElementAttribs(
			const GU_Detail *geo, const GA_Range &range, const GA_AttributeDict &attribs, const UT_StringMMPattern &attribFilter,
			AttributeMap &attributeMap, IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation
		) const;
		
		void transferAttribData(
			IECore::Primitive *result, IECore::PrimitiveVariable::Interpolation interpolation,
			const GA_ROAttributeRef &attrRef, const GA_Range &range, const RemapInfo *remapInfo = 0
		) const;
		
		/// Utility functions for extracting attrib data from Houdini and storing it as a DataPtr of type T
		/// @parm index allows a single component to be extracted from a larger container
		template <typename T>
		typename T::Ptr extractData( const GA_Attribute *attr, const GA_Range &range, int elementIndex = -1 ) const;
		
		template <typename T>
		typename T::Ptr extractData( const GA_Attribute *attr ) const;
		
		IECore::DataPtr extractStringVectorData( const GA_Attribute *attr, const GA_Range &range, IECore::IntVectorDataPtr &indexData ) const;
		IECore::DataPtr extractStringData( const GU_Detail *geo, const GA_Attribute *attr ) const;

	private :
		
		void constructCommon();
		
		// This extra factory function is provided for the python bindings
		static FromHoudiniGeometryConverterPtr create( const SOP_Node *sop, IECore::TypeId resultType=IECore::InvalidTypeId );
		
		// function to map standard Houdini names to IECore PrimitiveVariable names
		const std::string processPrimitiveVariableName( const std::string &name ) const;
		
		// the handle to the GU_Detail
		GU_DetailHandle m_geoHandle;
		IECore::StringParameterPtr m_attributeFilterParameter;
		IECore::BoolParameterPtr m_convertStandardAttributesParameter;
		
		struct Types
		{
			Types( IECore::TypeId result );
			IECore::TypeId resultType;
			bool operator < ( const Types &other ) const;
		};

		typedef std::map<Types, std::pair<CreatorFn, ConvertabilityFn> > TypesToFnsMap;
		static TypesToFnsMap *typesToFns();
};

}

#include "FromHoudiniGeometryConverter.inl"

#endif // IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_H
