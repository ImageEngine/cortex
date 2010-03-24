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

import os, re

import maya.cmds

import IECore

from UIElement import UIElement
from FnParameterisedHolder import FnParameterisedHolder

__all__ = [ 'PresetManagerUI' ]

### This class provides a UI for loading and saving presets for nodes
### derived from the ParameterisedHolder class.
class PresetManagerUI() :
	
	def __init__( self, node, defaultParameters=None ) :
	
		try :
			fn = FnParameterisedHolder( node )
		except:
			raise ValueError, 'PresetManagerUI: "%s" is not a valid Parameterised object.' % node
	
		self.__node = node
	
	### Call to save a preset.
	def save( self ) :
		SaveUI( self.__node )
	
	### Call to load a preset.
	def load( self ) :
		LoadUI( self.__node )


# Private implementation classes

# Save window
class SaveUI( UIElement ) :

	def __init__( self, node ) :

		self.__fnP = FnParameterisedHolder( node )

		parameterised = self.__fnP.getParameterised()		
		parameters = parameterised[0].parameters()

		self.__envVar = parameterised[3].replace( "_PATHS", "_PRESET_PATHS" )

		if self.__envVar not in os.environ :
			maya.cmds.confirmDialog( message="Environment variable not set:\n\n$%s\n\nPlease set this variable to point to one or more paths.\nPresets can then be saved to these locations." % self.__envVar, button="OK" )
			return

		self.__presetManager = IECore.PresetManager.defaultManager( self.__envVar )

		self.__window = maya.cmds.window(
			title="Save: %s" % node,
			width=300,
			height=500
		)

		UIElement.__init__( self, self.__window )

		self.__form = maya.cmds.formLayout()

		self.__scroll = maya.cmds.scrollLayout( parent=self.__form )

		self.__selector = ParameterSelector( parameters, self.__scroll )	

		self.__location = SearchPathMenu(
			self.__presetManager.searchPaths(),
			self.__form,
			label="Save to:",
			ann=self.__envVar,
			cw=( 1, 65 )
		)

		self.__name = maya.cmds.textFieldGrp(
			parent=self.__form,
			label="Name:",
			adj=2,
			columnWidth=( 1, 65 )
		)

		self.__saveButton = maya.cmds.button(
			l="Save",
			parent=self.__form,
			height=30,
			c=self._createCallback( self.__doSave )
		)

		maya.cmds.formLayout( self.__form, edit=True,

			attachForm=[	( self.__scroll,			 "top", 	 0  ),
							( self.__scroll,			 "left",	 0  ),
							( self.__scroll,			 "right",	 0  ), 
							( self.__location.menu(),	 "left",	 10 ),
							( self.__location.menu(),	 "right",	 10 ),
							( self.__name,  			 "left",	 10 ),
							( self.__name,  			 "right",	 10 ),
							( self.__saveButton,		 "bottom",   0  ),
							( self.__saveButton,		 "left",	 0  ),
							( self.__saveButton,		 "right",	 0  ) ],

			attachControl=[	( self.__scroll, 	 		"bottom",	5,	self.__location.menu() 	),
							( self.__location.menu(),	"bottom",   3,  self.__name  		    ), 
							( self.__name,				"bottom",   5,  self.__saveButton	    )  ]
		)

		maya.cmds.showWindow( self.__window )

	def __doSave( self ) :

		name = maya.cmds.textFieldGrp( self.__name, query=True, text=True )
		if not name:
			maya.cmds.confirmDialog( message="Please enter a name for the preset.", button="OK" )
			return

		# Sanitise the name a little
		name = name.replace( " ", "_" )
		name = re.sub( '[^a-zA-Z0-9_]*', "", name )

		parameters = self.__selector.getActiveParameters()

		if not parameters :
			maya.cmds.confirmDialog( message="Saelect at least one paremeter to save.", button="OK" )
			return

		path = self.__location.getValue()

		self.__presetManager.savePreset( self.__fnP.getParameterised()[0], parameters, path, name )

		maya.cmds.deleteUI( self.__window )

