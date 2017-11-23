##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

			return maya.mel.eval( "interToUI(\"" + n + "\")" )

		else :

			return maya.mel.eval( "interToUI(\"" + self.parameter.name + "\")" )

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

		IECoreMaya.Menu( definition = IECore.curry( self.__popupMenuDefinition, **kw ), parent = parentUI )

		if "button1" in kw and kw["button1"] :
			IECoreMaya.Menu( definition = IECore.curry( self.__popupMenuDefinition, **kw ), parent = parentUI, button = 1 )

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

		maya.mel.eval( melCmd.encode('ascii') )

	def __deleteNode( self, nodeName = None ) :

		cmds.delete( nodeName )

	def __expressionEditor( self, attributeName = None ) :

		split = attributeName.split('.', 1 )
		node = split[0]
		attr = split[1]

		melCmd = 'expressionEditor EE "' + node + '" "' + attr + '"'

		maya.mel.eval( melCmd.encode('ascii') )

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

		if parameter.presetsOnly and len( parameter.getPresets() ) :
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

