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

import os, re

import maya.cmds

import IECore

from UIElement import UIElement
from FnParameterisedHolder import FnParameterisedHolder
from ClassParameterUI import ClassParameterUI
from ClassVectorParameterUI import ClassVectorParameterUI
from FnTransientParameterisedHolderNode import FnTransientParameterisedHolderNode

__all__ = [ 'PresetsUI', 'SavePresetUI', 'LoadPresetUI' ]

def __savePresetMenuModifierVectorClass( menuDefinition, parent, parameter, node ) :
	__savePresetMenuModifier( menuDefinition, parameter, node, parent=parent )

def __savePresetMenuModifier( menuDefinition, parameter, node, parent=None ) :

	fnPh = FnParameterisedHolder( node )
	plugPath = fnPh.parameterPlugPath( parameter )

	if len( menuDefinition.items() ):
		menuDefinition.append( "/PresetsDivider", { "divider" : True } )

	saveItemName = "/Presets/Save Preset..."
	loadItemName = "/Presets/Load Preset..."

	# If we are actually a class in a vector, use slightly different names
	# so that its more obvious whats going on
	## \todo Add an item to save the class as a preset, rather than its values.
	if parent is not None and (
		 isinstance( parent, IECore.ClassVectorParameter )
		 or isinstance( parent, IECore.ClassParameter )
	):
		saveItemName = "/Presets/Save Parameter Values Preset..."
		loadItemName = "/Presets/Load Parameter Values Preset..."

	menuDefinition.append( saveItemName, { "command" : IECore.curry( maya.cmds.evalDeferred, 'import IECoreMaya; IECoreMaya.SavePresetUI( "%s", "%s" )' % ( fnPh.fullPathName(), plugPath ) ) } )
	menuDefinition.append( loadItemName, { "command" : IECore.curry( maya.cmds.evalDeferred, 'import IECoreMaya; IECoreMaya.LoadPresetUI( "%s", "%s" )' % ( fnPh.fullPathName(), plugPath ) ) } )

ClassParameterUI.registerClassMenuCallback( __savePresetMenuModifier )
ClassVectorParameterUI.registerClassMenuCallback( __savePresetMenuModifierVectorClass )
ClassVectorParameterUI.registerToolsMenuCallback( __savePresetMenuModifier )

### @name Wrapper functions
### These wrappers take only string arguments, to allow the PresetsUI
### To be invoked from a evalDeferred call. This is needed to make sure that
### all the tasks performed internally by the UI undo in one step.
### @{
def SavePresetUI( nodeName, attribute ) :

	fnPh = FnParameterisedHolder( nodeName )
	rootParam = fnPh.plugParameter( attribute )

	PresetsUI( nodeName, rootParam ).save()

def LoadPresetUI( nodeName, attribute ) :

	fnPh = FnParameterisedHolder( nodeName )
	rootParam = fnPh.plugParameter( attribute )

	PresetsUI( nodeName, rootParam ).load()
### @}

### This class provides a UI for loading and saving presets for nodes
### derived from the ParameterisedHolder class. Currently, it creates
### BasicPresets in one of the locations set in the relevant search
### paths for the Parameterised objects. Categories, and titles aren't
### yet implemented.
###
### \todo Currently, the LoadUI, has to instantiate every preset in the
### search path, and call 'applicableTo'. This is potentially a huge
### bottle neck, so, well see what happens when we use it in earnest...
class PresetsUI() :

	def __init__( self, node, rootParameter=None ) :

		try :
			fn = FnParameterisedHolder( node )
		except:
			raise ValueError, 'PresetsUI: "%s" is not a valid Parameterised object.' % node

		self.__node = node
		self.__rootParameter = rootParameter

	### Call to save a preset.
	def save( self ) :
		SaveUI( self.__node, self.__rootParameter )

	### Call to copy a preset.
	## \param callback, f( preset ), A callable, that will be
	## called with the Preset instance after the user has selected
	## a number of prameters
	def copy( self, callback ) :
		CopyUI( self.__node, self.__rootParameter, callback )

	### Call to load a preset.
	def load( self ) :
		LoadUI( self.__node, self.__rootParameter )

	### Call to select parameters within the current rootParameter
	## \param callback, f( node, rootParameter, parameters ), A Callable, that will be
	## called with the node, and chosed parameters after the user has
	## made their selection. This can be usefull for a variety of cases where
	## it's needed for the user to select parameters within a hierarchy.
	def selectParameters( self, callback ) :
		SelectUI( self.__node, self.__rootParameter, callback )


