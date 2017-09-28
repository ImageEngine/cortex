##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

from __future__ import with_statement

import IECore
import IECoreMaya

import maya.cmds

## The ManipulatorUI functions add a 'Manipulate' menu item to a parameter's
## context menu if a suitable manipulator type is registered.
## Suitable types are nodes registered with the classification:
## 'ieParameterManipulator', and a name that matches the convention
##  ie<manipulatorTypeHint><parameterTypeName>ParameterManipulator
## \see IECoreMaya::ParameterisedHolderManipContext
## \see IECoreMaya::ParameterisedHolderManipContextCommand

__all__ = [ 'manipulateParameter' ]

def __manupulateMenuModifier( menuDefinition, parameter, node, parent=None ) :

	with IECore.IgnoredExceptions( KeyError ) :
		if not parameter.userData()['UI']['disableManip'].value :
			return

	typeHint = ""
	with IECore.IgnoredExceptions( KeyError ) :
		typeHint = parameter.userData()['UI']['manipTypeHint'].value

	parameterManip = "ie%s%sManipulator" % ( typeHint, parameter.staticTypeName() )

	if parameterManip not in maya.cmds.listNodeTypes( 'ieParameterManipulator' ) :
		return

	if len( menuDefinition.items() ):
		menuDefinition.append( "/ManipulateDivider", { "divider" : True } )

	menuDefinition.append(
		"/Manipulate...",
		{
			"command" : IECore.curry( manipulateParameter, node, parameter ),
		}
	)

IECoreMaya.ParameterUI.registerPopupMenuCallback( __manupulateMenuModifier )

## Starts manipulation of the specified node and parameter,.
## \param node MObject or str. A parameterisedHolder node.
## \param parameter IECore.Parameter the parameter to manipulate
## \param contextName An optional context to use, if multiple manipulators
## need controlling simultaneously.
## If there is no manipulator registered for the specified parameter,
## the tool will be activated but no manipulator will show.
def manipulateParameter( node, parameter, contextName="ieParameterManipulatorContext" ) :

	fnPH = IECoreMaya.FnParameterisedHolder( node )
	plugPath = fnPH.parameterPlugPath( parameter )

	if not maya.cmds.contextInfo( contextName, exists=True ) :
		maya.cmds.ieParameterisedHolderManipContext( contextName )

	maya.cmds.ieParameterisedHolderManipContext(
		contextName,
		edit=True,
		mode="targeted",
		targetPlug=plugPath.split(".")[-1]
	)

	maya.cmds.setToolTo( contextName )

