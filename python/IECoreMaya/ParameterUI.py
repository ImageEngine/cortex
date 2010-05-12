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

import maya.cmds as cmds
import maya.OpenMaya

import IECore
import IECoreMaya

## Base class for objects which are able to create an Attribute Editor widget for a single IECore.Parameter
# held on an IECoreMaya.ParameterisedHolder node.
# \todo Make member functions protected or private as necessary - do this for the derived classes too.
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

		self.__node = parameterisedHolderNode
		self.parameter = parameter #IECore.Parameter

		self.__labelWithNodeName = kw.get( "labelWithNodeName", False )
		self.__longParameterName = kw.get( "longParameterName", parameter.name )

	## Derived classes should override this method. The override should first call the base class method and
	# then reconnect all created widgets to the new node/parameter. The node and parameter arguments are as
	# for the __init__ function.
	def replace( self, node, parameter ) :

		self.__node = node
		self.parameter = parameter

	## Returns the Maya node associated with this UI in the form of an OpenMaya.MObject
	def node( self ) :

		return self.__node

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

		cmds.popupMenu( parent = parentUI, postMenuCommand = IECore.curry( self.__buildPopupMenu, **kw ) )

		if "button1" in kw and kw["button1"] :
			cmds.popupMenu( parent = parentUI, button=1, postMenuCommand = IECore.curry( self.__buildPopupMenu, **kw ) )


	def __buildConnectionsPopupMenu( self, popupMenu, ownerControl, **kw ):

		connections = cmds.listConnections(
			kw['attributeName'],
			d = False,
			s = True,
			plugs = True,
			connections = True,
			skipConversionNodes = True
		)
		
		cmds.menuItem(
			parent = popupMenu,
			label = "Connection Editor...",
			command = IECore.curry( self.connectionEditor )
		)

		if connections :
			
			cmds.menuItem(
				parent = popupMenu,
				divider = True
			)

			for i in xrange( 0, len(connections), 2 ):

				conItem = cmds.menuItem(
					parent = popupMenu,
					subMenu = True,
					label = connections[i+1],

				)

				cmds.menuItem(
					parent = conItem,
					label = "Connection Editor...",
					command = IECore.curry( self.connectionEditor, leftHandNode = connections[i+1] )
				)

				cmds.menuItem(
					parent = conItem,
					label = "Open AE...",
					command = IECore.curry( self.showEditor, attributeName = connections[i+1] )
				)

				cmds.menuItem(
					parent = conItem,
					label = "Break Connection",
					command = IECore.curry( self.disconnect, source = connections[i+1],
											destination = connections[i], refreshAE = self.nodeName() )
				)
				
			cmds.menuItem(
				parent = popupMenu,
				divider = True
			)			
						
			return True

		else:

			return False

	def __buildPopupMenu( self, popupMenu, ownerControl, **kw ):

		cmds.popupMenu(
			popupMenu,
			edit = True,
			deleteAllItems = True
		)

		if cmds.getAttr( kw['attributeName'], lock = True) == 0:

			settable = maya.cmds.getAttr( kw["attributeName"], settable=True )
			if settable :

				# make menu items for all presets and for the default value

				for k in self.parameter.presetNames() :
					cmds.menuItem(
						parent = popupMenu,
						label = k,
						command = IECore.curry( self.__selectValue, selection = k )
					)

				if len( self.parameter.presetNames() ) > 0:
					cmds.menuItem(
						parent = popupMenu,
						divider = True
					)

				cmds.menuItem(
						parent = popupMenu,
						label = "Default",
						command = IECore.curry( self.__selectValue, selection = self.parameter.defaultValue )
				)

				cmds.menuItem(
					parent = popupMenu,
					divider = True
				)

			controlType = cmds.objectTypeUI( ownerControl)
			if controlType == "floatField" or controlType == "intField":

				if cmds.getAttr( kw['attributeName'], keyable=True) and settable :
					cmds.menuItem(
						parent = popupMenu,
						label = "Set Key",
						command = IECore.curry( self.__setKey, **kw )
					)

				expressions = cmds.listConnections(
					kw['attributeName'],
					d = False,
					s = True,
					type = "expression"
				)

				if not expressions:

					hasConnections = self.__buildConnectionsPopupMenu( popupMenu, ownerControl, **kw )
					if not hasConnections and settable :

						cmds.menuItem(
							parent = popupMenu,
							label = "Create New Expression...",

							command = IECore.curry( self.expressionEditor, **kw )
						)

				else:

					cmds.menuItem(
						parent = popupMenu,
						label = "Edit Expression...",

						command = IECore.curry( self.expressionEditor, **kw )
					)


					cmds.menuItem(
						parent = popupMenu,
						label = "Delete Expression",

						command = IECore.curry( self.__deleteNode, nodeName = expressions[0] )
					)

			else:

				self.__buildConnectionsPopupMenu( popupMenu, ownerControl, **kw )

			cmds.menuItem(
					parent = popupMenu,
					label = "Lock attribute",
					command = IECore.curry( self.__lock, **kw )
			)

		else:

			cmds.menuItem(
					parent = popupMenu,
					label = "Unlock attribute",
					command = IECore.curry( self.__unlock, **kw )
			)

	def showEditor( self, args, attributeName = None ):

		split = attributeName.split('.', 1 )
		node = split[0]

		melCmd = 'showEditor "' + node + '"'

		IECoreMaya.mel( melCmd.encode('ascii') )

	def __deleteNode( self, args, nodeName = None ):

		cmds.delete( nodeName )

	def expressionEditor( self, args, attributeName = None ):

		split = attributeName.split('.', 1 )
		node = split[0]
		attr = split[1]

		melCmd = 'expressionEditor EE "' + node + '" "' + attr + '"'

		IECoreMaya.mel( melCmd.encode('ascii') )

	def connectionEditor( self, args, leftHandNode = None ) :
	
		import maya.mel
		maya.mel.eval(
				str("ConnectionEditor;"+
				"nodeOutliner -e -replace %(right)s connectWindow|tl|cwForm|connectWindowPane|rightSideCW;"+
				"connectWindowSetRightLabel %(right)s;") % { 'right' : self.nodeName() } )
		
		if leftHandNode :
	
			maya.mel.eval(
				str("nodeOutliner -e -replace %(left)s connectWindow|tl|cwForm|connectWindowPane|leftSideCW;"+
				"connectWindowSetLeftLabel %(left)s;" ) % { 'left' : leftHandNode.split(".")[0] } )

	def disconnect( self, args, source = None, destination = None, refreshAE = None ):

		cmds.disconnectAttr( source, destination )
		
		if refreshAE :
			import maya.mel
			maya.mel.eval( 'evalDeferred( "updateAE %s;")' % refreshAE )


	def __setKey( self, args, **kw ):

		cmds.setKeyframe(
			kw['attributeName']
		)

	def __lock( self, args, **kw ):

		cmds.setAttr(
			kw['attributeName'],
			lock = True
		)

	def __unlock( self, args, **kw  ):

		cmds.setAttr(
			kw['attributeName'],
			lock = False
		)

	def __selectValue( self, args, selection = None):

		self.parameter.setValue( selection )
		IECoreMaya.FnParameterisedHolder( self.__node ).setNodeValue( self.parameter )

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
