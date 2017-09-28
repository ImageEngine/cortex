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

import IECoreMaya
import maya.cmds

import os
import re, fnmatch

__all__ = [ "FileBrowser" ]

## The Browser class provides a file picker interface within a Maya formLayout.
## User actions cause several signals to be emitted, see the signals section.
## Behvaiour of the dialog can be modified by a variety of creation arguments:
##
## \param uiParent The name of a maya UI element to parent the browser layout
##
## \param options A callable or class with the signature ( <FileBrowser> instance, string uiParent )
## that can be used to draw additional controls in the area below the list, above the
## path box. uiParent is an empty columnLayout.
## If the object passed has a method 'update' it will be connected to the pathChangedSignal
## of the brower instance.
## If the object passed has a method 'selectionChanged' it will be connected to the
## selectionChangedSignal of the browser instance.
##
## \param filter A callable with the signature ( string path, ( {}, ... ) items ), which
## is allowed to modify the items list however it sees fit. See the section below on the
## structure of the items list. The result of the filter will be used as the item list for
## display.
##
## \param validate A callable with the signature ( string path, ( {}, ... ) items ), which
## must return True of False as to wether the items in the supplied list are considered a
## valid 'selection'.
##
## \param showHidden (bool) Wether or not hidden files should be shown in the browser.
##
## \param buttonTitle (string) The label for the main button
##
## \param withCancel (bool) Wether or not a cancel button should be drawn
##
## \param cancelButtonTitle (string) The label for the cancel button.
##
## \param rightHanded (bool) The main button defaults to the left, to match Maya's
## look. If you prefer the other side, to match other environments, this can be set
## to True.
##
## \param allowMultiSelection (bool) Can the user select more than one item at once.
##
## \param saveMode (bool) When enabled, the user is allowed to choose paths to files
## that don't exist yet. Otherwise, the selection is always conformed to an item in the
## list, or the current directory itself if nothing is selected.
##
## The item list syntax:
##
## ( {
##		"name" : (string) the name of the item as it will be displayed in the list,
##		"path" : (string) the full path to the item, that will be returned when querying the users selection
##	   	"mode" : as per os.stat()
##		"uid" : as per os.stat()
##		"gid" : as per os.stat()
##		"size" : (bytes) as per os.stat()
##		"atime" : (seconds) as per os.stat()
##		"mtime" : (seconds) as per os.stat()
##		"ctime" : (seconds) as per os.stat()
##	}, ... )
##
## NOTE: Neither 'path', nor 'name' must be valid filesystem entries. They can be modified to substitute
## variables or similar into the paths or item names. Validation and filtering functions are called
## with the 'real' working path of the browser, as well as the item list. It is up to these functions
## to valiate the 'correctness' of the result of the users selection.
## If a filter is modifying the items list, it is it's responsibility to ensure that the the relevant metadata
## is created for any 'synthesized' items. For example, when collapsing a file sequence into a single item,
## appropriate dates should be generated to permit 'by date' sorting.