# Private implementation classes

# This is a base class for all the UIs which need to display a list of available parameters
# and obtain a subset which the user is interested in. Thi takes care of drawing a list in
# a form layout. Derived classes can then edit/add to this layout to add additional controls.
#   self._fnP will contain a parameterised holder around the node passed to the constructor.
#   self._rootParamter will contain the rootParameter passed to the constructor.
#   self._form is the main form layout
#   self._scroll is the main scroll layout
#   self._selector is the actual parameter list
class ParamSelectUI( UIElement ) :

	def __init__( self, node, rootParameter=None, buttonTitle="Select", autoCollapseDepth=2 ) :

		self._fnP = FnParameterisedHolder( node )

		parameterised = self._fnP.getParameterised()
		self._rootParameter = rootParameter if rootParameter else parameterised[0].parameters()

		self._window = maya.cmds.window(
			title="%s: %s" % ( buttonTitle, node ),
			width=500,
			height=600
		)

		UIElement.__init__( self, self._window )

		self._form = maya.cmds.formLayout()

		self._scroll = maya.cmds.scrollLayout( parent=self._form )

		self._selector = ParameterSelector( self._rootParameter, self._scroll, autoCollapseDepth=autoCollapseDepth )

		maya.cmds.formLayout( self._form, edit=True,

			attachForm=[	( self._scroll, 			"top",  	0  ),
							( self._scroll, 			"left", 	0  ),
							( self._scroll, 			"right",	0  ),
							( self._scroll, 			"bottom",	0  ), ],
		)

# The SelectUI allows parameter selection, then calls the supplied callback with
# the node, rootParameter, and a list of chosen parameters. The button title can
# be customised with the label argument to the constructor. This may be useful
# outside this file, and can be accessed by the PresetUI.selectParameters() method
# which takes a callback.
class SelectUI( ParamSelectUI ) :

	def __init__( self, node, rootParameter=None, callback=None, label="Select" ) :

		self.__callback = callback
		self.__node = node

		ParamSelectUI.__init__( self, node, rootParameter )

		self.__button = maya.cmds.button(
			l=label,
			parent=self._form,
			height=30,
			c=self._createCallback( self.__doAction )
		)

		maya.cmds.formLayout( self._form, edit=True,

			attachForm=[	( self._scroll, 			"top",  	0  ),
							( self._scroll, 			"left", 	0  ),
							( self._scroll, 			"right",	0  ),
							( self.__button, 			"bottom",	0  ),
							( self.__button, 			"left", 	0  ),
							( self.__button, 			"right",	0  ) ],

			attachControl=[	( self._scroll, 	"bottom", 	0, 	self.__button ),  ],
		)

		maya.cmds.showWindow( self._window )

	def __doAction( self ) :

		parameters = self._selector.getActiveParameters()

		if not parameters :
			maya.cmds.confirmDialog( message="Please select at least one paremeter.", button="OK" )
			return

		maya.cmds.deleteUI( self._window )

		if self.__callback:
			self.__callback( self.__node, self._rootParameter, parameters )

# The CopyUI extends the selector to create a preset from the users selection, and call a callback
# passing that preset.
class CopyUI( SelectUI ) :

	def __init__( self, node, rootParameter=None, callback=None ) :

		self.__callback = callback
		SelectUI.__init__( self, node, rootParameter, callback=self.__copyCallback, label="copy" )

	# The copy callback simply creates a preset, then forwards this to whatever other callback was registered
	def __copyCallback( self, node, rootParameter, parameters ) :

		preset = IECore.BasicPreset( self._fnP.getParameterised()[0], rootParameter, parameters=parameters )
		self.__callback( preset )

