
from _IECoreMaya import *

from UIElement import UIElement


import IECore
import IECoreMaya

import maya.cmds
import maya.mel
import maya.OpenMayaUI

## The UIElement base class assists in implementing ui elements in an
# object oriented manner in python.
## \todo Preexisting ui elements should be rewritten to take
# advantage of this base class wherever possible.
class UIElement :

	__instances = {}

	## Derived classes must create a ui element which is the parent of the
	# rest of their ui, and call this init function passing it as the topLevelUI
	# parameter. The base class will ensure that the UIElement instance is kept
	# alive as long as the ui exists, and that it will be destroyed when the
	# ui is destroyed.
	def __init__( self, topLevelUI ) :

		instanceRecord = IECore.Struct()
		instanceRecord.instance = self
		instanceRecord.callbacks = []
		# Saves errors in batch if UI work is still done....
		if not maya.cmds.about( batch=True ):
			instanceRecord.uiDeletedCallbackId = IECoreMaya.CallbackId( maya.OpenMayaUI.MUiMessage.addUiDeletedCallback( topLevelUI, self.__uiDeleted, topLevelUI ) )
		UIElement.__instances[topLevelUI] = instanceRecord

		self.__topLevelUI = topLevelUI

	## Returns the UIElement which forms the parent for this one.
	def parent( self ) :
	
		melUI = self._topLevelUI()
		while 1 :
			melUI = melUI.rpartition( "|" )[0]
			if melUI in self.__instances :
				return self.__instances[melUI].instance
			if not melUI :
				return None

	## Returns the name of the top level ui element for this instance.
	def _topLevelUI( self ) :

		return self.__topLevelUI

	## It's very natural to assign member functions to ui callbacks. There is a problem
	# with this however in that maya leaks references to any python callbacks passed
	# to it (tested in maya 2008). This causes a reference to be held to the UIElement
	# instance forever. This could be a real problem if the ui references significant amounts
	# of memory, which it may well do. This class wraps a callback function in such a way
	# that maya will not leak it.
	#
	# Example usage : maya.cmds.button( command=self._createCallback( self.__buttonPressed ) )
	#
	# If the mel parameter is True then it causes the creation of a mel command to call back and
	# invoke the python callback. This is useful for the cases where maya insists on treating callbacks as
	# mel commands even when they're added from python - for instance the outlinerEditor selectCommand
	# in maya 2008.
	def _createCallback( self, function, mel=False ) :

		callbacks = self.__instances[self._topLevelUI()].callbacks
		callbacks.append( function )

		pythonCmd = "import IECoreMaya; IECoreMaya.UIElement._UIElement__invokeCallback( '%s', %d )" % ( self._topLevelUI(), len( callbacks ) - 1 )
		if not mel :
			return pythonCmd
		else :
			return "python( \"%s\" )" % pythonCmd

	## This is called when the maya ui element corresponding to this
	# instance is deleted. It may be reimplemented by derived classes
	# to perform any necessary cleanup. One item of cleanup that might be
	# very important is to delete any IECoreMaya.CallbackId objects that
	# may be linking a maya message to a method of the instance - if this
	# is not done then a circular reference will prevent the instance from
	# dying, and the callbacks will continue to be despatched despite the
	# fact that the ui has long gone.
	def _topLevelUIDeleted( self ) :

		pass

	## Returns a list of all the active instances derived from the specified type.
	@staticmethod
	def instances( type = None ) :
	
		if type is None :
			type = UIElement
	
		result = []
		for v in UIElement.__instances.values() :
			instance = v.instance
			if isinstance( instance, type ) :
				result.append( instance )
				
		return result

	@staticmethod
	def __uiDeleted( topLevelUI ) :

		UIElement.__instances[topLevelUI].instance._topLevelUIDeleted()
		del UIElement.__instances[topLevelUI]

	@staticmethod
	def __invokeCallback( topLevelUI, callbackIndex, *args ) :

		callback = UIElement.__instances[topLevelUI].callbacks[callbackIndex]
		callback( *args )

from ParameterUI import ParameterUI


import maya.cmds as cmds
import maya.mel
import maya.OpenMaya

import IECore
import IECoreMaya

## Base class for objects which are able to create an Attribute Editor widget for a single IECore.Parameter
# held on an IECoreMaya.ParameterisedHolder node.
# \todo Separate control drawing from labelling and layout, so these classes just create the right
# hand side of what they're doing at the moment. Then we can use them in different layouts like spreadsheets
# and wotnot.
class ParameterUI( IECoreMaya.UIElement ) :

	textColumnWidthIndex = 145
	singleWidgetWidthIndex = 70
	sliderWidgetWidthIndex = 2 * 70

	handlers = {}

	## The parameterisedHolderNode is an MObject specifying the node holding the specified IECore.Parameter.
	# Derived class __init__ implementations must create a layout to hold all their contents and pass this
	# in the topLevelUI parameter (as for all UIElement derived classes).
	# \todo Document the meaning of the various keyword arguments - perhaps the names of these should be
	# prefixed with the name of the class which implements each argument so as to make it easier to find
	# the documentation too.
	def __init__( self, parameterisedHolderNode, parameter, topLevelUI, **kw ) :

		IECoreMaya.UIElement.__init__( self, topLevelUI )

		self.__node = maya.OpenMaya.MObjectHandle( parameterisedHolderNode )
		self.parameter = parameter #IECore.Parameter

		self.__labelWithNodeName = kw.get( "labelWithNodeName", False )
		self.__longParameterName = kw.get( "longParameterName", parameter.name )

	## Derived classes should override this method. The override should first call the base class method and
	# then reconnect all created widgets to the new node/parameter. The node and parameter arguments are as
	# for the __init__ function.
	def replace( self, node, parameter ) :

		self.__node = maya.OpenMaya.MObjectHandle( node )
		self.parameter = parameter

	## Returns the Maya node associated with this UI in the form of an OpenMaya.MObject
	def node( self ) :
		
		if not self.__node.isValid() :
			raise RuntimeError, "IECoreMaya.ParameterUI.node(): The requested node is not valid"
		
		return self.__node.object()

	## Returns an umambiguous full path for the Maya node associated with this UI.
	def nodeName( self ) :

		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
		return fnPH.fullPathName()

	## Returns the Maya plug associated with this UI in the form an OpenMaya.MPlug
	def plug( self ) :

		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
		return fnPH.parameterPlug( self.parameter )

	## Returns an unambiguous full path to the plug this ui represents.
	def plugName( self ) :

		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
		plug = fnPH.parameterPlug( self.parameter )
		return str( fnPH.fullPathName() + "." + plug.partialName() )

	def layout( self ) :

		return self._topLevelUI()

	## Computes a nice label for the ui.
	def label( self ):

		if self.__labelWithNodeName :

			n = self.nodeName() + "." + self.__longParameterName
			if not self.__longParameterName :
				# Top-level parameter comes through into here without a name
				n = self.nodeName() + ".parameters"

			return IECoreMaya.mel( "interToUI(\"" + n + "\")" ).value

		else :

			return IECoreMaya.mel( "interToUI(\"" + self.parameter.name + "\")" ).value

	## Computes a wrapped annotation/tooltip for the ui
	def description( self ):
		
		extended = "%s\n\n%s" % ( self.plugName().split(".")[1], self.parameter.description )
		return IECore.StringUtil.wrap( extended, 48 )

	@staticmethod
	def _defaultDragCallback( dragControl, x, y, modifiers, **kw ):

		# Pass the dictionary of arguments as a string so that it can be captured and eval'ed in the drop callback
		return [ 'ParameterUI', repr( kw ) ]

	def addDragCallback( self, ctrl, **kw ) :

		maya.cmds.control(
			ctrl,
			edit = True,
			dragCallback = IECore.curry( ParameterUI._defaultDragCallback, nodeName = self.nodeName(), layoutName = self.layout(), **kw )
		)

	## Can be called by derived classes to add a useful popup menu to the specified ui element. This
	# will replace any existing popup menus that are already there.
	## \todo Understand and document the available keyword arguments. I think the only one is "attributeName",
	# which is used to allow the name of specific elements of compound plugs to be specified to improve the box
	# and vector uis. That needs rethinking in any case, as we shouldn't be storing attribute names anywhere as it
	# makes us vulnerable to the names changing behind our backs.
	def _addPopupMenu( self, parentUI, **kw ) :

		existingMenus = maya.cmds.control( parentUI, query=True, popupMenuArray=True )
		if existingMenus :
			for m in existingMenus :
				maya.cmds.deleteUI( m, menu=True )

		IECoreMaya.createMenu( definition = IECore.curry( self.__popupMenuDefinition, **kw ), parent = parentUI, useInterToUI=False )
		
		if "button1" in kw and kw["button1"] :
			IECoreMaya.createMenu( definition = IECore.curry( self.__popupMenuDefinition, **kw ), parent = parentUI, button = 1, useInterToUI=False )

	## Returns an IECore.MenuDefinition used to create a popup menu for the ParameterUI. This may
	# be overridden by derived classes to add their own menu items. In this case they should first
	# call the base class implementation before adding their items to the result.
	def _popupMenuDefinition( self, **kw ) :

		definition = IECore.MenuDefinition()

		if cmds.getAttr( kw['attributeName'], lock = True) == 0:

			settable = maya.cmds.getAttr( kw["attributeName"], settable=True )
			if settable :

				# make menu items for all presets and for the default value

				for k in self.parameter.presetNames() :
					definition.append( "/" + k, { "command" : IECore.curry( self.__selectValue, selection = k ) } )					

				if len( self.parameter.presetNames() ) > 0 :
					definition.append( "/PresetDivider", { "divider" : True } )

				definition.append( "/Default", { "command" : IECore.curry( self.__selectValue, selection = self.parameter.defaultValue ) } )
				
				definition.append( "/ValueDivider", { "divider" : True } )

			attrType = cmds.getAttr( kw["attributeName"], type=True )
			if attrType in ( "float", "long" ) :

				if cmds.getAttr( kw['attributeName'], keyable=True) and settable :
					definition.append( "/Set Key", { "command" : IECore.curry( self.__setKey, **kw ) } )
					
				expressions = cmds.listConnections(
					kw['attributeName'],
					d = False,
					s = True,
					type = "expression"
				)

				if not expressions :

					hasConnections = self.__appendConnectionMenuDefinitions( definition, **kw )
					if not hasConnections and settable :
						definition.append( "/Create New Expression...", { "command" : IECore.curry( self.__expressionEditor, **kw ) } )

				else:

					definition.append( "/Edit Expression...", { "command" : IECore.curry( self.__expressionEditor, **kw ) } )
					definition.append( "/Delete Expression", { "command" : IECore.curry( self.__deleteNode, nodeName = expressions[0] ) } )

			else :

				self.__appendConnectionMenuDefinitions( definition, **kw )

			definition.append( "/ConnectionDivider", { "divider" : True } )

			definition.append( "/Lock Attribute", { "command" : IECore.curry( self.__lock, **kw ) } )

		else :

			definition.append( "/Unlock Attribute", { "command" : IECore.curry( self.__unlock, **kw ) } )

		return definition

	def __appendConnectionMenuDefinitions( self, definition, **kw ) :

		connections = cmds.listConnections(
			kw['attributeName'],
			d = False,
			s = True,
			plugs = True,
			connections = True,
			skipConversionNodes = True
		)
		
		definition.append( "/Connection Editor...", { "command" : IECore.curry( self.__connectionEditor ) } )

		if connections :
			
			definition.append( "/Open AE...",
				{ "command" : IECore.curry( self.__showEditor, attributeName = connections[1] ) }
			)				

			definition.append( "/Break Connection",
				{ 
					"command" : IECore.curry(
						self.__disconnect,
						source = connections[1],
						destination = connections[0],
						refreshAE = self.nodeName()
					)
				}
			)
						
			return True

		else:

			return False
	
	def __popupMenuDefinition( self, **kw ) :
	
		# call the protected function which can be overridden by
		# derived classes. then let the callbacks do what they want.
		definition = self._popupMenuDefinition( **kw )	
		for cb in self.__popupMenuCallbacks :
			cb( definition, self.parameter, self.node() )

		return definition

	def __showEditor( self, attributeName ) :

		split = attributeName.split('.', 1 )
		node = split[0]

		melCmd = 'showEditor "' + node + '"'

		IECoreMaya.mel( melCmd.encode('ascii') )

	def __deleteNode( self, nodeName = None ) :

		cmds.delete( nodeName )

	def __expressionEditor( self, attributeName = None ) :

		split = attributeName.split('.', 1 )
		node = split[0]
		attr = split[1]

		melCmd = 'expressionEditor EE "' + node + '" "' + attr + '"'

		IECoreMaya.mel( melCmd.encode('ascii') )

	def __connectionEditor( self ) :
	
		maya.mel.eval(
				str("ConnectionEditor;"+
				"nodeOutliner -e -replace %(right)s connectWindow|tl|cwForm|connectWindowPane|rightSideCW;"+
				"connectWindowSetRightLabel %(right)s;") % { 'right' : self.nodeName() } )

	def __disconnect( self, source = None, destination = None, refreshAE = None ) :

		cmds.disconnectAttr( source, destination )
		
		if refreshAE :
			maya.mel.eval( 'evalDeferred( "updateAE %s;")' % refreshAE )

	def __setKey( self, **kw ):

		cmds.setKeyframe(
			kw['attributeName']
		)

	def __lock( self, **kw ):

		cmds.setAttr(
			kw['attributeName'],
			lock = True
		)

	def __unlock( self, **kw  ):

		cmds.setAttr(
			kw['attributeName'],
			lock = False
		)

	def __selectValue( self, selection = None):

		self.parameter.setValue( selection )
		IECoreMaya.FnParameterisedHolder( self.node() ).setNodeValue( self.parameter )

	@staticmethod
	def registerUI( parameterTypeId, handlerType, uiTypeHint = None ):

		key = (parameterTypeId, uiTypeHint)
		if key in ParameterUI.handlers :
			IECore.msg( IECore.Msg.Level.Warning, "ParameterUI.registerUI", "Handler for %s already registered." % str( key ) )

		ParameterUI.handlers[key] = handlerType

	## Returns a new ParameterUI instance suitable for representing
	# the specified parameter on the specified parameterisedHolderNode.
	# The node may either be specified as an OpenMaya.MObject or as
	# a string or unicode object representing the node name.
	@staticmethod
	def create( parameterisedHolderNode, parameter, **kw ) :

		if not isinstance( parameterisedHolderNode, maya.OpenMaya.MObject ) :
			parameterisedHolderNode = IECoreMaya.StringUtil.dependencyNodeFromString( parameterisedHolderNode )

		if not parameter.isInstanceOf( IECore.Parameter.staticTypeId() ) :
			raise TypeError( "Parameter argument must derive from IECore.Parameter." )

		if parameter.presetsOnly and len( parameter.presets() ) :
			return IECoreMaya.PresetsOnlyParameterUI( parameterisedHolderNode, parameter, **kw )

		uiTypeHint = None
		try:
			uiTypeHint = parameter.userData()['UI']['typeHint'].value
		except:
			pass

		handlerType = None
		typeId = parameter.typeId()
		while typeId!=IECore.TypeId.Invalid :
			handlerType = ParameterUI.handlers.get( ( typeId, uiTypeHint ), None )
			if handlerType is not None :
				break
			handlerType = ParameterUI.handlers.get( ( typeId, None ), None )
			if handlerType is not None :
				break
			typeId = IECore.RunTimeTyped.baseTypeId( typeId )
				
		if handlerType is None :
			IECore.msg( IECore.Msg.Level.Warning, "ParameterUI.create", "No UI registered for parameters of type \"%s\"" % parameter.typeName() )
			return None

		if 'longParameterName' in kw and len( kw['longParameterName'] ) :
			kw['longParameterName'] += "." + parameter.name
		else :
			kw['longParameterName'] = parameter.name


		parameterUI = handlerType( parameterisedHolderNode, parameter, **kw )

		return parameterUI

	__popupMenuCallbacks = []
	## Registers a callback which is able to modify the popup menus associated
	# with ParameterUIs. The callback should have the following signature :
	#
	# callback( menuDefinition, parameter, holderNode ).
	@classmethod
	def registerPopupMenuCallback( cls, callback ) :
	
		cls.__popupMenuCallbacks.append( callback )
 		

from BoolParameterUI import BoolParameterUI


import maya.cmds

import IECore
import IECoreMaya

class BoolParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
		
		IECoreMaya.ParameterUI.__init__(
			
			self,
			node,
			parameter,
			maya.cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex * 3 ]
			),
			**kw
		
		)

		maya.cmds.text(
			label = "",
		)

		self.__checkBox = maya.cmds.checkBox(
			annotation = self.description(),
			label = self.label(),
			align = "left",
		)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		maya.cmds.connectControl( self.__checkBox, self.plugName() )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.BoolParameter, BoolParameterUI )

from StringParameterUI import StringParameterUI


from __future__ import with_statement

import fnmatch
import re

import maya.cmds

import IECore
import IECoreMaya

## A UI for StringParameters. Supports the following parameter user data :
#
# BoolData ["UI"]["acceptsProceduralObjectName"] False
# When true, menu items will be created to set the parameter value to the name
# of an object in a procedural.
#
# BoolData ["UI"]["acceptsProceduralObjectNames"] False
# When true, menu items will be created to add and remove selected procedural
# components names.
#
# BoolData ["UI"]["acceptsCoordinateSystemName"] False
# When true, menu items will be created to set the parameter value to the name
# of a coordinate system in a procedural.
#
# BoolData ["UI"]["acceptsNodeName"] False
# When true, menu items will be created to set the value to the name of
# a node in the scene.
#
# BoolData ["UI"]["acceptsNodeNames"] False
# When true, menu items will be created to set the value to the names of
# a number of nodes in the scene.
#
# StringVectorData ["UI"]["acceptedNodeTypes"] []
# A list of node types to be considered by the "acceptsNodeName" and
# "acceptsNodeNames" features.
#
# StringData ["UI"]["acceptedNodeNameFormat"] "partial"
# Specifies either "partial" or "full", to define whether the shortest
# unique node name will be used for the features above, or whether
# the full path will be used. Specify "parent" or "parentPartial" or
# "parentFull" to get the direct transform parent node path. "parent"
# behaves the same as "parentPartial".
class StringParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ):

		IECoreMaya.ParameterUI.__init__( self, node, parameter, maya.cmds.rowLayout( numberOfColumns = 2 ), **kw )

		self.__label = maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__textField = maya.cmds.textField()

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		maya.cmds.connectControl( self.__textField, self.plugName() )
		self._addPopupMenu( parentUI=self.__textField, attributeName = self.plugName() )

		# The popup on the text field itself seems not to be working, so also add it to the
		# label in the mean time.
		self._addPopupMenu( parentUI=self.__label, attributeName = self.plugName(), button1 = True )

	def _popupMenuDefinition( self, **kw ) :

		definition = IECoreMaya.ParameterUI._popupMenuDefinition( self, **kw )

		if not maya.cmds.getAttr( self.plugName(), settable=True ) :
			return definition

		wantsComponentName = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsComponentName = self.parameter.userData()["UI"]["acceptsProceduralObjectName"].value
			
		wantsComponentNames = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsComponentNames = self.parameter.userData()["UI"]["acceptsProceduralObjectNames"].value

		if wantsComponentName or wantsComponentNames  :

			definition.append( "/ObjectsDivider", { "divider" : True } )
			definition.append( "/Objects/Set To Selected", { "command" : IECore.curry( self.__setToSelectedComponents, not wantsComponentNames ) } )
			if wantsComponentNames :
				definition.append( "/Objects/Add Selected", { "command" : self.__addSelectedComponents } )
				definition.append( "/Objects/Remove Selected", { "command" : self.__removeSelectedComponents } )
			
			definition.append( "/Objects/Select", { "command" : self.__selectComponents } )

		wantsNodeName = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsNodeName = self.parameter.userData()["UI"]["acceptsNodeName"].value

		wantsNodeNames = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsNodeNames = self.parameter.userData()["UI"]["acceptsNodeNames"].value

		if wantsNodeName or wantsNodeNames :

			lskw = {}
			with IECore.IgnoredExceptions( KeyError ) :
				lskw["type"] = list( self.parameter.userData()["UI"]["acceptedNodeTypes"] )
			with IECore.IgnoredExceptions( KeyError ) :
				if self.parameter.userData()["UI"]["acceptedNodeNameFormat"].value == "full" :
					lskw["long"] = True

			nodeNames = maya.cmds.ls( **lskw )
			with IECore.IgnoredExceptions( KeyError ) :
				if "parent" in self.parameter.userData()["UI"]["acceptedNodeNameFormat"].value :
					for i in range( len( nodeNames ) ) :
						nodeNames[i] = maya.cmds.listRelatives(
							nodeNames[i], parent=True, path=True,
						 	fullPath = (self.parameter.userData()["UI"]["acceptedNodeNameFormat"].value == "parentFull")
						)[0]

			if nodeNames :

				definition.append( "/NodesDivider", { "divider" : True } )

				if wantsNodeName :

					currentValue = maya.cmds.getAttr( self.plugName() )
					for nodeName in nodeNames :
						if nodeName!=currentValue :
							definition.append( "/Nodes/%s" % nodeName, { "command" : IECore.curry( self.__addNodeName, nodeName, clearFirst=True ) } )

				elif wantsNodeNames :

					currentNodes = set( maya.cmds.getAttr( self.plugName() ).split() )
					for nodeName in nodeNames :
						if nodeName in currentNodes :
							definition.append( "/Nodes/Remove/%s" % nodeName, { "command" : IECore.curry( self.__removeNodeName, nodeName ) } )
						else :
							definition.append( "/Nodes/Add/%s" % nodeName, { "command" : IECore.curry( self.__addNodeName, nodeName ) } )
							
		wantsCoordinateSystem = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsCoordinateSystem = self.parameter.userData()["UI"]["acceptsCoordinateSystemName"].value	

		if wantsCoordinateSystem :
		
			definition.append( "/CoordinateSystemsDivider", { "divider" : True } )
			definition.append( "/Coordinate Systems/Set To Selected", { "command" : self.__setToSelectedCoordinateSystem } )
			definition.append( "/Coordinate Systems/Select", { "command" : self.__selectCoordinateSystem } )

		return definition

	def __setToSelectedComponents( self, oneOnly ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = fnPH.selectedComponentNames()
		components = list( components )
		components.sort()

		if oneOnly :
			maya.cmds.setAttr( self.plugName(), components[0] if components else "", type="string" )
		else :
			maya.cmds.setAttr( self.plugName(), " ".join( components ), type="string" )

	def __addSelectedComponents( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = fnPH.selectedComponentNames()
		components |= set( maya.cmds.getAttr( self.plugName() ).split() )
		components = list( components )
		components.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( components ), type="string" )

	def __removeSelectedComponents( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = set( maya.cmds.getAttr( self.plugName() ).split() )
		components -= fnPH.selectedComponentNames()
		components = list( components )
		components.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( components ), type="string" )

	def __selectComponents( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		
		regexes = [ re.compile( fnmatch.translate( x ) ) for x in maya.cmds.getAttr( self.plugName() ).split() ]
		
		toSelect = set()
		for name in fnPH.componentNames() :
			for r in regexes :
				if r.match( name ) is not None :
					toSelect.add( name )
					break
		
		fnPH.selectComponentNames( toSelect )

	def __setToSelectedCoordinateSystem( self ) :
	
		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = fnPH.selectedComponentNames()
		components = list( components )
		components = [ c for c in components if c.startswith( "coordinateSystem:" ) ]
		
		coordSys = ""
		if components :
			components.sort()
			coordSys = components[0][len( "coordinateSystem:" ):]
			
		maya.cmds.setAttr( self.plugName(), coordSys, type="string" )
		
	def __selectCoordinateSystem( self ) :
	
		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		fnPH.selectComponentNames( set( [ "coordinateSystem:" + maya.cmds.getAttr( self.plugName() ) ] ) )

	def __addNodeName( self, nodeName, clearFirst=False ) :

		names = set()
		if not clearFirst :
			names = set( maya.cmds.getAttr( self.plugName() ).split() )

		names.add( nodeName )
		names = list( names )
		names.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( names ), type="string" )

	def __removeNodeName( self, nodeName ) :

		names = set( maya.cmds.getAttr( self.plugName() ).split() )
		names.remove( nodeName )
		names = list( names )
		names.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( names ), type="string" )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.StringParameter, StringParameterUI )

from PathParameterUI import PathParameterUI


import os.path
import warnings

import maya.cmds

import IECore
import IECoreMaya

class PathParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		IECoreMaya.ParameterUI.__init__(
			
			self,
			node,
			parameter,
			maya.cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex * 3, 26 ]
			),
			**kw
		
		)

		self.__label = maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__textField = maya.cmds.textField()

		maya.cmds.iconTextButton(
			label = "",
			image = "fileOpen.xpm",
			command = self._fileDialog,
			height = 23,
			style = "iconOnly"
		)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )
		# We can't see the menu if its on the field...
		self._addPopupMenu( parentUI=self.__label, attributeName = self.plugName() )
		maya.cmds.connectControl( self.__textField, self.plugName() )

	## \deprecated
	def openDialog( self ) :
		
		warnings.warn( "PathParameterUI.openDialog() is deprecated, please implement _filePicker() instead if you need to customise the dialog/returned path.", DeprecationWarning, 2 )
		
		dialogPath = self._initialPath()
		
		dialogPath = os.path.expandvars( dialogPath )
		dialogPath = os.path.join( dialogPath, '*' )
		
		selection = maya.cmds.fileDialog( directoryMask=dialogPath ).encode('ascii')
		
		return selection
	
	## This can be implemented in derived classes to show a customised file picker.
	## Typically, you would call the base class passing additional kw arguments,
	## which are passed to the FileDialog call. If omitted, "path", "key" and
	## "callback" will be set to appropriate values depending on the parameter.
	def _fileDialog( self, **kw ) :
		
		if not kw:
	
			## \todo Remove at some suitable point, once we're happy no one is relying on
			## a custom openDialog implementation to achieve their desired functionality.
			warnings.warn( "PathParameterUI.openDialog() is deprecated, please implement _filePicker() instead if you need to customise the dialog/returned path.", DeprecationWarning, 2 )
			self.openDialog()
	
		else:
		
			# Allow a class to enforce a path if the default behaviour for
			# using existing paths or the default userData path is not desired.
			if "path" not in kw:
				kw["path"] = self._initialPath()

			# Allow the parameter userData to override the key from the caller.
			# This is a little different in concept to the path, but they key is
			# never dynamic in terms of the parameter, so I think it makes sense.
			uiUserData = self.parameter.userData().get( 'UI', {} )
			key = uiUserData.get( 'fileDialogKey', IECore.StringData() ).value
			if key:
				kw["key"] = key

			if "callback" not in kw:
				kw["callback"] = self.__defaultFileDialogCallback

			IECoreMaya.FileDialog( **kw )	
	
	# Simply sets the parameter value
	def __defaultFileDialogCallback( self, selection ) :
		
		if selection:

			self.parameter.setValue( IECore.StringData( selection[0] ) )
			fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
			fnPH.setNodeValue( self.parameter )
	
	## Returns the initial path for a FileDialog, implied by the current
	## parameter value, and the status of the following userData["UI"] entries:
	##  - defaultPath (IECore.StringData()) A path to use as a default.
	##  - obeyDefaultPath (IECore.BoolData()) If True, the default path will
	##     be used, regardless of the curent parameter value. Otherwise, the
	##     parent directory of the current path is used.
	def _initialPath( self, parameter=None ) :
		
		if not parameter:
			parameter = self.parameter
		
		uiUserData = parameter.userData().get( 'UI', {} )
		dialogPath = uiUserData.get( 'defaultPath', IECore.StringData() ).value
		obeyDefaultPath = uiUserData.get( 'obeyDefaultPath', IECore.BoolData( False ) ).value
		currentPath = parameter.getTypedValue()
		
		if currentPath and not obeyDefaultPath :
			dialogPath = os.path.dirname( currentPath )
			
		return dialogPath
		
		

