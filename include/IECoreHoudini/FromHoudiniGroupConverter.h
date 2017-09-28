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

#ifndef IECOREHOUDINI_FROMHOUDINIGROUPCONVERTER_H
#define IECOREHOUDINI_FROMHOUDINIGROUPCONVERTER_H

#include "IECore/Group.h"

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

namespace IECoreHoudini
{

/// Converter which converts from a Houdini GU_Detail to an IECore::Group containing
/// any number of IECore::Primitives or IECore::Groups
class FromHoudiniGroupConverter : public IECoreHoudini::FromHoudiniGeometryConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromHoudiniGroupConverter, FromHoudiniGroupConverterTypeId, FromHoudiniGeometryConverter );

		FromHoudiniGroupConverter( const GU_DetailHandle &handle );
		FromHoudiniGroupConverter( const SOP_Node *sop );

		virtual ~FromHoudiniGroupConverter();

		/// Determines if the given GU_Detail can be converted
		static FromHoudiniGeometryConverter::Convertability canConvert( const GU_Detail *geo );

		enum GroupingMode
		{
			PrimitiveGroup,
			NameAttribute
		};

	protected :

		/// Re-implemented to perform conversion to an IECore::Group
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;

		/// Uses the factory mechanism to find the best converter for the given GU_Detail
		virtual IECore::ObjectPtr doDetailConversion( const GU_Detail *geo, const IECore::CompoundObject *operands ) const;

	private :

		typedef std::map<unsigned, GA_PrimitiveGroup*> PrimIdGroupMap;
		typedef PrimIdGroupMap::value_type PrimIdGroupPair;
		typedef PrimIdGroupMap::iterator PrimIdGroupMapIterator;

		typedef std::pair<std::string, unsigned> AttributePrimIdPair;
		typedef std::map<AttributePrimIdPair, GA_PrimitiveGroup*> AttributePrimIdGroupMap;
		typedef AttributePrimIdGroupMap::value_type AttributePrimIdGroupPair;
		typedef AttributePrimIdGroupMap::iterator AttributePrimIdGroupMapIterator;

		// add parameters needed by both contructors
		void constructCommon();

		/// Converts the contents of the GA_PrimitiveGroup into a VisibleRenderable
		size_t doGroupConversion( const GU_Detail *geo, GA_PrimitiveGroup *group, IECore::VisibleRenderablePtr &result, const IECore::CompoundObject *operands ) const;

		/// Converts the given GA_PrimitiveGroup to an IECore::Primitive and adds it to the IECore::Group
		void convertAndAddPrimitive( GU_Detail *geo, GA_PrimitiveGroup *group, IECore::GroupPtr &result, const IECore::CompoundObject *operands, const std::string &name = "" ) const;

		/// Regroups a single GA_PrimitiveGroup into several groups, based on GA_PrimitiveTypeId
		/// @param geo The GU_Detail containing the orginal group. New groups will be added based on GA_PrimitiveTypeId
		/// @param groupMap A map from GA_PrimitiveTypeId to the newly created group for that type
		/// @return The number of newly created groups ( groupMap.size() )
		size_t regroup( GU_Detail *geo, PrimIdGroupMap &groupMap ) const;

		/// Used in NameAttribute mode to handle any unnamed or mismatched portions of the src detail.
		void doUnnamedConversion( const GU_Detail *geo, IECore::Group *result, const IECore::CompoundObject *operands, const std::string &name = "" ) const;

		static FromHoudiniGeometryConverter::Description<FromHoudiniGroupConverter> m_description;
};

// register our converter
IE_CORE_DECLAREPTR( FromHoudiniGroupConverter );

}

#endif // IECOREHOUDINI_FROMHOUDINIGROUPCONVERTER_H
