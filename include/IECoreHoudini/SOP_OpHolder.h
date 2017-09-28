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

#ifndef IECOREHOUDINI_SOPOPHOLDER_H
#define IECOREHOUDINI_SOPOPHOLDER_H

#include "IECore/Op.h"

#include "IECoreHoudini/SOP_ParameterisedHolder.h"

namespace IECoreHoudini
{

/// SOP class for representing an IECore::Op in Houdini. The held op will operate multiple times
/// over its primary input, splitting by name. Each operation adds a single GU_CortexPrimitive
/// to the output geometry. The held op will operate on any named shaped in the primary input
/// which matches the nameFilter. Non-matching shapes will be passed through without modification.
/// The other inputs will be treated as they normally would by SOP_ParameterisedHolder.
class SOP_OpHolder : public SOP_ParameterisedHolder
{
	public :

		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );

	protected :

		SOP_OpHolder( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~SOP_OpHolder();

		virtual OP_ERROR cookMySop( OP_Context &context );

		/// Overridden for the primary input since that value is used during cook to control
		/// the number of operations. Falls back to default implementation for all other inputs.
		virtual void setInputParameterValue( IECore::Parameter *parameter, const GU_DetailHandle &handle, unsigned inputIndex );

		/// Run the op once all parameters have been set. This may be called several times
		/// when using a nameFilter on the primary input.
		virtual void doOperation( IECore::Op *op, const GU_DetailHandle &handle, const std::string &name );
		/// Pass-through the primary input shapes that do not match the nameFilter.
		virtual void doPassThrough( const GU_DetailHandle &handle, const std::string &name );

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_SOPOPHOLDER_H
