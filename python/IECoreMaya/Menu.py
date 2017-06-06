##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

import maya.cmds
import maya.mel

import IECore

from UIElement import UIElement

## A class for making maya menus from an IECore.MenuDefinition. The menu is built dynamically when it's
# displayed, so the definition can be edited at any time to change the menu. 
class Menu( UIElement ) :

	# Creates a menu defined by the specified definition. parent may be a
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
	# button :
	# specifies the mouse button which may be used to raise a popup menu, in the same format as
	# expected by the maya.cmds.popupMenu() command.
	#
	# replaceExistingMenu :
	# determines whether we add the menu as a submenu, or overwrite the contents of the existing menu
	# (if the parent is a menu)
	def __init__( self, definition, parent, label="", insertAfter=None, radialPosition=None, button = 3, replaceExistingMenu = False ) :

		menu = None
		if maya.cmds.window( parent, query=True, exists=True ) or maya.cmds.menuBarLayout( parent, query=True, exists=True ) :
			# parent is a window - we're sticking it in the menubar
			menu = maya.cmds.menu( label=label, parent=parent, allowOptionBoxes=True, tearOff=True )
		elif maya.cmds.menu( parent, query=True, exists=True ) :
			if replaceExistingMenu:
				# parent is a menu - we're appending to it:
				menu = parent
				self.__postMenu( menu, definition )
			else:
				# parent is a menu - we're adding a submenu
				kw = {}
				if not (insertAfter is None) :
					kw["insertAfter"] = insertAfter
				if radialPosition :
					kw["radialPosition"] = radialPosition
				menu = maya.cmds.menuItem( label=label, parent=parent, tearOff=True, subMenu=True, allowOptionBoxes=True, **kw )
		else :
			# assume parent is a control which can accept a popup menu
			menu = maya.cmds.popupMenu( parent=parent, button=button, allowOptionBoxes=True )

		maya.cmds.menu( menu, edit=True, postMenuCommand = IECore.curry( self.__postMenu, menu, definition ) )
		
		UIElement.__init__( self, menu )
	
	def __wrapCallback( self, cb ) :
		
		if ( callable( cb ) ) :
			return self._createCallback( cb )
		else :
			# presumably a command in string form
			return cb

	def __postMenu( self, parent, definition, *args ) :

		if callable( definition ) :
			definition = definition()

		maya.cmds.menu( parent, edit=True, deleteAllItems=True )

		done = set()
		for path, item in definition.items() :

			pathComponents = path.strip( "/" ).split( "/" )
			name = pathComponents[0]

			if len( pathComponents ) > 1 :
				# a submenu
				if not name in done :
					subMenu = maya.cmds.menuItem( label=name, subMenu=True, allowOptionBoxes=True, parent=parent, tearOff=True )
					subMenuDefinition = definition.reRooted( "/" + name + "/" )
					maya.cmds.menu( subMenu, edit=True, postMenuCommand=IECore.curry( self.__postMenu, subMenu, subMenuDefinition ) )
					done.add( name )
			else :
			
				kw = {}
				
				if getattr( item, "bold", False ) :
					kw["boldFont"] = True
					
				if getattr( item, "italic", False ) :
					kw["italicized"] = True
					
				if item.divider :

					menuItem = maya.cmds.menuItem( parent=parent, divider=True )

				elif item.subMenu :

					subMenu = maya.cmds.menuItem( label=name, subMenu=True, allowOptionBoxes=True, parent=parent, **kw )
					maya.cmds.menu( subMenu, edit=True, postMenuCommand=IECore.curry( self.__postMenu, subMenu, item.subMenu ) )

				else :

					active = item.active
					if callable( active ) :
						active = active()
					
					checked = item.checkBox
					if callable( checked ) :
						checked = checked()
						kw["checkBox"] = checked
					
					menuItem = maya.cmds.menuItem( label=name, parent=parent, enable=active, annotation=item.description, **kw )
					if item.command :
						maya.cmds.menuItem( menuItem, edit=True, command=self.__wrapCallback( item.command ) )
					if item.secondaryCommand :
						optionBox = maya.cmds.menuItem( optionBox=True, enable=active, command=self.__wrapCallback( item.secondaryCommand ), parent=parent )