class FileBrowser( IECoreMaya.UIElement ) :


	def __init__(
		self, uiParent=None,
		options=None, filter=None, validate=None,  showHidden=False,
		buttonTitle="Select", withCancel=True, cancelButtonTitle="Cancel",
		allowMultiSelection=False, saveMode=False, rightHanded=False,
	):

		self.__path = None

		# For consistency, we're not exposing these are attributes
		self.__filter = filter
		self.__validate = validate
		self.__showHidden = showHidden
		self.__saveMode = saveMode

		self.__s_select = _Signal()
		self.__s_cancel = _Signal()
		self.__s_pathChanged = _Signal()

		if not uiParent:
			uiParent = maya.cmds.setParent( q=True )

		self.__layout = maya.cmds.formLayout( parent=uiParent )

		IECoreMaya.UIElement.__init__( self, self.__layout )

		self.__itemList = _FileList( self.__layout, allowMultiSelection=allowMultiSelection )
		self.__pathField = _PathField( self.__layout, height=25 )

		listUI = self.__itemList._topLevelUI()
		pathUI = self.__pathField._topLevelUI()

		maya.cmds.formLayout(
			self.__layout, edit=True,
			attachForm = (
				( listUI, "left", 0 ), ( listUI, "top", 0 ), ( listUI, "right", 0 ),
				( pathUI, "left", 0 ), ( pathUI, "right", 0 ),
			),
			attachControl = (
				( listUI, "bottom", 2, pathUI ),
			)
		)

		self.__selectButton = maya.cmds.button(
			label = buttonTitle,
			command=self.__emitSelect,
			parent = self.__layout,
			width=200, height=30,
		)

		edge = "right" if rightHanded else "left"

		maya.cmds.formLayout(
			self.__layout, edit=True,
			attachForm = (
				( self.__selectButton, edge, 4 ), ( self.__selectButton, "bottom", 4 )
			),
			attachControl = (
				( pathUI, "bottom", 0, self.__selectButton ),
			)
		)

		if withCancel:

			edge = "left" if rightHanded else "right"

			self.__cancelButton = maya.cmds.button(
				label = cancelButtonTitle,
				command=self.__emitCancel,
				width=200, height=30,
				parent = self.__layout,
			)

			maya.cmds.formLayout(
				self.__layout, edit=True,
				attachForm = (
					( self.__cancelButton, edge, 4 ), ( self.__cancelButton, "bottom", 4 )
				),
			)

		else:

			self.__cancelButton = None

		if options :

			optionsColumn = maya.cmds.columnLayout( adj=True, parent=self.__layout )
			self.__optionsProcInstance = options( self, optionsColumn )

			maya.cmds.separator( style="none", width=5, height=10 )

			if hasattr( self.__optionsProcInstance, "update" ):
				self.pathChangedSignal.connect( self.__optionsProcInstance.update )

			if hasattr( self.__optionsProcInstance, "selectionChanged" ):
				self.__itemList.selectionChangedSignal.connect( self.__optionsProcInstance.selectionChanged )

			maya.cmds.formLayout(
				self.__layout, edit=True,
				attachForm = (
					( optionsColumn, "left", 15 ), ( optionsColumn, "right", 15 ),
				),
				attachControl = (
					( optionsColumn, "bottom", 2, self.__selectButton ),
					( pathUI, "bottom", 6, optionsColumn ),
				)
			)

		# Listen for changes the user makes to the path field. True allows immediate entry
		# in applicable modes. For 'type a name and hit enter to save'.
		self.__pathField.valueChangedSignal.connect( lambda p: self.setPath( p, True ) )
		# When the main working path changes, we need to update the items in that path
		self.pathChangedSignal.connect( self.__getItemsForPath )
		# When the selection in the list changes, we need to update the path/validate
		self.__itemList.selectionChangedSignal.connect( self.__selectionChanged )
		# Handle the double-click situation
		self.__itemList.itemChosenSignal.connect( self.__itemChosen )

	## Sets the working path for the file browser.
	## \param path <string> If this is the path to a file, then the dialog will display
	## the parent directory and select the file. The pathChangedSignal is then emitted
	## with the path to the directory. If None is passed, the listing of the current path
	## will be refreshed.
	## \param allowImmediateSelection. When enabled, this allows a valid file name to be
	## set as the path, and immediately validated and used. It's off by default to allow
	## the dialogue opener to use the setPath method to set  default name without thinking
	## about it too much.
	def setPath( self, path=None, allowImmediateSelection=False ) :

		if not path:
			path = self.__path

		path = os.path.expandvars( path )

		item = ""
		if not os.path.isdir( path ):
			item = os.path.basename( path )
			path = os.path.dirname( path )

		if not os.access( path, os.R_OK ) or not os.path.isdir( path ) :
			if not self.__path:
				path = os.getcwd()
			else:
				maya.cmds.evalDeferred( "import maya.cmds; maya.cmds.confirmDialog( b='OK', title='Error navigating...', message='Unable to read directory:\\n%s' )" % path )
				return

		self.__path = os.path.normpath(path)
		self.__emitPathChanged()

		if item:

			self.__itemList.setSelection( item )

			# If we in file creation mode (save), then we allow the user to enter
			# a full path to a file, if this validates, we then potentially immediately
			# choose that file. This is, essentially, to allow you to 'type a name and hit enter'.
			if self.__saveMode:

				userItem = {
					"name" : item,
					"path" : "%s/%s" % ( self.__path, item )
				}

				itemList = (userItem,)
				self.__selectionChanged( itemList )

				if allowImmediateSelection:
					if self._validate( itemList ) :
						self.__itemChosen( userItem )

	## This function should be used to query the users selection in clases using an
	## instance of the FileDialog.
	## \return <tuple> This returns a list of the items selected in the file browser.
	## Each item is specified by its full path. If items names have been modified by
	## filtering, the full path is returned with the filtered name instead of the
	## file name. If validation is setup, and fails, () is returned.
	def getCurrentSelection( self ) :

		items = self.__getSelectedItems()

		if not self._validate( items ):
			return ()

		return [ i["path"] for i in items ]

	# See whats on disk...
	def __getItemsForPath( self, path, *args ) :

		try:
			fullDirContents = os.listdir( self.__path )
		except Exception, e :
			print e
			maya.cmds.evalDeferred( 'import maya.cmds; maya.cmds.confirmDialog( b="OK", title="Error retrieving file list...", message="%s" )' % e )
			return

		# We'll do basic hidden item filtering here
		# to make life simpler in most common cases
		items = []
		for f in fullDirContents:

			if f[0] == "." and not self.__showHidden:
				continue

			items.append( self.__getItemInfo( self.__path, f ) )

		# If a filter is registered, it can mess with the list
		# and corresponding fileInfo as much as it likes. but it must
		# also update fileInfo accordinly with some meaningfull data.
		if self.__filter:
			self.__filter( path, items )

		self.__itemList.setItems( items )

	# Populate the info entry for a specific file. This is called pre-filtering
	# so item should always correspond to an actual file system entry.
	# The 'path' field is what is actually returned by getCurrentSelection()
	def __getItemInfo( self, path, item ) :

		info = {}
		info["name"] = item
		info["path"] = os.path.normpath( "%s/%s" % ( path, item ) )

		stat = os.stat( info["path"] )
		info["mode"] = stat[0]
		info["uid"] = stat[4]
		info["gid"] = stat[5]
		info["size"] = stat[6]
		info["atime"] = stat[7]
		info["mtime"] = stat[8]
		info["ctime"] = stat[9]

		return info

	# This function returns an unvalidated list of items considered 'selected', along with
	# their info. This is either the actual selection in the item list, or the current contents
	# of the path field, if this has been modified by the user since the list was read.
	def __getSelectedItems( self ) :

		selection = self.__itemList.getSelection()

		requestedPath = self.__pathField.value
		requestedFile = os.path.basename( requestedPath )

		items = []

		# This allows 'save' behavoiur, where they might type a
		# file name onto the current path. We may want to make it
		# so that this behaviour is enabled only when saveMode is True
		if ( requestedPath != self.__path and requestedFile not in selection
				and requestedFile != "<multiple items>" ):

			# The user had modified the path by hand
			userItem = {
				"name" : requestedFile,
				"path" : "%s/%s" % ( self.__path, requestedFile ),
			}
			items.append( userItem )

		elif not selection:

			# The current path can be considered a selection in the case of
			# directory picking.
			cwdItem = {
				"name" : os.path.basename( self.__path ),
				"path" : self.__path,
			}
			items.append( cwdItem )

		else :

			items.extend( selection )

		return items

	def __selectionChanged( self, items ) :

		# Update the path field to reflect the changes in the selection
		if not items:
			val = self.__path
		elif len(items) == 1 :
			val = ( "%s/%s" % ( self.__path, items[0]["name"] ) ).replace( "//", "/" )
		else :
			val = ( "%s/<multiple items>" % self.__path ).replace( "//", "/" )

		self.__pathField.setValue( val, False )

		itemsOk = self._validate( items )
		# If were in saveMode, we don't want to disable the button, otherwise
		# they probably won't be able to press 'save' when validation is active.
		if not self.__saveMode:
			maya.cmds.button( self.__selectButton, edit=True, enable=itemsOk )

	# Called to enforce selection of a particulat item
	def __itemChosen( self, item ) :

		path = "%s/%s" % ( self.__path, item["name"] )

		# If its a directory, we don't want to navigate, rather than
		# 'choose' the item.
		if os.path.isdir( path ) :

			path = os.path.normpath(path).replace( "//", "/" )
			self.setPath( path )

		else:

			self.__emitSelect( items=(item,) )

	def _validate( self, items ):

		if not self.__path:
			return False

		if self.__validate:
			return self.__validate( self.__path, items )

		return True

	##! \name Signals
	## These signals will be emitted in response to user actions. Classes using the FileDialog
	## should connect to these signals in order to act upon the user's selection.
	##! {

	@property
	## This will be called when the user has selected one or more items. By either:
	##  - Making a selection in the file list, and clicking the main button.
	##  - Double clicking on a file (directories cause navigation).
	## Connected callables will receive the following args:
	##  - browserInstance <IECore.FileDialog.Browser>
	## If validation has been setup, and the current selection fails validation,
	## the signal will not be emitted.
	def selectSignal( self ):
		return self.__s_select

	@property
	## This will be emitted if the user clicks the Cancel button. Connected callables
	## will receive the following args:
	##  - browserInstance <IECore.FileDialog.Browser>
	def cancelSignal( self ):
		return self.__s_cancel

	@property
	## This will be emitted whenever the current directory has changed as a result
	## of the user browsing around the file system. Connected callables will be called
	## with the following args:
	##  - path <string>
	def pathChangedSignal( self ):
		return self.__s_pathChanged

	##! }

	def __emitSelect( self, *args, **kw ) :

		if "items" in kw:
			items = kw["items"]
		else:
			items = self.__getSelectedItems()

		if self._validate( items ):
			self.__s_select( self )

	def __emitCancel( self, *args ) :
		self.__s_cancel( self )

	def __emitPathChanged( self, *args ) :
		self.__s_pathChanged( self.__path )

	## Sets the title of the main button
	def setButtonTitle( self, title ):

		maya.cmds.button( self.__selectButton, edit=True, label=title )

	## \return the title of the main button.
	def getButtonTitle( self, title ):

		maya.cmds.button( self.__selectButton, query=True, label=True )

	## Sets the title of the Cancel button, if one exists
	def setCancelButtonTitle( self, title ):

		if self.__cancelButton:
			maya.cmds.button( self.__cancelButton, edit=True, label=title )

	## \return the title of the cancel button, else None if none exists.
	def getCancelButtonTitle( self, title ):

		if self.__cancelButton:
			return maya.cmds.button( self.__cancelButton, query=True, label=True )
		else:
			return None

	##! \name Filters
	## Filters and Validation for common customisations of the Browser.
	##! {
	## The FileExtensionFilter allows only certain extensions to be picked or
	## displayed. Register the appropriate method(s) depending on the desired
	## behaviour
	class FileExtensionFilter():

		### \param extentions One or more extensions to allow, testing is case insensitive.
		def __init__( self, extensions ) :

			if not isinstance( extensions, list ) and not isinstance( extensions, tuple ):
				extensions = ( extensions, )

			self.__exts = []

			for e in extensions:
				self.__exts.append( ( ".%s" % e.lower(), len(e) ) )

		def filter( self, path, items ) :

			if not self.__exts:
				return

			# Removal during the for loop fails, and re-assigning
			# to 'items' fails as it just reassigns the pointer.
			allItems = list(items)
			del items[:]

			for i in allItems:
				if os.path.isdir( "%s/%s" % ( path, i["name"] ) ) :
					items.append( i )
				else:
					if self.__check( i["name"] ) :
						items.append( i )

		def validate( self, path, items ) :

			if not items:
				return False

			for i in items:
				if os.path.isdir( "%s/%s" % ( path, i["name"] ) ) :
					return False
				elif not self.__check( i["name"] ):
					return False

			return True

		def __check( self, itemName ) :

			if not self.__exts:
				return True

			item = itemName.lower()

			for e in self.__exts:
				if (".%s" % item[-e[1]:]) == e[0] :
					return True

			return False

	## A simple filter that only shows or validates directories.
	class DirectoriesOnlyFilter():

		def filter( self, path, items ) :

			allitems = list(items)
			del items[:]

			for i in allitems:
				if os.path.isdir( "%s/%s" % ( path, i["name"] ) ) :
					items.append( i )

		def validate( self, path, items ) :

			if not items:
				return False

			for i in items:
				if not os.path.isdir( "%s/%s" % ( path, i["name"] ) ) :
					return False

			return True

	## A filter that matches a pattern to the filenames
	class FnMatchFilter() :

		def __init__( self, pattern ) :

			self.__reobj = re.compile( fnmatch.translate( pattern ) )

		def filter( self, path, items ) :

			items[:] = [ i for i in items if self.__reobj.match( i["path"] ) or os.path.isdir( i["path"] ) ]

		def validate( self, path, items ) :

			if not items :
				return False

			for i in items:
				if not self.__reobj.match( i["path"] ) :
					return False

			return True
	##! }


