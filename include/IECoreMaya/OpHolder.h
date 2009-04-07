//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_OPHOLDER_H
#define IE_COREMAYA_OPHOLDER_H

#include "IECore/Op.h"

#include "IECoreMaya/ParameterisedHolder.h"

namespace IECoreMaya
{

/// \addtogroup environmentgroup
///
/// <b>IECORE_OP_PATHS</b><br>
/// This environment variable is used to find Op classes to be held by the OpHolder
/// node.

/// The OpHolder class presents op parameters as maya attributes, evaluating the result
/// of the Op within compute() when appropriate.
template< typename BaseType >
class OpHolder : public ParameterisedHolder<BaseType>
{

	public:
		
		OpHolder();
		virtual ~OpHolder();
		
		static void *creator();				
		static MStatus initialize();		
		static MTypeId id;
		
		//// \bug See comments in ParameterisedHolder.h
		bool isAbstractClass();
		
		virtual MStatus setDependentsDirty( const MPlug &plug, MPlugArray &plugArray );
		
		virtual MStatus compute( const MPlug &plug, MDataBlock &block );
		
		virtual IECore::RunTimeTypedPtr getParameterised( std::string *className = 0, int *classVersion = 0, std::string *searchPathEnvVar = 0 );
		
		virtual MStatus setOp( const std::string &className, int classVersion  );
		virtual IECore::OpPtr getOp( std::string *className = 0, int *classVersion = 0, std::string *searchPathEnvVar = 0 );	
	
};

typedef OpHolder<MPxNode> OpHolderNode;

}

#endif // IE_COREMAYA_OPHOLDER_H
