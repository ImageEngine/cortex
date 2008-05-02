//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "maya/MGlobal.h"
#include "maya/MCommandResult.h"
#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"

#include "maya/MVector.h"
#include "maya/MVectorArray.h"
#include "maya/MMatrix.h"
#include "maya/MString.h"

#include "IECore/Data.h"
#include "IECore/Convert.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/StatusException.h"
#include "IECoreMaya/MayaPythonUtil.h"

namespace IECoreMaya
{

/// \todo Add a parameter to echo the command?
/// \todo Put the M*Array->IECore::*VectorData conversion
/// somewhere useful.
IECore::DataPtr executeMel( const std::string &cmd )
{
	MCommandResult result;
	MStatus s = MGlobal::executeCommand( cmd.c_str(), result, false, false );
	
	if (s)
	{
		return IECore::convert<IECore::DataPtr>( result );
	}
	else
	{
		throw StatusException(s);
	}
	
	return 0;
}

}