from FileNameParameterUI import FileNameParameterUI


import IECore
import IECoreMaya

class FileNameParameterUI( IECoreMaya.PathParameterUI ) :

	def __init__( self, node, parameter, **kw ):

		IECoreMaya.PathParameterUI.__init__( self, node, parameter, **kw )

	def _fileDialog( self ) :
		
		tools = IECoreMaya.FileBrowser.FileExtensionFilter( self.parameter.extensions )

		IECoreMaya.PathParameterUI._fileDialog( self, 
			filter = tools.filter,
			validate = tools.validate
		)

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.FileNameParameter, FileNameParameterUI )

from DirNameParameterUI import DirNameParameterUI


import os.path

import IECore
import IECoreMaya

class DirNameParameterUI( IECoreMaya.PathParameterUI ) :

	def __init__( self, node, parameter, **kw ):

		IECoreMaya.PathParameterUI.__init__( self, node, parameter, **kw )

	def _fileDialog( self ) :
		
		IECoreMaya.PathParameterUI._fileDialog( self,
			filter = IECoreMaya.FileBrowser.DirectoriesOnlyFilter().filter,
		)

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.DirNameParameter, DirNameParameterUI )

from FileSequenceParameterUI import FileSequenceParameterUI


import os.path

import IECore
import IECoreMaya

class FileSequenceParameterUI( IECoreMaya.PathParameterUI ) :

	def __init__( self, node, parameter, **kw ):

		IECoreMaya.PathParameterUI.__init__( self, node, parameter, **kw )

	def _fileDialog( self ) :

		tools = FileSequenceParameterUI.FileSequenceFilter( self.parameter.extensions )

		IECoreMaya.PathParameterUI._fileDialog( self,
			filter = tools.filter,
			validate = tools.validate,
		)
		
	class FileSequenceFilter :

		def __init__( self, extensions=None ) :
		
			if extensions:
				self.__extensions = IECore.StringVectorData( extensions )
			else:
				self.__extensions = IECore.StringVectorData()

		def filter( self, path, items ) :
		
			fsOp = IECore.SequenceLsOp()
		
			oldItems = list( items )
			del items[:]
			
			for i in oldItems:
				if os.path.isdir( i["path"] ) :
					items.append( i )
										
			sequences = fsOp( 
				dir=path,
				type="files", 
				resultType="stringVector", 
				format="<PREFIX><#PADDING><SUFFIX> <FRAMES>",
				extensions=self.__extensions,
			)

			for s in sequences :
				
				firstFrame = IECore.FileSequence( s ).fileNames()[0]
				stat = os.stat( firstFrame )		
				
				seqItem = {
					"path" : s,
					"name" : s.replace( "%s/" % path, "" ),
					"mode" :  stat[0],
					"uid" :  stat[4],
					"gid" :  stat[5],
					"size" :  stat[6],
					"atime" :  stat[7],
					"mtime" :  stat[8],
					"ctime" :  stat[9],
				}
			
				items.append( seqItem )
	
		# FileExtensionFilter will get confused by the extra info on
		# the end of the sequence string.
		def validate( self, path, items ):
			
			if not items:
				return False
				
			for i in items:
				if os.path.isdir( "%s/%s" % ( path, i["name"] ) ) :
					return False
				
			return True
				
		
			
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.FileSequenceParameter, FileSequenceParameterUI )

from NumericParameterUI import NumericParameterUI


import maya.cmds

import IECore
import IECoreMaya

class NumericParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		if parameter.hasMinValue() and parameter.hasMaxValue():

			layout = maya.cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.sliderWidgetWidthIndex ]
			)

		else:

			layout = maya.cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
			)
			
		IECoreMaya.ParameterUI.__init__( self, node, parameter, layout, **kw )

		self.__field = None
		self.__slider = None

		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description()
		)

		kw = {}

		if parameter.hasMinValue():
			kw['minValue'] = parameter.minValue

		if parameter.hasMaxValue():
			kw['maxValue'] = parameter.maxValue


		# \todo Add a way of overriding precision for both float and double parameters, giving
		# each a sensible (and probably different) default
		if self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ) :

			kw['precision'] = 12

		self.__field = self.__fieldType()(
			value = parameter.getNumericValue(),
			**kw
		)

		if parameter.hasMinValue() and parameter.hasMaxValue():

			self.__slider = self.__sliderType()(
				minValue = parameter.minValue,
				maxValue = parameter.maxValue,

				value = parameter.getNumericValue(),
			)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		maya.cmds.connectControl( self.__field, self.plugName() )
		self._addPopupMenu( parentUI = self.__field, attributeName = self.plugName() )

		if self.__slider:
			maya.cmds.connectControl( self.__slider, self.plugName() )

	def __sliderType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.FloatParameter ) or self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ):
			return maya.cmds.floatSlider
		elif self.parameter.isInstanceOf( IECore.TypeId.IntParameter ):
			return maya.cmds.intSlider
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")

	def __fieldType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.FloatParameter ) or self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ):
			return maya.cmds.floatField
		elif self.parameter.isInstanceOf( IECore.TypeId.IntParameter ):
			return maya.cmds.intField
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")


IECoreMaya.ParameterUI.registerUI( IECore.TypeId.FloatParameter, NumericParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.DoubleParameter, NumericParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.IntParameter, NumericParameterUI )

from VectorParameterUI import VectorParameterUI


import maya.cmds

import IECore
import IECoreMaya

class VectorParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		self.__dim = parameter.getTypedValue().dimensions()
		if self.__dim == 2:
			layout = maya.cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
			)
		elif self.__dim == 3:
			layout = maya.cmds.rowLayout(
				numberOfColumns = 4,
				columnWidth4 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
			)
		else:
			raise RuntimeError("Unsupported vector dimension in VectorParameterUI")
		
		IECoreMaya.ParameterUI.__init__( self, node, parameter, layout, **kw )

		self.__fields = []

		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description()
		)

		plug = self.plug()
		for i in range(0, self.__dim) :
			self.__fields.append(
				self.__fieldType()(
					value = parameter.getTypedValue()[i]
				)
			)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		plug = self.plug()
		for i in range(0, self.__dim):

			childPlugName = self.nodeName() + "." + plug.child(i).partialName()
			maya.cmds.connectControl( self.__fields[i], childPlugName )
			self._addPopupMenu( parentUI = self.__fields[i], attributeName = childPlugName )

	def __fieldType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.V2iParameter ) or self.parameter.isInstanceOf( IECore.TypeId.V3iParameter ):
			return maya.cmds.intField
		else:
			return maya.cmds.floatField

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V2iParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V3iParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V2fParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V2dParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V3fParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V3dParameter, VectorParameterUI )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Color3fParameter, VectorParameterUI, "numeric" )

from ColorParameterUI import ColorParameterUI


import maya.cmds

import IECore
import IECoreMaya

class ColorParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		IECoreMaya.ParameterUI.__init__(
			
			self,
			node,
			parameter,
			maya.cmds.attrColorSliderGrp(),
			**kw
			
		)

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		maya.cmds.attrColorSliderGrp(
			self._topLevelUI(),
			edit = True,
			attribute = self.plugName(),
			annotation = self.description(),
			label = self.label(),
			showButton = False,
		)

		self._addPopupMenu( parentUI = self._topLevelUI(), attributeName = self.plugName() )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Color3fParameter, ColorParameterUI )

from BoxParameterUI import BoxParameterUI


import maya.cmds

import IECore
import IECoreMaya

class BoxParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		IECoreMaya.ParameterUI.__init__( self, node, parameter, maya.cmds.columnLayout(), **kw )

		self.__fields = []

		self.__dim = parameter.getTypedValue().dimensions()

		plug = self.plug()
		for childIndex in range( 0, 2 ) :

			if self.__dim == 2:
				maya.cmds.rowLayout(
					numberOfColumns = 3,
					columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
				)
			elif self.__dim == 3:
				maya.cmds.rowLayout(
					numberOfColumns = 4,
					columnWidth4 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
				)
			else:
				raise RuntimeError("Unsupported vector dimension in BoxParameterUI")

			parameterLabel = self.label()
			if childIndex==0 :
				parameterLabel = parameterLabel + "Min"
			else:
				parameterLabel = parameterLabel + "Max"

			maya.cmds.text(
				label = parameterLabel,
				font = "smallPlainLabelFont",
				align = "right",
				annotation = self.description()
			)

			vectorPlug = plug.child( childIndex )

			for i in range( 0, self.__dim ) :

				self.__fields.append( self.__fieldType()() )

			maya.cmds.setParent("..")

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		fieldNum = 0
		plug = self.plug()
		for childIndex in range( 0, 2 ) :

			vectorPlug = plug.child( childIndex )
			for i in range( 0, self.__dim ) :

				vectorPlugChild = vectorPlug.child( i )
				vectorPlugChildName = self.nodeName() + "." + vectorPlugChild.partialName()
				maya.cmds.connectControl( self.__fields[ fieldNum ], vectorPlugChildName )
				self._addPopupMenu( parentUI = self.__fields[fieldNum], attributeName = vectorPlugChildName )

				fieldNum += 1

	def __fieldType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.Box2iParameter ) or self.parameter.isInstanceOf( IECore.TypeId.Box3iParameter ):
			return maya.cmds.intField
		else:
			return maya.cmds.floatField

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Box2iParameter, BoxParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Box2fParameter, BoxParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Box2dParameter, BoxParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Box3iParameter, BoxParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Box3fParameter, BoxParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Box3dParameter, BoxParameterUI )



from SplineParameterUI import SplineParameterUI


import IECore
import IECoreMaya
import maya.cmds
from ParameterUI import ParameterUI

class SplineParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		ParameterUI.__init__(
			
			self,
			node,
			parameter,
			maya.cmds.rowLayout(
				numberOfColumns = 3,
				rowAttach = [ ( 1, "top", 0 ), ( 2, "both", 0 ), ( 3, "both", 0 ) ]
			),
			**kw
			
		)

		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__gradientControl = maya.cmds.gradientControl()
		self.__button = maya.cmds.button( label = ">")
		self.__editWindow = None

		self.replace( node, parameter )

	def replace( self, node, parameter ) :

		ParameterUI.replace( self, node, parameter )
		maya.cmds.gradientControl( self.__gradientControl, edit=True, attribute=self.plugName() )
		maya.cmds.button( self.__button, edit=True, command=self.__openEditWindow )
		self.__editWindow = None

	## Returns True if we're a color ramp and False if we're a greyscale curve.
	def __colored( self ) :

		plugName = self.plugName()
		attrName = plugName.split( "." )[-1]
		return maya.cmds.objExists( plugName + "[0]." + attrName + "_ColorR" )

	def __openEditWindow( self, unused ) :

		if not self.__editWindow :

			self.__editWindow = maya.cmds.window( self.nodeName() + " " + self.label(), retain=True, widthHeight=[ 600, 300 ] )

			layout = maya.cmds.formLayout()

			positionControl = maya.cmds.attrFieldSliderGrp( label = "Selected position", columnWidth=[ ( 1, 100 ) ] )

			if self.__colored() :
				valueControl = maya.cmds.attrColorSliderGrp( label = "Selected colour", showButton=False, columnWidth=[ ( 1, 90 ) ] )
			else :
				valueControl = maya.cmds.attrFieldSliderGrp( label = "Selected value", columnWidth=[ ( 1, 90 ) ] )

			gradientControl = maya.cmds.gradientControl(
				attribute=self.plugName(),
				selectedColorControl=valueControl,
				selectedPositionControl=positionControl
			)

			maya.cmds.formLayout( layout,
				edit=True,
				attachForm = [
					( positionControl, "left", 5 ),
					( positionControl, "bottom", 15 ),
					( valueControl, "bottom", 15 ),
					( gradientControl, "top", 5 ),
					( gradientControl, "left", 5 ),
					( gradientControl, "right", 5 ),
				],
				attachControl = [
					( gradientControl, "bottom", 5, positionControl ),
					( valueControl, "left", 5, positionControl ),
				]
			)
		maya.cmds.showWindow( self.__editWindow )

ParameterUI.registerUI( IECore.TypeId.SplinefColor3fParameter, SplineParameterUI )
ParameterUI.registerUI( IECore.TypeId.SplinefColor4fParameter, SplineParameterUI )
ParameterUI.registerUI( IECore.TypeId.SplineffParameter, SplineParameterUI )
ParameterUI.registerUI( IECore.TypeId.SplineddParameter, SplineParameterUI )

from NoteParameterUI import NoteParameterUI


import maya.cmds

import IECore
import IECoreMaya

## A UI for a StringParameters that uses the "note" type hint.
#
# This is for displaying user annotations

class NoteParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ):
		
		
		IECoreMaya.ParameterUI.__init__( self, node, parameter, maya.cmds.rowLayout( numberOfColumns = 3, columnWidth3 = [ 30, 300, 30 ] ), **kw )
		
		self.__editWindow = ""
		
		maya.cmds.text( label = "" )
		
		self.__label = maya.cmds.text(
			font = "smallPlainLabelFont",
			align = "left",
			annotation = "Notes",
		)
		
		maya.cmds.iconTextButton(
			label = "",
			image = "ie_editTextIcon.xpm",
			command = self._launchEditWindow,
			height = 23,
			style = "iconOnly"
		)
		
		maya.cmds.setParent("..")
		
		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )
		
		maya.cmds.text(
			self.__label,
			e = True,
			label = str( parameter.getValue() ),
		)
		
	def _launchEditWindow( self ):
		
		if not maya.cmds.window( self.__editWindow, exists=True ):
			
			self.__editWindow = maya.cmds.window(
				title = "Edit Annotation",
				iconName = "Edit Annotation",
				width = 400,
				height = 500
			)
			
			form = maya.cmds.formLayout(numberOfDivisions=100)
			
			button = maya.cmds.button(
				label='Edit',
				command = self._editText,
			)
		
			self.__textField = maya.cmds.scrollField(
				editable = True,
				height = 50,
				wordWrap = True,
				text = str( self.parameter.getValue() ),
			)
			
			maya.cmds.formLayout(
				form,
				edit=True,
				attachForm=[(self.__textField, 'top', 5), (self.__textField, 'left', 5), (self.__textField, 'right', 5), (button, 'left', 5), (button, 'bottom', 5), (button, 'right', 5) ],
				attachControl=[ ( self.__textField, 'bottom', 5, button ) ],
				#attachPosition=[ ( self.__textField, 'right', 5, 75 ) ],
				attachNone=(button, 'top'),
			)
		
			maya.cmds.showWindow( self.__editWindow )
	
	def _editText( self, state ):
		
		newStr = maya.cmds.scrollField(
			self.__textField,
			q = True,
			text = True,
		)
		
		self.parameter.setValue( IECore.StringData( newStr ) )
		
		maya.cmds.deleteUI( self.__editWindow , window=True )
		
		maya.cmds.text(
			self.__label,
			e = True,
			label = str( self.parameter.getValue() ),
		)
		
		
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.StringParameter, NoteParameterUI, "note" )

from NodeParameter import NodeParameter


import IECore
from maya.OpenMaya import *
import re

"""
Parameter class for specifying Maya dependency nodes.
"""
class NodeParameter( IECore.StringParameter ):

	## \todo Use Enum for this
	class CheckType:

		DontCare = 0
		MustExist = 1
		MustNotExist = 2

	"""
	name - name of the parameter.
	description - description of the parameter.
	allowEmptyString - boolean that will accept the empty value even if the node should exist.
	check - can be CheckType.DontCare, CheckType.MustExist or CheckType.MustNotExist.
	typeRegex - regular expression used on parameter validation that validates based on the maya node type. Disable filtering using None.
	typeRegexDescription - human readable description for the regular expression used to filter node types. It's used when the validation fails.
	"""
	def __init__( self, name, description, defaultValue = "", allowEmptyString = True,
		check = CheckType.DontCare, typeRegex = None, typeRegexDescription = "", presets = {}, presetsOnly = False, userData = IECore.CompoundObject() ) :

		IECore.StringParameter.__init__( self, name, description, defaultValue, presets, presetsOnly, userData )

		self.allowEmptyString = allowEmptyString
		self.mustExist = bool( check == NodeParameter.CheckType.MustExist )
		self.mustNotExist = bool( check == NodeParameter.CheckType.MustNotExist )
		if typeRegex is None:
			self.typeRegex = None
		else:
			self.typeRegex = re.compile( typeRegex )
			if typeRegexDescription == "":
				self.typeRegexDesc = "Invalid type."
			else:
				self.typeRegexDesc = typeRegexDescription

	"""
	Defines two attributes: mustExist and mustNotExist and allowEmptyString exactly like PathParameter class.
	"""
	def __getattr__( self, attrName ):
		if attrName == "mustExist":
			return self.__mustExist
		elif attrName == "mustNotExist":
			return self.__mustNotExist
		elif attrName == "allowEmptyString":
			return self.__allowEmptyString
		else:
			return self.__dict__[ attrName ]

	"""
	Returns a regular expression that matches only valid Maya dependency nodes
	"""
	@staticmethod
	def pathValidator():
		return re.compile( "^(\|?[^\t\n\r\f\v\|]+)+\|?$" )

	"""
	Returns (True, "") only if the value is a correct dependency nodestring and also checks that the node exists or doesn't exist
	based on the CheckType passed to the constructor.
	Otherwise returns (False, errorMessage).
	"""
	def valueValid( self, value ) :

		v = IECore.StringParameter.valueValid( self, value )
		if not v[0] :
			return v

		if self.allowEmptyString and value.value=="" :
			return True, ""

		if not self.pathValidator().match( value.value ) :
			return False, "Not a valid Maya dependency node."

		list = MSelectionList ()
		try:
			list.add( value.value )
		except:
			exist = False
		else:
			exist = True

			try:
				obj = MObject()
				list.getDependNode( 0, obj )
				depNode = MFnDependencyNode( obj )
			except:
				IECore.debugException("failed to instantiate MObject from", value.value )
				return False, "'%s' is not a dependency node" % value.value

			if not self.typeRegex is None:
				nodeType = str(depNode.typeName())
				if self.typeRegex.match( nodeType ) is None:
					return False, ("Type '%s' not accepted: " % nodeType) + self.typeRegexDesc

		if self.mustExist :

			if not exist:
				return False, "Node %s does not exist" % value.value

		elif self.mustNotExist :

			if exist:
				return False, "Node %s already exists" % value.value

		return True, ""

	"""
	Sets the internal StringData value from the given dependency node MObject
	"""
	def setNodeValue( self, node ) :
		self.setValue( IECore.StringData( MFnDependencyNode( node ).name() ) )

	"""
	Returns an MObject that corresponds to the dependency node of the current value.
	"""
	def getNodeValue( self ) :
		nodeName = self.getValidatedValue().value
		list = MSelectionList ()
		list.add( nodeName )
		obj = MObject()
		list.getDependNode( 0, obj )
		return obj

IECore.registerRunTimeTyped( NodeParameter )

from DAGPathParameter import DAGPathParameter


import IECore
from maya.OpenMaya import *
import re

"""
Parameter class for specifying Maya DAG paths.
"""
## \todo Dervive from IECoreMaya.NodeParameter
class DAGPathParameter( IECore.StringParameter ):

	## \todo Use Enum for this
	class CheckType:

		DontCare = 0
		MustExist = 1
		MustNotExist = 2

	"""
	name - name of the parameter.
	description - description of the parameter.
	allowEmptyString - boolean that will accept the empty value even if the node should exist.
	check - can be CheckType.DontCare, CheckType.MustExist or CheckType.MustNotExist.
	typeRegex - regular expression used on parameter validation that validates based on the maya node type. Disable filtering using None.
	typeRegexDescription - human readable description for the regular expression used to filter node types. It's used when the validation fails.
	"""
	def __init__( self, name, description, defaultValue = "", allowEmptyString = True,
		check = CheckType.DontCare, typeRegex = None, typeRegexDescription = "", presets = (), presetsOnly = False, userData = IECore.CompoundObject() ) :

		IECore.StringParameter.__init__( self, name, description, defaultValue, presets, presetsOnly, userData )

		self.__allowEmptyString = allowEmptyString
		self.__mustExist = bool( check == DAGPathParameter.CheckType.MustExist )
		self.__mustNotExist = bool( check == DAGPathParameter.CheckType.MustNotExist )
		if typeRegex is None:
			self.typeRegex = None
		else:
			self.typeRegex = re.compile( typeRegex )
			if typeRegexDescription == "":
				self.typeRegexDesc = "Invalid type."
			else:
				self.typeRegexDesc = typeRegexDescription

	"""
	Defines two attributes: mustExist and mustNotExist and allowEmptyString exactly like PathParameter class.
	"""
	def __getattr__( self, attrName ):
		if attrName == "mustExist":
			return self.__mustExist
		elif attrName == "mustNotExist":
			return self.__mustNotExist
		elif attrName == "allowEmptyString":
			return self.__allowEmptyString
		else:
			return self.__dict__[ attrName ]

	"""
	Returns a regular expression that matches only valid Maya DAG paths.
	"""
	@staticmethod
	def pathValidator():
		return re.compile( "^(\|?[^\t\n\r\f\v\|]+)+\|?$" )

	"""
	Returns (True, "") only if the value is a correct DAG path string and also checks that the DAG node exists or doesn't exist
	based on the CheckType passed to the constructor.
	Otherwise returns (False, errorMessage).
	"""
	def valueValid( self, value ) :

		v = IECore.StringParameter.valueValid( self, value )
		if not v[0] :
			return v

		if self.allowEmptyString and value.value=="" :
			return True, ""

		if not self.pathValidator().match( value.value ) :
			return False, "Not a valid Maya DAG path."

		list = MSelectionList ()
		try:
			list.add( value.value )
		except:
			exist = False
		else:
			exist = True

			try:
				dp = MDagPath()
				list.getDagPath(0, dp)
				depNode = MFnDagNode( dp )
			except:
				IECore.debugException("failed to instantiate MDagPath from", value.value )
				return False, "'%s' is not a DAG node" % value.value

			if not self.typeRegex is None:
				nodeType = str(depNode.typeName())
				if self.typeRegex.match( nodeType ) is None:
					return False, ("Type '%s' not accepted: " % nodeType) + self.typeRegexDesc

		if self.mustExist :

			if not exist:
				return False, "DAG node %s does not exist" % value.value

		elif self.mustNotExist :

			if exist:
				return False, "DAG node %s already exists" % value.value

		return True, ""

	"""
	Sets the internal StringData value from the given MDagPath object
	"""
	def setDAGPathValue( self, dagNode ) :
		self.setValue( IECore.StringData( dagNode.fullPathName() ) )

	"""
	Returns a MDagPath for the current node.
	Note that this can return None if check is DontCare and no matching node exists in Maya.
	"""
	def getDAGPathValue( self ) :
		dagNodePath = self.getValue().value
		try:
			list = MSelectionList()
			list.add( dagNodePath )
			dp = MDagPath()
			list.getDagPath(0, dp)
			return dp
		except:
			if self.mustExist :
				raise Exception, "Node '%s' does not exist!" % dagNodePath
			return None

IECore.registerRunTimeTyped( DAGPathParameter )

from DAGPathVectorParameter import DAGPathVectorParameter


import IECore
from maya.OpenMaya import *
import re

from DAGPathParameter import DAGPathParameter

