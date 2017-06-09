##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

import maya.cmds

import IECore
import IECoreMaya

## A ParameterUI for ClassVectorParameters. Supports the following Parameter userData entries :
#
#   BoolData ["UI"]["collapsible"]
#     Specifies if the UI may be collapsed or not - defaults to True. 
#
# The icon used for a child UI can be overridden by setting the icon name, minus extension in either
# the child classes blindData, or its top level compound parameters userData() as follows:
# 
#   StringData <class.blindData()|class.parameters().userData()>["UI"]["icon"]
# 
# If a parameter of a class has one of the following userData elements set to True, a minimal version
# of the control for that parameter will be placed in the header row for the class entry,
# either before, or after the icon/name.
# 
#   BoolData ["UI"]["classVectorParameterPreHeader"]
#   BoolData ["UI"]["classVectorParameterHeader"]
#
# \TODO See if there is a better way to deal with problematic UI elements (eg: spline) that crash
# when a redraw is called and constituent plugs no longer exist. This can happen when removing
# classes, or changing parameter types, as an AE redraw may happen before the related UI
# has been deleted. It seems for some parameter types, maya doesn't check to see if all 
# the required plugs are there during a redraw, and consequently falls over.
# This behavior is currently compensated for by pre-updating the UI before removing a class
# or changing its version.
class ClassVectorParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
		
		IECoreMaya.ParameterUI.__init__(
			
			self,
			node,
			parameter,
			maya.cmds.frameLayout(
				labelVisible = False,
				collapsable = False,
			),
			**kw
			
		)
		
		# passing borderVisible=False to the constructor does bugger all so we do it here instead.
		maya.cmds.frameLayout( self._topLevelUI(), edit=True, borderVisible=False )
				
		collapsible = self.__parameterIsCollapsible()
		
		self.__kw = kw.copy()
		if collapsible :
			self.__kw["hierarchyDepth"] = self.__kw.get( "hierarchyDepth", -1 ) + 1
		
		collapsed = self._retrieveCollapsedState( collapsible )
		
		self.__collapsible = IECoreMaya.Collapsible(
		
			annotation = self.description(),
			label = self.label(),
			labelFont = IECoreMaya.CompoundParameterUI._labelFont( self.__kw["hierarchyDepth"] ),
			labelIndent = IECoreMaya.CompoundParameterUI._labelIndent( self.__kw["hierarchyDepth"] ),
			labelVisible = collapsible,
			collapsed = collapsed,
			expandCommand = self.__expand,
			collapseCommand = self.__collapse,
			
		)
					
		self.__formLayout = maya.cmds.formLayout( parent=self.__collapsible.frameLayout() )
	
		self.__buttonRow = maya.cmds.rowLayout( nc=3, adj=3, cw3=( 25, 25, 40 ), parent=self.__formLayout )
	
		self.__addButton = maya.cmds.picture( image="ie_addIcon_grey.xpm", parent=self.__buttonRow, width=21 )
		IECoreMaya.Menu( IECore.curry( self.__classMenuDefinition, None ), self.__addButton )
		IECoreMaya.Menu( IECore.curry( self.__classMenuDefinition, None ), self.__addButton, button=1 )
		
		self.__toolsButton = maya.cmds.picture( image="ie_actionIcon_grey.xpm", parent=self.__buttonRow, width=21 )
		IECoreMaya.Menu( IECore.curry( self.__toolsMenuDefinition, None ), self.__toolsButton )
		IECoreMaya.Menu( IECore.curry( self.__toolsMenuDefinition, None ), self.__toolsButton, button=1 )

		self.__classInfo = []
		self.__childUIs = {} # mapping from parameter name to ui name
		
		self.replace( node, parameter )
			
	def replace( self, node, parameter ) :
		
		## This is to catch the fact that self.node() will raise
		# if the old node no longer exists.
		nodeChanged = True
		with IECore.IgnoredExceptions( RuntimeError ) :
			nodeChanged = self.node() != node

		IECoreMaya.ParameterUI.replace( self, node, parameter )
				
		if self.__parameterIsCollapsible() :
			collapsed = self._retrieveCollapsedState( self.getCollapsed() )
			self.setCollapsed( collapsed, **self.__kw )
					
		self.__updateChildUIs( startFromScratch=nodeChanged )
			
	## Gets the collapsed state for the frame holding the child parameter uis.
	def getCollapsed( self ) :
	
		return self.__collapsible.getCollapsed()
		
	## Sets the collapsed state for the frame holding the child parameter uis.
	# In the case that this ui itself is not collapsible, it will still propagate
	# \param propagateToChildren How many levels of hierarchy to propagate 
	# the new state to. If a Bool is passed, rather than an int, then
	# 'all' or 'none' is assumed, for backwards compatibility		
	def setCollapsed( self, collapsed, propagateToChildren=0, **kw ) :
		
		if type(propagateToChildren) == bool :	
			propagateToChildren = 999 if propagateToChildren else 0
			
		if self.__parameterIsCollapsible() :
			self.__collapsible.setCollapsed( collapsed )
			self._storeCollapsedState( collapsed )

		if propagateToChildren > 0 :
			propagateToChildren = propagateToChildren - 1
			self.__propagateCollapsed( collapsed, propagateToChildren, **kw )
	
	# This will retrieve the collapsedState from the parameters userData. It uses the
	# default key if 'collapsedUserDataKey' was not provided in the UI constructor's **kw.
	def _retrieveCollapsedState( self, default=True ) :
		
		key = self.__kw.get( "collapsedUserDataKey", IECoreMaya.CompoundParameterUI._collapsedUserDataKey )
		if "UI" in self.parameter.userData() and key in self.parameter.userData()["UI"] :		
			return self.parameter.userData()["UI"][ key ].value
		else :
			return default
	
	# This will store \param state in the parameters userData, under the default key,
	# unless 'collapsedUserDataKey' was provided in the UI constructor's **kw.		
	def _storeCollapsedState( self, state ) :

		if "UI" not in self.parameter.userData() :
			self.parameter.userData()["UI"] = IECore.CompoundObject()
	
		key = self.__kw.get( "collapsedUserDataKey", IECoreMaya.CompoundParameterUI._collapsedUserDataKey )
		self.parameter.userData()["UI"][key] = IECore.BoolData( state )
		
	def __classMenuDefinition( self, parameterName ) :
	
		result = IECore.MenuDefinition()
		
		classNameFilter = "*"
		try :
			classNameFilter = self.parameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		
		### \todo We can't assume that * is immediately after /
		menuPathStart = max( 0, classNameFilter.find( "*" ) )
			
		loader = IECore.ClassLoader.defaultLoader( self.parameter.searchPathEnvVar() )
		for className in loader.classNames( classNameFilter ) :
			classVersions = loader.versions( className )
			for classVersion in classVersions :
				
				active = True
				if parameterName :
					active = self.parameter.getClass( parameterName, True )[1:] != ( className, classVersion )
				
				menuPath = "/" + className[menuPathStart:]
				if len( classVersions ) > 1 :
					menuPath += "/v" + str( classVersion )

				result.append(
					
					# applies interToUI to each component of the menu path individually, and then joings them back up again
					"/".join( [ maya.mel.eval( "interToUI \"" + x + "\"" ) for x in menuPath.split( "/" ) ] ),
					
					IECore.MenuItemDefinition(
						command = IECore.curry( self._setClass, parameterName, className, classVersion ),
						active = active
					)
					
				)
	
		for cb in self.__addClassMenuCallbacks :
			cb( result, self.parameter, self.node() )
	
		return result	
		
	def __toolsMenuDefinition( self, parameterName ) :
	
		result = IECore.MenuDefinition()
	
		if not len( self.__toolsMenuCallbacks ) :
			result.append( "/No tools available", { "active" : False } )
	
		for cb in self.__toolsMenuCallbacks :
			cb( result, self.parameter, self.node() )
	
		return result
			
	def _removeClass( self, parameterName ) :
	
		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
		
		classes = [ c[1:] for c in self.parameter.getClasses( True ) if c[1] != parameterName ]
		
		# We have to do a pre-update here, to make sure we delete the UI for any classes we
		# are going to remove, before we remove them in C++. See the notes at the top.
		self.__updateChildUIs( classes )
		
		with fnPH.parameterModificationContext() :
			self.parameter.setClasses( classes )
	
	def _setClass( self, parameterName, className, classVersion ) :
	
		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
		
		# We have to remove this class from the UI first, or, if the parameter list changes,
		# maya can crash as it tries to redraw the AE. See the notes at the top.
		classesWithoutThis = [ c[1:] for c in self.parameter.getClasses( True ) if c[1] != parameterName ]
		self.__updateChildUIs( classesWithoutThis )
		
		classes = [ c[1:] for c in self.parameter.getClasses( True ) ]
		if parameterName :
			foundParameter = False
			for i in range( 0, len( classes ) ) :
				if classes[i][0]==parameterName :
					foundParameter = True
					break
			if not foundParameter :
				raise RuntimeError( "Parameter \"%s\" not present" % parameterName )
			classes[i] = ( parameterName, className, classVersion )
		else :
			# no parameter name - add a new parameter
			parameterNames = set( [ c[0] for c in classes ] )
			for i in range( 0, len( classes ) + 1 ) :
				parameterName = "p%d" % i
				if parameterName not in parameterNames :
					break
		
			classes.append( ( parameterName, className, classVersion ) )
		
		with fnPH.parameterModificationContext() :
			self.parameter.setClasses( classes )
	
	# \param classes A sequence type based on the list comprehension:
	#     [ c[1:] for c in self.parameter.getClasses(True) ]
	#
	# \param startFromScratch If this is true, then all child uis are
	# removed and rebuilt.
	## \todo If we could reuse child uis (because they had a replace() method)
	# then we wouldn't need the startFromScratch argument.
	def __updateChildUIs( self, classes=None, startFromScratch=False ) :
		
		if classes == None:	
			classes = [ c[1:] for c in self.parameter.getClasses( True ) ]
		
		# delete any uis for parameters which have disappeared
		
		parameterNamesSet = set( [ c[0] for c in classes ] )
		for parameterName in self.__childUIs.keys() :
			if parameterName not in parameterNamesSet or startFromScratch :
				maya.cmds.deleteUI( self.__childUIs[parameterName]._topLevelUI() )
				del self.__childUIs[parameterName]
		
		# and create or reorder uis for remaining parameters
	
		attachForm = [
			( self.__buttonRow, "left", IECoreMaya.CompoundParameterUI._labelIndent( self.__kw["hierarchyDepth"] + 1 ) ),
			( self.__buttonRow, "bottom", 5 ),
		]
		attachControl = []
		attachNone = []
		prevChildUI = None
		for i in range( 0, len( classes ) ) :
		
			parameterName = classes[i][0]

			longParameterName = self.__kw['longParameterName']
			if longParameterName :
				# If we have a path already, we need a separator, otherwise, not
				longParameterName += "."
			longParameterName += parameterName
			
			visible = True
			if 'visibleOnly' in self.__kw :

				visible = longParameterName in self.__kw['visibleOnly']

				if not visible :
					for i in self.__kw['visibleOnly'] :
						if i.startswith( longParameterName + "." ) :
							visible = True
							break

			if not visible:
				continue
			
			
			childUI = self.__childUIs.get( parameterName, None )
			if childUI :
				# delete it if it's not the right sort any more
				if childUI.__className!=classes[i][1] or childUI.__classVersion!=classes[i][2] :
					maya.cmds.deleteUI( childUI._topLevelUI() )
					childUI = None
			
			if not childUI :
				with IECoreMaya.UITemplate( "attributeEditorTemplate" ) :
					maya.cmds.setParent( self.__formLayout )
									
					if "longParameterName" in self.__kw :
						# We have to append our 'name' (as worked out above), otherwise,
						# the parameter path misseses it out as the parameter were passing down
						# has '' as a name.
						kw = self.__kw.copy()
						kw["longParameterName"] = longParameterName
						childUI = ChildUI( self.parameter[parameterName], **kw )
					else:
						childUI = ChildUI( self.parameter[parameterName], **self.__kw )
					
					childUI.__className = classes[i][1]
					childUI.__classVersion = classes[i][2]
					self.__childUIs[parameterName] = childUI
						
			attachForm += [ 
				( childUI._topLevelUI(), "left", 0 ),
				( childUI._topLevelUI(), "right", 0 ),
			]
			
			if i==0 :
				attachForm.append( ( childUI._topLevelUI(), "top", 5 ) )
			else :
				attachControl.append( ( childUI._topLevelUI(), "top", 0, prevChildUI._topLevelUI() ) )
			
			attachNone.append( ( childUI._topLevelUI(), "bottom" ) )
									
			prevChildUI = childUI

		if prevChildUI :
			attachControl.append( ( self.__buttonRow, "top", 5, prevChildUI._topLevelUI() ) )
		else :
			attachForm.append( ( self.__buttonRow, "top", 5 ) )
			
		maya.cmds.formLayout(
			self.__formLayout,
			edit=True,
			attachForm = attachForm,
			attachControl = attachControl,
			attachNone = attachNone
		)

	def __expand( self ) :
	
		self._storeCollapsedState( False )
	
		modifiers = maya.cmds.getModifiers()
		if modifiers & 1 :
			# shift is held
			self.__propagateCollapsed( False, 999, lazy=True )
		elif modifiers & 8 :
			# ctrl is held
			depth = 1;
			with IECore.IgnoredExceptions( KeyError ) :
				depth = self.parameter.userData()["UI"]["autoExpandDepth"].value
			self.__propagateCollapsed( False, depth, lazy=True )
			
	def __collapse(self):

		self._storeCollapsedState( True )

		# \todo Store collapse state
		modifiers = maya.cmds.getModifiers()
		if modifiers & 1 :
			# shift is held
			self.__propagateCollapsed( True, 999 )
		elif modifiers & 8 :
			# ctrl is held
			depth = 1;
			with IECore.IgnoredExceptions( KeyError ) :
				depth = self.parameter.userData()["UI"]["autoExpandDepth"].value
			self.__propagateCollapsed( True, depth )
		
	def __propagateCollapsed( self, collapsed, propagateDepth=999, **kw ) :
		
		for ui in self.__childUIs.values() :
			if hasattr( ui, "setCollapsed" ) :
				ui.setCollapsed( collapsed, propagateDepth, **kw )
	
	def __parameterIsCollapsible( self ) :
	
		collapsible = True
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = self.parameter.userData()["UI"]["collapsible"].value
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = self.parameter.userData()["UI"]["collapsable"].value

		return collapsible
				
	@staticmethod
	def _classesSetCallback( fnPH, parameter ) :
			
		for instance in IECoreMaya.UIElement.instances( ClassVectorParameterUI ) :
			if instance.parameter.isSame( parameter ) :
				instance.__updateChildUIs()
	
	# only protected so it can be accessed by the ChildUI
	_classMenuCallbacks = []
	## Registers a callback which is able to modify the popup menu used to
	# edit an individual class entry within the vector of classes. Callbacks
	# should have the following signature :
	#
	# callback( menuDefinition, classVectorParameter, childParameter, holderNode )
	@classmethod
	def registerClassMenuCallback( cls, callback ) :
	
		cls._classMenuCallbacks.append( callback )

	__addClassMenuCallbacks = []
	## Registers a callback which is able to modify the popup menu used
	# to add class entries into the vector of classes. Callbacks should have
	# the following signature :
	#
	# callback( menuDefinition, classVectorParameter, holderNode )
	@classmethod
	def registerAddClassMenuCallback( cls, callback ) :
	
		cls.__addClassMenuCallbacks.append( callback )
		
	__toolsMenuCallbacks = []
	## Registers a callback which is able to modify the tools popup menu
	# Callbacks should have the following signature :
	#
	# callback( menuDefinition, classVectorParameter, holderNode )
	@classmethod
	def registerToolsMenuCallback( cls, callback ) :
	
		cls.__toolsMenuCallbacks.append( callback )

