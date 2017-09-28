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

#ifndef IECOREMAYA_PARAMETERMANIPCONTAINER_H
#define IECOREMAYA_PARAMETERMANIPCONTAINER_H

#include <maya/MPxManipContainer.h>
#include <maya/MPlug.h>
#include <maya/MString.h>

namespace IECoreMaya
{

/// This class should be used as a base class for any custom
/// manipulators designed to operate on IECore::Parameters.
/// When used in conjunction with the ParameterisedHolderManipContext,
/// it ensures that the manipulator knows which MPlug it is meant
/// to target, and any Parameter defined labeling is also
/// transferred.
class ParameterManipContainer : public MPxManipContainer
{
	public:

		ParameterManipContainer();

		/// Called by ParameterisedHolderManipContext right before
		/// connectToDependNode(), to set the MPlug that the user is
		/// wishing to manipulate.
		void setPlug( MPlug &plug );

		/// \return The MPlug that the manipulator is currently set
		/// to operate on.
		MPlug getPlug();

		/// Called by ParameterisedHolderManipContext right before
		/// connectToDependNode() if a custom manipulator label has
		/// been defined in the Parameters userData().
		void setLabel( MString &label );

		/// \return The custom label for the manipulator, if one has
		/// been set.
		MString getLabel();

	protected:

		/// To be used in derived classes to determine which plug
		/// the user is interested in manipulating.
		MPlug m_plug;

		/// This label may be set by certain parameters, it is up to
		/// the derived classes to display it, or not...
		MString m_label;
};

}

#endif // IECOREMAYA_PARAMETERMANIPCONTAINER_H