"""
Parameter class for specifying a list of Maya DAG paths.
"""
class DAGPathVectorParameter( IECore.StringVectorParameter ):

	class CheckType:

		DontCare = 0
		MustExist = 1
		MustNotExist = 2

	"""
	name - name of the parameter.
	description - description of the parameter.
	allowEmptyList - boolean that will accept the empty list even if the nodes should exist.
	check - can be CheckType.DontCare, CheckType.MustExist or CheckType.MustNotExist.
	typeRegex - regular expression used on parameter validation that validates based on the maya node type. Disable filtering using None.
	typeRegexDescription - human readable description for the regular expression used to filter node types. It's used when the validation fails.
	"""
	def __init__( self, name, description, defaultValue = IECore.StringVectorData(), allowEmptyList = True,
		check = CheckType.DontCare, typeRegex = None, typeRegexDescription = "",
		presets = (), presetsOnly = False, userData = IECore.CompoundObject() ) :

		IECore.StringVectorParameter.__init__( self, name, description, defaultValue, presets, presetsOnly, userData )

		self.__allowEmptyList = allowEmptyList
		self.__mustExist = bool( check == DAGPathVectorParameter.CheckType.MustExist )
		self.__mustNotExist = bool( check == DAGPathVectorParameter.CheckType.MustNotExist )
		if typeRegex is None:
			self.typeRegex = None
		else:
			self.typeRegex = re.compile( typeRegex )
			if typeRegexDescription == "":
				self.typeRegexDesc = "Invalid type."
			else:
				self.typeRegexDesc = typeRegexDescription

	"""
	Defines two attributes: mustExist and mustNotExist and allowEmptyString exactly like PathParameter class.
	"""
	def __getattr__( self, attrName ):
		if attrName == "mustExist":
			return self.__mustExist
		elif attrName == "mustNotExist":
			return self.__mustNotExist
		elif attrName == "allowEmptyList":
			return self.__allowEmptyList
		else:
			return self.__dict__[ attrName ]

	"""
	Returns a regular expression that matches only valid Maya DAG paths.
	"""
	@staticmethod
	def pathValidator():
		return DAGPathParameter.pathValidator()

	"""
	Returns (True, "") only if the value is a correct DAG path string and also checks that the DAG node exists or doesn't exist
	based on the CheckType passed to the constructor.
	Otherwise returns (False, errorMessage).
	"""
	def valueValid( self, value ) :

		v = IECore.StringVectorParameter.valueValid( self, value )
		if not v[0] :
			return v

		if len( value ) == 0 and not self.allowEmptyList:
			return False, "Empty list!"

		for item in value:

			if not self.pathValidator().match( item ) :
				return False, "%s is not a valid Maya DAG path." % item

			list = MSelectionList ()
			try:
				list.add( item )
			except:
				exist = False
			else:
				exist = True

				try:
					dp = MDagPath()
					list.getDagPath(0, dp)
					depNode = MFnDagNode( dp )
				except:
					IECore.debugException("failed to instantiate MDagPath from", item )
					return False, "'%s' is not a DAG node" % item

				if not self.typeRegex is None:
					nodeType = str(depNode.typeName())
					if self.typeRegex.match( nodeType ) is None:
						return False, ("Type '%s' not accepted: " % nodeType) + self.typeRegexDesc

			if self.mustExist :

				if not exist:
					return False, "DAG node %s does not exist" % item

			elif self.mustNotExist :

				if exist:
					return False, "DAG node %s already exists" % item

		return True, ""

	"""
	Sets the internal VectorStringData value from the given MDagPath list
	"""
	def setDAGPathVectorValue( self, dagNodeList ) :
		l = []
		for dagNode in dagNodeList:
			l.append( dagNode.fullPathName() )
		self.setValue( IECore.StringVectorData( l ) )

	"""
	Returns a list of MDagPath objects from the current selection.
	"""
	def getDAGPathVectorValue( self ) :
		dagNodePathList = self.getValue().value
		result = []
		for dagNodePath in dagNodePathList:

			try:
				list = MSelectionList()
				list.add( dagNodePath )
				dp = MDagPath()
				list.getDagPath(0, dp)
				result.append( dp )
			except:
				if self.mustExist :
					raise Exception, "Node '%s' does not exist!" % dagNodePath

		return result

IECore.registerRunTimeTyped( DAGPathVectorParameter )

from mayaDo import mayaDo


import IECore
import os, sys

"""
Utility function that provides easy access to Ops inside maya using a similar interface as the DO command line.
It tries to find the actions in the following contexts: (1) maya/<MAYA_MAJOR_VERSION>/<OP>, (2) maya/<MAYA_MAJOR_VERSION>/*/<OP>, (3) maya/<OP> and lastly, (4) */<OP>
Returns the Op result if execution succeeds, and None if execution fails for any reason.
"""
def mayaDo( opName, opVersion = None, help = False, **opArgs ):

	loader = IECore.ClassLoader.defaultOpLoader()

	try:

		plugins = loader.classNames( "maya/%s/%s" % (os.environ["MAYA_MAJOR_VERSION"], opName) )
		if len( plugins ) == 0:
			plugins = loader.classNames( "maya/%s/*/%s" % (os.environ["MAYA_MAJOR_VERSION"], opName) )
			if len( plugins ) == 0:
				plugins = loader.classNames( "maya/%s" % (opName) )
				if len( plugins ) == 0:
					plugins = loader.classNames( "*/%s" % opName )
					# eliminate actions in maya because they may be for other maya versions.
					plugins = filter( lambda x: not x.startswith( "maya/" ), plugins )

		if not len( plugins ) :
			IECore.error( "Action \"%s\" does not exist.\n" % opName )
			return None
		elif len( plugins )>1 :
			IECore.error( "Action name \"%s\" is ambiguous - could be any of the following : \n\t%s" % ( opName, "\n\t".join( plugins ) ) )
			return None

		actionName = plugins[0]

		actionVersions = loader.versions( actionName )
		if opVersion is None :
			opVersion = actionVersions[-1]
		else:
			if not opVersion in actionVersions :
				IECore.error( "Version \"%s\" of action \"%s\" does not exist.\n" % (opVersion, actionName) )
				return None

		opType = loader.load( actionName, opVersion )
		myOp = opType()

	except Exception, e:
		IECore.debugException( "Failed on trying to load ", opName )
		IECore.error( "Error loading", opName, ":", str(e) )
		return None

	if help:
		formatter = IECore.WrappedTextFormatter( sys.stdout )
		formatter.paragraph( "Name    : " + myOp.name + "\n" )
		formatter.paragraph( myOp.description + "\n" )
		formatter.heading( "Parameters" )
		formatter.indent()
		for p in myOp.parameters().values() :
			IECore.formatParameterHelp( p, formatter )
		formatter.unindent()
		return None

	try:
		res = myOp( **opArgs )
	except Exception, e:
		IECore.error( 'Error executing Op', myOp.name, ':', str(e) )
		return None
	else:
		try:
			if myOp.userData()['UI']['showResult'].value:
				print res
		except:
			pass

	return res

from Menu import Menu, createMenu


import maya.cmds
import maya.mel

import IECore

from UIElement import UIElement

## A class for making maya menus from an IECore.MenuDefinition. The menu is built dynamically when it's
# displayed, so the definition can be edited at any time to change the menu. 
class Menu( UIElement ) :

	# maya.cmds.about doesn't exist in maya standalone
	__defaultBoldFont = False if hasattr( maya.cmds, "about" ) and maya.cmds.about( windows=True ) else True

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
	# useInterToUI :
	# determines whether or not the interToUI mel command is used when creating menu labels from the
	# paths in the definition. default to True but may well be defaulted to False in a future version
	# and then deprecated.
	#
	# replaceExistingMenu :
	# determines whether we add the menu as a submenu, or overwrite the contents of the existing menu
	# (if the parent is a menu)
	#
	# \todo Change useInterToUI default value to False and deprecate its use.
	def __init__( self, definition, parent, label="", insertAfter=None, radialPosition=None, button = 3, useInterToUI = True, replaceExistingMenu = False ) :

		menu = None
		if maya.cmds.window( parent, query=True, exists=True ) or maya.cmds.menuBarLayout( parent, query=True, exists=True ) :
			# parent is a window - we're sticking it in the menubar
			menu = maya.cmds.menu( label=label, parent=parent, allowOptionBoxes=True, tearOff=True )
		elif maya.cmds.menu( parent, query=True, exists=True ) :
			if replaceExistingMenu:
				# parent is a menu - we're appending to it:
				menu = parent
				self.__postMenu( menu, definition, useInterToUI )
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

		maya.cmds.menu( menu, edit=True, postMenuCommand = IECore.curry( self.__postMenu, menu, definition, useInterToUI ) )
		
		UIElement.__init__( self, menu )
	
	def __wrapCallback( self, cb ) :
		
		if ( callable( cb ) ) :
			return self._createCallback( cb )
		else :
			# presumably a command in string form
			return cb

	def __postMenu( self, parent, definition, useInterToUI, *args ) :

		if callable( definition ) :
			definition = definition()

		maya.cmds.menu( parent, edit=True, deleteAllItems=True )

		done = set()
		for path, item in definition.items() :

			pathComponents = path.strip( "/" ).split( "/" )
			name = pathComponents[0]

			if useInterToUI :
				label = maya.mel.eval( 'interToUI( "%s" )' % name )
			else :
				label = name
				
			boldFont = Menu.__defaultBoldFont
			if hasattr( item, "bold" ) :
				boldFont = bool(item.bold)
			
			italicized = False
			if hasattr( item, "italic" ) :
				italicized = bool(item.italic)
				
			if len( pathComponents ) > 1 :
				# a submenu
				if not name in done :
					subMenu = maya.cmds.menuItem( label=label, subMenu=True, allowOptionBoxes=True, parent=parent, tearOff=True )
					subMenuDefinition = definition.reRooted( "/" + name + "/" )
					maya.cmds.menu( subMenu, edit=True, postMenuCommand=IECore.curry( self.__postMenu, subMenu, subMenuDefinition, useInterToUI ) )
					done.add( name )
			else :

				if item.divider :

					menuItem = maya.cmds.menuItem( parent=parent, divider=True )

				elif item.subMenu :

					subMenu = maya.cmds.menuItem( label=label, subMenu=True, allowOptionBoxes=True, parent=parent, boldFont=boldFont, italicized=italicized )
					maya.cmds.menu( subMenu, edit=True, postMenuCommand=IECore.curry( self.__postMenu, subMenu, item.subMenu, useInterToUI ) )

				else :

					active = item.active
					if callable( active ) :
						active = active()

					kw = {}
					
					checked = item.checkBox
					if callable( checked ) :
						checked = checked()
						kw["checkBox"] = checked
					
					menuItem = maya.cmds.menuItem( label=label, parent=parent, enable=active, annotation=item.description, boldFont=boldFont, italicized=italicized, **kw )
					if item.command :
						maya.cmds.menuItem( menuItem, edit=True, command=self.__wrapCallback( item.command ) )
					if item.secondaryCommand :
						optionBox = maya.cmds.menuItem( optionBox=True, enable=active, command=self.__wrapCallback( item.secondaryCommand ), parent=parent )


# \deprecated Use IECoreMaya.Menu instead.
# \todo Remove for next major version.
def createMenu( *args, **kw ) :

	return Menu( *args, **kw )._topLevelUI()


from BakeTransform import BakeTransform


from maya.OpenMaya import *
import maya.cmds
import math
import IECore
import IECoreMaya

