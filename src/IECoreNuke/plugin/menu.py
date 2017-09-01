##########################################################################
#
#  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

## \addtogroup environmentGroup
#
# IECORENUKE_DISABLE_MENU
# Set this to a value of 1 to disable the creation of the Cortex menu
# in Nuke. You can then use the helper functions in IECoreNuke.Menus
# to build your own site-specific menu structure.

import os

if os.environ.get( "IECORENUKE_DISABLE_MENU", "0" ) != "1" :

	import IECore
	import IECoreNuke
	import nuke

	nodesMenu = nuke.menu( "Nodes" )
	cortexMenu = nodesMenu.addMenu( "Cortex" )

	cortexMenu.addCommand( "Display", "nuke.createNode( 'ieDisplay' )" )
	cortexMenu.addCommand( "LensDistort", "nuke.createNode( 'ieLensDistort' )" )
	cortexMenu.addCommand( "SceneCacheReader", "nuke.createNode( 'ieSceneCacheReader' )" )

	proceduralMenu = cortexMenu.addMenu( "Procedural" )
	IECoreNuke.Menus.addProceduralCreationCommands( proceduralMenu )

	opMenu = cortexMenu.addMenu( "Op" )
	IECoreNuke.Menus.addOpCreationCommands( opMenu )
