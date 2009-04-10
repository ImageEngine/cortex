//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_PARAMETERISEDHOLDERSETVALUECMD_H
#define IECOREMAYA_PARAMETERISEDHOLDERSETVALUECMD_H

#include "maya/MPxCommand.h"
#include "maya/MSyntax.h"

#include "IECoreMaya/ParameterisedHolderInterface.h"

namespace IECoreMaya
{

/// This command is used to implement the IECoreMaya.FnParameterisedHolder
/// setNodeValue and setNodeValues methods in a way which supports undo. It's better
/// to use those methods than call this directly.
class ParameterisedHolderSetValueCmd : public MPxCommand
{

	public :

		ParameterisedHolderSetValueCmd();
		virtual ~ParameterisedHolderSetValueCmd();
		
		static void *creator();
		static MSyntax newSyntax();
		
		virtual bool isUndoable() const;
		
		virtual MStatus doIt( const MArgList &argList );
		virtual MStatus undoIt();
		virtual MStatus redoIt();
	
	private :
	
		ParameterisedHolderInterface *m_parameterisedHolder;
		IECore::ParameterPtr m_parameter; // only set if we're setting a specific parameter rather than all of them
		IECore::ObjectPtr m_originalValue;
		IECore::ObjectPtr m_newValue;
};

}

#endif // IECOREMAYA_PARAMETERISEDHOLDERSETVALUECMD_H