# The SaveUI extends the selector to add path selection, as well as description and name fields.
class SaveUI( ParamSelectUI ) :

	def __init__( self, node, rootParameter=None, autoCollapseDepth=2 ) :

		fnP = FnParameterisedHolder( node )
		parameterised = fnP.getParameterised()

		self.__envVar = parameterised[3].replace( "_PATHS", "_PRESET_PATHS" )

		if self.__envVar not in os.environ :
			maya.cmds.confirmDialog( message="Environment variable not set:\n\n$%s\n\nPlease set "%self.__envVar+\
			"this variable to point to one or more paths.\nPresets can then be saved to these "+\
			"locations.", button="OK" )
			return

		ParamSelectUI.__init__( self, node, rootParameter, autoCollapseDepth=autoCollapseDepth )

		self.__location = SearchPathMenu(
			os.getenv( self.__envVar ),
			self._form,
			label = "Save to:",
			ann = self.__envVar,
			cw = ( 1, 65 ),
			adj = 2,
		)

		self.__name = maya.cmds.textFieldGrp(
			parent = self._form,
			label = "Name:",
			adj = 2,
			columnWidth = ( 1, 65 )
		)

		descripLabel = maya.cmds.text(
			parent = self._form,
			label = "Description:",
			align = "left",
		)

		self.__description = maya.cmds.scrollField(
			parent = self._form,
			numberOfLines = 5,
			height = 100,
		)

		self.__saveButton = maya.cmds.button(
			l = "Save",
			parent = self._form,
			height = 30,
			c = self._createCallback( self.__doSave )
		)

		maya.cmds.formLayout( self._form, edit=True,

			attachForm=[	( self._scroll, 			"top",  	0  ),
							( self._scroll, 			"left", 	0  ),
							( self._scroll, 			"right",	0  ),
							( self.__location.menu(),	"left", 	10 ),
							( self.__location.menu(),	"right",	10 ),
							( self.__name,				"left", 	10 ),
							( self.__name,				"right",	10 ),
							( descripLabel,				"left", 	10 ),
							( descripLabel,				"right",	10 ),
							( self.__description,		"left", 	10 ),
							( self.__description,		"right",	10 ),
							( self.__saveButton, 		"bottom",	0  ),
							( self.__saveButton, 		"left", 	0  ),
							( self.__saveButton, 		"right",	0  ) ],

			attachControl=[	( self._scroll, 	 		"bottom",	5,	self.__location.menu() 	),
							( self.__location.menu(),	"bottom",   3,  self.__name  		    ),
							( self.__name,				"bottom",   5,  descripLabel	    ),
							( descripLabel,				"bottom",   5,  self.__description	    ),
							( self.__description,				"bottom",   5,  self.__saveButton	    ), ]
		)


		maya.cmds.showWindow( self._window )

	def __doSave( self ) :

		name = maya.cmds.textFieldGrp( self.__name, query=True, text=True )
		if not name:
			maya.cmds.confirmDialog( message="Please enter a name for the preset.", button="OK" )
			return

		# Sanitise the name a little
		name = name.replace( " ", "_" )
		name = re.sub( '[^a-zA-Z0-9_]*', "", name )
		# We have to also make sure that the name doesnt begin with a number,
		# as it wouldn't be a legal class name in the resulting py stub.
		name = re.sub( '^[0-9]+', "", name )

		description = maya.cmds.scrollField( self.__description, query=True, text=True )

		parameters = self._selector.getActiveParameters()

		if not parameters :
			maya.cmds.confirmDialog( message="Select at least one parameter to save.", button="OK" )
			return

		path = self.__location.getValue()

		self._fnP.setParameterisedValues()

		preset = IECore.BasicPreset(
			self._fnP.getParameterised()[0],
			self._rootParameter,
			parameters = parameters
		)

		preset.save(
			path,
			name,
			description = description,
		)

		maya.cmds.deleteUI( self._window )


