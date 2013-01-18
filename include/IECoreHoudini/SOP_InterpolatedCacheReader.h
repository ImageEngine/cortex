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

#ifndef IECOREHOUDINI_SOPINTERPOLATEDCACHEREADER_H
#define IECOREHOUDINI_SOPINTERPOLATEDCACHEREADER_H

#include "SOP/SOP_Node.h"
#include "PRM/PRM_Name.h"

#include "IECore/InterpolatedCache.h"
#include "IECore/TransformationMatrix.h"

namespace IECoreHoudini
{

/// SOP for applying an IECore::InterpolatedCache to the incoming Houdini geometry. The GB_PointGroups
/// found on the incoming GU_Detail will be combined with the Object Prefix/Suffix parameters to
/// form the IECore::InterpolatedCache::ObjectHandles. If IECore::InterpolatedCache::AttributeHandles
/// exist for an ObjectHandle, they will be added to the GU_Detail as a GA_Attribute and the values will
/// be transfered for the GA_Range definied by the GA_PointGroup. The GA_Attribute name will be the
/// difference between the AttributeHandle and the Attribute Prefix/Suffix parameters. If transformAttribute
/// is specified, and the associated data is a TransformationMatrix, it will be used to transform the GA_Range.
class SOP_InterpolatedCacheReader : public SOP_Node
{
	public :

		SOP_InterpolatedCacheReader( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~SOP_InterpolatedCacheReader();

		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );
		static PRM_Template parameters[];
		
		enum GroupingMode
		{
			PrimitiveGroup,
			PointGroup,
		};
		
		static PRM_ChoiceList interpolationList;
		static PRM_ChoiceList groupingModeList;
	
	protected :
	
		virtual OP_ERROR cookMySop( OP_Context &context );

	private :
		
		IECore::InterpolatedCachePtr m_cache;
		IECore::InterpolatedCache::Interpolation m_interpolation;
		int m_samplesPerFrame;
		std::string m_cacheFileName;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_SOPINTERPOLATEDCACHEREADER_H