class BakeTransform( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "Bakes transforms from one object onto another.",
			IECoreMaya.DAGPathParameter(
				name = "result",
				description = "The name of the baked transform",
				defaultValue = "",
				allowEmptyString = True,
			)
		)

		self.parameters().addParameters(
			[
				IECoreMaya.DAGPathParameter(
					name = "src",
					description = "The source transform node to bake from.",
					defaultValue = "",
					check = IECoreMaya.DAGPathParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				IECoreMaya.DAGPathParameter(
					name = "dst",
					description = "The destination transform node to apply the bake to. If this doesn't exist"
						"then it'll be created for you.",
					defaultValue = "",
					check = IECoreMaya.DAGPathParameter.CheckType.DontCare,
					allowEmptyString = False,
				),
				IECore.FrameListParameter(
					name = "frames",
					description = "The frame range over which to perform the bake."
						"Keyframes will be made at every frame in this range.",
					defaultValue = IECoreMaya.PlaybackFrameList( IECoreMaya.PlaybackFrameList.Range.Playback ),
				),
				IECore.BoolParameter(
					name = "lock",
					description = "If this is specified then the transform attributes which are"
						"keyframed on the destination object are also locked.",
					defaultValue = False,
				),
			]
		)

	@staticmethod
	def transferTransform( src, dst, setKey=True ) :

		worldMatrixConverter = IECoreMaya.FromMayaPlugConverter.create( str(src.fullPathName()) + ".worldMatrix" )
		worldMatrix = worldMatrixConverter.convert().value
		e = worldMatrix.rotate
		maya.cmds.xform( dst.fullPathName(), translation=tuple( worldMatrix.translate ), rotation=[math.degrees(x) for x in e], scale=tuple( worldMatrix.scale ) )
		maya.cmds.setKeyframe( dst.fullPathName(), attribute=["translate", "rotate", "scale"] )

	@staticmethod
	def lockTransform( transform ) :

		transformName = transform.fullPathName()
		maya.cmds.setAttr( str( transformName ) + ".translate", lock=True )
		maya.cmds.setAttr( str( transformName ) + ".rotate", lock=True )
		maya.cmds.setAttr( str( transformName ) + ".scale", lock=True )
		maya.cmds.setAttr( str( transformName ) + ".shear", lock=True )

	def doOperation( self, operands ) :

		if maya.cmds.objExists( operands.dst.value ) :
			dst = self.dst.getDAGPathValue()
		else :
			dst = maya.cmds.createNode( "transform", name=operands.dst.value, skipSelect=True )
			list = MSelectionList()
			list.add( dst )
			dst = MDagPath()
			list.getDagPath(0, dst)

		dstPath = str( dst.fullPathName() )

		src = self.src.getDAGPathValue()

		for f in self.frames.getFrameListValue().asList() :

			maya.cmds.currentTime( float( f ) )
			self.transferTransform( src, dst, True )

		# fix discontinuous rotations
		maya.cmds.filterCurve( dstPath + ".rotateX", dstPath + ".rotateY", dstPath + ".rotateZ" )

		if operands.lock.value :
			self.lockTransform( dst )

		return IECore.StringData( dstPath )

IECore.registerRunTimeTyped( BakeTransform )

from MeshOpHolderUtil import create


import maya.OpenMaya as OpenMaya
import maya.cmds as cmds

import IECoreMaya
import IECore

def __getFloat3PlugValue(plug):

	# Retrieve the value as an MObject
	object = plug.asMObject()

	# Convert the MObject to a float3
	numDataFn = OpenMaya.MFnNumericData(object)
	xParam = OpenMaya.MScriptUtil()
	xParam.createFromDouble(0.0)
	xPtr = xParam.asFloatPtr()
	yParam = OpenMaya.MScriptUtil()
	yParam.createFromDouble(0.0)
	yPtr = yParam.asFloatPtr()
	zParam = OpenMaya.MScriptUtil()
	zParam.createFromDouble(0.0)
	zPtr = zParam.asFloatPtr()
	numDataFn.getData3Float(xPtr, yPtr, zPtr)
	return OpenMaya.MFloatVector(
		OpenMaya.MScriptUtil(xPtr).asFloat(),
		OpenMaya.MScriptUtil(yPtr).asFloat(),
		OpenMaya.MScriptUtil(zPtr).asFloat()
	)

def __hasTweaks( meshDagPath ):

	fnDN = OpenMaya.MFnDependencyNode( meshDagPath.node() )

	# Tweaks exist only if the multi "pnts" attribute contains plugs
	# which contain non-zero tweak values.
	tweakPlug = fnDN.findPlug("pnts")
	if not tweakPlug.isNull():

		if not tweakPlug.isArray():
			raise RuntimeError( "tweakPlug is not an array plug" )

		numElements = tweakPlug.numElements()
		for i in range(numElements):
			tweak = tweakPlug.elementByPhysicalIndex(i)
			if not tweak.isNull():
				tweakData = __getFloat3PlugValue(tweak)
				if 0 != tweakData.x or 0 != tweakData.y or 0 != tweakData.z:
					return True

	return False

def __hasHistory( meshDagPath ):

	fnDN = OpenMaya.MFnDependencyNode( meshDagPath.node() )

	return fnDN.findPlug("inMesh").isConnected()

def __processUpstreamNode(data, meshDagPath, dgModifier):

	if __hasHistory( meshDagPath ):
		# Just swap the connections around
		tempPlugArray = OpenMaya.MPlugArray()
		data.meshNodeDestPlug.connectedTo(tempPlugArray, True, False)
		assert( tempPlugArray.length() == 1 )

		data.upstreamNodeSrcPlug = tempPlugArray[0]

		data.upstreamNodeShape = data.upstreamNodeSrcPlug.node()

		data.upstreamNodeSrcAttr = data.upstreamNodeSrcPlug.attribute()

		dgModifier.disconnect(data.upstreamNodeSrcPlug, data.meshNodeDestPlug)
		dgModifier.doIt()

	else:
		# Duplicate mesh, mark as "intermediate", and reconnect in the DAG
		dagNodeFn = OpenMaya.MFnDagNode( data.meshNodeShape )

		data.upstreamNodeTransform = dagNodeFn.duplicate(False, False)
		dagNodeFn.setObject(data.upstreamNodeTransform)

		fDagModifier = OpenMaya.MDagModifier()

		if dagNodeFn.childCount() < 1:
			raise RuntimeError( "Duplicated mesh has no shape" )

		data.upstreamNodeShape = dagNodeFn.child(0)

		fDagModifier.reparentNode(data.upstreamNodeShape, data.meshNodeTransform)
		fDagModifier.doIt()

		dagNodeFn.setObject(data.upstreamNodeShape)
		dagNodeFn.setIntermediateObject(True)

		data.upstreamNodeSrcAttr = dagNodeFn.attribute("outMesh")
		data.upstreamNodeSrcPlug = dagNodeFn.findPlug("outMesh")

		fDagModifier.deleteNode(data.upstreamNodeTransform)
		fDagModifier.doIt()

def __processTweaks(data, dgModifier, modifierNode):

	tweakIndexArray = OpenMaya.MIntArray()

	fnDN = OpenMaya.MFnDependencyNode()

	tweakDataArray = OpenMaya.MObjectArray()
	tweakSrcConnectionCountArray = OpenMaya.MIntArray()
	tweakSrcConnectionPlugArray = OpenMaya.MPlugArray()
	tweakDstConnectionCountArray = OpenMaya.MIntArray()
	tweakDstConnectionPlugArray = OpenMaya.MPlugArray()
	tempPlugArray = OpenMaya.MPlugArray()

	tweakNode = dgModifier.createNode("polyTweak")
	fnDN.setObject(tweakNode)
	tweakNodeSrcAttr = fnDN.attribute("output")
	tweakNodeDestAttr = fnDN.attribute("inputPolymesh")
	tweakNodeTweakAttr = fnDN.attribute("tweak")

	fnDN.setObject(data.meshNodeShape)
	meshTweakPlug = fnDN.findPlug("pnts")

	if not meshTweakPlug.isArray() :
		raise RuntimeError( "meshTweakPlug is not an array plug" )

	numElements = meshTweakPlug.numElements()
	for i in range(numElements):
		tweak = meshTweakPlug.elementByPhysicalIndex(i)

		if not tweak.isNull():

			tweakIndexArray.append( tweak.logicalIndex() )

			tweakData = tweak.asMObject()
			tweakDataArray.append(tweakData)

			if not tweak.isCompound():
				raise RuntimeError( "Element tweak plug is not a compound" )

			numChildren = tweak.numChildren()
			for j in range(numChildren):
				tweakChild = tweak.child(j)
				if tweakChild.isConnected():

					tempPlugArray.clear()
					if tweakChild.connectedTo(tempPlugArray, False, True):
						numSrcConnections = tempPlugArray.length()
						tweakSrcConnectionCountArray.append(numSrcConnections)

						for k in range(numSrcConnections):
							tweakSrcConnectionPlugArray.append(tempPlugArray[k])
							dgModifier.disconnect(tweakChild, tempPlugArray[k])
					else:
						tweakSrcConnectionCountArray.append(0)

					tempPlugArray.clear()
					if tweakChild.connectedTo(tempPlugArray, True, False):
						assert( tempPlugArray.length() == 1 )

						tweakDstConnectionCountArray.append(1)
						tweakDstConnectionPlugArray.append(tempPlugArray[0])
						dgModifier.disconnect(tempPlugArray[0], tweakChild)
					else:
						tweakDstConnectionCountArray.append(0)
				else:
					tweakSrcConnectionCountArray.append(0)
					tweakDstConnectionCountArray.append(0)

	polyTweakPlug = OpenMaya.MPlug(tweakNode, tweakNodeTweakAttr)
	numTweaks = tweakIndexArray.length()
	srcOffset = 0
	dstOffset = 0
	for i in range(numTweaks):

		tweak = polyTweakPlug.elementByLogicalIndex(tweakIndexArray[i])
		tweak.setMObject(tweakDataArray[i])

		if not tweak.isCompound():
			raise RuntimeError( "Element plug 'tweak' is not a compound" )

		numChildren = tweak.numChildren()
		for j in range(numChildren):
			tweakChild = tweak.child(j)

			if 0 < tweakSrcConnectionCountArray[i*numChildren + j]:
				k = 0
				while (k < tweakSrcConnectionCountArray[i*numChildren + j]):
					dgModifier.connect(tweakChild, tweakSrcConnectionPlugArray[srcOffset])
					srcOffset += 1
					k += 1

			if 0 < tweakDstConnectionCountArray[i*numChildren + j]:
				dgModifier.connect(tweakDstConnectionPlugArray[dstOffset], tweakChild)
				dstOffset += 1

	tweakDestPlug = OpenMaya.MPlug( tweakNode, tweakNodeDestAttr )
	dgModifier.connect( data.upstreamNodeSrcPlug, tweakDestPlug )

	tweakSrcPlug = OpenMaya.MPlug( tweakNode, tweakNodeSrcAttr)
	modifierDestPlug = OpenMaya.MPlug( modifierNode, data.modifierNodeDestAttr )
	dgModifier.connect( tweakSrcPlug, modifierDestPlug )

def __connectNodes( modifierNode, meshDagPath ):
	class MeshOpHolderData:
		def __init__(self):
			self.meshNodeTransform = OpenMaya.MObject()
			self.meshNodeShape = OpenMaya.MObject()
			self.meshNodeDestPlug = OpenMaya.MPlug()
			self.meshNodeDestAttr = OpenMaya.MObject()

			self.upstreamNodeTransform = OpenMaya.MObject()
			self.upstreamNodeShape = OpenMaya.MObject()
			self.upstreamNodeSrcPlug = OpenMaya.MPlug()
			self.upstreamNodeSrcAttr = OpenMaya.MObject()

			self.modifierNodeSrcAttr = OpenMaya.MObject()
			self.modifierNodeDestAttr = OpenMaya.MObject()

	data = MeshOpHolderData()

	fnDN = OpenMaya.MFnDependencyNode( modifierNode )
	data.modifierNodeSrcAttr = fnDN.attribute("result")
	data.modifierNodeDestAttr = fnDN.attribute("parm_input")

	data.meshNodeShape = meshDagPath.node()
	dagNodeFn = OpenMaya.MFnDagNode( data.meshNodeShape )

	if dagNodeFn.parentCount() == 0:
		raise RuntimeError( "Mesh shape has no parent transform" )

	data.meshNodeTransform = dagNodeFn.parent(0)
	data.meshNodeDestPlug = dagNodeFn.findPlug("inMesh")
	data.meshNodeDestAttr = data.meshNodeDestPlug.attribute()

	dgModifier = OpenMaya.MDGModifier()
	__processUpstreamNode(data, meshDagPath, dgModifier)

	if __hasTweaks( meshDagPath ):
		__processTweaks(data, dgModifier, modifierNode)
	else:
		modifierDestPlug = OpenMaya.MPlug(modifierNode, data.modifierNodeDestAttr)
		dgModifier.connect(data.upstreamNodeSrcPlug, modifierDestPlug)

	modifierSrcPlug = OpenMaya.MPlug(modifierNode, data.modifierNodeSrcAttr)
	meshDestAttr = OpenMaya.MPlug(data.meshNodeShape, data.meshNodeDestAttr)

	dgModifier.connect(modifierSrcPlug, meshDestAttr)

	dgModifier.doIt()

def __setParameters( op, kw ):

	for paramName, paramValue in kw.items():
		op.parameters().setValidatedParameterValue( paramName, paramValue )


def __createMeshOpNode( className, classVersion, **kw ):

	shortClassName = className.split( '/' ).pop()

	modifierNodeName = cmds.createNode( "ieOpHolderNode", name = shortClassName + "#" )

	ph = IECoreMaya.FnParameterisedHolder( modifierNodeName )
	op = ph.setParameterised( className, classVersion, "IECORE_OP_PATHS" )

	__setParameters( op, kw )

	selList = OpenMaya.MSelectionList()
	selList.add( modifierNodeName )
	modifierNode = OpenMaya.MObject()
	s = selList.getDependNode( 0, modifierNode )

	return modifierNode

def __applyMeshOp( meshNode, className, classVersion, kw ):

	op = IECore.ClassLoader.defaultOpLoader().load( className, classVersion )

	__setParameters( op, **kw )

	# \todo Apply op and convert result back into original object


def create( meshDagPath, className, classVersion, **kw):

	if type(meshDagPath) is str:
		sel = OpenMaya.MSelectionList()
		sel.add( meshDagPath )
		meshDagPath = OpenMaya.MDagPath()
		sel.getDagPath( 0,  meshDagPath)
		meshDagPath.extendToShape()

	constructionHistoryEnabled = IECoreMaya.mel("constructionHistory -q -tgl").value

	if not __hasHistory( meshDagPath ) and constructionHistoryEnabled == 0:

		# \todo we can't actually do this right now because we're unable to convert the resultant MeshPrimitive
		# back into the original meshNode MObject given to us
		raise RuntimeError( "Currently unable to apply MeshOp in-place " )

		meshNode = meshDagPath.node()

		__applyMeshOp(meshNode, className, classVersion, **kw )

		return None
	else:
		modifierNode = __createMeshOpNode( className, classVersion, **kw )

		__connectNodes( modifierNode, meshDagPath )

		fnDN = OpenMaya.MFnDependencyNode( modifierNode )

		return str( fnDN.name() )


def createUI( className, classVersion, **kw ):

	# \todo This below selection determination code fails with an unclear error if
	# a mesh component is currently selected
	selectedTransforms = cmds.ls( selection = True, type = "transform" ) or []
	selectedTransformMeshShapes = cmds.listRelatives( selectedTransforms, type = "mesh" ) or []

	selectedMeshes = cmds.ls( selection = True, type = "mesh" ) or []
	selectedMeshes += selectedTransformMeshShapes

	if not selectedMeshes:
		raise RuntimeError( "No mesh selected" )

	modifierNodes = []

	for mesh in selectedMeshes:

		sel = OpenMaya.MSelectionList()
		sel.add( mesh )
		meshDagPath = OpenMaya.MDagPath()
		sel.getDagPath( 0,  meshDagPath)
		meshDagPath.extendToShape()

		modifierNode = create( meshDagPath, className, classVersion, **kw )

		if modifierNode :

			modifierNodes += [ modifierNode ]


	return modifierNodes

from MeshOpHolderUtil import createUI


import maya.OpenMaya as OpenMaya
import maya.cmds as cmds

import IECoreMaya
import IECore

def __getFloat3PlugValue(plug):

	# Retrieve the value as an MObject
	object = plug.asMObject()

	# Convert the MObject to a float3
	numDataFn = OpenMaya.MFnNumericData(object)
	xParam = OpenMaya.MScriptUtil()
	xParam.createFromDouble(0.0)
	xPtr = xParam.asFloatPtr()
	yParam = OpenMaya.MScriptUtil()
	yParam.createFromDouble(0.0)
	yPtr = yParam.asFloatPtr()
	zParam = OpenMaya.MScriptUtil()
	zParam.createFromDouble(0.0)
	zPtr = zParam.asFloatPtr()
	numDataFn.getData3Float(xPtr, yPtr, zPtr)
	return OpenMaya.MFloatVector(
		OpenMaya.MScriptUtil(xPtr).asFloat(),
		OpenMaya.MScriptUtil(yPtr).asFloat(),
		OpenMaya.MScriptUtil(zPtr).asFloat()
	)

def __hasTweaks( meshDagPath ):

	fnDN = OpenMaya.MFnDependencyNode( meshDagPath.node() )

	# Tweaks exist only if the multi "pnts" attribute contains plugs
	# which contain non-zero tweak values.
	tweakPlug = fnDN.findPlug("pnts")
	if not tweakPlug.isNull():

		if not tweakPlug.isArray():
			raise RuntimeError( "tweakPlug is not an array plug" )

		numElements = tweakPlug.numElements()
		for i in range(numElements):
			tweak = tweakPlug.elementByPhysicalIndex(i)
			if not tweak.isNull():
				tweakData = __getFloat3PlugValue(tweak)
				if 0 != tweakData.x or 0 != tweakData.y or 0 != tweakData.z:
					return True

	return False

def __hasHistory( meshDagPath ):

	fnDN = OpenMaya.MFnDependencyNode( meshDagPath.node() )

	return fnDN.findPlug("inMesh").isConnected()

def __processUpstreamNode(data, meshDagPath, dgModifier):

	if __hasHistory( meshDagPath ):
		# Just swap the connections around
		tempPlugArray = OpenMaya.MPlugArray()
		data.meshNodeDestPlug.connectedTo(tempPlugArray, True, False)
		assert( tempPlugArray.length() == 1 )

		data.upstreamNodeSrcPlug = tempPlugArray[0]

		data.upstreamNodeShape = data.upstreamNodeSrcPlug.node()

		data.upstreamNodeSrcAttr = data.upstreamNodeSrcPlug.attribute()

		dgModifier.disconnect(data.upstreamNodeSrcPlug, data.meshNodeDestPlug)
		dgModifier.doIt()

	else:
		# Duplicate mesh, mark as "intermediate", and reconnect in the DAG
		dagNodeFn = OpenMaya.MFnDagNode( data.meshNodeShape )

		data.upstreamNodeTransform = dagNodeFn.duplicate(False, False)
		dagNodeFn.setObject(data.upstreamNodeTransform)

		fDagModifier = OpenMaya.MDagModifier()

		if dagNodeFn.childCount() < 1:
			raise RuntimeError( "Duplicated mesh has no shape" )

		data.upstreamNodeShape = dagNodeFn.child(0)

		fDagModifier.reparentNode(data.upstreamNodeShape, data.meshNodeTransform)
		fDagModifier.doIt()

		dagNodeFn.setObject(data.upstreamNodeShape)
		dagNodeFn.setIntermediateObject(True)

		data.upstreamNodeSrcAttr = dagNodeFn.attribute("outMesh")
		data.upstreamNodeSrcPlug = dagNodeFn.findPlug("outMesh")

		fDagModifier.deleteNode(data.upstreamNodeTransform)
		fDagModifier.doIt()

def __processTweaks(data, dgModifier, modifierNode):

	tweakIndexArray = OpenMaya.MIntArray()

	fnDN = OpenMaya.MFnDependencyNode()

	tweakDataArray = OpenMaya.MObjectArray()
	tweakSrcConnectionCountArray = OpenMaya.MIntArray()
	tweakSrcConnectionPlugArray = OpenMaya.MPlugArray()
	tweakDstConnectionCountArray = OpenMaya.MIntArray()
	tweakDstConnectionPlugArray = OpenMaya.MPlugArray()
	tempPlugArray = OpenMaya.MPlugArray()

	tweakNode = dgModifier.createNode("polyTweak")
	fnDN.setObject(tweakNode)
	tweakNodeSrcAttr = fnDN.attribute("output")
	tweakNodeDestAttr = fnDN.attribute("inputPolymesh")
	tweakNodeTweakAttr = fnDN.attribute("tweak")

	fnDN.setObject(data.meshNodeShape)
	meshTweakPlug = fnDN.findPlug("pnts")

	if not meshTweakPlug.isArray() :
		raise RuntimeError( "meshTweakPlug is not an array plug" )

	numElements = meshTweakPlug.numElements()
	for i in range(numElements):
		tweak = meshTweakPlug.elementByPhysicalIndex(i)

		if not tweak.isNull():

			tweakIndexArray.append( tweak.logicalIndex() )

			tweakData = tweak.asMObject()
			tweakDataArray.append(tweakData)

			if not tweak.isCompound():
				raise RuntimeError( "Element tweak plug is not a compound" )

			numChildren = tweak.numChildren()
			for j in range(numChildren):
				tweakChild = tweak.child(j)
				if tweakChild.isConnected():

					tempPlugArray.clear()
					if tweakChild.connectedTo(tempPlugArray, False, True):
						numSrcConnections = tempPlugArray.length()
						tweakSrcConnectionCountArray.append(numSrcConnections)

						for k in range(numSrcConnections):
							tweakSrcConnectionPlugArray.append(tempPlugArray[k])
							dgModifier.disconnect(tweakChild, tempPlugArray[k])
					else:
						tweakSrcConnectionCountArray.append(0)

					tempPlugArray.clear()
					if tweakChild.connectedTo(tempPlugArray, True, False):
						assert( tempPlugArray.length() == 1 )

						tweakDstConnectionCountArray.append(1)
						tweakDstConnectionPlugArray.append(tempPlugArray[0])
						dgModifier.disconnect(tempPlugArray[0], tweakChild)
					else:
						tweakDstConnectionCountArray.append(0)
				else:
					tweakSrcConnectionCountArray.append(0)
					tweakDstConnectionCountArray.append(0)

	polyTweakPlug = OpenMaya.MPlug(tweakNode, tweakNodeTweakAttr)
	numTweaks = tweakIndexArray.length()
	srcOffset = 0
	dstOffset = 0
	for i in range(numTweaks):

		tweak = polyTweakPlug.elementByLogicalIndex(tweakIndexArray[i])
		tweak.setMObject(tweakDataArray[i])

		if not tweak.isCompound():
			raise RuntimeError( "Element plug 'tweak' is not a compound" )

		numChildren = tweak.numChildren()
		for j in range(numChildren):
			tweakChild = tweak.child(j)

			if 0 < tweakSrcConnectionCountArray[i*numChildren + j]:
				k = 0
				while (k < tweakSrcConnectionCountArray[i*numChildren + j]):
					dgModifier.connect(tweakChild, tweakSrcConnectionPlugArray[srcOffset])
					srcOffset += 1
					k += 1

			if 0 < tweakDstConnectionCountArray[i*numChildren + j]:
				dgModifier.connect(tweakDstConnectionPlugArray[dstOffset], tweakChild)
				dstOffset += 1

	tweakDestPlug = OpenMaya.MPlug( tweakNode, tweakNodeDestAttr )
	dgModifier.connect( data.upstreamNodeSrcPlug, tweakDestPlug )

	tweakSrcPlug = OpenMaya.MPlug( tweakNode, tweakNodeSrcAttr)
	modifierDestPlug = OpenMaya.MPlug( modifierNode, data.modifierNodeDestAttr )
	dgModifier.connect( tweakSrcPlug, modifierDestPlug )

def __connectNodes( modifierNode, meshDagPath ):
	class MeshOpHolderData:
		def __init__(self):
			self.meshNodeTransform = OpenMaya.MObject()
			self.meshNodeShape = OpenMaya.MObject()
			self.meshNodeDestPlug = OpenMaya.MPlug()
			self.meshNodeDestAttr = OpenMaya.MObject()

			self.upstreamNodeTransform = OpenMaya.MObject()
			self.upstreamNodeShape = OpenMaya.MObject()
			self.upstreamNodeSrcPlug = OpenMaya.MPlug()
			self.upstreamNodeSrcAttr = OpenMaya.MObject()

			self.modifierNodeSrcAttr = OpenMaya.MObject()
			self.modifierNodeDestAttr = OpenMaya.MObject()

	data = MeshOpHolderData()

	fnDN = OpenMaya.MFnDependencyNode( modifierNode )
	data.modifierNodeSrcAttr = fnDN.attribute("result")
	data.modifierNodeDestAttr = fnDN.attribute("parm_input")

	data.meshNodeShape = meshDagPath.node()
	dagNodeFn = OpenMaya.MFnDagNode( data.meshNodeShape )

	if dagNodeFn.parentCount() == 0:
		raise RuntimeError( "Mesh shape has no parent transform" )

	data.meshNodeTransform = dagNodeFn.parent(0)
	data.meshNodeDestPlug = dagNodeFn.findPlug("inMesh")
	data.meshNodeDestAttr = data.meshNodeDestPlug.attribute()

	dgModifier = OpenMaya.MDGModifier()
	__processUpstreamNode(data, meshDagPath, dgModifier)

	if __hasTweaks( meshDagPath ):
		__processTweaks(data, dgModifier, modifierNode)
	else:
		modifierDestPlug = OpenMaya.MPlug(modifierNode, data.modifierNodeDestAttr)
		dgModifier.connect(data.upstreamNodeSrcPlug, modifierDestPlug)

	modifierSrcPlug = OpenMaya.MPlug(modifierNode, data.modifierNodeSrcAttr)
	meshDestAttr = OpenMaya.MPlug(data.meshNodeShape, data.meshNodeDestAttr)

	dgModifier.connect(modifierSrcPlug, meshDestAttr)

	dgModifier.doIt()

def __setParameters( op, kw ):

	for paramName, paramValue in kw.items():
		op.parameters().setValidatedParameterValue( paramName, paramValue )


def __createMeshOpNode( className, classVersion, **kw ):

	shortClassName = className.split( '/' ).pop()

	modifierNodeName = cmds.createNode( "ieOpHolderNode", name = shortClassName + "#" )

	ph = IECoreMaya.FnParameterisedHolder( modifierNodeName )
	op = ph.setParameterised( className, classVersion, "IECORE_OP_PATHS" )

	__setParameters( op, kw )

	selList = OpenMaya.MSelectionList()
	selList.add( modifierNodeName )
	modifierNode = OpenMaya.MObject()
	s = selList.getDependNode( 0, modifierNode )

	return modifierNode

def __applyMeshOp( meshNode, className, classVersion, kw ):

	op = IECore.ClassLoader.defaultOpLoader().load( className, classVersion )

	__setParameters( op, **kw )

	# \todo Apply op and convert result back into original object


def create( meshDagPath, className, classVersion, **kw):

	if type(meshDagPath) is str:
		sel = OpenMaya.MSelectionList()
		sel.add( meshDagPath )
		meshDagPath = OpenMaya.MDagPath()
		sel.getDagPath( 0,  meshDagPath)
		meshDagPath.extendToShape()

	constructionHistoryEnabled = IECoreMaya.mel("constructionHistory -q -tgl").value

	if not __hasHistory( meshDagPath ) and constructionHistoryEnabled == 0:

		# \todo we can't actually do this right now because we're unable to convert the resultant MeshPrimitive
		# back into the original meshNode MObject given to us
		raise RuntimeError( "Currently unable to apply MeshOp in-place " )

		meshNode = meshDagPath.node()

		__applyMeshOp(meshNode, className, classVersion, **kw )

		return None
	else:
		modifierNode = __createMeshOpNode( className, classVersion, **kw )

		__connectNodes( modifierNode, meshDagPath )

		fnDN = OpenMaya.MFnDependencyNode( modifierNode )

		return str( fnDN.name() )


def createUI( className, classVersion, **kw ):

	# \todo This below selection determination code fails with an unclear error if
	# a mesh component is currently selected
	selectedTransforms = cmds.ls( selection = True, type = "transform" ) or []
	selectedTransformMeshShapes = cmds.listRelatives( selectedTransforms, type = "mesh" ) or []

	selectedMeshes = cmds.ls( selection = True, type = "mesh" ) or []
	selectedMeshes += selectedTransformMeshShapes

	if not selectedMeshes:
		raise RuntimeError( "No mesh selected" )

	modifierNodes = []

	for mesh in selectedMeshes:

		sel = OpenMaya.MSelectionList()
		sel.add( mesh )
		meshDagPath = OpenMaya.MDagPath()
		sel.getDagPath( 0,  meshDagPath)
		meshDagPath.extendToShape()

		modifierNode = create( meshDagPath, className, classVersion, **kw )

		if modifierNode :

			modifierNodes += [ modifierNode ]


	return modifierNodes

from ScopedSelection import ScopedSelection


import maya.cmds
import weakref

## It's common to need to save the current maya selection, change it, and restore
# the old selection afterwards. This is error prone, especially when taking exception
# handling into account. The ScopedSelection object saves the selection when it's
# created, and restores it when it dies.
## \todo Cope with objects changing names, being deleted etc - can probably do this
# by storing DagNode and Node objects.
## \todo This should be reimplemented as a context object called SelectionSaved to
# follow the model set by UndoDisabled.
class ScopedSelection :

	def __init__( self ) :

		selection = maya.cmds.ls( selection=True )
		ScopedSelection.__selections[weakref.ref(self, ScopedSelection.__weakRefCallback)] = selection

	@classmethod
	def __weakRefCallback( cls, w ) :

		maya.cmds.select( cls.__selections[w], replace=True )
		del cls.__selections[w]

	__selections = {}


from FnParameterisedHolder import FnParameterisedHolder


from __future__ import with_statement

import warnings

import maya.OpenMaya
import maya.cmds

import IECore

import _IECoreMaya
import StringUtil

## A function set for operating on the various IECoreMaya::ParameterisedHolder
# types. This allows setting and getting of plug and parameter values, and
# setting and getting of the Parameterised object being held.
class FnParameterisedHolder( maya.OpenMaya.MFnDependencyNode ) :

	## Initialise the function set for the given object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		if isinstance( object, str ) or isinstance( object, unicode ) :
			object = StringUtil.dependencyNodeFromString( object )

		maya.OpenMaya.MFnDependencyNode.__init__( self, object )

	## Sets the IECore.Parameterised object held by the node. This function can be called
	# in two ways :
	#
	# setParameterised( Parameterised p )
	# Directly sets the held object to the Parameterised instance p. Note that this
	# form doesn't provide enough information for the node to be reinstantiated
	# after saving and reloading of the maya scene - see the form below for that.
	# Also note that this form is not undoable, and that the undoable parameter will therefore
	# be ignored.
	#
	# setParameterised( string className, int classVersion, string searchPathEnvVar, bool undoable )
	# Sets the held object by specifying a class that will be loaded using the IECore.ClassLoader.
	# searchPathEnvVar specifies an environment variable which holds a colon separated search path for the
	# ClassLoader. This form allows the held class to be reinstantiated across scene save/load, and is
	# also undoable if requested using the undoable parameter. If classVersion is omitted, None, or negative,
	# then the highest available version will be used.
	def setParameterised( self, classNameOrParameterised, classVersion=None, envVarName=None, undoable=True ) :

		if isinstance( classNameOrParameterised, str ) :
			if classVersion is None or classVersion < 0 :
				classVersions = IECore.ClassLoader.defaultLoader( envVarName ).versions( classNameOrParameterised )
				classVersion = classVersions[-1] if classVersions else 0 
			if undoable  :
				if self.getParameterised()[0] :
					self.setParameterisedValues()
					_IECoreMaya._parameterisedHolderAssignModificationState(
						self.getParameterised()[0].parameters().getValue().copy(),
						self._classParameterStates(),
						None,
						None
					)
				else :
					_IECoreMaya._parameterisedHolderAssignModificationState( None, None, None, None	)
				maya.cmds.ieParameterisedHolderModification( self.fullPathName(), classNameOrParameterised, classVersion, envVarName )
				# no need to despatch callbacks as that is done by the command, so that the callbacks happen on undo and redo too.
			else :
				_IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised, classVersion, envVarName )
				self._despatchSetParameterisedCallbacks( self.fullPathName() )
		else :
			result = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised )
			self._despatchSetParameterisedCallbacks( self.fullPathName() )

	## Returns a tuple of the form (parameterised, className, classVersion, searchPathEnvVar).
	def getParameterised( self ) :

		return _IECoreMaya._parameterisedHolderGetParameterised( self )

	## Returns a context manager for use with the with statement. This can be used to
	# scope edits to Parameter values (including the classes held by ClassParameters and
	# ClassVectorParameters) in such a way that they are automatically transferred onto
	# the maya attributes and furthermore in an undoable fashion.
	def parameterModificationContext( self ) :
	
		return _ParameterModificationContext( self )
		
	## Sets the values of the plugs representing the parameterised object,
	# using the current values of the parameters. If the undoable parameter is True
	# then this method is undoable using the standard maya undo mechanism.
	# \note If this is applied to a node in a reference, then reference edits will
	# be produced for every parameter plug, even if the values are not changing.
	# You may prefer to set parameter values within a parameterModificationContext()
	# instead as this automatically transfers the values to maya, while also avoiding
	# the reference edit problem.
	def setNodeValues( self, undoable=True ) :

		if undoable :
			maya.cmds.ieParameterisedHolderSetValue( self.fullPathName() )
		else :
			_IECoreMaya._parameterisedHolderSetNodeValues( self )

	## Set the value for the plug representing parameter, using the current
	# value of the parameter. If the undoable parameter is True
	# then this method is undoable using the standard maya undo mechanism.
	def setNodeValue( self, parameter, undoable=True ) :

		if undoable :
			maya.cmds.ieParameterisedHolderSetValue( self.fullPathName(), plug=self.parameterPlug( parameter ).partialName() )
		else :
			_IECoreMaya._parameterisedHolderSetNodeValue( self, parameter )

	## Transfers the values from the plugs of the node onto the
	# parameters of the held Parameterised object.
	def setParameterisedValues( self ) :

		return _IECoreMaya._parameterisedHolderSetParameterisedValues( self )

	## Sets the value of parameter from the value held by the plug representing it.
	def setParameterisedValue( self, parameter ) :

		return _IECoreMaya._parameterisedHolderSetParameterisedValue( self, parameter )

	## Returns the OpenMaya.MPlug object responsible for representing the given parameter.
	def parameterPlug( self, parameter ) :

		plugName = _IECoreMaya._parameterisedHolderParameterPlug( self, parameter )
		if plugName == "" :
			return maya.OpenMaya.MPlug()
		
		return StringUtil.plugFromString( self.fullPathName() + "." + plugName )

	## Returns a string containing a full pathname for the plug representing the given parameter.
	def parameterPlugPath( self, parameter ) :

		plugName = _IECoreMaya._parameterisedHolderParameterPlug( self, parameter )
		if not plugName :
			return ""

		return self.fullPathName() + "." + plugName

	## Returns the IECore.Parameter object being represented by the given fullPathName
	# of the maya plug or its OpenMaya.MPlug instance.
	def plugParameter( self, plug ) :

		if isinstance( plug, str ) or isinstance( plug, unicode ) :
			plug = StringUtil.plugFromString( plug )

		return _IECoreMaya._parameterisedHolderPlugParameter( self, plug )

	## Returns the full path name to this node.
	def fullPathName( self ) :

		try :
			f = maya.OpenMaya.MFnDagNode( self.object() )
			return f.fullPathName()
		except :
			pass

		return self.name()

	## Add a callback which will be invoked whenever FnParameterisedHolder.setParameterised
	# is called. The expected function signature is callback( FnParameterisedHolder ).
	@classmethod
	def addSetParameterisedCallback( cls, callback ) :

		cls.__setParameterisedCallbacks.add( callback )

	## Remove a previously added callback.
	@classmethod
	def removeSetParameterisedCallback( cls, callback ) :

		cls.__setParameterisedCallbacks.remove( callback )

	__setParameterisedCallbacks = set()
	@classmethod
	def _despatchSetParameterisedCallbacks( cls, nodeName ) :
	
		fnPH = FnParameterisedHolder( nodeName )
		for c in cls.__setParameterisedCallbacks :
			c( fnPH )

	## Adds a callback which will be invoked whenever FnParameterisedHolder.setClassVectorParameterClasses
	# is called. The expected function signature is callback( FnParameterisedHolder, parameter )
	@classmethod
	def addSetClassVectorParameterClassesCallback( cls, callback ) :
	
		cls.__setClassVectorParameterClassesCallbacks.add( callback )
	
	## Removes a callback added previously with addSetClassVectorParameterClassesCallback()
	@classmethod
	def removeSetClassVectorParameterClassesCallback( cls, callback ) :
	
		cls.__setClassVectorParameterClassesCallbacks.remove( callback )
		
	__setClassVectorParameterClassesCallbacks = set()
	
	# Invoked by the ieParameterisedHolderModification MPxCommand. It must be invoked from there
	# rather than the methods above so that callbacks get correctly despatched during undo and redo.
	@classmethod
	def _despatchSetClassVectorParameterClassesCallbacks( cls, plugPath ) :
		
		# This function gets called deferred (on idle) from ParameterisedHolderSetClassParameterCmd.cpp.
		# Because of the deferred nature of the call, it's possible that the plug has been destroyed before
		# we're called - in this case we just don't despatch callbacks.
		## \todo It might be better to not defer the call to this function, and have any callbacks which
		# need deferred evaluation (the ui callbacks in ClassVectorParameterUI for instance) arrange for that
		# themselves.
		if not maya.cmds.objExists( plugPath ) :
			return
		
		fnPH = FnParameterisedHolder( StringUtil.nodeFromAttributePath( plugPath ) )
		parameter = fnPH.plugParameter( plugPath )
		for c in cls.__setClassVectorParameterClassesCallbacks :
			c( fnPH, parameter )
	
	## Adds a callback which will be invoked whenever FnParameterisedHolder.setClassParameterClass
	# is called. The expected function signature is callback( FnParameterisedHolder, parameter )
	@classmethod
	def addSetClassParameterClassCallback( cls, callback ) :
	
		cls.__setClassParameterClassCallbacks.add( callback )
	
	## Removes a callback added previously with addSetClassParameterClassCallback()
	@classmethod
	def removeSetClassParameterClassCallback( cls, callback ) :
	
		cls.__setClassParameterClassCallbacks.remove( callback )  
		
	__setClassParameterClassCallbacks = set()
	
	# Invoked by the ieParameterisedHolderModification MPxCommand. It must be invoked from there
	# rather than the methods above so that callbacks get correctly despatched during undo and redo.
	@classmethod
	def _despatchSetClassParameterClassCallbacks( cls, plugPath ) :
		
		# See comment in _despatchSetClassVectorParameterClassesCallbacks
		if not maya.cmds.objExists( plugPath ) :
			return

		fnPH = FnParameterisedHolder( StringUtil.nodeFromAttributePath( plugPath ) )
		parameter = fnPH.plugParameter( plugPath )
		for c in cls.__setClassParameterClassCallbacks :
			c( fnPH, parameter )
			
	def _classParameterStates( self, parameter=None, parentParameterPath="", result=None ) :
	
		if result is None :
			result = IECore.CompoundData()
			
		if parameter is None :
			parameter = self.getParameterised()[0].parameters()	
	
		parameterPath = parameter.name
		if parentParameterPath :
			parameterPath = parentParameterPath + "." + parameterPath
			
		if isinstance( parameter, IECore.ClassParameter ) :
				
			classInfo = parameter.getClass( True )
			result[parameterPath] = IECore.CompoundData( {
				"className" : IECore.StringData( classInfo[1] ),
				"classVersion" : IECore.IntData( classInfo[2] ),
				"searchPathEnvVar" : IECore.StringData( classInfo[3] ),	
			} )

		elif isinstance( parameter, IECore.ClassVectorParameter ) :
		
			classInfo = parameter.getClasses( True )
			if classInfo :
				classInfo = zip( *classInfo )
			else :
				classInfo = [ [], [], [], [] ]
						
			result[parameterPath] = IECore.CompoundData({
				"parameterNames" : IECore.StringVectorData( classInfo[1] ),
				"classNames" : IECore.StringVectorData( classInfo[2] ),
				"classVersions" : IECore.IntVectorData( classInfo[3] ),
			} )

		if isinstance( parameter, IECore.CompoundParameter ) :
		
			for c in parameter.values() :
			
				self._classParameterStates( c, parameterPath, result )

		return result
			
						
class _ParameterModificationContext :

	def __init__( self, fnPH ) :
	
		self.__fnPH = fnPH

	def __enter__( self ) :
	
		self.__fnPH.setParameterisedValues()
		self.__originalValues = self.__fnPH.getParameterised()[0].parameters().getValue().copy()
		self.__originalClasses = self.__fnPH._classParameterStates()
		
		return self.__fnPH.getParameterised()[0]
		
	def __exit__( self, type, value, traceBack ) :
	
		_IECoreMaya._parameterisedHolderAssignModificationState(
			self.__originalValues,
			self.__originalClasses,
			self.__fnPH.getParameterised()[0].parameters().getValue().copy(),
			self.__fnPH._classParameterStates(),
		)
		
		maya.cmds.ieParameterisedHolderModification( self.__fnPH.fullPathName() )


from TransientParameterisedHolderNode import TransientParameterisedHolderNode


import maya.OpenMaya
import maya.cmds
import IECoreMaya
import _IECoreMaya
import StringUtil

## \deprecated Use the FnTransientParameterisedHolderNode instead.
class TransientParameterisedHolderNode( maya.OpenMaya.MFnDependencyNode ) :

	def __init__( self, layoutName, classNameOrParameterised, classVersion=None, envVarName=None ) :
		""" Creates a temporary TransientParameterisedHolderNode in order to present the UI for the specified
		    parameterised object in the given layout. The node is automatically deleted when the holding
		    layout is destroyed """

		nodeName = maya.cmds.createNode( "ieTransientParameterisedHolderNode", skipSelect = True )

		# Script jobs aren't available from maya.cmds. Maya Python bindings generate swig warnings
		# such as "swig/python detected a memory leak of type 'MCallbackId *', no destructor found"
		IECoreMaya.mel( 'scriptJob -uiDeleted "%s" "delete %s" -protected' % ( layoutName, nodeName ) )

		object = StringUtil.dependencyNodeFromString( nodeName )

		maya.OpenMaya.MFnDependencyNode.__init__( self, object )

		if isinstance( classNameOrParameterised, str ) :
			res = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised, classVersion, envVarName )
		else :
			assert( not classVersion )
			assert( not envVarName )

			res = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised )

		parameterised = self.getParameterised()[0]

		if parameterised:
			maya.cmds.setParent( layoutName )
			IECoreMaya.ParameterUI.create( object, parameterised.parameters() )
			maya.cmds.setParent( layoutName )

		return res

	def getParameterised( self ) :

		return _IECoreMaya._parameterisedHolderGetParameterised( self )

