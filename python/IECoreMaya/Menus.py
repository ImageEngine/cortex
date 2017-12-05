##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

import os

import maya.cmds

import IECore
import IECoreMaya

## \addtogroup environmentGroup
#
# IECOREMAYA_DISABLE_MENU
# Set this to a value of 1 to disable the creation of the Cortex menu
# in Maya. You can then use the helper functions in IECoreMaya.Menus
# to build your own site-specific menu structure.

__cortexMenu = None
def createCortexMenu() :

	if os.environ.get( "IECOREMAYA_DISABLE_MENU", "0" ) == "1" :
		return

	m = IECore.MenuDefinition()

	m.append(
		"/Create Op",
		{
			"subMenu" : opCreationMenuDefinition,
		}
	)

	global __cortexMenu
	__cortexMenu = IECoreMaya.Menu( m, "MayaWindow", "Cortex" )._topLevelUI()

def removeCortexMenu() :

	global __cortexMenu
	if __cortexMenu is not None :
		maya.cmds.deleteUI( __cortexMenu )
		__cortexMenu = None

def __createOp( className ) :

	fnOH = IECoreMaya.FnOpHolder.create( os.path.basename( className ), className )
	maya.cmds.select( fnOH.fullPathName() )

def opCreationMenuDefinition() :

	m = IECore.MenuDefinition()
	loader = IECore.ClassLoader.defaultOpLoader()
	for className in loader.classNames() :
		m.append(
			"/" + className,
			{
				"command" : IECore.curry( __createOp, className ),
			}
		)

	return m