class LoadUI( UIElement ) :

	def __init__( self, node, rootParameter=None ) :

		fn = FnParameterisedHolder( node )
		parameterised = fn.getParameterised()

		self.__parameterised = parameterised

		# Just using 'not' on a ClassVector takes its length, which equates to False if its empty.
		self.__rootParameter = rootParameter if rootParameter is not None else parameterised[0].parameters()

		self.__fnP = fn
		self.__envVar = parameterised[3].replace( "_PATHS", "_PRESET_PATHS" )

		if self.__envVar not in os.environ :
			maya.cmds.confirmDialog( message="Environment variable not set:\n\n$%s\n\nPlease set "%self.__envVar+\
			"this variable to point to one or more paths.\nPresets can then be loaded from these "+\
			"locations.", button="OK" )
			return

		paths = os.environ[self.__envVar]
		sp = IECore.SearchPath( os.path.expandvars( paths ) )
		self.__classLoader = IECore.ClassLoader( sp )

		presets = self.__getPresets( parameterised[0], self.__rootParameter )
		if not presets:
			maya.cmds.confirmDialog( message="No presets applicable to %s found in the current search paths ($%s)." % ( self.__rootParameter.name, self.__envVar ), button="OK" )
			return

		self.__loadedPresets = {}

		self.__window = maya.cmds.window( title="Load: %s" % node, width=300, height=500 )

		UIElement.__init__( self, self.__window )

		self.__form = maya.cmds.formLayout()

		self.__infoColumn = PresetInfo( parent=self.__form )
		self.__selector = PresetSelector( presets, self.__form, allowMultiSelection=True, selectCommand=self._createCallback( self.__selectionChanged ) )
		self.__loadButton = maya.cmds.button( l="Load", parent=self.__form, height=30, c=self._createCallback( self.__doLoad ) )

		if not presets:
			maya.cmds.control( self.__loadButton, edit=True, enable=False )

		maya.cmds.formLayout( self.__form, edit=True,

			attachForm=[	( self.__selector.list(), 		"top" ,		0 ),
							( self.__selector.list(),		"left" , 	0 ),
							( self.__selector.list(),		"right" , 	0 ),
							( self.__infoColumn.layout(),	"left" ,   	5 ),
							( self.__infoColumn.layout(),	"right" ,   0 ),
							( self.__loadButton,			"bottom",   0 ),
							( self.__loadButton,			"left" ,	0 ),
							( self.__loadButton,			"right" ,   0 )  ],

			attachControl=[	( self.__selector.list(), 	"bottom", 	4, 	self.__infoColumn.layout() ),
							( self.__infoColumn.layout(), 	"bottom", 	5, 	self.__loadButton ), ]
		)

		maya.cmds.showWindow( self.__window )

	def __selectionChanged( self, *args ) :

		self.__loadedPresets = {}

		classNames = self.__classLoader.classNames()
		selected = [ s for s in self.__selector.selected() if s in classNames ]
		presets = []
		for s in selected:
			self.__loadedPresets[s] = self.__classLoader.load( s )()
			presets.append( self.__loadedPresets[s] )

		self.__infoColumn.setPresets( presets )

	def __doLoad( self ) :

		loaded = self.__loadedPresets.keys()
		selected = [ s for s in self.__selector.selected() if s in loaded ]

		if not selected :
			maya.cmds.confirmDialog( message="Please select at least one preset to load.", button="OK" )
			return

		parameterised = self.__fnP.getParameterised()[0]

		# Make sure the any parameter changes get set back into
		# the parameterised objects for each preset.
		self.__infoColumn.commitParameters()

		# We need to make sure we have the right values in the first place.
		self.__fnP.setParameterisedValues()

		with self.__fnP.parameterModificationContext() :

			for s in selected:
				# These should have been loaded by the selectCommand callback
				self.__loadedPresets[s]( self.__parameterised, self.__rootParameter )

		maya.cmds.deleteUI( self.__window )
		self.__loadedPrestes = {}

	def __getPresets( self, parameterised, parameter ) :

		validPresets = []

		self.__classLoader.refresh()
		presets = self.__classLoader.classNames( "*" )

		for name in presets:
			p = self.__classLoader.load( name )()
			if not isinstance( p, IECore.Preset ):
				continue
			if p.applicableTo( parameterised, parameter ):
				validPresets.append( ( name, p ) )

		return validPresets