from FnConverterHolder import FnConverterHolder


import maya.cmds
import IECore
import IECoreMaya
import os.path

class FnConverterHolder( IECoreMaya.FnParameterisedHolder ) :

	def __init__( self, node ) :

		IECoreMaya.FnParameterisedHolder.__init__( self, node )

		if self.typeName()!="ieConverterHolder" :

			raise TypeError( "\"%s\" is not a ConverterHolder." )

	## Returns the converter held by this node
	def converter( self ) :

		c = self.getParameterised()
		c = c[0]
		if not c or not c.isInstanceOf( IECoreMaya.Converter.staticTypeId() ) :
			return None

		return c

	## Performs a conversion at the specified frame
	def convertAtFrame( self, frame ) :

		c = self.converter()
		if not c :
			raise RuntimeError( "No converter found on node \"%s\"." % self.name() )

		fileName = str( maya.cmds.getAttr( self.name() + ".fileName", asString = True ) )
		try :
			f = IECore.FileSequence( fileName, IECore.FrameRange( frame, frame ) )
			fileName = f.fileNameForFrame( frame )
		except :
			pass

		if fileName=="" :
			raise RuntimeError( "No filename specified on node \"%s\"." % self.name() )

		maya.cmds.currentTime( frame )
		o = c.convert()
		if not o :
			raise RuntimeError( "Conversion failed for node \"%s\"." % self.name() )

		w = IECore.Writer.create( o, fileName )
		if not w :
			ext = os.path.splitext( fileName )[1]
			raise RuntimeError(	"Unable to create a Writer for object of type \"%s\" and file type \"%s\"." % ( o.typeName(), ext ) )

		w.write()

	## Performs a conversion for every frame in an IECore.FrameList object
	def convertAtFrames( self, frameList ) :

		for f in frameList.asList() :

			self.convertAtFrame( f )

from StringUtil import *


import IECore
import maya.OpenMaya
import re

__all__ = [ "dependencyNodeFromString", "dagPathFromString", "plugFromString" ]

## Utility function to return a dependency node as an MObject when
# given it's name as a string.
def dependencyNodeFromString( s ) :

	sl = maya.OpenMaya.MSelectionList()
	sl.add( s )

	if sl.length() > 1 :
		IECore.msg( IECore.Msg.Level.Warning, "IECoreMaya.dependencyNodeFromString", "Name \"%s\" is not unique." % s )

	result = maya.OpenMaya.MObject()
	sl.getDependNode( 0, result )
	return result

## Utility function to return the parent string when
# given a UI or DAG node's full path name as a string.
def parentFromString( s ) :

	tokens = s.split('|')
	tokens.pop()
	parent = '|'.join( tokens )

	return parent

## Utility function to return an MDagPath when
# given it's name as a string.
def dagPathFromString( s ) :

	sl = maya.OpenMaya.MSelectionList()
	sl.add( s )

	if sl.length() > 1 :
		IECore.msg( IECore.Msg.Level.Warning, "IECoreMaya.dagPathFromString", "Name \"%s\" is not unique." % s )

	result = maya.OpenMaya.MDagPath()
	sl.getDependNode( 0, result )
	return result

## Utility function to return an MPlug when
# given it's name as a string.
def plugFromString( s ) :

	sl = maya.OpenMaya.MSelectionList()
	sl.add( s )

	if sl.length() > 1 :
		IECore.msg( IECore.Msg.Level.Warning, "IECoreMaya.plugFromString", "Name \"%s\" is not unique." % s )

	result = maya.OpenMaya.MPlug()
	sl.getPlug( 0, result )
	return result

## Returns a full path to an MPlug.
def pathFromPlug( p ) :

	try :
		f = maya.OpenMaya.MFnDagNode( p.node() )
		nodePath = f.fullPathName()
	except :
		f = maya.OpenMaya.MFnDependencyNode( p.node() )
		nodePath = f.name()

	return nodePath + "." + p.partialName()

## Extracts the node name from a full path to an attribute.
def nodeFromAttributePath( a ) :

	return re.match( "^[^.]*", a ).group( 0 )

from MayaTypeId import MayaTypeId


import maya.OpenMaya
import _IECoreMaya

class _MayaTypeIdMeta(type):

	def __getattr__( self, n ) :

		if not hasattr( _IECoreMaya._MayaTypeId, n ) :

			raise RuntimeError( "MayaTypeId '" + str(n) + "' has not been bound" )

		return maya.OpenMaya.MTypeId( getattr( _IECoreMaya._MayaTypeId, n ) )

class MayaTypeId :

	__metaclass__ = _MayaTypeIdMeta


from ParameterPanel import ParameterPanel


##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import maya.cmds as cmds
import maya.OpenMaya

import IECore
import IECoreMaya

## \todo Reimplement this in terms of the new Panel base class.
## \todo Prefix methods which aren't intended to be public to make them either private or protected.
class ParameterPanel :

	panels = {}

	@staticmethod
	def trashDropCallback( dragControl, dropControl, messages, x, y, dragType, container ) :

		if len(messages) == 2 and messages.pop(0) == "ParameterUI" :

			argsDictStr = messages.pop()
			argsDict = eval( argsDictStr )

			container.removeControl( argsDict['nodeName'], argsDict['longParameterName'], deferredUIDeletion = True )

	@staticmethod
	def newControlDropCallback( dragControl, dropControl, messages, x, y, dragType, container ) :

		if len(messages) == 2 and messages.pop(0) == "ParameterUI" :

			argsDictStr = messages.pop()
			argsDict = eval( argsDictStr )

			container.addControl( argsDict['nodeName'], argsDict['longParameterName'] )


	class ParameterUIContainer :

		def __init__( self ) :

			self.formLayout = None
			self.containerLayout = maya.cmds.setParent( query = True )
			self.parameters = []
			self.parameterLayouts = {}

			self.restoreParameters = []

		def addControl( self, node, longParameterName ) :

			fnPH = IECoreMaya.FnParameterisedHolder( node )
			parameterised = fnPH.getParameterised()[0]

			parameter = parameterised.parameters()

			for p in longParameterName.split( '.' ) :

				if p :

					parameter = getattr( parameter, p )

			assert( self.containerLayout )
			maya.cmds.setParent( self.containerLayout )

			if not ( node, longParameterName ) in self.parameterLayouts:

				n = longParameterName.split(".")

				if type( n ) is list:

					# Take off the last element (to retrieve the parent parameter name),
					# because ParameterUI.create will add it again.
					n.pop()
					parentParameterName = ".".join( n )

				else :

					parentParameterName = longParameterName

				newLayout = IECoreMaya.ParameterUI.create(
					node,
					parameter,
					labelWithNodeName = True,
					longParameterName = parentParameterName,
					withCompoundFrame = True
				).layout()

				self.parameters.append( ( node, longParameterName ) )
				self.parameterLayouts[ ( node, longParameterName ) ] = newLayout

				maya.cmds.file(
					modified = True
				)


		def removeControl( self, node, longParameterName, deferredUIDeletion = False ) :

			# In case another panel's control has been dropped onto our trash can
			if not ( node, longParameterName ) in self.parameterLayouts :

				return

			layoutToDelete = self.parameterLayouts[ ( node, longParameterName ) ]

			if maya.cmds.layout( layoutToDelete, query = True, exists = True ) :

				if deferredUIDeletion :

					# We don't want to delete the UI in the middle of a drag'n'drop event, it crashes Maya
					maya.cmds.evalDeferred( "import maya.cmds as cmds; cmds.deleteUI( '%s', layout = True)" % ( layoutToDelete ) )

				else :

					maya.cmds.deleteUI( '%s' % ( layoutToDelete ), layout = True)

			del self.parameterLayouts[ ( node, longParameterName ) ]

			self.parameters.remove( ( node, longParameterName ) )

			maya.cmds.file(
				modified = True
			)

		# We have the "args" argument to allow use in a Maya UI callback, which passes us extra arguments that we don't need
		def removeAllControls( self, args = None ) :

			toRemove = list( self.parameters )

			for i in toRemove :

				self.removeControl( i[0], i[1] )

			assert( len( self.parameters ) == 0 )
			assert( len( self.parameterLayouts ) == 0 )


		def add( self, panel ):

			try :
				self.parameterLayouts = {}

				# If "add" has been called, and we already have some parameters, it means we're been torn-off and should
				# recreate all our controls in the new window.

				if not self.restoreParameters and self.parameters :

					self.restoreParameters = list( self.parameters )

				self.parameters = []

				maya.cmds.waitCursor( state = True )

				menuBar = maya.cmds.scriptedPanel(
					panel,
					query = True,
					control = True
				)

				self.formLayout = maya.cmds.formLayout(
					numberOfDivisions = 100
				)

				maya.cmds.setParent(
					menuBar
				)

				editMenu = maya.cmds.menu(
					label = "Edit"
				)

				maya.cmds.menuItem(
					label = "Remove All",
					parent = editMenu,
					command = IECore.curry( ParameterPanel.ParameterUIContainer.removeAllControls, self )
				)

				maya.cmds.setParent( self.formLayout )

				scrollPane = maya.cmds.scrollLayout(
					dropCallback = IECore.curry( ParameterPanel.newControlDropCallback, container = self ),
					parent = self.formLayout
				)

				trashCan = maya.cmds.iconTextStaticLabel(
					image = "smallTrash.xpm",
					label = "",
					height = 20,
					dropCallback = IECore.curry( ParameterPanel.trashDropCallback, container = self ),
					parent = self.formLayout
				)

				maya.cmds.rowLayout( parent = scrollPane )

				self.containerLayout = maya.cmds.columnLayout(
				)

				maya.cmds.formLayout(
					self.formLayout,
					edit=True,
					attachForm = [
						( trashCan, 'bottom', 5 ),
						( trashCan, 'right', 5 ),

						( scrollPane, 'top', 5 ),
						( scrollPane, 'left', 5 ),
						( scrollPane, 'right', 5 ),
						( scrollPane, 'bottom', 25 )
					],

					attachNone = [
						( trashCan, 'left' ),
						( trashCan, 'top' )
					]
				)

				for i in self.restoreParameters :

					self.addControl( i[0], i[1] )

				self.restoreParameters = []

			except :

				raise

			finally :

				maya.cmds.waitCursor( state = False )

		def delete( self ) :

			self.removeAllControls()

		def init( self ) :

			# Called after a scene has been loaded (or after a "file | new" operation), and "add" has already been called.

			pass

		def remove( self ) :

			# Called before tearing-off

			pass

		def restoreData( self ) :

			version = 1

			return repr( ( version, self.parameters ) )

		def restore( self, data ) :

			self.removeAllControls()

			dataTuple = eval( data )

			version = dataTuple[0]

			self.restoreParameters = dataTuple[1]

			if self.formLayout :

				for i in self.restoreParameters :

					self.addControl( i[0], i[1] )

				self.restoreParameters = []


	@staticmethod
	def create( panel ) :

		ParameterPanel.panels[ panel ] = ParameterPanel.ParameterUIContainer()


	@staticmethod
	def init( panel ) :

		assert( panel in ParameterPanel.panels )

		ParameterPanel.panels[ panel ].init()


	@staticmethod
	def add( panel ) :

		assert( panel in ParameterPanel.panels )

		ParameterPanel.panels[ panel ].add( panel )

	@staticmethod
	def remove( panel ) :

		assert( panel in ParameterPanel.panels )

		ParameterPanel.panels[ panel ].remove()

	@staticmethod
	def delete( panel ) :

		assert( panel in ParameterPanel.panels )

		ParameterPanel.panels[ panel ].delete()

	@staticmethod
	def save( panel ) :

		assert( panel in ParameterPanel.panels )

		return 'ieParameterPanelRestore("%s", "%s")' % ( panel, ParameterPanel.panels[ panel ].restoreData() )

	@staticmethod
	def restore( panel, data ) :

		assert( panel in ParameterPanel.panels )

		ParameterPanel.panels[ panel ].restore( data )

from AttributeEditorControl import AttributeEditorControl


import maya.cmds
import maya.mel

## A base class to help in creating custom attribute editor controls
# in a nice object oriented manner. After deriving from this class you
# can instantiate a control from an attribute editor template using mel
# of the following form :
#
# ieAttributeEditorControl( "DerivedClassName", "attributeName" )
class AttributeEditorControl :

	## Derived classes should first call the base class __init__, before
	# building their ui.
	def __init__( self, attribute ) :

		self.__nodeName = attribute.split( "." )[0]
		self.__attributeName = attribute

	## Derived classes should first call the base class replace, before
	# reattaching their ui to the new attribute.
	def replace( self, attribute ) :

		self.__nodeName = attribute.split( "." )[0]
		self.__attributeName = attribute

	## Returns the name of the node this ui is used for.
	def nodeName( self ) :

		return self.__nodeName

	## Returns the name of the attribute this ui is used for.
	def attributeName( self ) :

		return self.__attributeName

	@staticmethod
	def _new( className, attribute ) :

		# we smuggle the class name as a fake attribute name so we
		# need to get it back out now.
		className = ".".join( className.split( "." )[1:] )

		# the class name might also be in a namespace that isn't imported
		# in this scope. so import it.
		if not "." in className :
			cls = eval( className )
		else :
			names = className.split( "." )
			namespace = __import__( ".".join( names[:-1] ) )
			cls = getattr( namespace, names[-1] )

		parent = maya.cmds.setParent( q=True )
		control = cls( attribute )
		maya.cmds.setParent( parent )

		AttributeEditorControl.__instances[parent] = control

		# Script jobs aren't available from maya.cmds. Maya Python bindings generate swig warnings
		# such as "swig/python detected a memory leak of type 'MCallbackId *', no destructor found"
		maya.mel.eval( 'scriptJob -protected -uiDeleted "%s" "python \\"IECoreMaya.AttributeEditorControl._uiDeleted( \'%s\' )\\""' % ( parent, parent ) )

	@staticmethod
	def _replace( attribute ) :

		parent = maya.cmds.setParent( q=True )
		control = AttributeEditorControl.__instances[parent]
		control.replace( attribute )

	@staticmethod
	def _uiDeleted( parent ) :

		del AttributeEditorControl.__instances[parent]

	# Maps from parent ui names to AttributeEditorControl instances
	__instances = {}

from FnProceduralHolder import FnProceduralHolder


from __future__ import with_statement

import maya.OpenMaya
import maya.cmds

import IECore
import IECoreMaya
import _IECoreMaya
from FnParameterisedHolder import FnParameterisedHolder
from FnDagNode import FnDagNode

## A function set for operating on the IECoreMaya::ProceduralHolder type.
class FnProceduralHolder( FnParameterisedHolder ) :

	## Initialise the function set for the given procedural object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		FnParameterisedHolder.__init__( self, object )

	## Creates a new node under a transform of the specified name and holding
	# the specified procedural class type. Returns a function set instance operating on this new node.
	@staticmethod
	def create( parentName, className, classVersion=-1 ) :

		fnDN = FnDagNode.createShapeWithParent( parentName, "ieProceduralHolder" )
		fnPH = FnProceduralHolder( fnDN.object() )
		maya.cmds.sets( fnPH.fullPathName(), add="initialShadingGroup" )
		fnPH.setProcedural( className, classVersion, undoable=False ) # undo for the node creation is all we need
		
		return fnPH

	## Convenience method to call setParameterised with the environment variable
	# for the searchpaths set to "IECORE_PROCEDURAL_PATHS".
	def setProcedural( self, className, classVersion=None, undoable=True ) :

		self.setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS", undoable )

	## Convenience method to return the ParameterisedProcedural class held inside this
	# node.
	def getProcedural( self ) :

		return self.getParameterised()[0]

	## Returns a set of the names of the components within the procedural. These names
	# are specified by the procedural by setting the "name" attribute in the
	# renderer
	def componentNames( self ) :
	
		# Makes sure that the scene has been built at least once with the
		# current node values, so there will be something in the plug
		# for us to read.
		self.scene()
		
		attributeName = "%s.proceduralComponents" % self.fullPathName()
	
		result = set()
		for i in range( maya.cmds.getAttr( attributeName, size=True ) ) :
			result.add( maya.cmds.getAttr( "%s[%i]" % ( attributeName, i ) ) )
		
		return result
		
	## Returns a set of the names of any currently selected components. These names
	# are specified by the procedural by setting the "name" attribute in the
	# renderer.
	def selectedComponentNames( self ) :

		result = set()

		s = maya.OpenMaya.MSelectionList()
		maya.OpenMaya.MGlobal.getActiveSelectionList( s )

		fullPathName = self.fullPathName()
		for i in range( 0, s.length() ) :

			try :

				p = maya.OpenMaya.MDagPath()
				c = maya.OpenMaya.MObject()
				s.getDagPath( i, p, c )

				if p.node()==self.object() :

					fnC = maya.OpenMaya.MFnSingleIndexedComponent( c )
					a = maya.OpenMaya.MIntArray()
					fnC.getElements( a )

					for j in range( 0, a.length() ) :

						result.add( maya.cmds.getAttr( fullPathName + ".proceduralComponents[" + str( a[j] ) + "]" ) )

			except :
				pass

		return result

	## Selects the components specified by the passed names. If replace is True
	# then the current selection is deselected first.
	def selectComponentNames( self, componentNames ) :

		if not isinstance( componentNames, set ) :
			componentNames = set( componentNames )

		fullPathName = self.fullPathName()
		validIndices = maya.cmds.getAttr( fullPathName + ".proceduralComponents", multiIndices=True )
		toSelect = []
		for i in validIndices :
			componentName = maya.cmds.getAttr( fullPathName + ".proceduralComponents[" + str( i ) + "]" )
			if componentName in componentNames :
				toSelect.append( fullPathName + ".f[" + str( i ) + "]" )

		maya.cmds.select( clear=True )
		maya.cmds.selectMode( component=True )
		maya.cmds.hilite( fullPathName )
		for s in toSelect :
			maya.cmds.select( s, add=True )

	## Returns the IECoreGL.Scene displayed by this node.
	def scene( self ) :

		return _IECoreMaya._proceduralHolderScene( self )

	## Creates a hierarchy of maya nodes representing the output of the procedural
	# at the current frame. If parent is specified then the geometry will be parented
	# under it, otherwise it will share the parent of the procedural. Returns the
	# path to the top level transform of the new geometry.
	def convertToGeometry( self, parent=None ) :
	
		procedural = self.getProcedural()
		if not procedural :
			return None
			
		renderer = IECore.CapturingRenderer()
		with IECore.WorldBlock( renderer ) :
			procedural.render( renderer )
		
		world = renderer.world()
		
		if parent is None :
			parent = maya.cmds.listRelatives( self.fullPathName(), fullPath=True, parent=True )
			if parent is not None :
				parent = parent[0]
			
		IECoreMaya.ToMayaGroupConverter( world ).convert( parent )
		
		return maya.cmds.listRelatives( parent, fullPath=True )[-1]
	
	## Returns a path to a plug which outputs the transform for the specified component. The plug
	# will have "componentTranslate", "componentRotate" and "componentScale" children with the appropriate values.
	def componentTransformPlugPath( self, componentName ) :
	
		i = self.__componentIndex( componentName )
		return self.fullPathName() + ".componentTransform[" + str( i ) + "]"

	## Returns a path to a plug which outputs the bounding box of the specified component. The 
	# plug will have "componentBoundMin", "componentBoundMax" and "componentBoundCenter" children with the
	# appropriate values.
	def componentBoundPlugPath( self, componentName ) :
	
		i = self.__componentIndex( componentName )
		return self.fullPathName() + ".componentBound[" + str( i ) + "]"

	def __componentIndex( self, componentName ) :
	
		fullPathName = self.fullPathName()

		queryIndices = maya.cmds.getAttr( fullPathName + ".componentQueries", multiIndices=True )
		if not queryIndices :
			i = 0
		else :
			for i in queryIndices :
				if maya.cmds.getAttr( fullPathName + ".componentQueries[" + str( i ) + "]" ) == componentName :
					return i
			i += 1
			
		maya.cmds.setAttr( fullPathName + ".componentQueries[" + str( i ) + "]", componentName, type="string" )
		return i
				

from OpWindow import OpWindow


import IECoreMaya

import maya.cmds
import traceback

## This class creates a window holding an instance of IECore.Op, presenting a ui
# for the modification of it's parameters. The user can then execute the op interactively.
## \todo We need to close this window if the node is deleted - for instance when the user makes a fresh scene.
class OpWindow( IECoreMaya.UIElement ) :

	def __init__( self, op ) :

		window = maya.cmds.window( title="Op Settings" )

		IECoreMaya.UIElement.__init__( self, window )

		form = maya.cmds.formLayout()

		infoRow = maya.cmds.rowLayout()
		maya.cmds.text( label = op.typeName(), annotation = op.description, font = "boldLabelFont" )

		maya.cmds.setParent( form )

		scrollLayout = maya.cmds.scrollLayout( horizontalScrollBarThickness=0 )
		column = maya.cmds.columnLayout()
		maya.cmds.separator( parent=column, style="none" )
		maya.cmds.separator( parent=column, style="none" )

		maya.cmds.setParent( form )

		cancelButton = maya.cmds.button( label="Cancel", command="import maya.cmds; maya.cmds.deleteUI( \"%s\" )" % window )
		resetButton = maya.cmds.button( label="Reset", command=self._createCallback( self.__revertPressed ) )
		goButton = maya.cmds.button( label="Go!", command=self._createCallback( self.__goPressed ) )

		self.__fnTPH = IECoreMaya.FnTransientParameterisedHolderNode.create( column, op )

		maya.cmds.setParent( form )

		maya.cmds.formLayout(
			form,
			edit = True,
			attachForm = [
				( infoRow, "top", 5 ),
				( infoRow, "left", 5 ),
				( infoRow, "right", 5 ),
				( scrollLayout, "left", 5 ),
				( scrollLayout, "right", 5 ),
				( cancelButton, "right", 5 ),
				( goButton, "left", 5 ),
				( cancelButton, "bottom", 5 ),
				( resetButton, "bottom", 5 ),
				( goButton, "bottom", 5 ),
			],
			attachPosition = [
				( cancelButton, "left", 5, 66 ),
				( goButton, "right", 5, 33 ),
			],
			attachControl = [
				( scrollLayout, "top", 5, infoRow ),
				( scrollLayout, "bottom", 5, cancelButton ),
				( resetButton, "left", 5, goButton ),
				( resetButton, "right", 5, cancelButton ),
			],
			attachNone = [
				( cancelButton, "top" ),
				( resetButton, "top" ),
				( goButton, "top" ),
			]
		)

		maya.cmds.showWindow( window )

	def __revertPressed( self ) :

		op = self.__fnTPH.getParameterised()[0]
		op.parameters().setValue( op.parameters().defaultValue )
		self.__fnTPH.setNodeValues()

	def __goPressed( self ) :

		self.__fnTPH.setParameterisedValues()
		op = self.__fnTPH.getParameterised()[0]

		valid = op.parameters().valueValid()
		if not valid[0] :
			maya.cmds.confirmDialog(
				title="Parameter Error",
				message=valid[1], button="OK",
				defaultButton="OK",
				parent=self._topLevelUI()
			)
			return

		result = None
		try :
			result = op()
		except Exception, e :
			buttonPressed = maya.cmds.confirmDialog(
				title="Execution Error",
				message="Error : " + e.message,
				button=( "Print Details", "OK" ),
				defaultButton="OK",
				parent=self._topLevelUI()
			)
			if buttonPressed=="Print Details" :
				traceback.print_exc()

		if result is not None :
			maya.cmds.deleteUI( self._topLevelUI() )

from FnTransientParameterisedHolderNode import FnTransientParameterisedHolderNode


from __future__ import with_statement

import maya.OpenMaya
import maya.cmds
import IECoreMaya
import _IECoreMaya
import StringUtil

class FnTransientParameterisedHolderNode( IECoreMaya.FnParameterisedHolder ) :

	def __init__( self, object ) :

		IECoreMaya.FnParameterisedHolder.__init__( self, object )

	## Creates a temporary TransientParameterisedHolderNode in order to present the UI for the specified
	# parameterised object in the given layout. The node is automatically deleted when the holding
	# layout is destroyed. Returns a FnTransientParameterisedHolderNode object operating on the new node.
	@staticmethod
	def create( layoutName, classNameOrParameterised, classVersion=None, envVarName=None ) :

		with IECoreMaya.UndoDisabled() :

			nodeName = maya.cmds.createNode( "ieTransientParameterisedHolderNode", skipSelect = True )

			# Script jobs aren't available from maya.cmds. Maya Python bindings generate swig warnings
			# such as "swig/python detected a memory leak of type 'MCallbackId *', no destructor found"
			IECoreMaya.mel( 'scriptJob -uiDeleted "%s" "python( \\\"import IECoreMaya; IECoreMaya.FnTransientParameterisedHolderNode._deleteNode( \'%s\' )\\\" )" -protected' % ( layoutName, nodeName ) )

			fnTPH = FnTransientParameterisedHolderNode( nodeName )

			if isinstance( classNameOrParameterised, str ) :
				fnTPH.setParameterised( classNameOrParameterised, classVersion, envVarName )
			else :
				assert( not classVersion )
				assert( not envVarName )

				fnTPH.setParameterised( classNameOrParameterised )

			parameterised = fnTPH.getParameterised()[0]

			if parameterised :
				maya.cmds.setParent( layoutName )
				IECoreMaya.ParameterUI.create( fnTPH.fullPathName(), parameterised.parameters() )
				maya.cmds.setParent( layoutName )

		return fnTPH

	@staticmethod
	def _deleteNode( nodeName ) :

		with IECoreMaya.UndoDisabled() :

			if maya.cmds.objExists( nodeName ) :
				maya.cmds.delete( nodeName )




from UndoDisabled import UndoDisabled


import maya.cmds

## A context object intended for use with python's "with" syntax. It ensures
# that all operations in the with block are performed with maya's undo disabled,
# and that undo is returned to its previous state when the block exits.
class UndoDisabled :

	def __enter__( self ) :

		self.__prevState = maya.cmds.undoInfo( query=True, state=True )
		maya.cmds.undoInfo( stateWithoutFlush=False )

	def __exit__( self, type, value, traceBack ) :

		maya.cmds.undoInfo( stateWithoutFlush=self.__prevState )


from ModalDialogue import ModalDialogue


import IECore
import IECoreMaya
import maya.cmds

