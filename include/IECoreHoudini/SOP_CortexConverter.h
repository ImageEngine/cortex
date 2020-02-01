//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_SOPCORTEXCONVERTER_H
#define IECOREHOUDINI_SOPCORTEXCONVERTER_H

#include "IECoreHoudini/Export.h"

#include "PRM/PRM_Name.h"
#include "SOP/SOP_Node.h"

namespace IECoreHoudini
{

/// SOP class for converting between GU_CortexPrimitives and native Houdini geometry.
class IECOREHOUDINI_API SOP_CortexConverter : public SOP_Node
{
	public :

		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );

		static const char *typeName;

		static PRM_Template parameters[];
		static CH_LocalVariable variables[];

		enum ResultType
		{
			Cortex = 0,
			Houdini
		};

		static PRM_Name pNameFilter;
		static PRM_Name pAttributeFilter;
		static PRM_Name pResultType;
		static PRM_Name pConvertStandardAttributes;

		static PRM_Default convertStandardAttributesDefault;
		static PRM_Default filterDefault;
		static PRM_Default resultTypeDefault;

		static PRM_ChoiceList resultTypeList;

		virtual void getNodeSpecificInfoText( OP_Context &context, OP_NodeInfoParms &parms );

	protected :

		SOP_CortexConverter( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~SOP_CortexConverter();

		virtual OP_ERROR cookMySop( OP_Context &context );

	private :

		void doConvert( const GU_DetailHandle &handle, const std::string &name, ResultType type, const std::string &attributeFilter, bool convertStandardAttributes );
		void doPassThrough( const GU_DetailHandle &handle, const std::string &name );

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_SOPCORTEXCONVERTER_H
