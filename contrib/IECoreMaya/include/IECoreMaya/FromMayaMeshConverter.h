//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaObjectConverter.h"

#include "IECore/VectorTypedData.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/Primitive.h"

#include "maya/MString.h"
#include "maya/MGlobal.h"

namespace IECoreMaya
{

/// The FromMayaMeshConverter converts types compatible with
/// MFnMesh into IECore::MeshPrimitive objects.
/// \todo Vertex color support. Blind data support?
/// \todo The "space" parameter should probably go in a base class from which all geometry 
/// converters inherit
/// \bug This reverses the winding order unecessarily.
/// \todo Fix the reversing of winding order for the next major release - not we may be relying on
/// the reversed order in external code at the moment.
class FromMayaMeshConverter : public FromMayaObjectConverter
{

	public :
	
		typedef enum
		{
			Transform = 0,
			PreTransform = 1,
			PostTransform = 2,
			World = 3,
			Object = 4
		} Space;

		FromMayaMeshConverter( const MObject &object );
		
		virtual ~FromMayaMeshConverter();
		
		
		/// Returns just the points for the mesh.
		IECore::V3fVectorDataPtr points() const;
		/// Returns just the normals for the mesh.
		IECore::V3fVectorDataPtr normals() const;
		/// Returns just the s for the mesh.
		IECore::FloatVectorDataPtr s( const MString &uvSet="" ) const;
		/// Returns just the t for the mesh.
		IECore::FloatVectorDataPtr t( const MString &uvSet="" ) const;
		/// Creates primitive variables for the specified primitive using
		/// attributes whose names begin with prefix.
		/// \todo We should probably have a FromMayaPrimitiveConverter base
		/// class to put this functionality in - it can then be shared by
		/// a future FromMayaNurbsConverter etc.
		void addPrimVars( IECore::PrimitivePtr primitive, const MString &prefix ) const;
		
		IECore::IntParameterPtr spaceParameter();
		IECore::ConstIntParameterPtr spaceParameter() const;		
		
		/// Retrieves the space that the converter's parameters specify
		MSpace::Space space() const;
		
	protected :
	
		virtual IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;

		IECore::StringParameterPtr m_interpolation;
		IECore::BoolParameterPtr m_points;
		IECore::BoolParameterPtr m_normals;
		IECore::BoolParameterPtr m_st;
		IECore::BoolParameterPtr m_extraST;
		IECore::StringParameterPtr m_primVarAttrPrefix;
		
	private :

		IECore::FloatVectorDataPtr sOrT( const MString &uvSet, unsigned int index ) const;

		static FromMayaObjectConverterDescription<FromMayaMeshConverter> m_description;
		
	public :
	
		struct ExtraData;

};

IE_CORE_DECLAREPTR( FromMayaMeshConverter );

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYAMESHCONVERTER_H