# A basic signal mechanism to allow arbitrary connections
# between objects.
class _Signal() :

	def __init__( self ) :
		self.__slots = []

	def __call__( self, *args, **kw ) :
		for c in self.__slots :
			c( *args, **kw )

	def connect( self, callable ) :
		self.__slots.append( callable )

	def disconnect( self, callable ) :
		self.__slots.remove( callable )


class _PathField( object, IECoreMaya.UIElement ) :

	def __init__( self, uiParent=None, **kw ) :

		if not uiParent:
			uiParent = maya.cmds.setParent( q=True )

		self.__layout = maya.cmds.formLayout( parent = uiParent )
		IECoreMaya.UIElement.__init__( self, self.__layout )

		self.__upButton = maya.cmds.button( label="Up", parent=self.__layout, command=self.up, width=50, height=30 )
		self.__field = maya.cmds.textField( changeCommand=self.__emitValueChanged, parent=self.__layout, **kw )

		maya.cmds.formLayout(
			self.__layout, edit=True,
			attachForm = (
				( self.__upButton, "left", 0 ), ( self.__upButton, "top", 0 ),
				( self.__field, "left", 54 ), ( self.__field, "right", 0 ), ( self.__field, "top", 0 ),
			),
		)

		self.__s_valueChanged = _Signal()

	def up( self, *args ) :

		path = self.value
		if not os.path.isdir( path ):
			path = os.path.dirname( path )
		path = os.path.dirname( path )

		self.setValue( path, True )

	def getValue( self ):
		return str( maya.cmds.textField( self.__field, query=True, text=True ) )

	def setValue( self, value, emit=True ):
		maya.cmds.textField( self.__field, edit=True, text=value )
		if emit:
			self.__emitValueChanged()

	value = property( getValue, setValue )

	@property
	def valueChangedSignal( self ):
		return self.__s_valueChanged

	def __emitValueChanged( self, *args ) :
		self.__s_valueChanged( self.value )


