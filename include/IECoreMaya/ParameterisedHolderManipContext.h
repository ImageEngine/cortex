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

#ifndef IECOREMAYA_PARAMETERISEDHOLDERMANIPCONTEXT_H
#define IECOREMAYA_PARAMETERISEDHOLDERMANIPCONTEXT_H

#include <maya/MPxSelectionContext.h>
#include <maya/MEvent.h>
#include <maya/MModelMessage.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MPxManipContainer.h>

#include "IECore/Parameter.h"

namespace IECoreMaya
{

/// This Class provides a generic manipulator context, that allows
/// Parameters on any node derived from IECore::ParameterisedHolder
/// to be manipulated.
///
/// When a node is in the selection, and this context is used as a tool,
/// the selection is recursively walked for supported dependency/DAG nodes.
/// When one is found, its parameters are traversed, and depending on the
/// mode, one or more manipulators are created.
///
/// In order for a parameter to display controls, a suitable manipulator
/// must first be registered. This is done in a way similar to Maya's
/// 'Show Manipulator Tool'. The name of registered manipulator should
/// take the following form:
///
///      ie[<manipulatorTypeHint>]<parameterTypeName>ParameterManipulator
///
/// The optional <em>manipulatorTypeHint</em>, used for specialisation,
/// is read from the StringData member "manipTypeHint" in the the "UI"
/// CompoundObject of the Parameters userData(), if present.
///
/// If the manipulator derives from IECore::ParameterManipulatorContainer
/// then two additional methods are called after creation to specify
/// which parameter the manipulator should target, and wether a label has
/// been specified by the optional StringData member "manipLabel" in
/// the "UI" CompoundObject in the Parameters userData().
///
/// \see ParameterisedManipContainer for more information.
///
/// Because it may not be desirable to have all manipulatable parameters
/// display, you may set parameter->userData()["UI"]["disableManip"] to
/// BoolData( true ), and the context will skip that Parameter.

class ParameterisedHolderManipContext : public MPxSelectionContext
{
	public:

		/// The context supports three modes of operation. These can be
		/// managed by the ParameterisedHolderManipContextCmd
		/// \see ParameterisedHolderManipContextCmd
		enum Mode {
			/// All Parameters with registered manipulators will display.
			All,
			/// The first supported Parameter that is encountered is drawn. (Default)
			First,
			/// The attribute name set in the context is drawn, if present.
			Targeted
		};

		/// Used to set the target plug name for the context when in Targeted mode.
		/// \param plugName The Maya attribute name, without the node prefix. This
		/// should be the name of the Parameter plug itself, rather than any children.
		void setTarget( MString &plugName );
		/// \return The Maya attribute name the context is currently operating on.
		MString getTarget();

		/// Sets the Context's mode to one of the above.
		void setMode( Mode m );
		/// \return The Context's current mode.
		Mode getMode();


		ParameterisedHolderManipContext();

		virtual void toolOnSetup( MEvent &event );
		virtual void toolOffCleanup();

		static void updateManipulators( void *blindData );

	private:

		MCallbackId selectionChangeCallback;

		bool m_toolOn;
		Mode m_mode;
		MString m_targetPlugPath;

		void updateManipulators();

		void dagWalk( MObject &node );
		void processNode( MObject &node );
		MPxManipContainer *createManipulatorWalk( IECore::ParameterPtr parameter, MFnDependencyNode &nodeFn );
		MPxManipContainer *createAndConnectManip( IECore::ParameterPtr parameter, MFnDependencyNode &nodeFn );

		void updateHelpString();
};

}

#endif
