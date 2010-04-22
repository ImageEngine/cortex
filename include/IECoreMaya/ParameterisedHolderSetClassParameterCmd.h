//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_PARAMETERISEDHOLDERSETCLASSPARAMETERCMD_H
#define IECOREMAYA_PARAMETERISEDHOLDERSETCLASSPARAMETERCMD_H

#include "maya/MPxCommand.h"
#include "maya/MSyntax.h"

#include "IECoreMaya/ParameterisedHolderInterface.h"

namespace IECoreMaya
{

/// This command is used to set the classes held by both the IECore.ClassParameter
/// and IECore.ClassVectorParameter in an undoable way. It should never be used
/// directly - instead the methods of IECoreMaya.FnParameterisedHolder should be
/// used.
class ParameterisedHolderSetClassParameterCmd : public MPxCommand
{

	public :

		ParameterisedHolderSetClassParameterCmd();
		virtual ~ParameterisedHolderSetClassParameterCmd();

		static void *creator();

		virtual bool isUndoable() const;
		virtual bool hasSyntax() const;

		virtual MStatus doIt( const MArgList &argList );
		virtual MStatus undoIt();
		virtual MStatus redoIt();

	private :

		void despatchCallbacks();

		ParameterisedHolderInterface *m_parameterisedHolder;
		IECore::ParameterPtr m_parameter;
		
		IECore::ObjectPtr m_originalValues;
		
		MStringArray m_originalParameterNames;
		MStringArray m_originalClassNames;
		MIntArray m_originalClassVersions;
		MString m_originalSearchPathEnvVar;
		
		MStringArray m_newParameterNames;
		MStringArray m_newClassNames;
		MIntArray m_newClassVersions;
		MString m_newSearchPathEnvVar;
		
};

}

#endif // IECOREMAYA_PARAMETERISEDHOLDERSETCLASSPARAMETERCMD_H