class _FileList( object, IECoreMaya.UIElement ) :

	def __init__( self, uiParent=None, **kw ) :

		if not uiParent:
			uiParent = maya.cmds.setParent( q=True )

		self.__layout = maya.cmds.formLayout( parent=uiParent )
		IECoreMaya.UIElement.__init__( self, self.__layout )

		self.__sort = _DefaultFileListSort( self.__layout )
		sortUI = self.__sort._topLevelUI()

		self.__list = maya.cmds.textScrollList(
			parent = self.__layout,
			selectCommand=self.__emitSelectionChanged,
			doubleClickCommand=self.__emitItemChosen,
			**kw
		)

		maya.cmds.formLayout(
			self.__layout, edit=True,
			attachForm = (
				( sortUI, "left", 0 ), ( sortUI, "top", 0 ), ( sortUI, "right", 0 ),
				( self.__list, "left", 0 ), ( self.__list, "right", 0 ), ( self.__list, "bottom", 0 ),
			),
			attachControl = (
				( self.__list, "top", 0, sortUI ),
			)
		)

		self.__items = []

		self.__s_selectionChanged = _Signal()
		self.__s_itemsChanged = _Signal()
		self.__s_itemChosen = _Signal()

		self.itemsChangedSignal.connect( self.sortItems )
		self.__sort.termsChangedSignal.connect( self.sortItems )

	def setSelection( self, items, emit=True ):

		if not items:
			maya.cmds.textScrollList( self.__list, edit=True, deselectAll=True )
			if emit:
				self.__emitSelectionChanged()
			return

		if not ( isinstance( items, list ) or isinstance( items, tuple ) ):
			items = list((items,))

		for i in items:
			if self.hasItem( i ):
				maya.cmds.textScrollList( self.__list, edit=True, selectItem=self.itemName(i) )

		if emit:
				self.__emitSelectionChanged()

	def getSelection( self ) :

		selected = maya.cmds.textScrollList( self.__list, query=True, si=True )
		if not selected :
			return ()

		items = []
		for s in selected:
			items.append( self.getItem(s) )

		return items

	def setItems( self, items, emit=True ) :

		self.__items = items

		# This is emited before we re-populate to allow sorting etc... to
		# take place if need be.
		if emit:
			self.__emitItemsChanged()

		maya.cmds.textScrollList( self.__list, edit=True, removeAll=True )

		for i in self.__items:
			maya.cmds.textScrollList( self.__list, edit=True, append=self.itemName(i) )

		if emit:
			self.__emitSelectionChanged()

	def getItems( self  ) :

		return list( self.__items )

	# \arg item can be the item dictionary, or name
	def hasItem( self, item ) :

		if isinstance( item, dict ) :

			for i in self.__items:
				if i == item :
					return True

		else:

			for i in self.__items:
				if i["name"] == item :
					return True

		return False

	def itemName( self, item ) :

		return item["name"] if isinstance( item, dict ) else item

	def getItem( self, itemName ) :

			for i in self.__items:
				# itemName may be unicode.
				if i["name"] == str(itemName) :
					return i

			return {}


	def sortItems( self, *args ):

		if not self.__sort:
			return

		oldSelection = maya.cmds.textScrollList( self.__list, query=True, si=True )

		self.__sort.sort( self.__items )

		## \todo This effectively calls setItems inside setItems
		self.setItems( self.__items, False )

		if oldSelection:
			for i in oldSelection:
				if str(i) in self.__items:
					maya.cmds.textScrollList( self.__list, edit=True, si=oldSelection )

	@property
	def selectionChangedSignal( self ) :
		return self.__s_selectionChanged

	def __emitSelectionChanged( self, *args ) :

		items = self.getSelection()
		self.__s_selectionChanged( items )

	@property
	def itemsChangedSignal( self ):
		return self.__s_itemsChanged

	def __emitItemsChanged( self, *args ) :
		self.__s_itemsChanged( self.__items )

	@property
	def itemChosenSignal( self ):
		return self.__s_itemChosen

	def __emitItemChosen( self, *args ) :
		selection = maya.cmds.textScrollList( self.__list, query=True, si=True )
		self.__s_itemChosen( self.getItem( selection[0] ) )


