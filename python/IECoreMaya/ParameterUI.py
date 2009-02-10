##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

import math
import os.path
import traceback

import maya.cmds as cmds
import maya.OpenMaya

import IECore
import IECoreMaya

## Base class for objects which are able to create an Attribute Editor widget for a single IECore.Parameter
# held on an IECoreMaya.ParameterisedHolder node.
# \todo Make member functions protected or private as necessary - do this for the derived classes too.
# \todo Split derived classes out into their own files.
class ParameterUI :

	textColumnWidthIndex = 145
	singleWidgetWidthIndex = 70
	sliderWidgetWidthIndex = 2 * 70
	
	handlers = {}

	## The parameterisedHolderNode is an MObject specifying the node holding the specified IECore.Parameter.
	# Derived class __init__ implementations should create relevant widgets in the current ui layout,
	# and leave the parent layout unchanged on exit.
	# \todo Document the expected behaviour of derived classes with respect to setting up self._layout,
	# or provide a more explicit mechanism for the same thing.
	# \todo Document the meaning of the various keyword arguments.
	def __init__( self, parameterisedHolderNode, parameter, **kw ) :

		self.__node = parameterisedHolderNode
		self.parameter = parameter #IECore.Parameter
		self.__popupControl = None
		self._layout = None
		
		self.labelWithNodeName = False
		
		if "labelWithNodeName" in kw :
		
			self.labelWithNodeName = kw['labelWithNodeName']		
			
		self.longParameterName = parameter.name
		if "longParameterName" in kw :
		
			self.longParameterName = kw['longParameterName']		
	
	## Derived classes should override this method. The override should first call the base class method and
	# then reconnect all created widgets to the new node/parameter. The node and parameter arguments are as
	# for the __init__ function.
	def replace( self, node, parameter ) :
	
		self.__node = node		
		self.parameter = parameter
		
		if self.__popupControl:
		
			cmds.iconTextStaticLabel(			
				self.__popupControl,
				edit = True,
				label = self.parameter.getCurrentPresetName(),			
			)

			self._addPopupMenu( parentUI=self.__popupControl, attributeName = self.plugName() )
	
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
	
		return self._layout	
	
	## Computes a nice label for the ui.
	def label( self ):
	
		if self.labelWithNodeName :
		
			n = self.nodeName() + "." + self.longParameterName
			if not self.longParameterName :
				# Top-level parameter comes through into here without a name
				n = self.nodeName() + ".parameters"
		
			return IECoreMaya.mel( "interToUI(\"" + n + "\")" ).value
		
		else :

			return IECoreMaya.mel( "interToUI(\"" + self.parameter.name + "\")" ).value
	
	## Computes a wrapped annotation/tooltip for the ui	
	def description( self ):
	
		return IECore.StringUtil.wrap( self.parameter.description, 48 )
		
	## Creates a drop-down selection list and returns True if the parameter is set to "presets only". Otherwise returns False.
	def presetsOnly( self ):
	
		self.__popupControl = None
	
		if self.parameter.presetsOnly:
			self._layout = cmds.rowLayout(
				numberOfColumns = 2,
			)
		
			cmds.text(
				label = self.label(),
				font = "smallPlainLabelFont",
				align = "right",
				annotation = self.description(),				
			)
			
			self.__popupControl = cmds.iconTextStaticLabel(
				image = "arrowDown.xpm",
				font = "smallBoldLabelFont",
				label = self.parameter.getCurrentPresetName(),
				style = "iconAndTextHorizontal",
				height = 23 #\ todo			
			)
			
			self._addPopupMenu( parentUI=self.__popupControl, attributeName = self.plugName() )
			
			cmds.setParent("..")
	
			return True
			
		else:
		
			return False
	
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
			
	## \deprecated Use _addPopupMenu instead.
	def addPopupMenu( self, parentUI=None, **kw ):
	
		IECore.msg( IECore.Msg.Level.Warning, self.__class__.__name__ + ".addPopupMenu", "Deprecated method called." )
		self._addPopupMenu( parentUI = maya.cmds.setParent( query=True ), **kw )
	
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
				
		cmds.popupMenu( parent = parentUI, postMenuCommand = IECore.curry( self.buildPopupMenu, **kw ) )
		
	def __buildConnectionsPopupMenu( self, popupMenu, ownerControl, **kw ):
	
		connections = cmds.listConnections( 
			kw['attributeName'],
			d = False,
			s = True,
			plugs = True,
			connections = True,
			skipConversionNodes = True			
		)

		if connections:

			cmds.menuItem(
				parent = popupMenu,
				label = connections[1],
				
				command = IECore.curry( self.showEditor, attributeName = connections[1] )
			)

			cmds.menuItem(
				parent = popupMenu,
				label = "Break Connection",
				
				command = IECore.curry( self.disconnect, source = connections[1], destination = connections[0] )
			)
			
			return True
			
		else:
		
			return False
			
	def buildPopupMenu( self, popupMenu, ownerControl, **kw ):

		cmds.popupMenu(
			popupMenu,
			edit = True,
			deleteAllItems = True
		)
		
		hasConnections = False			
				
		if cmds.getAttr( kw['attributeName'], lock = True) == 0:
		
			for k in sorted( self.parameter.presets().keys() ):
				cmds.menuItem(
					parent = popupMenu,
					label = k,
					command = IECore.curry( self.selectValue, selection = k )
				)

			if len( self.parameter.presets().keys() ) > 0:
				cmds.menuItem(
					parent = popupMenu,
					divider = True
				)	
		
			cmds.menuItem(
					parent = popupMenu,
					label = "Default",
					command = IECore.curry( self.selectValue, selection = self.parameter.defaultValue )
			)

			cmds.menuItem(
				parent = popupMenu,
				divider = True
			)		
		
			controlType = cmds.objectTypeUI( ownerControl)
			
			if controlType == "floatField" or controlType == "intField":

				if cmds.getAttr( kw['attributeName'], keyable=True) == 1:
					cmds.menuItem(
						parent = popupMenu,
						label = "Set Key",
						command = IECore.curry(self.setKey, **kw)
					)

				expressions = cmds.listConnections( 
					kw['attributeName'],
					d = False,
					s = True,
					type = "expression"
				)

				if not expressions:

					hasConnections = self.__buildConnectionsPopupMenu( popupMenu, ownerControl, **kw )
					
					if not hasConnections:

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
						
						command = IECore.curry( self.deleteNode, nodeName = expressions[0] )
					)
					
			else:
			
				hasConnections = self.__buildConnectionsPopupMenu( popupMenu, ownerControl, **kw )
		
			cmds.menuItem(
					parent = popupMenu,
					label = "Lock attribute",
					command = IECore.curry(self.lock, **kw)					
			)
			
		else:
		
			cmds.menuItem(
					parent = popupMenu,
					label = "Unlock attribute",
					command = IECore.curry(self.unlock, **kw)
			)
	
	def showEditor( self, args, attributeName = None ):
	
		split = attributeName.split('.', 1 )
		node = split[0]
		
		melCmd = 'showEditor "' + node + '"'
		
		IECoreMaya.mel( melCmd.encode('ascii') )
			
	def deleteNode( self, args, nodeName = None ):	
	
		cmds.delete( nodeName )	
			
	def expressionEditor( self, args, attributeName = None):
	
		split = attributeName.split('.', 1 )
		node = split[0]
		attr = split[1]
		
		melCmd = 'expressionEditor EE "' + node + '" "' + attr + '"'
		
		IECoreMaya.mel( melCmd.encode('ascii') )
		
	def disconnect( self, args, source = None, destination = None):	
	
		cmds.disconnectAttr( source, destination )
	
	def setKey( self, args, **kw ):
	
		cmds.setKeyframe( 
			kw['attributeName']
		)
		
	def lock( self, args, **kw ):
	
		cmds.setAttr( 
			kw['attributeName'],
			lock = True
		)
		
	def unlock( self, args, **kw  ):
	
		cmds.setAttr( 
			kw['attributeName'],
			lock = False
		)		
			
	def selectValue( self, args, selection = None):

		self.parameter.setValue( selection )
		IECoreMaya.FnParameterisedHolder( self.__node ).setNodeValue( self.parameter )
		
		if self.__popupControl:
			cmds.iconTextStaticLabel(
				self.__popupControl,
				edit = True,
				label = self.parameter.getCurrentPresetName()
			)
	

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

		uiTypeHint = None
		try:
			uiTypeHint = parameter.userData()['UI']['typeHint'].value
		except:
			pass
	
		if not ( parameter.typeId(), uiTypeHint ) in ParameterUI.handlers:
			# \todo Issue a warning
			return None
		
		if 'longParameterName' in kw and len( kw['longParameterName'] ) :
			kw['longParameterName'] += "." + parameter.name
		else :
			kw['longParameterName'] = parameter.name
			

		handlerType = ParameterUI.handlers[ (parameter.typeId(), uiTypeHint) ]
		parameterUI = handlerType( parameterisedHolderNode, parameter, **kw )
		
		return parameterUI

	
		
		
class CompoundParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw  ) :
	
		ParameterUI.__init__( self, node, parameter, **kw )
		
		visible = True
		try:
			visible = parameter.userData()['UI']['visible'].value
		except:
			pass
			
		if 'visibleOnly' in kw :
		
			visible = False
		
			for i in kw['visibleOnly'] :
			
				if kw['longParameterName'] == "" or i.startswith( kw['longParameterName'] + "." ) :
				
					visible = True
				
							
		if not visible :
		
			return
		
		self.__childUIsLayout = None		
		self.__childUIs = {}
		
		self._layout = None
		
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		
		withCompoundFrame = False
		if 'withCompoundFrame' in kw :
			withCompoundFrame = kw['withCompoundFrame']
				
		if not withCompoundFrame and parameter.isSame( fnPH.getParameterised()[0].parameters() ) :
			self._layout = cmds.columnLayout()
			self.__createChildUIs( **kw )
			cmds.setParent("..")
		else:
			# \todo Retrieve the "collapsed" state
			collapsed = True
		
			self._layout = cmds.frameLayout( 
				label = self.label(),
				borderVisible = False,
				preExpandCommand = IECore.curry( self.__createChildUIs, **kw),
				collapseCommand = self.__collapse,
				collapsable = True,
				collapse = collapsed,				
			)
				
			if not collapsed:
				self.__createChildUIs( **kw )
			
			cmds.setParent("..")
		
			
		
	def replace( self, node, parameter ) :
	
		ParameterUI.replace( self, node, parameter )
		
		for pName in self.__childUIs.keys():
			ui = self.__childUIs[pName]
			p = self.parameter[pName]
			
			ui.replace( node, p )
	
	def __collapse(self):
		# \todo Store collapsed state of self._layout
		pass

	def __createChildUIs(self, **kw):
	
		# this is the most common entry point into the ui
		# creation code, and unfortunately it's called from
		# a maya ui callback. maya appears to suppress all
		# exceptions which occur in such callbacks, so we
		# have to wrap with our own exception handling to
		# make sure any errors become visible.
		try :
		
			if self.__childUIsLayout:
				return
				
			kw['labelWithNodeName'] = False	

			# \todo Store collapsed state of self._layout

			cmds.setUITemplate(
				"attributeEditorTemplate",
				pushTemplate = True
			)

			self.__childUIsLayout = cmds.columnLayout( 
				parent = self._layout,
				width = 381
			)
			
			draggable = False
			try:
				draggable = self.parameter.userData()['UI']['draggable'].value
			except :
				pass
			
			if draggable :
			
				cmds.rowLayout(
					numberOfColumns = 2,
					columnWidth2 = ( 361, 20 )

				)

				cmds.text( label = "" )

				dragIcon = cmds.iconTextStaticLabel(					
					image = "pick.xpm",
					height = 20
				)
				self.addDragCallback( dragIcon, **kw )			
						
			for pName in self.parameter.keys():							
				
				p = self.parameter[pName]
				
				visible = True
				try:
					visible = p.userData()['UI']['visible'].value
				except:
					pass
					
				if 'visibleOnly' in kw :							
				
					fullChildName = kw['longParameterName'] 
					
					if len( fullChildName ) :
						fullChildName += "."
						
					fullChildName += pName 
				
					visible = fullChildName in kw['visibleOnly']
					
					if not visible and p.isInstanceOf( IECore.TypeId.CompoundParameter ) :
					
						for i in kw['visibleOnly'] :
						
							if i.startswith( fullChildName + "." ) :
								
								visible = True
												
				if visible:
					cmds.setParent( self.__childUIsLayout )
				
					ui = ParameterUI.create( self.node(), p, **kw )

					if ui:			
						self.__childUIs[pName] = ui

			cmds.setParent("..")

			cmds.setUITemplate(
				"attributeEditorTemplate",
				popTemplate = True
			)		

		except :
		
			IECore.msg( IECore.Msg.Level.Error, "ParameterUI", traceback.format_exc() )


class BoolParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ):
		ParameterUI.__init__( self, node, parameter, **kw )
		
		self._layout = cmds.rowLayout(
			numberOfColumns = 2,
			columnWidth2 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex * 3 ]
		)
		
		cmds.text(
			label = "",
		)
				
		self.__checkBox = cmds.checkBox(
			annotation = self.description(),			
			label = self.label(),
			align = "left",
		)
				
		cmds.setParent("..")
		
		self.replace( self.node(), self.parameter )
		
	def replace( self, node, parameter ) :		
	
		ParameterUI.replace( self, node, parameter )
			
		cmds.connectControl( self.__checkBox, self.plugName() )
		

class StringParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ):
		ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__textField = None
		if self.presetsOnly():
			return	
		
		self._layout = cmds.rowLayout(
			numberOfColumns = 2,			
		)
		
		cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),				
		)
		
		self.__textField = cmds.textField()
				
		cmds.setParent("..")
		
		self.replace( self.node(), self.parameter )
		
	def replace( self, node, parameter ) :	
	
		ParameterUI.replace( self, node, parameter )	
		
		if self.__textField:	
			self._addPopupMenu( parentUI=self.__textField, attributeName = self.plugName() )
			cmds.connectControl( self.__textField, self.plugName() )


class PathParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ):
		ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__textField = None
		if self.presetsOnly():
			return	
		
		self._layout = cmds.rowLayout(
			numberOfColumns = 3,
			columnWidth3 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex * 3, 26 ]			
		)
		
		cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),				
		)
		
		self.__textField = cmds.textField()
				
		cmds.iconTextButton(
			label = "",
			image = "fileOpen.xpm",
			command = self.openDialog,
			height = 23,
			style = "iconOnly"
		)
								
		cmds.setParent("..")

		self.replace( self.node(), self.parameter )
		
	def replace( self, node, parameter ) :	
	
		ParameterUI.replace( self, node, parameter )	
		
		if self.__textField:	
			self._addPopupMenu( parentUI=self.__textField, attributeName = self.plugName() )
			cmds.connectControl( self.__textField, self.plugName() )
			
	def openDialog( self ) :
	
		pass