class ChildUI( IECoreMaya.UIElement ) :

	def __init__( self, parameter, **kw ) :
		
		IECoreMaya.UIElement.__init__( self, maya.cmds.columnLayout() )
		
		if not isinstance( self.__vectorParent(), ClassVectorParameterUI ) :
			raise RuntimeError( "Parent must be a ClassVectorParameterUI" )
		
		self.__kw = kw.copy()
		self.__kw["hierarchyDepth"] = self.__kw.get( "hierarchyDepth", -1 ) + 1
			
		self.__parameter = parameter
		
		headerFormLayout = maya.cmds.formLayout()
		attachForm = []
		attachControl = []
				
		# triangle for expanding to view all parameters
		
		self.__parameterVisibilityIcon = maya.cmds.iconTextButton(
			style="iconOnly",
			height = 20,
			width = 20,
			image="arrowRight.xpm",
			command = self._createCallback( self.__toggleParameterVisibility ),
			annotation = "Show parameters",
			visible = self.__parameterIsCollapsible(),
		)
		
		attachForm += [
			( self.__parameterVisibilityIcon, "left",  IECoreMaya.CompoundParameterUI._labelIndent( self.__kw["hierarchyDepth"] ) ),
			( self.__parameterVisibilityIcon, "top", 0 ),
			( self.__parameterVisibilityIcon, "bottom", 0 ),
		]
		
		lastControl = self.__buildOptionalPreHeaderUI( headerFormLayout, attachForm, attachControl, self.__parameterVisibilityIcon )

		# layer icon

		layerIcon = maya.cmds.picture(
			width = 20,
			image = "%s.xpm" % self.__classIconName(),
			annotation = IECore.StringUtil.wrap(
				self.__class()[0].description + "\n\n" + "Click to reorder or remove.",
				48,
			)
		)
		IECoreMaya.Menu( self.__layerMenu, layerIcon )
		IECoreMaya.Menu( self.__layerMenu, layerIcon, button=1 )
		
		attachControl += [
			( layerIcon, "left", 0, lastControl ),
		]
		
		attachForm += [
			( layerIcon, "top", 0 ),
			( layerIcon, "bottom", 0 ),
		]
				
		# class specific fields
		
		self.__attributeChangedCallbackId = None
		self.__presetParameters = []
		self.__presetUIs = []

		self.__buildOptionalHeaderUI( headerFormLayout, attachForm, attachControl, layerIcon )
		
		maya.cmds.formLayout( 
			headerFormLayout,
			edit = True,
			attachForm = attachForm,
			attachControl = attachControl,
		)
		
		# CompoundParameterUI to hold child parameters
		
		maya.cmds.setParent( self._topLevelUI() )
				
		self.__compoundParameterUI = IECoreMaya.CompoundParameterUI( self.__vectorParent().node(), parameter, labelVisible = False, **kw )
		
		self.setCollapsed( self.getCollapsed(), False )
		
	def getCollapsed( self ) :
		
		return self.__compoundParameterUI.getCollapsed()
		
	def setCollapsed( self, collapsed, propagateToChildren=False, **kw ) :
	
		self.__compoundParameterUI.setCollapsed( collapsed, propagateToChildren=propagateToChildren, **kw )
		
		image = "arrowRight.xpm" if collapsed else "arrowDown.xpm"
		annotation = "Show parameters" if collapsed else "Hide parameters" 
		maya.cmds.iconTextButton(
			self.__parameterVisibilityIcon,
			edit = True,
			image = image,
			annotation = annotation,
		)
	
	def __parameterIsCollapsible( self ) :
	
		collapsible = True
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = self.__parameter.userData()["UI"]["collapsible"].value
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = self.__parameter.userData()["UI"]["collapsable"].value

		return collapsible
	
	def _topLevelUIDeleted( self ) :
	
		self.__attributeChangedCallbackId = None
		
	def __class( self ) :
	
		classes = self.__vectorParent().parameter.getClasses( True )
		for c in classes :
			if c[1] == self.__parameter.name :
				return c
				
		raise RunTimeError( "Couldn't find class entry" )
						
	def __classVersionAnnotation( self ) :
	
		c = self.__class()
			
		return "%s v%d" % ( c[2], c[3] ) + "\n\nClick to change version"

	def __classIconName( self ) :
		
		c = self.__class()[0]
		
		iconName = "out_displayLayer"
		with IECore.IgnoredExceptions( KeyError ) :
			iconName = self.__vectorParent().parameter.userData()["UI"]["defaultChildIcon"].value
		
		sources = []
		
		if hasattr( c, "blindData" ):
			sources.append( c.blindData() )
		
		if hasattr( c, "parameters" ):
			sources.append( c.parameters().userData() )
		
		for data in sources:
			with IECore.IgnoredExceptions( KeyError ) :
				icon = data["UI"]["icon"].value
				if icon :
					return icon
		
		return iconName
	
	
	def __versionMenuDefinition( self ) :
	
		c = self.__class()
		
		# make a menu with just the versions in
			
		loader = IECore.ClassLoader.defaultLoader( self.__vectorParent().parameter.searchPathEnvVar() )
		result = IECore.MenuDefinition()
		for classVersion in loader.versions( c[2] ) :
					
			result.append(

				"/%d" % classVersion, 

				IECore.MenuItemDefinition(
					command = IECore.curry( self.__vectorParent()._setClass, self.__parameter.name, c[2], classVersion ),
					active = c[3] != classVersion
				)

			)
			
		return result

	def __toggleParameterVisibility( self ) :
			
		collapsed = not self.getCollapsed()
		depth = 0
		
		modifiers = maya.cmds.getModifiers()
		if modifiers & 1 :
			# shift is held
			depth = 999
		elif modifiers & 8 :
			# alt is held
			depth = 1;
			c = self.__class()[0]	
			if c and hasattr( c, "parameters" ):
				with IECore.IgnoredExceptions( KeyError ) :
					depth = c.parameters().userData()["UI"]["autoExpandDepth"].value
		
		if depth :
			self.setCollapsed( collapsed, propagateToChildren=depth, lazy=True )
		else :
			self.setCollapsed( collapsed, propogateToChildren=False )
	
	def __layerMenu( self ) :
	
		result = IECore.MenuDefinition()
		
		layerNames = self.__vectorParent().parameter.keys()
		layerIndex = layerNames.index( self.__parameter.name )
		
		result.append(
			"/Move/To Top",
			IECore.MenuItemDefinition(
				command = IECore.curry( self.__moveLayer, layerIndex, 0 ),
				active = layerIndex != 0
			)
		)
		
		result.append(
			"/Move/Up",
			IECore.MenuItemDefinition(
				command = IECore.curry( self.__moveLayer, layerIndex, layerIndex-1 ),
				active = layerIndex >= 1
			)
		)
		
		result.append(
			"/Move/Down",
			IECore.MenuItemDefinition(
				command = IECore.curry( self.__moveLayer, layerIndex, layerIndex+1 ),
				active = layerIndex < len( layerNames ) - 1
			)
		)
		
		result.append(
			"/Move/To Bottom",
			IECore.MenuItemDefinition(
				command = IECore.curry( self.__moveLayer, layerIndex, len( layerNames ) - 1 ),
				active = layerIndex < len( layerNames ) - 1
			)
		)
		
		result.append(
			"/RemoveSeparator",
			IECore.MenuItemDefinition(
				divider = True,
			)
		)
		
		# This has to be deferred as we update the UI from within the _removeClass method.
		# Unles it is, maya will crash as its still preoccupied with the popup menu.
		result.append(
			"/Remove",
			IECore.MenuItemDefinition(
				command = IECore.curry( maya.cmds.evalDeferred, IECore.curry( self.__vectorParent()._removeClass, self.__parameter.name ) )
			)
		)
		
		result.append(
			"/VersionSeparator",
			IECore.MenuItemDefinition(
				divider = True,
			)
		)
		
		result.append( 
			"/Change Version",
			IECore.MenuItemDefinition(
				subMenu = self.__versionMenuDefinition
			)
		)
		
		for cb in ClassVectorParameterUI._classMenuCallbacks :
			cb( result, self.__vectorParent().parameter, self.__parameter, self.__vectorParent().node() )
		
		return result
		
	def __moveLayer( self, oldIndex, newIndex ) :
	
		classes = [ c[1:] for c in self.__vectorParent().parameter.getClasses( True ) ]
		cl = classes[oldIndex]
		del classes[oldIndex]
		classes[newIndex:newIndex] = [ cl ]
				
		fnPH = IECoreMaya.FnParameterisedHolder( self.__vectorParent().node() )
		with fnPH.parameterModificationContext() :
			self.__vectorParent().parameter.setClasses( classes )

	def __buildOptionalPreHeaderUI( self, formLayout, attachForm, attachControl, lastControl ) :
		
		return self.__drawHeaderParameterControls( formLayout, attachForm, attachControl, lastControl, "classVectorParameterPreHeader" )

	def __buildOptionalHeaderUI( self, formLayout, attachForm, attachControl, lastControl ) :
		
		defaultLabel = ""
		try :
			parentUIUserData = self.__vectorParent().parameter.userData()["UI"]
			defaultLabelStyle = parentUIUserData["defaultChildLabel"].value
			
			if 'classname' in defaultLabelStyle.lower() :
				defaultLabel = self.__class()[2]
			
			if defaultLabelStyle.startswith( 'abbreviated' ) :
				if "classNameFilter" in parentUIUserData :
					classNameFilter = parentUIUserData["classNameFilter"].value
					defaultLabel = defaultLabel.replace( classNameFilter.strip( '*' ), '' )
			
			if defaultLabelStyle.endswith( 'ToUI' ) :
				defaultLabel = maya.mel.eval( 'interToUI( "%s" )' % defaultLabel )
			
			defaultDescription = self.__class()[0].description
		except :
			defaultLabel = ""
			defaultDescription = ""
		
		labelPlugPath = self.__labelPlugPath()
		if labelPlugPath or defaultLabel :
			
			label = maya.cmds.getAttr( labelPlugPath ) if labelPlugPath else defaultLabel
			description = self.__parameter["label"].description if labelPlugPath else defaultDescription
			
			self.__label = maya.cmds.text(
				parent = formLayout,
				align = "left",
				label = label,
				font = IECoreMaya.CompoundParameterUI._labelFont( self.__kw["hierarchyDepth"] ),
				annotation = IECore.StringUtil.wrap( description, 48 ),
				width = 190 - IECoreMaya.CompoundParameterUI._labelIndent( self.__kw["hierarchyDepth"] ),
				recomputeSize = False,
			)
			
			if labelPlugPath :
				
				lockedLabel = False
				with IECore.IgnoredExceptions( KeyError ) :
					lockedLabel = self.__parameter["label"].userData()["UI"]["locked"]
				
				if not lockedLabel :
					
					renameMenu = IECore.MenuDefinition(
						[
							( "Change label...", { "command" : self.__changeLabel } ),
						]
					)
					IECoreMaya.Menu( renameMenu, self.__label )
					IECoreMaya.Menu( renameMenu, self.__label, button = 1 )
			
			attachForm += [
				( self.__label, "top", 0 ),
				( self.__label, "bottom", 0 ),
			]
			attachControl += [
				( self.__label, "left", 4, lastControl ),
			]
			
			lastControl = self.__label
			
		return self.__drawHeaderParameterControls( formLayout, attachForm, attachControl, lastControl, "classVectorParameterHeader" )
		
		
	def __drawHeaderParameterControls( self, formLayout, attachForm, attachControl, lastControl, uiKey ) :
	
		fnPH = IECoreMaya.FnParameterisedHolder( self.__vectorParent().node() )
		for parameter in self.__parameter.values() :
			
			forHeader = False
			with IECore.IgnoredExceptions( KeyError ) :
				forHeader = parameter.userData()["UI"][ uiKey ].value
				
			if forHeader :
				
				control = self.__drawHeaderParameterControl( parameter, fnPH )
						
				if control :
				
					attachForm +=  [
						( control, "top", 0 ),
						( control, "bottom", 0 ),
					]
					attachControl += [
						( control, "left", 0, lastControl ),
					]

					lastControl = control
		
		return lastControl
					

	def __drawHeaderParameterControl( self, parameter, fnPH ) :
		
		## \todo This would be so much easier if we could just use ParameterUI
		# instances for each of the controls. We can't because they all do their
		# own labelling and are layed out for an attribute editor. if we do the
		# todo in ParameterUI to remove the labels and stuff then we can do the
		# todo here.
		
		control = None 
		
		parameterPlugPath = fnPH.parameterPlugPath( parameter )
		annotation = IECore.StringUtil.wrap( "%s\n\n%s" % ( parameterPlugPath.split( "." )[1], parameter.description ), 48 )
		if parameter.presetsOnly :

			control = maya.cmds.iconTextStaticLabel(
				image = "arrowDown.xpm",
				font = "smallBoldLabelFont",
				style = "iconAndTextHorizontal",
				height = 23,
				width = 80,
				annotation = annotation,
			)
			IECoreMaya.Menu( IECore.curry( self.__presetsMenu, parameter ), control )
			IECoreMaya.Menu( IECore.curry( self.__presetsMenu, parameter ), control, button=1 )
			self.__presetParameters.append( parameter )
			self.__presetUIs.append( control )
			if self.__attributeChangedCallbackId is None :
				self.__attributeChangedCallbackId = IECoreMaya.CallbackId(
					maya.OpenMaya.MNodeMessage.addAttributeChangedCallback( self.__vectorParent().node(), self.__attributeChanged )
				)

			self.__updatePresetLabel( len( self.__presetUIs ) - 1 )

		elif isinstance( parameter, IECore.BoolParameter ) :

			control = maya.cmds.checkBox( label="", annotation=annotation )
			maya.cmds.connectControl( control, parameterPlugPath )

		elif isinstance( parameter, IECore.FloatParameter ) :

			control = maya.cmds.floatField(
				annotation = annotation,
				minValue = parameter.minValue,
				maxValue = parameter.maxValue,
				width = 45,
				pre = 4
			)
			maya.cmds.connectControl( control, parameterPlugPath )
			
		elif isinstance( parameter, IECore.IntParameter ) :

			kw = {}
			if parameter.hasMinValue() :
				kw["minValue"] = parameter.minValue
			if parameter.hasMaxValue() :
				kw["maxValue"] = parameter.maxValue

			control = maya.cmds.intField(
				annotation = annotation,
				width = 45,
				**kw
			)
			maya.cmds.connectControl( control, parameterPlugPath )	

		elif isinstance( parameter, IECore.Color3fParameter ) :

			control = maya.cmds.attrColorSliderGrp(
				label = "",
				columnWidth = ( ( 1, 1 ), ( 2, 30 ), ( 3, 1 ) ),
				columnAttach = ( ( 1, "both", 0 ), ( 2, "both", 0  ), ( 3, "left", 0 ) ),
				attribute = parameterPlugPath,
				annotation = annotation,
				showButton = False
			)
			
		elif isinstance( parameter, IECore.StringParameter ) :
		
			control = maya.cmds.textField( annotation = annotation, width = 150 )
			maya.cmds.connectControl( control, parameterPlugPath )

		elif isinstance( parameter, IECore.V2fParameter ) :
			
			control = maya.cmds.rowLayout( nc=2, cw2=( 45, 45 ) )
			
			column1 = maya.cmds.floatField(
				annotation = annotation,
				width = 45,
				pre = 4,
				parent = control
			)
			
			maya.cmds.connectControl( column1, parameterPlugPath + "X" )
			column2 = maya.cmds.floatField(
				annotation = annotation,
				width = 45,
				pre = 4,
				parent = control
			)
			
			maya.cmds.connectControl( column2, parameterPlugPath + "Y" )
			
		else :

			IECore.msg( IECore.Msg.Level.Warning, "ClassVectorParameterUI", "Parameter \"%s\" has unsupported type for inclusion in header ( %s )." % ( parameter.name, parameter.typeName() ) )
		
		return control

	def __labelPlugPath( self ) :
	
		if "label" in self.__parameter :
			fnPH = IECoreMaya.FnParameterisedHolder( self.__vectorParent().node() )
			return fnPH.parameterPlugPath( self.__parameter["label"] )
			
		return ""
		
	def __changeLabel( self ) :
	
		labelPlugPath = self.__labelPlugPath()
	
		result = maya.cmds.promptDialog(
			title = "Change label",
			message = "New label : ",
			text = maya.cmds.getAttr( labelPlugPath ),
			button = [ "Change", "Cancel" ],
			defaultButton = "Change",
			cancelButton = "Cancel",
		)	
		
		if result == "Change" :
			newLabel = maya.cmds.promptDialog( query=True, text=True )
			maya.cmds.setAttr( labelPlugPath, newLabel, type="string" )
			maya.cmds.text(
				self.__label,
				edit = True,
				label = newLabel
			)
	
	def __presetsMenu( self, parameter ) :
	
		result = IECore.MenuDefinition()
		for p in parameter.presetNames() :
			result.append( "/" + p, { "command" : IECore.curry( self.__setPreset, parameter, p ) } )
			
		return result
		
	def __setPreset( self, parameter, name ) :
	
		parameter.setValue( name )
		IECoreMaya.FnParameterisedHolder( self.__vectorParent().node() ).setNodeValue( parameter )

	def __attributeChanged( self, changeType, plug, otherPlug, userData ) :
				
		if not ( changeType & maya.OpenMaya.MNodeMessage.kAttributeSet ) :
			return
		
		fnPH = IECoreMaya.FnParameterisedHolder( self.__vectorParent().node() )
		for index, parameter in enumerate( self.__presetParameters ) :
			
			try :
				myPlug = fnPH.parameterPlug( parameter )
			except :
				# this situation can occur when our parameter has been removed but the
				# ui we represent is not quite yet dead
				continue
		
			if plug == myPlug :
				self.__updatePresetLabel( index )
				
	def __updatePresetLabel( self, index ) :
	
		maya.cmds.iconTextStaticLabel( 
			self.__presetUIs[index],
			edit = True,
			label = self.__presetParameters[index].getCurrentPresetName()
		)
	
	## Returns the ClassVectorParameterUI which this ChildUI is hosted in.
	def __vectorParent( self ) :
	
		return self.parent().parent()
									
IECoreMaya.FnParameterisedHolder.addSetClassVectorParameterClassesCallback( ClassVectorParameterUI._classesSetCallback )
					
IECoreMaya.ParameterUI.registerUI( IECore.ClassVectorParameter.staticTypeId(), ClassVectorParameterUI )