class _DefaultFileListSort( object, IECoreMaya.UIElement ) :

	def __init__( self, uiParent=None, **kw ) :

		if not uiParent:
			uiParent = maya.cmds.setParent( q=True )

		self.__layout = maya.cmds.rowLayout( parent = uiParent, nc=3, adj=3, cw3=( 50, 120, 100 ) )
		IECoreMaya.UIElement.__init__( self, self.__layout )

		maya.cmds.text( label="Sort by:" )

		self.__keyMenu = maya.cmds.optionMenu(
			changeCommand = self.__emitTermsChanged,
			parent=self.__layout,
		)

		self.__directionMenu = maya.cmds.optionMenu(
			changeCommand = self.__emitTermsChanged,
			parent=self.__layout,
		)

		maya.cmds.menuItem( label="Name", parent=self.__keyMenu )
		maya.cmds.menuItem( label="Date Modified", parent=self.__keyMenu )

		maya.cmds.menuItem( label="Ascending", parent=self.__directionMenu )
		maya.cmds.menuItem( label="Descending", parent=self.__directionMenu )

		self.__s_termsChanged = _Signal()

	@property
	def termsChangedSignal( self ):
		return self.__s_termsChanged

	def __emitTermsChanged( self, *args ) :
		self.__s_termsChanged()

	def sort( self, items ) :

		key = maya.cmds.optionMenu( self.__keyMenu, query=True, value=True )
		direction = maya.cmds.optionMenu( self.__directionMenu, query=True, value=True )

		if key == "Name" :
			items.sort( key=lambda item: item["name"].lower() )
		else:
			items.sort( key=lambda item: item["mtime"] )

		if direction == "Descending" :
			items.reverse()