## This class provides a useful base for implementing modal dialogues in an
# object oriented manner.
class ModalDialogue( IECoreMaya.UIElement ) :

	## Derived classes should call the base class __init__ before constructing
	# their ui. The result of self._topLevelUI() will be a form layout in which
	# the ui should be constructed. Clients shouldn't construct ModalDialogues
	# directly but instead use the run() method documented below.
	def __init__( self ) :

		IECoreMaya.UIElement.__init__( self, maya.cmds.setParent( query=True ) )
		ModalDialogue.__returnObject = None;

	## Should be called by derived classes when they wish to close their dialogue.
	# The result will be returned to the caller of the run() method. This differs
	# from the default maya behaviour for a layoutDialog, as we allow any python
	# type to be returned. If the dialog is dismissed directly with the maya.cmd
	# then that value will be returned instead.
	def _dismiss( self, result ) :

		ModalDialogue.__returnObject = result
		maya.cmds.layoutDialog( dismiss = 'ModalDialogueReturnObject' )

	## Call this method to open a dialogue - the return value is the object supplied
	# by the derived class, or the string supplied to layoutDialog( dismiss=%s ) if
	# called directly. If the string 'None', 'True', or 'False' is passed to a
	# direct call, then the equivalent python object is returned instead of a string.
	@classmethod
	def run( cls, *args, **kwargs ) :

		ModalDialogue.__toInstantiate = cls
		ModalDialogue.__calltimeArgs = args
		ModalDialogue.__calltimeKwargs = kwargs
		
		title = maya.mel.eval( 'interToUI( "%s" )' % cls.__name__ )
		mayaResult = maya.cmds.layoutDialog( ui = 'import IECoreMaya; IECoreMaya.ModalDialogue._ModalDialogue__instantiate()', title = title )

		if mayaResult == 'ModalDialogueReturnObject' :
		
			stopMeLeaking = ModalDialogue.__returnObject
			ModalDialogue.__returnObject = None
			return stopMeLeaking
		
		elif mayaResult == 'None' :
			return None
		
		elif mayaResult == 'True' :
			return True
		
		elif mayaResult == 'False' :
			return False
		
		return mayaResult

	@classmethod
	def __instantiate( cls ) :

		ModalDialogue.__currentDialogue = cls.__toInstantiate( *ModalDialogue.__calltimeArgs, **ModalDialogue.__calltimeKwargs )
		ModalDialogue.__calltimeArgs = None
		ModalDialogue.__calltimeKwargs = None


from Panel import Panel


import IECore
import IECoreMaya

import maya.cmds
import sys

import pickle

## The Panel class provides a handy base class for the implementation
# of maya scripted panels. It allows the panel to be represented as
# an object with member data, reduces the number of methods that must
# be implemented, and allows normal python objects to be used as the
# state persistence mechanism.
class Panel( IECoreMaya.UIElement ) :

	## Derived classes must implement this to create their ui, calling
	# Panel.__init__ with the top level of the ui. A new instance is
	# made each time maya wishes to populate a ui with a panel (effectively
	# this method implements the create, init and add callbacks of the
	# maya scripted panel).
	def __init__( self, topLevelUI ) :

		IECoreMaya.UIElement.__init__( self, topLevelUI )

	## Must be implemented by subclasses to return a python object
	# representing their state. This is used for both persistence
	# across scene save and load, but also for when a panel is torn
	# off or reparented.
	def _saveState( self ) :

		raise NotImplementedError

	## Must be implemented to restore a state previously saved with
	# _saveState.
	def _restoreState( self, state ) :

		raise NotImplementedError

	## Call this to instantiate a panel. Note that one panel instantiation
	# as far as maya is concerned may result in multiple IECoreMaya.Panel instantiations -
	# typically one for each time maya wishes to reparent or copy the panel.
	@classmethod
	def create( cls, label=None ) :

		typeName = cls.__name__
		panel = maya.cmds.scriptedPanel( unParent=True, type=typeName, label=typeName )
		if label :
			maya.cmds.scriptedPanel( panel, edit=True, label=label )
		return panel

	## Must be called to register all subclasses. This should
	# typically be done during plugin initialisation.
	@classmethod
	def registerPanel( cls, subclass ) :

		typeName = subclass.__name__
		if not maya.cmds.scriptedPanelType( typeName, exists=True ) :

			# we have to implement the mel callbacks in iePanel.mel
			# as maya doesn't implement python scripted panels. the mel
			# methods we use call straight back to the implementations
			# below.
			t = maya.cmds.scriptedPanelType(
				typeName,
				createCallback="iePanelCreate",
				initCallback="iePanelInit",
				addCallback="iePanelAdd",
				removeCallback="iePanelRemove",
				saveStateCallback="iePanelSave",
				deleteCallback="iePanelDelete",
			)

			# t=0 is always returned in batch mode, probably because
			# batch mode can't handle UI callback creation
			assert( t==typeName or t==0)
			cls.__panelTypes[t] = subclass

		else :

			assert( cls.__panelTypes[typeName]==subclass )

	# maps from the maya panel name to a record of the Panel instance and state for
	# that panel
	__panels = {}
	# maps from the maya panel type to the Panel subclass for that type
	__panelTypes = {}

	## The methods below implement the callbacks maya requires by using the methods
	## defined above.
	##############################################################################

	@classmethod
	def __create( cls, panelName ) :

		t = maya.cmds.scriptedPanel( panelName, query=True, type=True )
		panelType = cls.__panelTypes[t]

		panelRecord = IECore.Struct()
		panelRecord.type = panelType
		panelRecord.instance = None
		panelRecord.state = None
		Panel.__panels[panelName] = panelRecord

	@classmethod
	def __init( cls, panelName ) :

		# we don't need to do anything here
		pass

	@classmethod
	def __add( cls, panelName ) :

		panelRecord = cls.__panels[panelName]
		assert( not panelRecord.instance )

		panelRecord.instance = panelRecord.type()
		if panelRecord.state :
			panelRecord.instance._restoreState( panelRecord.state )

	@classmethod
	def __remove( cls, panelName ) :

		panelRecord = cls.__panels[panelName]
		assert( panelRecord.instance )

		panelRecord.state = panelRecord.instance._saveState()
		panelRecord.instance = None

	@classmethod
	def __save( cls, panelName ) :

		panelRecord = cls.__panels[panelName]
		if panelRecord.instance :
			state = panelRecord.instance._saveState()
		else :
			state = panelRecord.state

		if not state :
			return ""

		pickledState = pickle.dumps( state )
		encodedState = pickledState.encode( "hex" )
		return "iePanelRestore( \"%s\", \"%s\" )" % ( panelName, encodedState )

	@classmethod
	def __delete( cls, panelName ) :

		del cls.__panels[panelName]

	# This one isn't actually a required part of the maya protocol, but
	# we call it in the saved state strings to restore layouts when files
	# are opened.
	@classmethod
	def __restore( cls, panelName, encodedState ) :

		panelRecord = cls.__panels[panelName]
		decodedState = encodedState.decode( "hex" )
		unpickledState = pickle.loads( decodedState )

		panelRecord.state = unpickledState
		if panelRecord.instance :
			panelRecord.instance._restoreState( panelRecord.state )

	__panelStates = {}

from WaitCursor import WaitCursor


import maya.cmds

## A context object intended for use with python's "with" syntax. It ensures
# that all operations in the with block are performed with maya's wait cursor enabled,
# and that the wait cursor is disabled when the block exits.
class WaitCursor :

	def __enter__( self ) :

		maya.cmds.waitCursor( state=True )

	def __exit__( self, type, value, traceBack ) :

		maya.cmds.waitCursor( state=False )


from FnOpHolder import FnOpHolder


from FnParameterisedHolder import FnParameterisedHolder

import maya.cmds

class FnOpHolder( FnParameterisedHolder ) :

	def __init__( self, objectOrObjectName ) :

		FnParameterisedHolder.__init__( self, objectOrObjectName )

	## Creates a new node holding a new instance of the op of the specified
	# type and version. Returns an FnOpHolder instance attached to this node.
	@staticmethod
	def create( nodeName, opType, opVersion=None ) :

		holder = maya.cmds.createNode( "ieOpHolderNode", name=nodeName, skipSelect=True )

		fnOH = FnOpHolder( holder )
		# not asking for undo, as this way we end up with a single undo action which will
		# delete the node. otherwise we get two undo actions, one to revert the setParameterised()
		# one to revert the createNode().
		fnOH.setOp( opType, opVersion, undoable=False )

		return fnOH

	## Convenience function which calls setParameterised( opType, opVersion, "IECORE_OP_PATHS" )
	def setOp( self, opType, opVersion=None, undoable=True ) :

		self.setParameterised( opType, opVersion, "IECORE_OP_PATHS", undoable )

	## Convenience function which returns getParameterised()[0]
	def getOp( self ) :

		return self.getParameterised()[0]


from UITemplate import UITemplate


import maya.cmds

## A context object for use with python's "with" syntax. This manages calls to maya.cmds.setUITemplate
# to ensure that everything in the with block is executed with the appropriate template, and the
# template is popped on exit from the block.
class UITemplate :

	def __init__( self, name ) :

		self.__name = name

	def __enter__( self ) :

		maya.cmds.setUITemplate( self.__name, pushTemplate=True )

	def __exit__( self, type, value, traceBack) :

		maya.cmds.setUITemplate( self.__name, popTemplate=True )

from FnParameterisedHolderSet import FnParameterisedHolderSet


import maya.cmds

import IECoreMaya

## A function set for operating on ieParameterisedHolderSet node type.
class FnParameterisedHolderSet( IECoreMaya.FnParameterisedHolder ) :

	## Initialise the function set for the given object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		IECoreMaya.FnParameterisedHolder.__init__( self, object )

	@staticmethod
	def create( nodeName, className, classVersion, searchPathEnvVar ) :

		node = maya.cmds.createNode( "ieParameterisedHolderSet", name=nodeName )

		fnSH = IECoreMaya.FnParameterisedHolderSet( node )
		fnSH.setParameterised( className, classVersion, searchPathEnvVar, undoable=False )

		return fnSH

from TemporaryAttributeValues import TemporaryAttributeValues


import maya.cmds
import maya.OpenMaya

import IECore

import StringUtil

## A context manager for controlling attribute values in with statements. It
# sets attributes to requested values on entering the block and resets them to
# their previous values on exiting the block.
class TemporaryAttributeValues :

	def __init__( self, attributeAndValues = {}, **kw ) :

		self.__attributesAndValues = attributeAndValues
		self.__attributesAndValues.update( kw )

	def __enter__( self ) :

		handlers = {
			"enum" : self.__simpleAttrHandler,
			"bool" : self.__simpleAttrHandler,
			"float" : self.__simpleAttrHandler,
			"long" : self.__simpleAttrHandler,
			"short" : self.__simpleAttrHandler,
			"float2" : IECore.curry( self.__numeric2AttrHandler, attributeType="float2" ),
			"long2" : IECore.curry( self.__numeric2AttrHandler, attributeType="long2" ),
			"short2" : IECore.curry( self.__numeric2AttrHandler, attributeType="short2" ),
			"float3" : IECore.curry( self.__numeric3AttrHandler, attributeType="float3" ),
			"long3" : IECore.curry( self.__numeric3AttrHandler, attributeType="long3" ),
			"short3" : IECore.curry( self.__numeric3AttrHandler, attributeType="short3" ),
			"string" : self.__stringAttrHandler,
		}

		self.__restoreCommands = []
		for attr, value in self.__attributesAndValues.items() :

			# check we can handle this type
			attrType = maya.cmds.getAttr( attr, type=True )
			handler = handlers.get( attrType, None )
			if not handler :
				raise TypeError( "Attribute \"%s\" has unsupported type \"%s\"." % ( attr, attrType ) )

			# store a command to restore the attribute value later
			node = StringUtil.nodeFromAttributePath( attr )
			s = maya.OpenMaya.MSelectionList()
			s.add( attr )
			p = maya.OpenMaya.MPlug()
			s.getPlug( 0, p )
			commands = []
			p.getSetAttrCmds( commands )
			for c in commands :

				c = c.lstrip()
				assert( c.startswith( "setAttr \"." ) )
				c = "setAttr \"" + node + c[9:]
				self.__restoreCommands.append( c )

			# and change the attribute value
			handler( attr, value )

	def __exit__( self, type, value, traceBack ) :

		for c in self.__restoreCommands :

			maya.mel.eval( c )

	def __simpleAttrHandler( self, attr, value ) :

		maya.cmds.setAttr( attr, value )

	def __numeric2AttrHandler( self, attr, value, attributeType ) :

		maya.cmds.setAttr( attr, value[0], value[1], type=attributeType )

	def __numeric3AttrHandler( self, attr, value, attributeType ) :

		maya.cmds.setAttr( attr, value[0], value[1], value[2], type=attributeType )

	def __stringAttrHandler( self, attr, value ) :

		maya.cmds.setAttr( attr, value, type="string" )

from GenericParameterUI import GenericParameterUI


import maya.cmds
import maya.mel

import IECore					   
import IECoreMaya.ParameterUI
import IECoreMaya.StringUtil

class GenericParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
		
		IECoreMaya.ParameterUI.__init__(
		
			self,
			node,
			parameter,
			maya.cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = [ self.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex * 3 + 25 + 25 ]
			),
			**kw
			
		)
				
		maya.cmds.text( label = self.label(), font="smallPlainLabelFont", align="right", annotation=self.description() )
		
		self.__connectionsLayout = maya.cmds.columnLayout()
		maya.cmds.setParent( ".." )

		maya.cmds.setParent( ".." )
		
		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :
	
		IECoreMaya.ParameterUI.replace( self, node, parameter )
	
		self.__updateDisplay()
		
		self.__attributeChangedCallbackId = IECoreMaya.CallbackId(
			maya.OpenMaya.MNodeMessage.addAttributeChangedCallback( self.node(), self.__attributeChanged )
		)
	
	def _topLevelUIDeleted( self ) :
	
		self.__attributeChangedCallbackId = None

	def __attributeChanged( self, changeType, plug, otherPlug, userData ) :
				
		if not (
			( changeType & maya.OpenMaya.MNodeMessage.kConnectionMade )
			or ( changeType & maya.OpenMaya.MNodeMessage.kConnectionBroken ) 
		) :
			return
		
		try :
			myPlug = self.plug()
		except :
			# this situation can occur when our parameter has been removed but the
			# ui we represent is not quite yet dead
			return
		
		if plug == myPlug :
			maya.cmds.evalDeferred( self.__updateDisplay )
		
	def __updateDisplay( self ) :
		
		if not maya.cmds.layout( self.__connectionsLayout, query=True, exists=True ) :
			return	
		
		currentParent = maya.cmds.setParent( query=True )
				
		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
		plugPath = fnPH.parameterPlugPath( self.parameter )
		
		connections = maya.cmds.listConnections( plugPath, source=True, plugs=True, skipConversionNodes=True )
		numConnections = 0
		if connections :
			numConnections = len( connections )
				
		old = maya.cmds.columnLayout( self.__connectionsLayout, query=True, childArray=True )
		if old :
			for child in old :
				maya.cmds.deleteUI( child )
		
		maya.cmds.setParent( self.__connectionsLayout )
		
		if numConnections == 0 :

			maya.cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = [ IECoreMaya.ParameterUI.singleWidgetWidthIndex * 3 + 4, 20 ],
			)
			
			text = maya.cmds.text( align="left", label="Not connected", font="tinyBoldLabelFont" )
			self._addPopupMenu( parentUI=text, attributeName = self.plugName() )
			self._addPopupMenu( parentUI=text, attributeName = self.plugName(), button1=True )
			
			maya.cmds.iconTextButton(
				annotation = "Clicking this takes you the connection editor for this connection.",
				style = "iconOnly",
				image = "listView.xpm",
				font = "boldLabelFont",
				command = IECore.curry( self.__connectionEditor, leftHandNode = None ),
				height = 20,
				width = 20
			)
			
			maya.cmds.setParent( ".." )
			
		else :
		
			for i in range( numConnections ) :
			
				self.__drawConnection( connections[i] )
		
		
		maya.cmds.setParent( currentParent )
		
	def __drawConnection( self, plugName ) :
	
		fieldWidth = IECoreMaya.ParameterUI.singleWidgetWidthIndex * 3 - 25 
	
		maya.cmds.rowLayout(
			numberOfColumns = 3,
			columnWidth3 = [ fieldWidth , 25, 25 ]
		)
	
		name = maya.cmds.text( l=plugName, font="tinyBoldLabelFont", align="left",
							   width=fieldWidth, height = 20, ann=plugName )
							   
		self._addPopupMenu( parentUI=name, attributeName = self.plugName() )
		self._addPopupMenu( parentUI=name, attributeName = self.plugName(), button1=True )
	
		maya.cmds.iconTextButton(
			annotation = "Clicking this takes you the connection editor for this connection.",
			style = "iconOnly",
			image = "listView.xpm",
			font = "boldLabelFont",
			command = IECore.curry( self.__connectionEditor, leftHandNode = plugName ),
			height = 20,
			width = 20
		)
			
		maya.cmds.iconTextButton(
			annotation = "Clicking this will take you to the node sourcing this connection.",
			style = "iconOnly",
			image = "navButtonConnected.xpm",
			command = IECore.curry( self.__showEditor, plugName ),
			height = 20,
		)
		
		maya.cmds.setParent( ".." )
		
	def __connectionEditor( self, leftHandNode ) :

		maya.mel.eval(

			str( "ConnectionEditor;" +
			"nodeOutliner -e -replace %(right)s connectWindow|tl|cwForm|connectWindowPane|rightSideCW;"+
			"connectWindowSetRightLabel %(right)s;") % { 'right' : self.nodeName() }
		
		)
	
		if leftHandNode :
		
			maya.mel.eval(

				str("nodeOutliner -e -replace %(left)s connectWindow|tl|cwForm|connectWindowPane|leftSideCW;"+
				"connectWindowSetLeftLabel %(left)s;" ) % { 'left' : leftHandNode.split(".")[0] }

			)
	
	def __showEditor( self, attributeName ) :

		maya.mel.eval( 'showEditor "' + IECoreMaya.StringUtil.nodeFromAttributePath( attributeName ) + '"' )
				
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Parameter, GenericParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Parameter, GenericParameterUI, 'generic' )

from FnDagNode import FnDagNode  


import re

import maya.OpenMaya
import maya.cmds

import IECore
import StringUtil

## This class extends Maya's MFnDagNode to add assorted helper functions.
class FnDagNode( maya.OpenMaya.MFnDagNode ) :

	## \param obj - MObject, This can also be a string or an MObjectHandle.
	def __init__( self, obj ) :
	
		if isinstance( obj, str ) or isinstance( obj, unicode ) :
		
			obj = StringUtil.dependencyNodeFromString( obj )
		
		elif isinstance( obj, maya.OpenMaya.MObjectHandle ) :
		
			assert( obj.isValid() )
			obj = obj.object()		

		maya.OpenMaya.MFnDagNode.__init__( self, obj )

	## Creates a shape node of the requested type under a transform with the
	# requested name. If necessary a numeric suffix will be appended to the
	# parent name to keep it unique. Returns a function set attached to the
	# shape.
	@staticmethod
	def createShapeWithParent( parentName, shapeNodeType ) :
	
		parentNode = maya.cmds.createNode( "transform", name=parentName, skipSelect=True )
		parentShort = parentNode.rpartition( "|" )[-1]
		
		numbersMatch = re.search( "[0-9]+$", parentShort )
		if numbersMatch is not None :
			numbers = numbersMatch.group()
			shapeName = parentShort[:-len(numbers)] + "Shape" + numbers
		else :
			shapeName = parentShort + "Shape"
			
		shapeNode = maya.cmds.createNode( shapeNodeType, name=shapeName, parent=parentNode, skipSelect=True )
		
		return FnDagNode( shapeNode )	

	## Determines whether the DAG node is actually hidden in Maya.
	# This includes the effect of any parents visibility.
	# \return Bool
	def isHidden( self ) :

		return bool( self.hiddenPathNames( True ) )

	## Retrieves the names of any part of the nodes parent hierarchy that is hidden.
	# \param includeSelf - Bool, When True, the object itself will be listed if
	# it is hidden. When False, only parents will be listed. Defaults to True.
	# \return A list of hidden objects by name.
	def hiddenPathNames( self, includeSelf=True ) :

		hidden = []
		
		# Maya always returns a list from listRelatives.
		parent = [] 

		if includeSelf :			
			parent = [ self.fullPathName() ]
		else :
			parent	= maya.cmds.listRelatives( self.fullPathName(), parent=True )
	
		while parent :
			
			assert( len(parent) == 1 )
			
			o = parent[0]	
			attr = "%s.visibility" % o
		
			if maya.cmds.objExists( attr ) and not maya.cmds.getAttr( attr ) :
				hidden.append( o )
		
			parent	= maya.cmds.listRelatives( o, parent=True )

		return hidden





from CompoundParameterUI import CompoundParameterUI


from __future__ import with_statement

import traceback

import maya.cmds

import IECore
import IECoreMaya

class CompoundParameterUI( IECoreMaya.ParameterUI ) :

	_collapsedUserDataKey = "aeCollapsed"

	## Supports the following keyword arguments :
	#
	# bool "withCompoundFrame"
	# May be specified as True to force the creation of a
	# frameLayout even when this parameter is the toplevel parameter
	# for the node.
	#
	# list "visibleOnly"
	# A list of strings specifying the full parameter paths for
	# parameters which should be displayed. Any parameters not in
	# this list will not be visible.
	#
	# bool "labelVisible"
	# If not None, specifies whether or not the parameter label is visible. This
	# is used by the ClassVectorParameterUI ChildUI class.
	def __init__( self, node, parameter, labelVisible=None, **kw  ) :
		
		self.__childUIs = {}
		self.__headerCreated = False
		self.__kw = kw.copy()
		
		self.__kw["hierarchyDepth"] = self.__kw.get( "hierarchyDepth", -1 ) + 1
	
		originalParent = maya.cmds.setParent( query=True )
				
		collapsible = self.__parameterIsCollapsible( node, parameter )
		collapsed = self._retrieveCollapsedState( collapsible, parameter )
		
		# we always use a Collapsible ui to hold our children, and just hide
		# the header if we don't actually want to collapse it ever.

		self.__collapsible = IECoreMaya.Collapsible(

			# need to specify a label on creation or maya gets the size wrong.
			# we'll update the label below, once we can call the base class label() method.
			label = "mustSpecifySomething",
			labelFont = self._labelFont( self.__kw["hierarchyDepth"] ),
			labelIndent = self._labelIndent( self.__kw["hierarchyDepth"] ),
			labelVisible = labelVisible if labelVisible is not None else collapsible,
			collapsed = collapsed,
			expandCommand = self.__expand,
			preExpandCommand = self.__preExpand,
			collapseCommand = self.__collapse,
		
		)

		IECoreMaya.ParameterUI.__init__( self,
			
			node,
			parameter,
			# stealing someone else's top level ui for use as your own is really breaking the rules.
			# but we need to do it to reduce the nesting associated with making a new top level to put
			# the Collapsible class in, because otherwise maya 2010 will crash with deeply nested
			# hierarchies. we could stop doing this when we no longer need maya 2010 support.
			self.__collapsible._topLevelUI(),
			**kw
			
		)				

		self.__collapsible.setLabel( self.label() )
		self.__collapsible.setAnnotation( self.description() )
				
		self.__columnLayout = maya.cmds.columnLayout(
			parent = self.__collapsible.frameLayout(),
			width = 381
		)

		if not collapsed :
			self.__preExpand()
			
		maya.cmds.setParent( originalParent )

	def replace( self, node, parameter ) :
		
		IECoreMaya.ParameterUI.replace( self, node, parameter )
				
		if self.__parameterIsCollapsible() :
			collapsed = self._retrieveCollapsedState( self.getCollapsed() )
			self.setCollapsed( collapsed, **self.__kw )	
				
		if len( self.__childUIs ) :
		
			for pName in self.__childUIs.keys() :

				ui = self.__childUIs[pName]
				p = self.parameter[pName]

				ui.replace( node, p )
				
		else :
		
			if not self.getCollapsed() :
				with IECoreMaya.UITemplate( "attributeEditorTemplate" ) :
					self.__createChildUIs()
	
	## Gets the collapsed state for the frame holding the child parameter uis.
	def getCollapsed( self ) :
	
		return self.__collapsible.getCollapsed()
		
	## Sets the collapsed state for the frame holding the child parameter uis.
	# \param propagateToChildren How many levels of hierarchy to propagate 
	# the new state to. If a Bool is passed, rather than an int, then
	# 'all' or 'none' is assumed, for backwards compatibility.
	def setCollapsed( self, collapsed, propagateToChildren=0, **kw ) :
	
		if type(propagateToChildren) == bool :	
			propagateToChildren = 999 if propagateToChildren else 0
		
		if not collapsed :
			# maya only calls preexpand when the ui is expanded by user action,
			# not by a script - how annoying.
			self.__preExpand()
		
		if self.__parameterIsCollapsible() :	
			self.__collapsible.setCollapsed( collapsed )
		
		self._storeCollapsedState( collapsed )
		
		if propagateToChildren > 0 :
			propagateToChildren = propagateToChildren - 1
			self.__propagateCollapsed( collapsed, propagateToChildren, **kw )
	
	# This will retrieve the collapsedState from the parameters userData. It uses the
	# default key if 'collapsedUserDataKey' was not provided in the UI constructor's **kw.
	def _retrieveCollapsedState( self, default=True, parameter=None ) :
		
		if parameter is None :	
			parameter = self.parameter
		
		key = self.__kw.get( "collapsedUserDataKey", CompoundParameterUI._collapsedUserDataKey )
		if "UI" in parameter.userData() and key in parameter.userData()["UI"] :	   
			return parameter.userData()["UI"][ key ].value
		else :
			return default
	
	# This will store \param state in the parameters userData, under the default key,
	# unless 'collapsedUserDataKey' was provided in the UI constructor's **kw.
	def _storeCollapsedState( self, state ) :

		if "UI" not in self.parameter.userData() :
			self.parameter.userData()["UI"] = IECore.CompoundObject()
		
		key = self.__kw.get( "collapsedUserDataKey", CompoundParameterUI._collapsedUserDataKey )
		self.parameter.userData()["UI"][key] = IECore.BoolData( state )
	
	# Returns True if the ui should be collapsible for this parameter, False
	# otherwise.
	def __parameterIsCollapsible( self, node=None, parameter=None ) :
	
		if node is None :
			node = self.node()
		if parameter is None :
			parameter = self.parameter
	
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		
		collapsible = not parameter.isSame( fnPH.getParameterised()[0].parameters() )
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = parameter.userData()["UI"]["collapsible"].value
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = parameter.userData()["UI"]["collapsable"].value
			
		collapsible = self.__kw.get( "withCompoundFrame", False ) or collapsible
		
		return collapsible
		
	@staticmethod
	def _labelFont( hierarchyDepth ) :
	
		if hierarchyDepth == 2 :
			return "smallBoldLabelFont"
		elif hierarchyDepth >= 3 :
			return "tinyBoldLabelFont"
		else :
			return "boldLabelFont"			
	
	@staticmethod
	def _labelIndent( hierarchyDepth ) :
	
		return 5 + ( 8 * max( 0, hierarchyDepth-1 ) )
				
	## May be implemented by derived classes to present some custom ui at the
	# top of the list of child parameters. Implementations should first call the
	# base class method and then perform their custom behaviour, placing ui elements
	# into the provided columnLayout.
	def _createHeader( self, columnLayout, **kw ) :
	
		draggable = False
		try:
			draggable = self.parameter.userData()['UI']['draggable'].value
		except :
			pass

		## \todo Figure out what this draggable stuff is all about and document it.
		# I think it's intended to allow parameters to be dragged and dropped onto
		# an IECoreMaya.ParameterPanel but I can't get that to work right now.
		if draggable :

			maya.cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = ( 361, 20 ),
				parent = columnLayout

			)

			maya.cmds.text( label = "" )

			dragIcon = maya.cmds.iconTextStaticLabel(
				image = "pick.xpm",
				height = 20
			)
			self.addDragCallback( dragIcon, **kw )
		
		self.__headerCreated = True
	
	## May be called by derived classes if for any reason the child parameter
	# uis are deemed invalid - for instance if the child parameters have changed.
	# The uis will then be rebuilt during the next call to replace().	
	def _deleteChildParameterUIs( self ) :
	
		maya.cmds.control( self.__columnLayout, edit=True, manage=False )
		
		for ui in self.__childUIs.values() :
			maya.cmds.deleteUI( ui.layout() )

		self.__childUIs = {}
			
		maya.cmds.control( self.__columnLayout, edit=True, manage=True )

	def __expand( self ) :
	
		self._storeCollapsedState( False )
		
		modifiers = maya.cmds.getModifiers()
				
		if modifiers & 1 :
			# shift is held
			self.__propagateCollapsed( False, 999, lazy=True )
		elif modifiers & 8 :
			# alt is held
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
			# alt is held
			depth = 1;
			with IECore.IgnoredExceptions( KeyError ) :
				depth = self.parameter.userData()["UI"]["autoExpandDepth"].value
			self.__propagateCollapsed( True, depth )
		
	def __preExpand( self ) :
		
		# this is the most common entry point into the ui
		# creation code, and unfortunately it's called from
		# a maya ui callback. maya appears to suppress all
		# exceptions which occur in such callbacks, so we
		# have to wrap with our own exception handling to
		# make sure any errors become visible.
		try :
			with IECoreMaya.UITemplate( "attributeEditorTemplate" ) :
				if not self.__headerCreated :
					self._createHeader( self.__columnLayout, **self.__kw )
				if not len( self.__childUIs ) :
					self.__createChildUIs()
		except :

			IECore.msg( IECore.Msg.Level.Error, "IECoreMaya.ParameterUI", traceback.format_exc() )

	def __propagateCollapsed( self, collapsed, propagateDepth=999, **kw ) :
		
		for ui in self.__childUIs.values() :
			if hasattr( ui, "setCollapsed" ) :
				ui.setCollapsed( collapsed, propagateDepth, **kw )

	def __createChildUIs( self ) :
						
		self.__kw['labelWithNodeName'] = False

		for pName in self.parameter.keys():

			p = self.parameter[pName]

			visible = True
			try:
				visible = p.userData()['UI']['visible'].value
			except:
				pass

			if 'visibleOnly' in self.__kw :

				fullChildName = self.__kw['longParameterName']
				if fullChildName :
					fullChildName += "."
				fullChildName += pName

				visible = fullChildName in self.__kw['visibleOnly']

				if not visible and p.isInstanceOf( IECore.TypeId.CompoundParameter ) :

					for i in self.__kw['visibleOnly'] :
						if i.startswith( fullChildName + "." ) :
							visible = True
							break

			if visible:

				maya.cmds.setParent( self.__columnLayout )

				ui = IECoreMaya.ParameterUI.create( self.node(), p, **self.__kw )

				if ui:
					self.__childUIs[pName] = ui
	
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.CompoundParameter, CompoundParameterUI )