class FileNameParameterUI( PathParameterUI ) :	

	def __init__( self, node, parameter, **kw ):
	
		PathParameterUI.__init__( self, node, parameter, **kw )				
			
	def openDialog( self ) :
	
		selection = cmds.fileDialog().encode('ascii')
			
		if len(selection):
		
			self.parameter.setValue( IECore.StringData( selection ) )
			fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
			fnPH.setNodeValue( self.parameter )	


class DirNameParameterUI( PathParameterUI ) :	

	def __init__( self, node, parameter, **kw ):
	
		PathParameterUI.__init__( self, node, parameter, **kw )				
			
	def openDialog( self ) :
	
		selection = cmds.fileDialog().encode('ascii')
			
		if len(selection):
			d = os.path.dirname(selection)
			self.parameter.setValue( IECore.StringData( d ) )
			fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
			fnPH.setNodeValue( self.parameter )	


class FileSequenceParameterUI( PathParameterUI ) :					

	def __init__( self, node, parameter, **kw ):
	
		PathParameterUI.__init__( self, node, parameter, **kw )
			
	def openDialog( self ) :
	
		selection = cmds.fileDialog().encode('ascii')
			
		if len(selection):
			d = os.path.dirname(selection)
			sequences = IECore.ls(d)
			
			if sequences:
			
				for seq in sequences:
				
					if os.path.basename(selection) in seq.fileNames():
					
						self.parameter.setValue( IECore.StringData( os.path.join( d, str( seq ) ) ) )
						fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
						fnPH.setNodeValue( self.parameter )	
						
						return	
		
# Specialized interface for nodes like kiwiRenderer that get the full path plus the file name prefix of a FIO file sequence.
# \todo Get rid of this once kiwiRenderer and the cache nodes uses FileSequenceParameter instead.
class CachePathPrefixParameterUI( PathParameterUI ) :

	def __init__( self, node, parameter, **kw ):
	
		PathParameterUI.__init__( self, node, parameter, **kw )

	def openDialog( self ):

		selection = cmds.fileDialog( dm='*.fio' ).encode('ascii')
			
		if len(selection):
			d = os.path.dirname(selection)
			sequences = ls(d)
			
			if sequences:
			
				for seq in sequences:
				
					if os.path.basename(selection) in seq.fileNames():
						newValue = seq.getPrefix()
						if newValue.endswith('.'):
							self.parameter.setValue( IECore.StringData( os.path.join( d, newValue[:len(newValue)-1] ) ) )
							fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
							fnPH.setNodeValue( self.parameter )	
							return	


class NumericParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ):
		ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__field = None
		self.__slider = None
		
		
		if self.presetsOnly():
			return		

		if parameter.hasMinValue() and parameter.hasMaxValue():
		
			self._layout = cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.sliderWidgetWidthIndex ]
			)
			
		else:
		
			self._layout = cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex ]
			)

		
		cmds.text(
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
			
		self.__field = self.fieldType()(				
			value = parameter.getNumericValue(),
			**kw			
		)
						
		if parameter.hasMinValue() and parameter.hasMaxValue():

			self.__slider = self.sliderType()(
				minValue = parameter.minValue,
				maxValue = parameter.maxValue,
				
				value = parameter.getNumericValue(),								
			)		
			
		cmds.setParent("..")
			
		self.replace( self.node(), self.parameter )
		
	def sliderType( self ):	
	
		if self.parameter.isInstanceOf( IECore.TypeId.FloatParameter ) or self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ):
			return cmds.floatSlider
		elif self.parameter.isInstanceOf( IECore.TypeId.IntParameter ):
			return cmds.intSlider
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")	
		
	def fieldType( self ):
	
		if self.parameter.isInstanceOf( IECore.TypeId.FloatParameter ) or self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ):
			return cmds.floatField
		elif self.parameter.isInstanceOf( IECore.TypeId.IntParameter ):
			return cmds.intField
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")
		

		
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
		
		if self.__field:
		
			cmds.connectControl( self.__field, self.plugName() )
			self._addPopupMenu( parentUI = self.__field, attributeName = self.plugName() )
					
			if self.__slider:
				cmds.connectControl( self.__slider, self.plugName() )
			
	

class VectorParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ):
		ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__fields = []		
		
		if self.presetsOnly():
			return	
			
		self.__dim = parameter.getTypedValue().dimensions()

		if self.__dim == 2:
			self._layout = cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.singleWidgetWidthIndex ]
			)
		elif self.__dim == 3:
			self._layout = cmds.rowLayout(
				numberOfColumns = 4,
				columnWidth4 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.singleWidgetWidthIndex ]
			)
		else:
			raise RuntimeError("Unsupported vector dimension in VectorParameterUI")

		cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description()
		)
		
		plug = self.plug()			
		for i in range(0, self.__dim) :
			self.__fields.append(
				self.fieldType()(	
					value = parameter.getTypedValue()[i]
				)
			)
			
		cmds.setParent("..")

		self.replace( self.node(), self.parameter )
		
	def fieldType( self ):
	
		if self.parameter.isInstanceOf( IECore.TypeId.V2iParameter ) or self.parameter.isInstanceOf( IECore.TypeId.V3iParameter ):
			return cmds.intField		
		else:
			return cmds.floatField		
		
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
		
		if len(self.__fields) and len(self.__fields) == self.__dim:
		
			plug = self.plug()		
			for i in range(0, self.__dim):	
				
				childPlugName = self.nodeName() + "." + plug.child(i).partialName()		
				cmds.connectControl( self.__fields[i], childPlugName )
				self._addPopupMenu( parentUI = self.__fields[i], attributeName = childPlugName )

class ColorParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ):
		ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__canvas = None
		
		if self.presetsOnly():
			return	
						
		self.__dim = parameter.getTypedValue().dimensions()
		
		self.__colorSlider = cmds.attrColorSliderGrp(
			label = self.label(),
			annotation = self.description(),
			attribute = self.plugName(),
			showButton = False 
		)
		
		self._layout = self.__colorSlider
		
		self.replace( self.node(), self.parameter )
					
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
	
		cmds.attrColorSliderGrp(
			self.__colorSlider,
			edit = True,
			attribute = self.plugName() 
		)
			
		self._addPopupMenu( parentUI = self.__colorSlider, attributeName = self.plugName() )		

class BoxParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__fields = []		
		
		if self.presetsOnly():
			return	
									
		self.__dim = parameter.getTypedValue().dimensions()		
		
		self._layout = cmds.columnLayout()
				
		plug = self.plug()				
		for childIndex in range( 0, 2 ) :
									
			if self.__dim == 2:
				cmds.rowLayout(
					numberOfColumns = 3,
					columnWidth3 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.singleWidgetWidthIndex ]
				)
			elif self.__dim == 3:
				cmds.rowLayout(
					numberOfColumns = 4,
					columnWidth4 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.singleWidgetWidthIndex ]
				)
			else:
				raise RuntimeError("Unsupported vector dimension in BoxParameterUI")
			
			parameterLabel = self.label()
			if childIndex==0 :
				parameterLabel = parameterLabel + "Min"
			else:
				parameterLabel = parameterLabel + "Max"
				
			cmds.text(
				label = parameterLabel,
				font = "smallPlainLabelFont",
				align = "right",
				annotation = self.description()
			)	
				
			vectorPlug = plug.child( childIndex )	 

			for i in range( 0, self.__dim ) :
			
				self.__fields.append( self.fieldType()() )
			
			cmds.setParent("..")
		
		cmds.setParent("..")
		
		self.replace( self.node(), self.parameter )
		
	def fieldType( self ):
	
		if self.parameter.isInstanceOf( IECore.TypeId.Box2iParameter ) or self.parameter.isInstanceOf( IECore.TypeId.Box3iParameter ):
			return cmds.intField		
		else:
			return cmds.floatField		
		
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
				
		if len(self.__fields) == self.__dim * 2:
		
			fieldNum = 0
			plug = self.plug()		
			for childIndex in range( 0, 2 ) :
			
				vectorPlug = plug.child( childIndex )									
				for i in range( 0, self.__dim ) :	
				
					vectorPlugChild = vectorPlug.child( i )
					vectorPlugChildName = self.nodeName() + "." + vectorPlugChild.partialName()
					cmds.connectControl( self.__fields[ fieldNum ], vectorPlugChildName )
					self._addPopupMenu( parentUI = self.__fields[fieldNum], attributeName = vectorPlugChildName )

					fieldNum += 1

ParameterUI.registerUI( IECore.TypeId.FloatParameter, NumericParameterUI )	
ParameterUI.registerUI( IECore.TypeId.DoubleParameter, NumericParameterUI )
ParameterUI.registerUI( IECore.TypeId.IntParameter, NumericParameterUI )
ParameterUI.registerUI( IECore.TypeId.BoolParameter, BoolParameterUI )
ParameterUI.registerUI( IECore.TypeId.CompoundParameter, CompoundParameterUI )

ParameterUI.registerUI( IECore.TypeId.StringParameter, StringParameterUI )
ParameterUI.registerUI( IECore.TypeId.ValidatedStringParameter, StringParameterUI )
ParameterUI.registerUI( IECore.TypeId.FrameListParameter, StringParameterUI )
ParameterUI.registerUI( IECore.TypeId.PathParameter, StringParameterUI )

ParameterUI.registerUI( IECore.TypeId.V2iParameter, VectorParameterUI )
ParameterUI.registerUI( IECore.TypeId.V3iParameter, VectorParameterUI )
ParameterUI.registerUI( IECore.TypeId.V2fParameter, VectorParameterUI )
ParameterUI.registerUI( IECore.TypeId.V2dParameter, VectorParameterUI )
ParameterUI.registerUI( IECore.TypeId.V3fParameter, VectorParameterUI )
ParameterUI.registerUI( IECore.TypeId.V3dParameter, VectorParameterUI )

ParameterUI.registerUI( IECore.TypeId.Box2iParameter, BoxParameterUI )
ParameterUI.registerUI( IECore.TypeId.Box2fParameter, BoxParameterUI )
ParameterUI.registerUI( IECore.TypeId.Box2dParameter, BoxParameterUI )
ParameterUI.registerUI( IECore.TypeId.Box3iParameter, BoxParameterUI )
ParameterUI.registerUI( IECore.TypeId.Box3fParameter, BoxParameterUI )
ParameterUI.registerUI( IECore.TypeId.Box3dParameter, BoxParameterUI )

ParameterUI.registerUI( IECore.TypeId.Color3fParameter, ColorParameterUI )

ParameterUI.registerUI( IECore.TypeId.FileSequenceParameter, FileSequenceParameterUI )
ParameterUI.registerUI( IECore.TypeId.DirNameParameter, DirNameParameterUI )
ParameterUI.registerUI( IECore.TypeId.FileNameParameter, FileNameParameterUI )

ParameterUI.registerUI( IECore.TypeId.StringParameter, CachePathPrefixParameterUI, 'cachePathPrefix' )

#\todo Store "collapsed" state of frameLayouts
