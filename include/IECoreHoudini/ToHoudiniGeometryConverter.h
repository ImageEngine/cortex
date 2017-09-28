//////////////////////////////////////////////////////////////////////////
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

#ifndef IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_H
#define IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_H

#include "GU/GU_Detail.h"
#include "GU/GU_DetailHandle.h"

#include "IECore/Primitive.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/VisibleRenderable.h"

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/ToHoudiniConverter.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( ToHoudiniGeometryConverter );

/// The ToHoudiniGeometryConverter class forms a base class for all classes able to perform
/// some kind of conversion from an IECore::Object to a Houdini GU_Detail.
class ToHoudiniGeometryConverter : public ToHoudiniConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniGeometryConverter, ToHoudiniGeometryConverterTypeId, ToHoudiniConverter );

		/// Converts the IECore::Object into the given GU_Detail and returns true if successful
		/// and false otherwise. Implemented to aquire the write lock on the GU_Detail held by the
		/// GU_DetailHandle, call doConversion(), and finally unlock the GU_Detail.
		bool convert( GU_DetailHandle handle ) const;

		/// Transfers the primitive variables from the IECore::Primitive to the GU_Detail. This is
		/// usually called by convert(), but is also provided here so attribs may be transfered onto
		/// existing topology.
		virtual void transferAttribs( GU_Detail *geo, const GA_Range &points, const GA_Range &prims ) const;

		/// Creates a converter which will convert the given IECore::Object to a Houdini GU_Detail.
		/// Returns 0 if no such converter can be found.
		static ToHoudiniGeometryConverterPtr create( const IECore::Object *object );

		/// Fills the passed vector with all the IECore::TypeIds for which
		/// a ToHoudiniGeometryConverter is available.
		static void supportedTypes( std::set<IECore::TypeId> &types );

		IECore::StringParameter *nameParameter();
		const IECore::StringParameter *nameParameter() const;

		IECore::StringParameter *attributeFilterParameter();
		const IECore::StringParameter *attributeFilterParameter() const;

		IECore::BoolParameter *convertStandardAttributesParameter();
		const IECore::BoolParameter *convertStandardAttributesParameter() const;

	protected :

		ToHoudiniGeometryConverter( const IECore::Object *object, const std::string &description );

		virtual ~ToHoudiniGeometryConverter();

		/// Must be implemented by derived classes to fill the given GU_Detail with data from the IECore::Object
		virtual bool doConversion( const IECore::Object *object, GU_Detail *geo ) const = 0;

		/// Utility to name the primitives based on the name parameter. This is called by the default
		/// implementation of transferAttribs(), and should be called by any overriding implementation.
		void setName( GU_Detail *geo, const GA_Range &prims ) const;

		/// May be implemented by derived classes to pre-process PrimitiveVariables before conversion.
		/// Default implementation simply returns a shallow copy of the input variable.
		virtual IECore::PrimitiveVariable processPrimitiveVariable( const IECore::Primitive *primitive, const IECore::PrimitiveVariable &primVar ) const;

		typedef ToHoudiniGeometryConverterPtr (*CreatorFn)( const IECore::Object *object );

		static void registerConverter( IECore::TypeId fromType, CreatorFn creator );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				Description( IECore::TypeId fromType );
			private :
				static ToHoudiniGeometryConverterPtr creator( const IECore::Object *object );
		};

		/// Appends points to the GA_Detail. Returns a GA_Range containing the GA_Offsets for the newly added points.
		GA_Range appendPoints( GA_Detail *geo, size_t numPoints ) const;

		/// Transfers the primitive variables from the IECore::Primitive to the GU_Detail. In most cases,
		/// derived classes will implement transferAttribs to call this method with the appropriate arguments.
		void transferAttribValues(
			const IECore::Primitive *primitive, GU_Detail *geo, const GA_Range &points, const GA_Range &prims,
			IECore::PrimitiveVariable::Interpolation vertexInterpolation = IECore::PrimitiveVariable::FaceVarying,
			IECore::PrimitiveVariable::Interpolation primitiveInterpolation = IECore::PrimitiveVariable::Uniform,
			IECore::PrimitiveVariable::Interpolation pointInterpolation = IECore::PrimitiveVariable::Vertex,
			IECore::PrimitiveVariable::Interpolation detailInterpolation = IECore::PrimitiveVariable::Constant
		) const;

	private :

		IECore::StringParameterPtr m_nameParameter;
		IECore::StringParameterPtr m_attributeFilterParameter;
		IECore::BoolParameterPtr m_convertStandardAttributesParameter;

		// function to handle the special case for P
		void transferP( const IECore::V3fVectorData *positions, GU_Detail *geo, const GA_Range &points ) const;

		// function to map standard IECore PrimitiveVariable names to Houdini names
		const std::string processPrimitiveVariableName( const std::string &name ) const;

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
