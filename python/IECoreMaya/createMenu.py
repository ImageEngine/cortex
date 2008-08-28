##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

import IECore
import maya.cmds
import maya.mel

## Creates a maya menu from an IECore.MenuDefinition. The menu is built dynamically when it's
# displayed, so the definition can be edited at any time to change the menu. parent may be a
# window (in which case the menu is added to the menu bar), a menu (in which case a submenu is created)
# or a control (in which case a popup menu is created). The optional keyword arguments operate as follows :
#
# label :
# specifies a label for the submenu (if the parent is a menu) or menubar item (if the parent is a window).
#
# insertAfter :
# specifies the menu item the submenu should be inserted after (if the parent is a menu).
#
# radialPosition :
# specifies the radial position of the submenu (if the parent is a marking menu).
#
# \bug This leaks the MenuDefinition, because maya leaks the postMenuCommand object. This could
# be a problem if the menu definition references methods on objects which are significant in terms
# of memory use - for instance ParameterUI objects (which reference Parameter values). This code
# demonstrates the problem :
#
# import maya.cmds
# import IECore
# import IECoreMaya
# import weakref
#
# w = maya.cmds.window( menuBar=True )
# m = IECore.MenuDefinition()
# mr = weakref.ref( m )
# mi = IECoreMaya.createMenu( m, w, "L" )
# del m
# maya.cmds.showWindow( w )
#
# maya.cmds.deleteUI( w )
# print mr, mr()
#
# You would hope the weakref would no longer be valid, but it is - indicating the leak.
# 
# \todo We might be able to work around this by passing executable strings to the postMenuCommand,
# with keys to some dictionary which stores the MenuDefinitions. Then we can remove the 
# definitions from some uiDeleted scriptjob for the menu. Alternatively Alias could fix the damn
# thing themselves.
def createMenu( definition, parent, label="", insertAfter=None, radialPosition=None ) :	
	
	menu = None
	if maya.cmds.window( parent, query=True, exists=True ) :
		# parent is a window - we're sticking it in the menubar
		menu = maya.cmds.menu( label=label, parent=parent, tearOff=True )
	elif maya.cmds.menu( parent, query=True, exists=True ) :
		# parent is a menu - we're adding a submenu
		kw = {}
		if not (insertAfter is None) :
			kw["insertAfter"] = insertAfter
		if radialPosition :
			kw["radialPosition"] = radialPosition
		menu = maya.cmds.menuItem( label=label, parent=parent, tearOff=True, subMenu=True, **kw )
	else :
		# assume parent is a control which can accept a popup menu 
		menu = maya.cmds.popupMenu( parent=parent )
		
	maya.cmds.menu( menu, edit=True, postMenuCommand = lambda : __postMenu( menu, definition ) )
	return menu
	
# We don't know what the extra argument maya wants to pass to callbacks is, so
# we have to wrap them so we can throw away that argument
def __callbackWrapper( cb, a ) :

	return cb()

def __wrapCallback( cb ) :

	if( callable( cb ) ) :
		return IECore.curry( __callbackWrapper, cb )
	else :
		# presumably a command in string form
		return cb

def __postMenu( parent, definition ) :

	if callable( definition ) :
		definition = definition()

	maya.cmds.menu( parent, edit=True, deleteAllItems=True )

	done = set()
	for path, item in definition.items() :

		pathComponents = path.strip( "/" ).split( "/" )
		name = pathComponents[0]
		label = maya.mel.eval( 'interToUI( "%s" )' % name )
		if len( pathComponents ) > 1 :
			# a submenu
			if not name in done :
				subMenu = maya.cmds.menuItem( label=label, subMenu=True, allowOptionBoxes=True, parent=parent, tearOff=True )
				subMenuDefinition = definition.reRooted( "/" + name + "/" )
				maya.cmds.menu( subMenu, edit=True, postMenuCommand=IECore.curry( __postMenu, subMenu, subMenuDefinition ) )
				done.add( name )
		else :

			if item.divider :

				menuItem = maya.cmds.menuItem( parent=parent, divider=True )

			elif item.subMenu :

				subMenu = maya.cmds.menuItem( label=label, subMenu=True, allowOptionBoxes=True, parent=parent )
				maya.cmds.menu( subMenu, edit=True, postMenuCommand=IECore.curry( __postMenu, subMenu, item.subMenu ) )

			else :

				active = item.active
				if callable( active ) :
					active = active()

				menuItem = maya.cmds.menuItem( label=label, parent=parent, enable=active, annotation=item.description )
				if item.command :
					maya.cmds.menuItem( menuItem, edit=True, command=__wrapCallback( item.command ) )
				if item.secondaryCommand :
					optionBox = maya.cmds.menuItem( optionBox=True, command=__wrapCallback( item.secondaryCommand ), parent=parent )