# Extracts metadata from a preset, and displays in a layout, complete
# with a UI for any parameters of the preset. Any selected presets
# are temporarily instantiated into a FnTransientParameterisedHolderNode.
class PresetInfo() :

	def __init__( self, parent=None ) :

		oldParent = maya.cmds.setParent( q=True )
		if not parent :
			parent = oldParent

		maya.cmds.setParent( parent )

		self.__parent = parent
		self.__layout = maya.cmds.columnLayout( co=( "both", 5 ), adj=True )

		maya.cmds.setParent( oldParent )

	def layout( self ):
		return self.__layout

	def setPresets( self, presets=() ) :

		children = maya.cmds.columnLayout( self.__layout, q=True, ca=True )
		if children :
			for c in children:
				maya.cmds.deleteUI( c )

		self.__parameterHolders = {}

		for p in presets:

			meta = p.metadata()

			name = meta["title"] if "description" in meta else p.__class__

			maya.cmds.text(
				parent = self.__layout,
				label = name,
				font = "boldLabelFont",
				recomputeSize = True,
				align = "left"
			)

			wrapWidth = ( int(maya.cmds.layout( self.__parent, query=True, width=True )) - 5 ) / 5

			if "description" in meta and meta["description"]:
				descripWrap = IECore.StringUtil.wrap( meta["description"], wrapWidth )
				lines = descripWrap.split( "\n" )
				for l in lines:
					maya.cmds.text( parent=self.__layout, label=l, font="smallPlainLabelFont", align="left" )

			maya.cmds.separator(
				parent = self.__layout,
				width = 5,
				height = 10,
				style = "none",
			)

			if len( p.parameters().keys() ) :
				self.__parameterHolders[ name ] = FnTransientParameterisedHolderNode.create( self.__layout, p )

	# This must be called before querying the parameters of any presets passed to this UI
	# section, in order to update the Parameterised object with any changed made in the UI
	def commitParameters( self ) :

		for s in self.__parameterHolders.keys():
			 self.__parameterHolders[s].setParameterisedValues()
			 del  self.__parameterHolders[s]

# Provides an optionMenu to select from paths in the supplied search path string.
class SearchPathMenu() :

	# *args, **kwargs are passed to maya.cmds.optionMenuGrp on creation.
	def __init__( self, searchPaths, parent=None, *args, **kwargs ) :

		oldParent = maya.cmds.setParent( q=True )
		if not parent :
			parent = oldParent

		maya.cmds.setParent( parent )

		self.__menu = maya.cmds.optionMenuGrp( *args, **kwargs )

		for p in searchPaths.split( ":" ) :
			maya.cmds.menuItem( label = p )

		maya.cmds.setParent( oldParent )

	def setValue( self, value ) :
		maya.cmds.optionMenuGrp( self.__menu, edit=True, value=value )

	def getValue( self ) :
		return maya.cmds.optionMenuGrp( self.__menu, query=True, value=True )

	def menu( self ):
		return self.__menu

# Provides a simple list of the supplied presets
class PresetSelector( UIElement ) :

	# *args, **kwargs are passed to maya.cmds.textScrollList on creation.
	def __init__( self, presets, parent=None, *args, **kwargs ) :

		oldParent = maya.cmds.setParent( q=True )
		if not parent :
			parent = oldParent

		maya.cmds.setParent( parent )

		self.__list = maya.cmds.textScrollList( *args, **kwargs )
		UIElement.__init__( self, self.__list )

		if not presets:

			maya.cmds.textScrollList(
				self.__list,
				edit=True,
				append="No presets found...",
				enable=False
			)

		else :
			presetsByPath = {}
			for ( name, p ) in presets :
				print name, p
				path = os.path.dirname( p._cob ).rpartition( p.typeName() )[0]
				if path not in presetsByPath :
					presetsByPath[path] = []
				presetsByPath[path].append( name )

			for ( path, names ) in presetsByPath.items() :

				maya.cmds.textScrollList( self.__list, edit=True, append=path )

				for name in names :

					maya.cmds.textScrollList( self.__list, edit=True, append=name )

				maya.cmds.textScrollList( self.__list, edit=True, append="" )

		maya.cmds.setParent( oldParent )

	# \return A list of selected names
	def selected( self ) :

		selection = maya.cmds.textScrollList( self.__list, query=True, selectItem=True )
		if not selection:
			return []
		else:
			return selection

	# \return The Maya ELF handle for the list.
	def list( self ) :
		return self.__list

