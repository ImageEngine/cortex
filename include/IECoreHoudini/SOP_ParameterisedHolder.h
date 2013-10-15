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

#ifndef IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H
#define IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H

#include "SOP/SOP_Node.h"
#include "UT/UT_StringMMPattern.h"

#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"

#include "IECoreHoudini/ParameterisedHolder.h"

namespace IECoreHoudini
{

/// Class representing a SOP node acting as a holder for the abstract Parameterised class.
class SOP_ParameterisedHolder : public ParameterisedHolder<SOP_Node>
{
	public :

		SOP_ParameterisedHolder( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~SOP_ParameterisedHolder();
		
		virtual void getNodeSpecificInfoText( OP_Context &context, OP_NodeInfoParms &parms );
	
	protected :
		
		/// Pushes the geometry data from the incomming connections into the associated Cortex parameters.
		/// This method will cook the incomming nodes. If the input node derives from SOP_ParameterisedHolder,
		/// it's Cortex output will be passed through. If it is a native Houdini node, it will be converted
		/// using the appropriate FromHoudiniGeometryConverter.
		virtual void setInputParameterValues( float now );
		
		/// Used by setInputParameterValues to set the value on each individual input parameter. Providing a
		/// null handle will result in the default behaviour of using the filteredInputValue for this input.
		virtual void setInputParameterValue( IECore::Parameter *parameter, const GU_DetailHandle &handle, unsigned inputIndex );
		
		/// Used to pre-filter the input geometry during setInputParameterValue. It checks for the existance of
		/// a nameFilter parm cooresponding to the input parameter, and uses FromHoudiniGeometryConverter::extract
		/// to limit the input geometry based on that filter.
		GU_DetailHandle filteredInputValue( const IECore::Parameter *parameter, unsigned inputIndex );
		
		/// Used to find and evaluate the nameFilter, if it is enabled.
		bool getNameFilter( const IECore::Parameter *parameter, UT_StringMMPattern &filter );
		
		/// Updates the input connections for parameters relevant to FromHoudiniGeometryConverters
		virtual void refreshInputConnections();

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_SOPPARAMETERISEDHOLDER_H