from ClassParameterUI import ClassParameterUI


from __future__ import with_statement

import os.path

import maya.cmds

import IECore
import IECoreMaya

class ClassParameterUI( IECoreMaya.CompoundParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		self.__menuParent = None
		self.__currentClassInfo = parameter.getClass( True )[1:]

		# We have to do this after initialising self.__menuParent as
		# CompoundParameterUI.__init__ will call _createHeader in this
		# class, which populates self.__menuParent.
		IECoreMaya.CompoundParameterUI.__init__( self, node, parameter, **kw )

	def _createHeader( self, columnLayout, **kw ) :

		IECoreMaya.CompoundParameterUI._createHeader( self, columnLayout, **kw )

		maya.cmds.rowLayout( numberOfColumns = 2, parent = columnLayout )

		collapsable = True
		with IECore.IgnoredExceptions( KeyError ) :
			collapsable = self.parameter.userData()["UI"]["collapsable"].value

		lable = "Class" if collapsable else self.label()
		font = "smallPlainLabelFont" if collapsable else "tinyBoldLabelFont"

		maya.cmds.text(
			label = lable,
			font = font,
			align = "right",
			annotation = self.description()
		)

		self.__menuParent = maya.cmds.iconTextStaticLabel(
			image = "arrowDown.xpm",
			font = "smallBoldLabelFont",
			label = self.__menuParentLabel(),
			style = "iconAndTextHorizontal",
			height = 23,
		)

		# popup menu can be activated with either right or left buttons
		IECoreMaya.createMenu( self.__menuDefinition, self.__menuParent )
		IECoreMaya.createMenu( self.__menuDefinition, self.__menuParent, button = 1 )

	def replace( self, node, parameter ) :

		newClassInfo = parameter.getClass( True )[1:]
		if newClassInfo != self.__currentClassInfo :
			self._deleteChildParameterUIs()

		IECoreMaya.CompoundParameterUI.replace( self, node, parameter )

		if self.__menuParent :
			maya.cmds.iconTextStaticLabel( self.__menuParent, edit=True, label=self.__menuParentLabel() )

		self.__currentClassInfo = newClassInfo
	
	## We add support for the 'lazy' keyword argument. If True, then the layout 
	# will not expand if the parameter doesn't contain a class.
	def setCollapsed( self, collapsed, propagateToChildren=0, **kw ) :
	
		lazy = kw["lazy"] if "lazy" in kw else False 
		
		if not collapsed :
			if lazy and not self.parameter.getClass( False ) :
				collapsed = True
				
		IECoreMaya.CompoundParameterUI.setCollapsed( self, collapsed, propagateToChildren, **kw )
		
	def __classNameFilter( self ) :

		with IECore.IgnoredExceptions( KeyError ) :
			return self.parameter.userData()["UI"]["classNameFilter"].value

		return "*"

	def __menuParentLabel( self ) :

		classInfo = self.parameter.getClass( True )
		if classInfo[1] :
			labelPathStart = max( 0, self.__classNameFilter().find( "*" ) )
			return "%s v%d" % ( classInfo[1][labelPathStart:], classInfo[2] )
		else :
			return "Choose..."

	def __menuDefinition( self ) :

		result = IECore.MenuDefinition()

		menuPathStart = max( 0, self.__classNameFilter().find( "*" ) )

		classInfo = self.parameter.getClass( True )

		loader = IECore.ClassLoader.defaultLoader( classInfo[3] )
		for className in loader.classNames( self.__classNameFilter() ) :
			classVersions = loader.versions( className )
			for classVersion in classVersions :

				menuPath = "/" + className[menuPathStart:]
				if len( classVersions ) > 1 :
					menuPath += "/v" + str( classVersion )

				result.append(

					menuPath,

					IECore.MenuItemDefinition(
						command = IECore.curry( self.__setClass, className, classVersion, classInfo[3] ),
						active = className != classInfo[1] or classVersion != classInfo[2]
					)

				)

		result.append(
			"/RemoveDivider",
			IECore.MenuItemDefinition(
				divider = True
			)
		)

		result.append(

			"/Remove",

			IECore.MenuItemDefinition(
				command = IECore.curry( self.__setClass, "", 0, classInfo[3] ),
				active = classInfo[0] is not None
			)

		)

		for cb in self.__classMenuCallbacks :
			cb( result, self.parameter, self.node() )

		return result

	def __setClass( self, className, classVersion, searchPathEnvVar ) :

		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )

		# To stop maya crashing, we need to delete the UI for the existing class
		# before we change it in C++, otherwise, an AE update gets triggered after
		# we have removed the parameters, but before we have removed the UI.
		self._deleteChildParameterUIs()

		with fnPH.parameterModificationContext() :
			self.parameter.setClass( className, classVersion, searchPathEnvVar )

	@staticmethod
	def _classSetCallback( fnPH, parameter ) :

		for instance in IECoreMaya.UIElement.instances( ClassParameterUI ) :
			if instance.parameter.isSame( parameter ) :
				instance.replace( instance.node(), instance.parameter )

	__classMenuCallbacks = []
	## Registers a callback which is able to modify the popup menu used to choose
	# the class held by this parameter. Callbacks should have the following signature :
	#
	# callback( menuDefinition, parameter, holderNode ).
	@classmethod
	def registerClassMenuCallback( cls, callback ) :
	
		cls.__classMenuCallbacks.append( callback )

IECoreMaya.FnParameterisedHolder.addSetClassParameterClassCallback( ClassParameterUI._classSetCallback )

IECoreMaya.ParameterUI.registerUI( IECore.ClassParameter.staticTypeId(), ClassParameterUI )

from ClassVectorParameterUI import ClassVectorParameterUI


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
		IECoreMaya.createMenu( IECore.curry( self.__classMenuDefinition, None ), self.__addButton, useInterToUI=False )
		IECoreMaya.createMenu( IECore.curry( self.__classMenuDefinition, None ), self.__addButton, useInterToUI=False, button=1 )
		
		self.__toolsButton = maya.cmds.picture( image="ie_actionIcon_grey.xpm", parent=self.__buttonRow, width=21 )
		IECoreMaya.createMenu( IECore.curry( self.__toolsMenuDefinition, None ), self.__toolsButton, useInterToUI=False )
		IECoreMaya.createMenu( IECore.curry( self.__toolsMenuDefinition, None ), self.__toolsButton, useInterToUI=False, button=1 )

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
		IECoreMaya.createMenu( self.__layerMenu, layerIcon, useInterToUI=False )
		IECoreMaya.createMenu( self.__layerMenu, layerIcon, useInterToUI=False, button=1 )
		
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
				renameMenu = IECore.MenuDefinition(
					[
						( "Change label...", { "command" : self.__changeLabel } ),
					]
				)
				IECoreMaya.createMenu( renameMenu, self.__label )
				IECoreMaya.createMenu( renameMenu, self.__label, button = 1 )
			
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
			IECoreMaya.createMenu( IECore.curry( self.__presetsMenu, parameter ), control )
			IECoreMaya.createMenu( IECore.curry( self.__presetsMenu, parameter ), control, button=1 )
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
				pre = 2
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
				width = 40,
				showButton = False
			)
			
		elif isinstance( parameter, IECore.StringParameter ) :
		
			control = maya.cmds.textField( annotation = annotation, width = 150 )
			maya.cmds.connectControl( control, parameterPlugPath )

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

from PresetsOnlyParameterUI import PresetsOnlyParameterUI


import maya.cmds
import maya.OpenMayaUI

import IECoreMaya

## A ui for any parameter for which parameter.presetsOnly is True.
class PresetsOnlyParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw  ) :

		IECoreMaya.ParameterUI.__init__(
		
			self,
			node,
			parameter,
			maya.cmds.rowLayout(
				numberOfColumns = 2,
			),
			**kw
			
		)
		
		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__popupControl = maya.cmds.iconTextStaticLabel(
			image = "arrowDown.xpm",
			font = "smallBoldLabelFont",
			style = "iconAndTextHorizontal",
			height = 23
		)

		self.replace( node, parameter )
				
	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		self.__updateLabel()

		self._addPopupMenu( parentUI=self.__popupControl, attributeName = self.plugName(), button1=True )
		
		self.__attributeChangedCallbackId = IECoreMaya.CallbackId(
			maya.OpenMaya.MNodeMessage.addAttributeChangedCallback( self.node(), self.__attributeChanged )
		)
	
	def _topLevelUIDeleted( self ) :
	
		self.__attributeChangedCallbackId = None
	
	def __attributeChanged( self, changeType, plug, otherPlug, userData ) :
				
		if not ( changeType & maya.OpenMaya.MNodeMessage.kAttributeSet ) :
			return
		
		try :
			myPlug = self.plug()
		except :
			# this situation can occur when our parameter has been removed but the
			# ui we represent is not quite yet dead
			return
		
		if plug == myPlug :
			self.__updateLabel()
			return
		
		if plug.isChild():
			if plug.parent() == myPlug :
				self.__updateLabel()
				return
		
	def __updateLabel( self ) :
	
		IECoreMaya.FnParameterisedHolder( self.node() ).setParameterisedValues()

		maya.cmds.iconTextStaticLabel(
			self.__popupControl,
			edit = True,
			label = self.parameter.getCurrentPresetName(),
		)

from TestCase import TestCase


import unittest

import maya.cmds

## A class to help implement unit tests for maya functionality. It
# implements setUp() to create a new maya scene to perform the test in.
class TestCase( unittest.TestCase ) :

	## Derived classes may override this, but they should call the
	# base class implementation too.
	def setUp( self ) :
	
		maya.cmds.file( new = True, force = True )
		maya.cmds.flushUndo()


from TestProgram import TestProgram


import sys
import unittest

## A test program which initializes Maya standalone before running the test suite.
# The list of named plugins is also loaded.
class TestProgram( unittest.TestProgram ) :

	def __init__( self, module='__main__', defaultTest=None, argv=None, testRunner=None, testLoader=unittest.defaultTestLoader, plugins = [] ) :

		self.__plugins = plugins
		
		unittest.TestProgram.__init__( self, module, defaultTest, argv, testRunner, testLoader )

	def runTests( self ) :
	
		try:
			import maya.standalone
			maya.standalone.initialize( name='IECoreMayaTest' )
		except:
			sys.stderr.write( "Failed to initialize Maya standalone application" )
			raise

		import maya.cmds
		for plugin in self.__plugins :
			maya.cmds.loadPlugin( plugin )

		if not self.testRunner :
			self.testRunner = unittest.TextTestRunner( verbosity = 2 )

		result = self.testRunner.run( self.test )

		exitStatus = int( not result.wasSuccessful() )

		try:
			if hasattr( maya.standalone, "cleanup" ):

				maya.standalone.cleanup( exitStatus )
			else:

				import IECoreMaya
				IECoreMaya.Standalone.cleanup( exitStatus )
		finally:

			# If cleanup fails for any reason, just exit.
			sys.exit( exitStatus )

from FileBrowser import FileBrowser


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

from FileDialog import FileDialog


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
			path = FileDialog.__getLastPath( self.__key )

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





from GeometryCombinerUI import *


from __future__ import with_statement

import maya.cmds

import IECore
import IECoreMaya

# we don't want to export anything, we just want to register callbacks
__all__ = []

def __geometryCombiner( plugPath, createIfMissing=False ) :

	connections = maya.cmds.listConnections( plugPath, source=True, destination=False, type="ieGeometryCombiner" )
	
	if connections :
		return connections[0]
		
	if not createIfMissing :
		return ""
	
	result = maya.cmds.createNode( "ieGeometryCombiner", skipSelect=True )
	maya.cmds.connectAttr( result + ".outputGroup", plugPath, force=True )
	maya.cmds.setAttr( result + ".convertPrimVars", 1 )
	maya.cmds.setAttr( result + ".convertBlindData", 1 )
	
	return result

def __selectedGeometry() :

	return maya.cmds.ls( selection=True, type=( "mesh", "nurbsCurve" ), noIntermediate=True, leaf=True, dag=True )

def __addSelected( plugPath ) :

	selection = __selectedGeometry()
	if not selection :
		return
			
	combiner = __geometryCombiner( plugPath, createIfMissing=True )
	existingInputs = __inputGeometry( plugPath )
	
	for s in selection :
	
		if s in existingInputs :
			continue
	
		if maya.cmds.nodeType( s )=="mesh" :
			maya.cmds.connectAttr( s + ".worldMesh", combiner + ".inputGeometry", nextAvailable=True, force=True )
		if maya.cmds.nodeType( s )=="nurbsCurve" :
			maya.cmds.connectAttr( s + ".worldSpace", combiner + ".inputGeometry", nextAvailable=True, force=True )	

def __inputGeometry( plugPath, plugs=False ) :

	combiner = __geometryCombiner( plugPath )
	if not combiner :
		return []
		
	connections = maya.cmds.listConnections( combiner + ".inputGeometry", source=True, destination=False, shapes=True, plugs=plugs )
	if connections is None :
		return []
	else :
		return connections
	
def __removeSelected( plugPath ) :

	combiner = __geometryCombiner( plugPath )
	if not combiner :
		return
		
	inputPlugs = __inputGeometry( plugPath, plugs=True )
	inputNodes = [ IECoreMaya.StringUtil.nodeFromAttributePath( x ) for x in inputPlugs ]
	
	for s in __selectedGeometry() :
		i = -1
		with IECore.IgnoredExceptions( ValueError ) :
			i = inputNodes.index( s )
		if s != -1 :
			maya.cmds.disconnectAttr( inputPlugs[i], combiner + ".inputGeometry", nextAvailable=True )
	
def __select( plugPath ) :

	maya.cmds.select( __inputGeometry( plugPath ), replace=True )

def __menuCallback( definition, parameter, node ) :
	
	active = False
	with IECore.IgnoredExceptions( KeyError ) :
		active = parameter.userData()["maya"]["useGeometryCombiner"].value
		
	if not active :
		return
			
	definition.append( "/InputGeometryDivider", { "divider" : True } )
	
	plugPath = IECoreMaya.FnParameterisedHolder( node ).parameterPlugPath( parameter )
	
	definition.append( "/Input Geometry/Add Selected", { "command" : IECore.curry( __addSelected, plugPath ) } )
	definition.append( "/Input Geometry/Remove Selected", { "command" : IECore.curry( __removeSelected, plugPath ) } )
	definition.append( "/Input Geometry/Select", { "command" : IECore.curry( __select, plugPath ) } )
	
IECoreMaya.ParameterUI.registerPopupMenuCallback( __menuCallback )

from PresetsUI import *


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
		sp = IECore.SearchPath( os.path.expandvars( paths ), ":" )
		self.__classLoader = IECore.ClassLoader( sp )
		
		presets = self.__getPresets( parameterised[0], self.__rootParameter )
		if not presets:
			maya.cmds.confirmDialog( message="No presets found in the current search paths ($%s)." % self.__envVar, button="OK" )
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
		
		selected = self.__selector.selected()
		presets = []
		for s in selected:
			self.__loadedPresets[s] = self.__classLoader.load( s )()
			presets.append( self.__loadedPresets[s] )
		
		self.__infoColumn.setPresets( presets )
	
	def __doLoad( self ) :

		selected = self.__selector.selected()
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
				validPresets.append( name )

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



from ParameterClipboardUI import *


from __future__ import with_statement

import IECore
import IECoreMaya

import maya.cmds

## The ParameterClipboardUI functions create menu items in relevant
## menus to allow the values of Parameterised object Parameters to be
## copied and pasted between nodes, or between different parameters on
## the same node. It currently does this using in-memory BasicPreset
## instances, held in a global variable. 
## Pasting 'values' as a connection is also supported, though not using
## the basic preset mechanism.
__all__ = [ 'copy', 'copyClass', 'paste', 'pasteLinked', '_clearReferences' ]

def __copyPasteMenuModifier( menuDefinition, parameter, node, parent=None ) :
	
	with IECore.IgnoredExceptions( KeyError ) :
		if not parameter.userData()['UI']['copyPaste'].value :
			return
	
	global _ieCoreParameterClipboardUIBuffer
	
	fnPh = IECoreMaya.FnParameterisedHolder( node )
	plugPath = fnPh.parameterPlugPath( parameter )
	
	commandBase = 'import IECoreMaya; IECoreMaya.ParameterClipboardUI'
	
	copyString = '%s.copy( "%s" )' % ( commandBase, plugPath ) 
	copySomeString = '%s.copy( "%s", True )' % ( commandBase, plugPath )
	pasteString = '%s.paste( "%s" )' % ( commandBase, plugPath ) 
	pasteLinkedString = '%s.pasteLinked( "%s" )' % ( commandBase, plugPath ) 

	if len( menuDefinition.items() ):
		menuDefinition.append( "/CopyPasteDivider", { "divider" : True } )

	copyActive = True
	
	if isinstance( parameter, IECore.ClassParameter ) :
		if parameter.getClass( False ) == None :
			copyActive = False
			
	elif isinstance( parameter, IECore.ClassVectorParameter ) :
		classes = parameter.getClasses( False )
		if not len(classes) :
			copyActive = False

	# If we are actually a class in a vector, also give them an option
	# to copy the class
	if parent is not None and (
		 isinstance( parent, IECore.ClassVectorParameter )
		 or isinstance( parent, IECore.ClassParameter )
	):
		menuDefinition.append( 
			"/Copy Values",
			{
				"command" : IECore.curry( maya.cmds.evalDeferred, copyString ),		
				"secondaryCommand" : IECore.curry( maya.cmds.evalDeferred, copySomeString ),
				"active" : copyActive,
			}
		)
		
		parentPlugPath = fnPh.parameterPlugPath( parent )
		copyComponentString = '%s.copyClass( "%s", "%s" )' % ( commandBase, plugPath, parentPlugPath ) 

		menuDefinition.append( 
		"/Copy Component",
		{
			"command" : IECore.curry( maya.cmds.evalDeferred, copyComponentString ),		
			"active" : copyActive,
		}
	)
	
	else:
	
		menuDefinition.append( 
			"/Copy",
			{
				"command" : IECore.curry( maya.cmds.evalDeferred, copyString ),		
				"secondaryCommand" : IECore.curry( maya.cmds.evalDeferred, copySomeString ),
				"active" : copyActive,
			}
		)
	
	pasteActive = False
	pasteLinkedActive = False

	if _ieCoreParameterClipboardUIBuffer and isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		pasteActive = _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter )
		if pasteActive and _ieCoreParameterClipboardUILastNode and maya.cmds.objExists( _ieCoreParameterClipboardUILastNode ) :
			pasteLinkedActive = True
		
	menuDefinition.append( 
		"/Paste",
		{
			"command" : IECore.curry( maya.cmds.evalDeferred, pasteString ),		
			"active" : pasteActive,
		}	
	)
	
	menuDefinition.append( 
		"/Paste Linked",
		{
			"command" : IECore.curry( maya.cmds.evalDeferred, pasteLinkedString ),		
			"active" : pasteLinkedActive,
		}	
	)

def __copyPasteVectorMenuModifier( menuDefinition, parent, parameter, node ) :
	__copyPasteMenuModifier( menuDefinition, parameter, node, parent=parent )

IECoreMaya.ParameterUI.registerPopupMenuCallback( __copyPasteMenuModifier )
IECoreMaya.ClassParameterUI.registerClassMenuCallback( __copyPasteMenuModifier )
IECoreMaya.ClassVectorParameterUI.registerClassMenuCallback( __copyPasteVectorMenuModifier )
IECoreMaya.ClassVectorParameterUI.registerToolsMenuCallback( __copyPasteMenuModifier )

## The copy buffer
_ieCoreParameterClipboardUIBuffer = None
## These track the last node/parameters that were copied so we can potentially
## paste with connections. We still need the preset to be able to restore
## any missing classes contained within the preset.
_ieCoreParameterClipboardUILastParameterList = None
_ieCoreParameterClipboardUILastNode = None
_ieCoreParameterClipboardUILastRoot = None

def copy( plugPath, showUI=False ) :

	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot

	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )

	def copyCallback( node, rootParameter, parameters ):
		
		global _ieCoreParameterClipboardUIBuffer
		global _ieCoreParameterClipboardUILastParameterList
		global _ieCoreParameterClipboardUILastNode
		global _ieCoreParameterClipboardUILastRoot
		
		fnPh = IECoreMaya.FnParameterisedHolder( node )
		preset = IECore.BasicPreset( fnPh.getParameterised()[0], rootParameter, parameters=parameters )
		
		_ieCoreParameterClipboardUIBuffer = preset
		_ieCoreParameterClipboardUILastParameterList = parameters
		_ieCoreParameterClipboardUILastNode = node
		_ieCoreParameterClipboardUILastRoot = rootParameter
	
	# We need to make sure that the values in the parameterised are in sync
	fnPh.setParameterisedValues()
	
	if showUI :

		IECoreMaya.PresetsUI( parts[0], parameter ).selectParameters( copyCallback )
	
	else :
		
		paramList = []
		__getParameters( parameter, paramList )

		_ieCoreParameterClipboardUIBuffer = IECore.BasicPreset( fnPh.getParameterised()[0], parameter )
		_ieCoreParameterClipboardUILastParameterList = paramList
		_ieCoreParameterClipboardUILastNode = fnPh.fullPathName()
		_ieCoreParameterClipboardUILastRoot = parameter

def copyClass( plugPath, parentPlugPath ) :

	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot
	
	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )
	parent = fnPh.plugParameter( parentPlugPath )

	# We need to make sure that the values in the parameterised are in sync
	fnPh.setParameterisedValues()
	
	# This bit is slightly irritating, but we have to get all 
	# The child parameters of the target class, so we only save that one
	paramList = []
	__getParameters( parameter, paramList )
	_ieCoreParameterClipboardUIBuffer = IECore.BasicPreset( fnPh.getParameterised()[0], parent, paramList )
	
	# For now, we only support pasting values linked. Otherwise, we'd have to go find
	# any classes we instantiated to know their plug prefix...
	_ieCoreParameterClipboardUILastParameterList = None
	_ieCoreParameterClipboardUILastNode = None
	_ieCoreParameterClipboardUILastRoot = None