# Provides a maya.cmds.columnLayout containing a hierarchical selection
# interface for the supplied parameter. Each parameter is presented with
# A checkbox to allow selection.
class ParameterSelector( UIElement ) :

	def __init__( self, parameter, parent=None, autoCollapseDepth=2 ) :

		oldParent = maya.cmds.setParent( query=True )

		if not parent :
			parent = oldParent

		self.__mainColumn = maya.cmds.columnLayout( adj=True, parent=parent )

		if isinstance( parameter, IECore.CompoundParameter ) :
			self.__controls = ParameterSelector.ParameterGroup( parameter, autoCollapseDepth=autoCollapseDepth )
		else :
			self.__controls = ParameterSelector.Parameter( parameter )

		maya.cmds.setParent( oldParent )

	# \return A list of the selected parameters.
	def getActiveParameters( self ) :
		return  self.__controls.getActiveParameters()

	# Provides an interface for selecting an individual parameter.
	class Parameter() :

		def __init__( self, parameter, **kw ) :

			self.__depth = kw["depth"] if "depth" in kw else 0

			self.__checkbox = maya.cmds.checkBox( label=parameter.name, align="left", value=True )
			self.__parameter = parameter

		# Sets the active state of the parameter
		def setState( self, state ) :
			maya.cmds.checkBox( self.__checkbox, edit=True, value=state )

		# Returns the active state of the parameter
		def getState( self ) :

			state = maya.cmds.checkBox( self.__checkbox, query=True, value=True )
			if state:
				return True
			else :
				return False

		# \return the IECore Parameter represented by the control.
		def parameter( self ) :
			return self.__parameter

		# \return Either an empty list, or a list with the parameter, depending
		# on its state. The list syntax is used for interchangeability with the
		# ParameterGroup class.
		def getActiveParameters( self ) :
			if self.getState():
				return [ self.__parameter ]
			else:
				return []

	# Provides a hierarchical interface for selecting parameters in a CompoundParameter
	class ParameterGroup( UIElement ) :

		def __init__( self, compoundParameter, **kw ) :

			self.__depth = kw["depth"] if "depth" in kw else 0
			self.__autoCollapseDepth = kw["autoCollapseDepth"] if "autoCollapseDepth" in kw else 2
			self.__parameter = compoundParameter

			self.__row = maya.cmds.rowLayout( numberOfColumns = 2, columnWidth=( 1, 20 ) )

			UIElement.__init__( self, self.__row )

			self.__checkbox = maya.cmds.checkBox( label = "", cc=self._createCallback( self.syncState ), value=True )

			name = compoundParameter.name if compoundParameter.name else "All Parameters"
			if "label" in compoundParameter:
				name = compoundParameter["label"].getTypedValue()

			collapsed = False if self.__depth < self.__autoCollapseDepth else True

			self.__frame = maya.cmds.frameLayout(
				label = name,
				labelIndent = 5,
				marginWidth = 5,
				borderVisible = False,
				collapsable = True,
				collapse = collapsed,
			)

			self.__column = maya.cmds.columnLayout( adj=True )

			self.__children = {}
			for p in compoundParameter.values() :

				if isinstance( p, IECore.CompoundParameter ) :
					self.__children[ p.name ] = ParameterSelector.ParameterGroup(
													p,
													depth = self.__depth+1,
													autoCollapseDepth = self.__autoCollapseDepth
												)

				else:
					self.__children[ p.name ] = ParameterSelector.Parameter( p, depth=self.__depth+1 )

			maya.cmds.setParent( ".." )
			maya.cmds.setParent( ".." )
			maya.cmds.setParent( ".." )

			maya.cmds.separator( style="none", height=3 )

		# Called by a callback or directly, to set the state of all child
		# parameters of the CompundParameter. If a state is not provided
		# then the curent checked state of the group is propogated
		def syncState( self, state=None ):

			if state == None:
				state = self.getState()

			for p in self.__children.values() :
				p.setState( state )

		# Can be called to set the state of the group and its children.
		def setState( self, state ) :
			maya.cmds.checkBox( self.__checkbox, edit=True, value=state )
			self.syncState( state )

		# \return (Bool) The checked state of the group itself. Note, this does not
		# take into account whether or not any children are checked.
		def getState( self ) :

			state = maya.cmds.checkBox( self.__checkbox, query=True, value=True )
			if state == 1 :
				return True
			else :
				return False

		# \return A list of active parameters in the group.
		def getActiveParameters( self ) :

			params = []

			if self.getState():
				params.append( self.__parameter )

			for p in self.__children.values() :
				params.extend( p.getActiveParameters() )

			return params