# Load window
class LoadUI( UIElement ) :

	def __init__( self, node ) :

		fn = FnParameterisedHolder( node )
		parameterised = fn.getParameterised()		
		parameters = parameterised[0].parameters()

		self.__fnP = fn
		self.__envVar = parameterised[3].replace( "_PATHS", "_PRESET_PATHS" )

		if self.__envVar not in os.environ :
			maya.cmds.confirmDialog( message="Environment variable not set:\n\n$%s\n\nPlease set this variable to point to one or more paths.\nPresets can then be loaded from these locations." % self.__envVar, button="OK" )
			return

		self.__presetManager = IECore.PresetManager.defaultManager( self.__envVar )
		
		# Clear the cache, otherwise presets saved since the last load won't show up
		self.__presetManager.refresh()
		presets = self.__presetManager.presets( parameterised[0] )
		if not presets:
			maya.cmds.confirmDialog( message="No presets found in the current search paths ($%s)." % self.__envVar, button="OK" )
			return

		self.__window = maya.cmds.window( title="Load: %s" % node, width=300, height=500 )
		
		UIElement.__init__( self, self.__window )

		self.__form = maya.cmds.formLayout()

		self.__selector = PresetSelector( presets, self.__form, allowMultiSelection=True )	
		self.__loadButton = maya.cmds.button( l="Load", parent=self.__form, height=30, c=self._createCallback( self.__doLoad ) )

		if not presets:
			maya.cmds.control( self.__loadButton, edit=True, enable=False )

		maya.cmds.formLayout( self.__form, edit=True,

			attachForm=[	( self.__selector.list(), 	"top" ,		0 ),
							( self.__selector.list(),	"left" , 	0 ),
							( self.__selector.list(),	"right" ,   0 ), 
							( self.__loadButton,		"bottom",   0 ),
							( self.__loadButton,		"left" ,	0 ),
							( self.__loadButton,		"right" ,   0 )  ],

			attachControl=[	( self.__selector.list(), 	"bottom", 	0, 	self.__loadButton )  ]
		)

		maya.cmds.showWindow( self.__window )

	def __doLoad( self ) :

		selected = self.__selector.selected()
		if not selected :
			maya.cmds.confirmDialog( message="Please select at least one preset to load.", button="OK" )
			return

		parameterised = self.__fnP.getParameterised()[0]	

		for s in selected:
			self.__presetManager.loadPreset( parameterised, s )

		# IMPORTANT
		self.__fnP.setNodeValues()

		maya.cmds.deleteUI( self.__window )	

# Provides an optionMenu to select from paths in the supplied IECore.SearchPath object.
class SearchPathMenu() :

	# *args, **kwargs are passed to maya.cmds.optionMenuGrp on creation.
	def __init__( self, searchPaths, parent=None, *args, **kwargs ) :

		oldParent = maya.cmds.setParent( q=True )
		if not parent :
			parent = oldParent

		maya.cmds.setParent( parent )

		self.__menu = maya.cmds.optionMenuGrp( *args, **kwargs )

		paths = searchPaths.getPaths( ":" )
		for p in paths.split( ":" ) :
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
			for p in presets:
				maya.cmds.textScrollList( self.__list, edit=True, append=p )

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
# A checkbox to allow selelection. 
class ParameterSelector( UIElement ) :

	def __init__( self, parameter, parent=None ) :

		oldParent = maya.cmds.setParent( query=True )		

		if not parent :	
			parent = oldParent	

		self.__mainColumn = maya.cmds.columnLayout( adj=True, parent=parent )

		if isinstance( parameter, IECore.CompoundParameter ) :
			self.__controls = ParameterSelector.ParameterGroup( parameter )
		else :
			self.__controls = ParameterSelector.Parameter( parameter )

		maya.cmds.setParent( oldParent )

	# \return A list of the selected parameters.
	def getActiveParameters( self ) :
		return  self.__controls.getActiveParameters() 

	# Provides an interface for selecting an individual parameter.
	class Parameter() :

		def __init__( self, parameter ) :
			self.__checkbox = maya.cmds.checkBox( label=parameter.name, align="left" )
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
		# on its state. The list syntax is used for interchangability with the
		# ParameterGroup class.
		def getActiveParameters( self ) :
			if self.getState():
				return [ self.__parameter ]
			else: 
				return []

	# Provides a hierarchical interface for selecting parameters in a CompoundParameter 
	class ParameterGroup( UIElement ) :

		def __init__( self, compoundParameter ) :

			self.__row = maya.cmds.rowLayout( numberOfColumns = 2, columnWidth=( 1, 20 ) )

			UIElement.__init__( self, self.__row )

			self.__checkbox = maya.cmds.checkBox( label = "", cc=self._createCallback( self.syncState ) )

			self.__frame = maya.cmds.frameLayout( 
				label = compoundParameter.name if compoundParameter.name else "All Parameters",
				labelIndent = 5,
				marginWidth = 5,
				borderVisible = False,
				collapsable = True
			)

			self.__column = maya.cmds.columnLayout( adj=True )

			self.__children = {}
			for p in compoundParameter.values() :

				if isinstance( p, IECore.CompoundParameter ) :
					self.__children[ p.name ] = ParameterSelector.ParameterGroup( p )

				else:
					self.__children[ p.name ] = ParameterSelector.Parameter( p )

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
		# take into account wether or not any children are checked.
		def getState( self ) :

			state = maya.cmds.checkBox( self.__checkbox, query=True, value=True )
			if state == 1 :
				return True
			else :	
				return False

		# \return A list of active parameters in the group.
		def getActiveParameters( self ) :

			params = []
			for p in self.__children.values() :
				params.extend( p.getActiveParameters() )

			return params