def paste( plugPath ) :

	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot
		
	if not _ieCoreParameterClipboardUIBuffer or not isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		return
		
	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )
	
	if not _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter ):
		raise RuntimeError, "The parameters on the clipboard are not applicable to '%s'" % plugPath
	
	fnPh.setParameterisedValues()
	
	with fnPh.parameterModificationContext() : 
		_ieCoreParameterClipboardUIBuffer( fnPh.getParameterised()[0], parameter )
	
def pasteLinked( plugPath ) :
	
	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot
		
	if not _ieCoreParameterClipboardUIBuffer or not isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		return
		
	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )
	
	if not _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter ):
		raise RuntimeError, "The parameters on the clipboard are not applicable to '%s'" % plugPath
	
	# Apply the preset to make sure that the children are there
	fnPh.setParameterisedValues()
	
	with fnPh.parameterModificationContext() : 
		_ieCoreParameterClipboardUIBuffer( fnPh.getParameterised()[0], parameter )
		
	# Connect up
	if not maya.cmds.objExists( _ieCoreParameterClipboardUILastNode ) :
		raise RuntimeError, "The source node '%s' no longer exists." % _ieCoreParameterClipboardUILastNode
	
	if not _ieCoreParameterClipboardUILastRoot :
		raise RuntimeError, "Unable to link, the source root parameter is not known." % _ieCoreParameterClipboardUILastNode
	
	
	sourceNodeHolder = IECoreMaya.FnParameterisedHolder( _ieCoreParameterClipboardUILastNode )
	sourceRootPlugPath = sourceNodeHolder.parameterPlugPath( _ieCoreParameterClipboardUILastRoot )
		
	if sourceRootPlugPath == plugPath :
		raise RuntimeError, "The source and destination parameters are the same, unable to link."
	
	for p in _ieCoreParameterClipboardUILastParameterList :
		sourcePlugPath = sourceNodeHolder.parameterPlugPath( p )
		destPlugPath = sourcePlugPath.replace( sourceRootPlugPath, plugPath )
		if maya.cmds.objExists( sourcePlugPath ) and maya.cmds.objExists( destPlugPath ) :
			maya.cmds.connectAttr( sourcePlugPath, destPlugPath, force=True )

def __getParameters( parameter, paramList=[] ) :

	if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :
		
		for p in parameter.values():
			__getParameters( p, paramList )
	
	elif isinstance( parameter, IECore.ClassParameter ) :
		
		c = parameter.getClass( False )
		if c:
			paramList.append( parameter )
			__getParameters( c.parameters(), paramList )
		
	elif isinstance( parameter, IECore.ClassVectorParameter ) :
		
		cl = parameter.getClasses( False )
		if cl :
			paramList.append( parameter )
			for c in cl :
				__getParameters( c.parameters(), paramList )
	
	else :
		paramList.append( parameter )
	
# We need to clear out the references we're holding on parameters when the scene changes
def _clearReferences( *args, **kwargs ) :

	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot
	
	_ieCoreParameterClipboardUILastParameterList = None
	_ieCoreParameterClipboardUILastNode = None
	_ieCoreParameterClipboardUILastRoot = None

_ieCoreParameterClipboardCallbacks = []
if hasattr( maya.cmds, "about" ) and not maya.cmds.about( batch=True ):
	_ieCoreParameterClipboardCallbacks.append( IECoreMaya.CallbackId( maya.OpenMaya.MSceneMessage.addCallback( maya.OpenMaya.MSceneMessage.kBeforeNew, _clearReferences ) ) )
	_ieCoreParameterClipboardCallbacks.append( IECoreMaya.CallbackId( maya.OpenMaya.MSceneMessage.addCallback( maya.OpenMaya.MSceneMessage.kBeforeOpen, _clearReferences ) ) )


from NumericVectorParameterUI import NumericVectorParameterUI


import maya.cmds

import IECore
import IECoreMaya

class NumericVectorParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
		
		topLevelUI = maya.cmds.columnLayout()
		IECoreMaya.ParameterUI.__init__( self, node, parameter, topLevelUI, **kw )
		
		self.__column = maya.cmds.columnLayout( parent=topLevelUI )
		
		row = maya.cmds.rowLayout(
			parent = topLevelUI,
			numberOfColumns = 2,
			columnAlign = ( 1, "right" ),
			columnWidth2 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
		)
		
		maya.cmds.text(
			parent = row,
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description()
		)
		
		addButton = maya.cmds.button( parent=row, label='Add Item', command=self._createCallback( self.__addItem ) )
		
		self.__fields = []
		
		self.__attributeChangedCallbackId = IECoreMaya.CallbackId(
			maya.OpenMaya.MNodeMessage.addAttributeChangedCallback( self.node(), self.__attributeChanged )
		)
		
		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )
		
		# disabling copy/paste from the ParameterClipboardUI as the results will be misleading to users
		parameter.userData().update( NumericVectorParameterUI.__disableCopyPaste )
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		
		# delete un-needed fields
		self.__fields = self.__fields[:len(vector)]
		rows = maya.cmds.columnLayout( self.__column, q=True, childArray=True ) or []
		rowsToKeep = rows[:len(vector)]
		rowsToDelete = rows[len(vector):]
		for row in rowsToDelete :
			maya.cmds.deleteUI( row )
		
		# create new fields
		for i in range( len(rowsToKeep), len(vector) ) :
			self.__createRow( self.label() + ": %d" % i )
		
		self.__setUIFromPlug()
	
	def _topLevelUIDeleted( self ) :
	
		self.__attributeChangedCallbackId = None
	
	def __createRow( self, label ) :
		
		row = maya.cmds.rowLayout(
			parent = self.__column,
			numberOfColumns = 2,
			columnAlign2 = ( "right", "left" ),
			columnWidth2 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
		)
		
		## \todo: there is a slight text misalignment if the window exists when __createRow is called
		maya.cmds.text(
			parent = row,
			label = label,
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
			width = IECoreMaya.ParameterUI.textColumnWidthIndex,
		)
		
		kw = {}
		
		# \todo Add a way of overriding precision for both float and double parameters, giving
		# each a sensible (and probably different) default
		if self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ) :
			kw['precision'] = 12

		self.__fields.append(
			self.__fieldType()(
				parent = row,
				changeCommand = self._createCallback( self.__setPlugFromUI ),
				width = IECoreMaya.ParameterUI.singleWidgetWidthIndex,
				**kw
			)
		)
		
		i = len(self.__fields) - 1
		self._addPopupMenu( parentUI=self.__fields[i], index=i )
	
	def _popupMenuDefinition( self, **kw ) :

		definition = IECore.MenuDefinition()
		definition.append( "/Remove Item", { "command" : self._createCallback( IECore.curry( self.__removeItem, index=kw['index'] ) ) } )
		
		return definition
	
	def __addItem( self ) :
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		vector.append( 0 )
		
		self.__setPlug( vector )
		self.replace( self.node(), self.parameter )
	
	def __removeItem( self, index ) :
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		vector = vector[:index] + vector[index+1:]
		
		self.__setPlug( vector )
		self.replace( self.node(), self.parameter )
	
	def __attributeChanged( self, changeType, plug, otherPlug, userData ) :
		
		if not ( changeType & maya.OpenMaya.MNodeMessage.kAttributeSet ) :
			return
		
		try :
			myPlug = self.plug()
		except :
			# this situation can occur when our parameter has been removed but the
			# ui we represent is not quite yet dead
			return
		
		if not plug == myPlug :
			return
		
		self.replace( self.node(), self.parameter )
	
	def __setUIFromPlug( self ) :
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		for i in range( 0, len(vector) ) :
			self.__fieldType()( self.__fields[i], e=True, value=vector[i] )
	
	def __setPlugFromUI( self ) :
		
		vector = []
		
		for field in self.__fields :
			vector.append( self.__fieldType()( field, q=True, value=True ) )
			
		self.__setPlug( vector )
		
	def __setPlug( self, value ) :
		
		plugType = maya.cmds.getAttr( self.plugName(), type=True )
		maya.cmds.setAttr( self.plugName(), value, type=plugType )
	
	def __fieldType( self ) :

		if self.parameter.isInstanceOf( IECore.TypeId.FloatVectorParameter ) or self.parameter.isInstanceOf( IECore.TypeId.DoubleVectorParameter ) :
			return maya.cmds.floatField
		elif self.parameter.isInstanceOf( IECore.TypeId.IntVectorParameter ) :
			return maya.cmds.intField
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")
	
	__disableCopyPaste = IECore.CompoundObject( {
		"UI" : IECore.CompoundObject( {
			"copyPaste" : IECore.BoolData( False ),
		} ),
	} )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.FloatVectorParameter, NumericVectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.DoubleVectorParameter, NumericVectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.IntVectorParameter, NumericVectorParameterUI )

from StringVectorParameterUI import StringVectorParameterUI


import maya.cmds

import IECore
import IECoreMaya

## \todo: this is incredibly similar to NumericVectorParameterUI. Is it possible to generalize
##	  a ParameterUI for all *VectorParameters?
class StringVectorParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
		
		topLevelUI = maya.cmds.columnLayout()
		IECoreMaya.ParameterUI.__init__( self, node, parameter, topLevelUI, **kw )
		
		self.__column = maya.cmds.columnLayout( parent=topLevelUI )
		
		row = maya.cmds.rowLayout(
			parent = topLevelUI,
			numberOfColumns = 2,
			columnAlign = ( 1, "right" ),
			columnWidth2 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
		)
		
		maya.cmds.text(
			parent = row,
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description()
		)
		
		addButton = maya.cmds.button( parent=row, label='Add Item', command=self._createCallback( self.__addItem ) )
		
		self.__fields = []
		
		self.__attributeChangedCallbackId = IECoreMaya.CallbackId(
			maya.OpenMaya.MNodeMessage.addAttributeChangedCallback( self.node(), self.__attributeChanged )
		)
		
		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )
		
		# disabling copy/paste from the ParameterClipboardUI as the results will be misleading to users
		parameter.userData().update( StringVectorParameterUI.__disableCopyPaste )
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		
		# delete un-needed fields
		self.__fields = self.__fields[:len(vector)]
		rows = maya.cmds.columnLayout( self.__column, q=True, childArray=True ) or []
		rowsToKeep = rows[:len(vector)]
		rowsToDelete = rows[len(vector):]
		for row in rowsToDelete :
			maya.cmds.deleteUI( row )
		
		# create new fields
		for i in range( len(rowsToKeep), len(vector) ) :
			self.__createRow( self.label() + ": %d" % i )
		
		self.__setUIFromPlug()
	
	def _topLevelUIDeleted( self ) :
	
		self.__attributeChangedCallbackId = None
	
	def __createRow( self, label ) :
		
		row = maya.cmds.rowLayout(
			parent = self.__column,
			numberOfColumns = 2,
			columnAlign2 = ( "right", "left" ),
			columnWidth2 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ],
		)
		
		## \todo: there is a slight text misalignment if the window exists when __createRow is called
		maya.cmds.text(
			parent = row,
			label = label,
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
			width = IECoreMaya.ParameterUI.textColumnWidthIndex,
		)

		self.__fields.append(
			maya.cmds.textField(
				parent = row,
				changeCommand = self._createCallback( self.__setPlugFromUI ),
				width = IECoreMaya.ParameterUI.singleWidgetWidthIndex,
			)
		)
		
		i = len(self.__fields) - 1
		self._addPopupMenu( parentUI=self.__fields[i], index=i )
	
	def _popupMenuDefinition( self, **kw ) :

		definition = IECore.MenuDefinition()
		definition.append( "/Remove Item", { "command" : self._createCallback( IECore.curry( self.__removeItem, index=kw['index'] ) ) } )
		
		return definition
	
	def __addItem( self ) :
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		vector.append( "" )
		
		self.__setPlug( vector )
		self.replace( self.node(), self.parameter )
	
	def __removeItem( self, index ) :
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		vector = vector[:index] + vector[index+1:]
		
		self.__setPlug( vector )
		self.replace( self.node(), self.parameter )
	
	def __attributeChanged( self, changeType, plug, otherPlug, userData ) :
		
		if not ( changeType & maya.OpenMaya.MNodeMessage.kAttributeSet ) :
			return
		
		try :
			myPlug = self.plug()
		except :
			# this situation can occur when our parameter has been removed but the
			# ui we represent is not quite yet dead
			return
		
		if not plug == myPlug :
			return
		
		self.replace( self.node(), self.parameter )
	
	def __setUIFromPlug( self ) :
		
		vector = maya.cmds.getAttr( self.plugName() ) or []
		for i in range( 0, len(vector) ) :
			maya.cmds.textField( self.__fields[i], e=True, text=vector[i] )
	
	def __setPlugFromUI( self ) :
		
		vector = []
		
		for field in self.__fields :
			vector.append( maya.cmds.textField( field, q=True, text=True ) )
			
		self.__setPlug( vector )
		
	def __setPlug( self, value ) :
		
		## \todo: do this in python if maya ever fixes the nonsense required to call setAttr on a stringArray
		plugType = maya.cmds.getAttr( self.plugName(), type=True )
		cmd = 'setAttr %s -type %s %d' % ( self.plugName(), plugType, len(value) )
		for val in value :
			cmd += ' "%s"' % val
		maya.mel.eval( cmd )
	
	__disableCopyPaste = IECore.CompoundObject( {
		"UI" : IECore.CompoundObject( {
			"copyPaste" : IECore.BoolData( False ),
		} ),
	} )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.StringVectorParameter, StringVectorParameterUI )

import ProceduralHolderUI
from ManipulatorUI import *


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
	

from TransformationMatrixParameterUI import TransformationMatrixParameterUI


from __future__ import with_statement

import maya.cmds

import IECore
import IECoreMaya

## The UI for the TransformationMatrixParameter supports the following
## userData()
##
##   - "visibleFields" IECore.StringVectorData, A list of fields to
##     display in the UI. Possible values are (D marks a default):
##         "translate" D
##         "rotate", D
##         "scale" D
##         "shear" D
##         "rotatePivot",
##         "rotatePivotTranslation",
##         "scalePivot"
##         "scalePivotTranslation"

class TransformationMatrixParameterUI( IECoreMaya.ParameterUI ) :

	_allFields = ( "translate", "rotate", "scale", "shear",	"scalePivot", "scalePivotTranslation", "rotatePivot", "rotatePivotTranslation" )

	def __init__( self, node, parameter, **kw ) :
			
		self._outerColumn = maya.cmds.columnLayout( adj=True )
		
		IECoreMaya.ParameterUI.__init__( self, node, parameter, self._outerColumn, **kw )
		
		maya.cmds.rowLayout( numberOfColumns=2, parent=self._outerColumn )

		self._label = maya.cmds.text(
			label = self.label(),
			font = "tinyBoldLabelFont",
			align = "right",
			annotation = self.description()
		)

		self._manip = maya.cmds.button( label="Manipulate" )

		maya.cmds.setParent("..")
		maya.cmds.setParent("..")

		self._fields = {}
		self.__kw = kw.copy()

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )
		currentParent = maya.cmds.setParent( query=True )
		
		visibleFields = IECore.StringVectorData( ( "translate", "rotate", "scale", "shear" ) )
		with IECore.IgnoredExceptions( KeyError ) :
			userDataFields = parameter.userData()["UI"]["visibleFields"]	
			visibleFields = []
			for u in userDataFields :
				if u not in TransformationMatrixParameterUI._allFields:
					IECore.msg( 
						IECore.Msg.Level.Warning,
						"TransformationMatrixParameterUI",
						"Invalid field '%s' requested in UI userData for '%s'. Available fields are %s."
						  % ( u, parameter.name, TransformationMatrixParameterUI._allFields )
					)
					continue
				visibleFields.append( u )
		
		for f in self._fields.keys() :
			if f not in visibleFields :
				maya.cmds.deleteUI( self._fields[f][0] )
				del self._fields[f]	
		
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		baseName = fnPH.parameterPlugPath( parameter )
		
		self._addPopupMenu( parentUI=self._label, attributeName=baseName )
		
		for f in visibleFields :
			
			if f not in self._fields :
				layout = maya.cmds.rowLayout(
					numberOfColumns = 4,
					parent = self._outerColumn,
					columnWidth4 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
				)
				maya.cmds.text( label=f, font="smallPlainLabelFont", align="right" )
				self._fields[f] = ( layout, maya.cmds.floatField(), maya.cmds.floatField(), maya.cmds.floatField() )				
					
			maya.cmds.connectControl( self._fields[f][1], "%s%s%i" % ( baseName, f, 0 ) )
			maya.cmds.connectControl( self._fields[f][2], "%s%s%i" % ( baseName, f, 1 ) )
			maya.cmds.connectControl( self._fields[f][3], "%s%s%i" % ( baseName, f, 2 ) )

		maya.cmds.button( 
			self._manip, 
			edit = True,
			# The manip is currently only registered for float types
			visible = isinstance( parameter, IECore.TransformationMatrixfParameter ),
			command = self._createCallback( IECore.curry( IECoreMaya.ManipulatorUI.manipulateParameter, node, parameter ) ) 
		)
		
		maya.cmds.setParent( currentParent )
		
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.TransformationMatrixfParameter, TransformationMatrixParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.TransformationMatrixdParameter, TransformationMatrixParameterUI )

from LineSegmentParameterUI import LineSegmentParameterUI


import maya.cmds

import IECore
import IECoreMaya

class LineSegmentParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		IECoreMaya.ParameterUI.__init__( self, node, parameter, maya.cmds.columnLayout(), **kw )

		self.__fields = []

		self.__dim = parameter.getTypedValue().dimensions()

		plug = self.plug()
		for childIndex in range( 0, 2 ) :

			if self.__dim == 2:
				maya.cmds.rowLayout(
					numberOfColumns = 3,
					columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
				)
			elif self.__dim == 3:
				maya.cmds.rowLayout(
					numberOfColumns = 4,
					columnWidth4 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
				)
			else:
				raise RuntimeError( "Unsupported vector dimension in LineSegmentParameterUI" )

			parameterLabel = self.label()
			if childIndex==0 :
				parameterLabel = parameterLabel + "Start"
			else:
				parameterLabel = parameterLabel + "End"

			maya.cmds.text(
				label = parameterLabel,
				font = "smallPlainLabelFont",
				align = "right",
				annotation = self.description()
			)

			vectorPlug = plug.child( childIndex )

			for i in range( 0, self.__dim ) :

				self.__fields.append( self.__fieldType()() )

			maya.cmds.setParent("..")

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		fieldNum = 0
		plug = self.plug()
		for childIndex in range( 0, 2 ) :

			vectorPlug = plug.child( childIndex )
			for i in range( 0, self.__dim ) :

				vectorPlugChild = vectorPlug.child( i )
				vectorPlugChildName = self.nodeName() + "." + vectorPlugChild.partialName()
				maya.cmds.connectControl( self.__fields[ fieldNum ], vectorPlugChildName )
				self._addPopupMenu( parentUI = self.__fields[fieldNum], attributeName = vectorPlugChildName )

				fieldNum += 1

	def __fieldType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.Box2iParameter ) or self.parameter.isInstanceOf( IECore.TypeId.Box3iParameter ):
			return maya.cmds.intField
		else:
			return maya.cmds.floatField

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.LineSegment3fParameter, LineSegmentParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.LineSegment3dParameter, LineSegmentParameterUI )


from Collapsible import Collapsible


import maya.cmds
import maya.OpenMaya

import IECoreMaya

## In Maya 2011 and 2012, the collapsible frameLayout became rather ugly,
# and stopped indenting the arrow with the label. This made complex uis
# consisting of lots of ClassVectorParameters and ClassParameters somewhat
# unreadable. So we introduce this class to get back some control. Aside
# from spelling collapsible properly and being prettier, this class also
# has the advantage of supporting annotations which are displayed on the label.
# As with the maya frameLayout, the preExpandCommand, expandCommand and
# collapseCommand are only called as a result of user action, and never as
# a result of a call to setCollapsed or getCollapsed. There are separate
# implementations for maya before qt and maya after qt.

class _CollapsibleMotif( IECoreMaya.UIElement ) :

	def __init__( self,
		label="",
		labelVisible=True,
		labelIndent=0,
		labelFont = "boldLabelFont",
		annotation="",
		collapsed = True,
		preExpandCommand = None,
		expandCommand = None,
		collapseCommand = None,
	) :
		
	
		kw = {}
		if preExpandCommand is not None :
			kw["preExpandCommand"] = preExpandCommand
		if expandCommand is not None :
			kw["expandCommand"] = expandCommand
		if collapseCommand is not None :
			kw["collapseCommand"] = collapseCommand

		# implementation for motif is pretty simple - just a frame layout
			
		IECoreMaya.UIElement.__init__( self,
			maya.cmds.frameLayout(
		
				label = label,
				labelVisible = labelVisible,
				labelIndent = labelIndent,
				labelAlign = "center",
				font = labelFont,
				borderVisible = False,
				collapsable = True,
				collapse = collapsed,
				marginWidth = 0,
				**kw
			
			)
		)
		
		# can't display it but at least we can store it
		self.__annotation = annotation
		
		self.__frameLayout = self._topLevelUI()
		
	## The maya frameLayout whose collapsibility is controlled by this
	# class. Add children by editing the contents of this layout.
	def frameLayout( self ) :
	
		return self._topLevelUI()

	def setLabel( self, label ) :
	
		maya.cmds.frameLayout( self.frameLayout(), edit=True, label = label )

	def getLabel( self ) :
	
		return maya.cmds.frameLayout( self.frameLayout(), query=True, label = True )

	def setAnnotation( self, annotation ) :
	
		self.__annotation = annotation
		
	def getAnnotation( self ) :
	
		return self.__annotation

	def getCollapsed( self ) :
	
		return maya.cmds.frameLayout( self.frameLayout(), query=True, collapse=True )
		
	def setCollapsed( self, collapsed ) :
	
		maya.cmds.frameLayout( self.frameLayout(), edit=True, collapse=collapsed )

class _CollapsibleQt(  IECoreMaya.UIElement ) :

	def __init__( self,
		label="",
		labelVisible=True,
		labelIndent=0,
		labelFont = "boldLabelFont",
		annotation="",
		collapsed = True,
		preExpandCommand = None,
		expandCommand = None,
		collapseCommand = None,
	) :
		
		IECoreMaya.UIElement.__init__( self, maya.cmds.formLayout() )
		
		attachForm = []
		attachControl = []
		
		# make the layout to put things in. this is actually a frameLayout, just
		# with the ugly header bit we don't like hidden.
		########################################################################
		
		self.__frameLayout = maya.cmds.frameLayout(
		
			labelVisible = False,
			borderVisible = False,
			collapsable = True,
			collapse = collapsed,
			marginWidth = 0,
				
		)
		
		# passing borderVisible=False to the constructor does bugger all so we have to do it with
		# an edit
		maya.cmds.frameLayout( self.__frameLayout, edit=True, borderVisible=False, marginWidth=0 )

		attachForm.append( ( self.__frameLayout, "left", 0 ) )
		attachForm.append( ( self.__frameLayout, "right", 0 ) )
		attachForm.append( ( self.__frameLayout, "bottom", 0 ) )
		
		# optional header, with the triangle for expanding and collapsing
		########################################################################
				
		self.__collapsibleIcon = None
		self.__labelControl = None
		if labelVisible :
						
			# have to make one button for the icon and one for the label
			# because otherwise the icon size changes when we toggle
			# the image, and the text moves.			
			self.__collapsibleIcon = maya.cmds.iconTextButton(
			
				parent = self._topLevelUI(),
				height = 20,
				width = 15,
				image = "arrowRight.xpm",
				command = self.__toggle,
				annotation = annotation,
				
			)
		
			self.__labelControl = maya.cmds.iconTextButton(
			
				parent = self._topLevelUI(),
				height = 20,
				label = label,
				# the font flag appears to do nothing, but maybe it will
				# miraculously be supported in the future?
				font = labelFont,
				style = "textOnly", 
				command = self.__toggle,
				annotation = annotation,
				
			)
			
			attachForm.append( ( self.__collapsibleIcon, "left", labelIndent ) )
			attachForm.append( ( self.__collapsibleIcon, "top", 0 ) )
			attachForm.append( ( self.__labelControl, "top", 0 ) )
			
			attachControl.append( ( self.__labelControl, "left", 0, self.__collapsibleIcon ) )
			attachControl.append( ( self.__frameLayout, "top", 0, self.__labelControl ) )
			
		else :
		
			attachForm.append( ( self.__frameLayout, "top", 0 ) )	
							
		maya.cmds.formLayout(
			self._topLevelUI(),
			edit = True,
			attachForm = attachForm,
			attachControl = attachControl,
		)
		
		maya.cmds.setParent( self.__frameLayout )
		
		self.__annotation = annotation
		self.__labelText = label	
		self.__preExpandCommand = preExpandCommand
		self.__expandCommand = expandCommand
		self.__collapseCommand = collapseCommand

	## The maya frameLayout whose collapsibility is controlled by this
	# class. Add children by editing the contents of this layout.
	def frameLayout( self ) :
	
		return self.__frameLayout

	def setLabel( self, label ) :
	
		self.__labelText = label
		if self.__labelControl is not None :
			maya.cmds.iconTextButton( self.__labelControl, edit=True, label=label )

	def getLabel( self ) :
	
		return self.__labelText

	def setAnnotation( self, annotation ) :
	
		self.__annotation = annotation
		if self.__labelControl is not None :
			maya.cmds.iconTextButton( self.__labelControl, edit=True, annotation=annotation )
			maya.cmds.iconTextButton( self.__collapsibleIcon, edit=True, annotation=annotation )
					
	def getAnnotation( self ) :
	
		return self.__annotation

	def getCollapsed( self ) :
	
		return maya.cmds.frameLayout( self.__frameLayout, query=True, collapse=True )
		
	def setCollapsed( self, collapsed ) :
	
		maya.cmds.frameLayout( self.__frameLayout, edit=True, collapse=collapsed )
		if self.__collapsibleIcon is not None :
			maya.cmds.iconTextButton(
				self.__collapsibleIcon,
				edit = True,
				image = "arrowRight.xpm" if collapsed else "arrowDown.xpm",
			)

	def __toggle( self ) :
	
		collapsed = not self.getCollapsed()
		if not collapsed and self.__preExpandCommand is not None :
				self.__preExpandCommand()
			
		self.setCollapsed( not self.getCollapsed() )

		if collapsed :
			if self.__collapseCommand is not None :
				self.__collapseCommand()
		else :
			if self.__expandCommand is not None :
				self.__expandCommand()

# choose the right implementation based on the current maya version
if maya.OpenMaya.MGlobal.apiVersion() >= 201100 :
	Collapsible = _CollapsibleQt
else :
	Collapsible = _CollapsibleMotif

import Menus
