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

import IECore
import IECoreMaya
import maya.OpenMayaUI
import maya.cmds

import os

### The FileDialog class provides an alternative to Maya's maya.cmds.fileDialog().
## It is not a complete drop-in replacement, as, in order to have nice, resizable
## window functionality, it is not modal. Instead, a callback is registered, which
## will be called with the result of the users selection, which will be empty in
## the case of dismissal or cancellation.
##
## If effectively provides an instance of the IECoreMaya.FileBrowser class in a
## window, along with path history, and bookmarking facilities.
##
## \param key (string) This key is used to provide context specific path history
## and bookmarks. It can be None, in which case, the global history/bookmarks are used.
##
## \param callback Should be a callable with the signature f( result ). result will be
## a list of absolute paths to the seleceted items, or an empty list if the dialog was
## dismissed or cancelled.
##
## \prarm title (string) A title for the window.
##
## \param path (string) if specified, this path will be used as the initial path for
## the dialog. If the string "last" is passed, then the last path picked in an instance
## with a matching key will be used. If the argument is omitted, then the current working
## directory is used.
##
## Other kw arguments are passed to the FileBrowser constructor. \see IECoreMaya.FileBrowser
class FileDialog():

	__pathPresets = {}

	def __init__( self,
		key=None,
		callback=None,
		title="Choose a file",
		path=None,
		**kw
	) :

		self.__key = key if key else "__global"

		self.__callback = callback

		self.__window = maya.cmds.window( title=title, mb=True )
		self.__bookmarksMenu = maya.cmds.menu( parent=self.__window, label="Bookmarks", postMenuCommand=self.__buildBookmarksMenu )

		# We need to turn the user closing the window into a 'cancel' event.
		callback = maya.OpenMayaUI.MUiMessage.addUiDeletedCallback( self.__window, self.__cancelled )
		self.__deletionCallback = IECoreMaya.CallbackId( callback )

		self.__browser = IECoreMaya.FileBrowser( self.__window, **kw )

		if path == "last" :
			path = FileDialog.__lastPath( self.__key )

		self.__browser.setPath( path if path else os.getcwd() )

		self.__browser.selectSignal.connect( self.__selected )
		self.__browser.cancelSignal.connect( self.__cancelled )

		maya.cmds.showWindow( self.__window )

	## Can be called to set the path being displayed in the Dialog.
	## \see IECoreMaya.FileBrowser.setPath
	def setPath( self, path, *args ) :
			self.__browser.setPath( path )

	def __selected( self, browser ) :

		selection = browser.getCurrentSelection()

		if selection:
			self.__addToHistory( selection )

		self.__exit( selection )

	def __cancelled( self, *args ) :
		self.__exit( () )

	# Called to close the window if it exists, and run the callback.
	def __exit( self, returnValue ) :

		if self.__deletionCallback:
			del self.__deletionCallback

		if maya.cmds.window( self.__window, exists=True ):
			maya.cmds.evalDeferred( "import maya.cmds; maya.cmds.deleteUI( '%s' )" % self.__window )

		self.__window = None

		self.__callback( returnValue )

	def __addToHistory( self, items ) :

		path = items[0]
		if not os.path.isdir( path ):
			path = os.path.dirname( path )

		## \todo Multiple item persistent history
		maya.cmds.optionVar( sv=( "cortexFileBrowserLastPath_%s" % self.__key, path ) )

	@staticmethod
	def __lastPath( key ) :

		if maya.cmds.optionVar( exists = "cortexFileBrowserLastPath_%s" % key ) :
			return str( maya.cmds.optionVar( query = "cortexFileBrowserLastPath_%s" % key ) )
		else:
			return None


	@staticmethod
	## Register a preset for the 'Bookmarks' menu.
	## \param The name (string) the name of the preset, as it will appear in the menu.
	## \pathOrProc (srting) or <callable> If a string, the path to go to when selected.
	## if a callable, it should return a tuple of ( name, path ) pairs. If the return
	## tuple has more than one item, a submenu will be created.
	## \param key (string) if specified, the preset will only be available for dialogs
	## with that ui key.
	def registerPreset( name, pathOrProc, key=None ) :

		if not key:
			key = "__global"

		if key not in FileDialog.__pathPresets:
			FileDialog.__pathPresets[key] = []

		FileDialog.__pathPresets[key].append( (name, pathOrProc) )

	@staticmethod
	## Removes the named preset with the given key, or global preset if no key is specified.
	def removePreset( name, key=None ):

		if not key :
			key = "__global"

		if key in FileDialog.__pathPresets :
			for p in FileDialog.__pathPresets[key] :
				if p[0] == name :
					FileDialog.__pathPresets[key].remove( p )

	def __buildBookmarksMenu( self ) :

		menu = self.__bookmarksMenu

		## \todo MenuDefinition here? Can we pickle the commands up as easily given the
		## references to self, etc...

		maya.cmds.menu( menu, edit=True, deleteAllItems=True )

		self.__bookmarkMenuItemsForKey( "__global", menu )

		if self.__key != "__global":
			self.__bookmarkMenuItemsForKey( self.__key, menu )

		lastPath = FileDialog.__lastPath( self.__key )
		if lastPath :

			maya.cmds.menuItem( divider=True, parent=menu )
			maya.cmds.menuItem( enable=False, label="Recent", parent=menu )
			maya.cmds.menuItem( label=lastPath, parent=menu, command=IECore.curry( self.setPath, lastPath ) )

		if maya.cmds.menu( menu, numberOfItems=True, query=True ) == 0:
			maya.cmds.menuItem( enable=False, parent=menu, label="No presets or history" )


	def __bookmarkMenuItemsForKey( self, key, menu ) :

		if key in FileDialog.__pathPresets :

			for p in FileDialog.__pathPresets[key] :

				if isinstance( p[1], str ) :

					maya.cmds.menuItem( parent=menu, label=p[0], command=IECore.curry( self.setPath, p[1] ) )

				else:

					items = p[1]()
					if not items:
						continue

					if len(items) == 1:
						maya.cmds.menuItem( parent=menu, label=items[0], command=IECore.curry( self.setPath, items[1] ) )
					else:
						subMenu = maya.cmds.menuItem( parent=menu, label=p[0], subMenu=True )
						for i in items:
							maya.cmds.menuItem( parent=subMenu, label=i[0], command=IECore.curry( self.setPath, i[1] ) )




