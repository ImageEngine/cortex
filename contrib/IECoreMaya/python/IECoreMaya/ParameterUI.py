##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

import maya.cmds as cmds

from IECore import *
from IECoreMaya import *

""" Base class for objects which are able to create a Attribute Editor widget for a single Parameter """
class ParameterUI :

	textColumnWidthIndex = 145
	singleWidgetWidthIndex = 70
	sliderWidgetWidthIndex = 2 * 70
	
	handlers = {}

	"""Derived classes should create relevant widgets in the current layout, and leave the parent layout unchanged on exit"""
	def __init__( self, node, parameter ) :
	
		self.__node = node #IECoreMaya.Node
		self.parameter = parameter #IECore.Parameter
		self.__popupControl = None		
	
	""" Derived classes should override (and call) this method, and reconnect all created widgets to the new node/parameter """		
	def replace( self, node, parameter ) :
	
		self.__node = node		
		self.parameter = parameter
		
		if self.__popupControl:
		
			cmds.iconTextStaticLabel(			
				self.__popupControl,
				edit = True,

				label = self.parameter.getCurrentPresetName(),			
			)
	
	""" Returns the Maya node associated with this piece of UI """	
	def node( self ):
	
		return self.__node
	
	""" Returns the full "node.attribute" plug name for the current parameter """
	def plugName( self ) :	
		
		return self.node().parameterPlug( self.parameter ).fullPathName()
	
	""" Computes a nice label for the parameter """	
	def label( self ):

		return mel("interToUI(\"" + self.parameter.name + "\")").value
	
	""" Computes a wrapped annotation/tooltip for the parameter """		
	def description( self ):
	
		return StringUtil.wrap( self.parameter.description, 48 )
		
	""" Creates a drop-down selection list and returns True if the parameter is set to "presets only". Otherwise returns False"""
	def presetsOnly( self ):
	
		self.__popupControl = None
	
		if self.parameter.presetsOnly:
			cmds.rowLayout(
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
			
			self.addPopupMenu( attributeName = self.plugName() )
			
			cmds.setParent("..")
	
			return True	
			
		else:
		
			return False
			
	def addPopupMenu( self, **kw ):
	
		self.__popupMenu = cmds.popupMenu(
			postMenuCommand =  curry( self.buildPopupMenu, **kw )
		)
		
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
				
				command = curry( self.showEditor, attributeName = connections[1] )
			)

			cmds.menuItem(
				parent = popupMenu,
				label = "Break Connection",
				
				command = curry( self.disconnect, source = connections[1], destination = connections[0] )
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
		
			for k in self.parameter.presets().keys():
				cmds.menuItem(
					parent = popupMenu,
					label = k,
					command = curry( self.selectValue, selection = k )
				)

			if len( self.parameter.presets().keys() ) > 0:
				cmds.menuItem(
					parent = popupMenu,
					divider = True
				)	
		
		
			cmds.menuItem(
					parent = popupMenu,
					label = "Default",
					command = curry( self.selectValue, selection = self.parameter.defaultValue )
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
						command = curry(self.setKey, **kw)
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
							
							command = curry( self.expressionEditor, **kw )
						)

				else:
					
					cmds.menuItem(
						parent = popupMenu,
						label = "Edit Expression...",
						
						command = curry( self.expressionEditor, **kw )
					)

	
					cmds.menuItem(
						parent = popupMenu,
						label = "Delete Expression",
						
						command = curry( self.deleteNode, nodeName = expressions[0] )
					)
					
			else:
			
				hasConnections = self.__buildConnectionsPopupMenu( popupMenu, ownerControl, **kw )
					
		
			cmds.menuItem(
					parent = popupMenu,
					label = "Lock attribute",
					command = curry(self.lock, **kw)					
			)
			
		else:
		
			cmds.menuItem(
					parent = popupMenu,
					label = "Unlock attribute",
					command = curry(self.unlock, **kw)
			)
			
			
		#if hasConnections:
		
		#	if hasattr( cmds, controlType ):
		#		controlCmd = getattr( cmds, controlType )
			
		#		controlCmd(
		#			ownerControl,
		#			edit = True,
		#			editable = False
		#		)
			
	
	def showEditor( self, args, attributeName = None ):
	
		split = attributeName.split('.', 1 )
		node = split[0]
		
		melCmd = 'showEditor "' + node + '"'
		
		mel( melCmd.encode('ascii') )
			
	def deleteNode( self, args, nodeName = None ):	
	
		cmds.delete( nodeName )	
			
	def expressionEditor( self, args, attributeName = None):
	
		split = attributeName.split('.', 1 )
		node = split[0]
		attr = split[1]
		
		melCmd = 'expressionEditor EE "' + node + '" "' + attr + '"'
		
		mel( melCmd.encode('ascii') )
		
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
		self.__node.setNodeValue( self.parameter )
		
		if self.__popupControl:
			cmds.iconTextStaticLabel(
				self.__popupControl,
				edit = True,
				label = self.parameter.getCurrentPresetName()
			)
	

	@staticmethod
	def registerUI( parameterTypeId, handlerType, uiTypeHint = None ):
	
		# \todo Make sure that handler supports new and replace methods
		# \todo Warn if handler for type already exists
		# \todo Warn if handler is not default-constructible	
		
		ParameterUI.handlers[ (parameterTypeId, uiTypeHint) ] = handlerType
				
	@staticmethod		
	def create( node, parameter ) :
		# \todo Make sure parameter is RTT instanceOf("Parameter")

		uiTypeHint = None
		try:
			uiTypeHint = parameter.userData()['UI']['typeHint'].value
		except:
			pass
	
		if not ( parameter.typeId(), uiTypeHint ) in ParameterUI.handlers:
			# \todo Issue a warning
			return None
		
		

		handlerType = ParameterUI.handlers[ (parameter.typeId(), uiTypeHint) ]
		
		parameterUI = handlerType( node, parameter)
		
		return parameterUI

	
		
		
class CompoundParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ) :
		ParameterUI.__init__( self, node, parameter )
		
		self.__childUIsLayout = None		
		self.__childUIs = {}
		
		self.__layoutName = None
		
		if parameter.isSame(node.getParameterised()[0].parameters()):
			self.__layoutName = cmds.columnLayout()
			self.__createChildUIs()
			cmds.setParent("..")
		else:
			# \todo Retrieve the "collapsed" state
			collapsed = True
		
			self.__layoutName = cmds.frameLayout( 
				label = self.label(),
				borderVisible = False,
				preExpandCommand = self.__createChildUIs,
				collapseCommand = self.__collapse,
				collapsable = True,
				collapse = collapsed
			)
				
			if not collapsed:
				self.__createChildUIs()
			
			cmds.setParent("..")
		
			
		
	def replace( self, node, parameter ) :
	
		ParameterUI.replace( self, node, parameter )
		
		for pName in self.__childUIs.keys():
			ui = self.__childUIs[pName]
			p = self.parameter[pName]
			
			ui.replace( node, p )
	
	def __collapse(self):
		# \todo Store collapsed state of self.__layoutName
		pass

	def __createChildUIs(self):
	
		if self.__childUIsLayout:
			return
		
		# \todo Store collapsed state of self.__layoutName
			
		cmds.setUITemplate(
			"attributeEditorTemplate",
			pushTemplate = True
		)

		self.__childUIsLayout = cmds.columnLayout( parent = self.__layoutName )
		
		for pName in self.parameter.keys():
			p = self.parameter[pName]
			
			ui = ParameterUI.create( self.node(), p )
			
			if ui:			
				self.__childUIs[pName] = ui
		
		cmds.setParent("..")
		
		cmds.setUITemplate(
			"attributeEditorTemplate",
			popTemplate = True
		)		



class BoolParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ):
		ParameterUI.__init__( self, node, parameter )
		
		cmds.rowLayout(
			numberOfColumns = 2,
			columnWidth2 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex * 3 ]
		)
		
		cmds.text(
			label = "",
		)
		
		self.__checkBox = cmds.checkBox(
			annotation = self.description(),			
			label = self.label(),
			align = "left"
		)
		
		cmds.connectControl( self.__checkBox, self.plugName() )
		
		cmds.setParent("..")
		
	def replace( self, node, parameter ) :		
	
		ParameterUI.replace( self, node, parameter )
			
		cmds.connectControl( self.__checkBox, self.plugName() )



class StringParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ):
		ParameterUI.__init__( self, node, parameter )
		
		self.__textField = None
		if self.presetsOnly():
			return	
		
		cmds.rowLayout(
			numberOfColumns = 2,			
		)
		
		cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),				
		)
		
		self.__textField = cmds.textField(
		)
		
		self.addPopupMenu( attributeName = self.plugName() )
		
		cmds.connectControl( self.__textField, self.plugName() )
		
		cmds.setParent("..")
		
	def replace( self, node, parameter ) :	
	
		ParameterUI.replace( self, node, parameter )	
		
		if self.__textField:	
			cmds.connectControl( self.__textField, self.plugName() )


class PathParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ):
		ParameterUI.__init__( self, node, parameter )
		
		self.__textField = None
		if self.presetsOnly():
			return	
		
		cmds.rowLayout(
			numberOfColumns = 3,
			columnWidth3 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex * 3, 26 ]			
		)
		
		cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),				
		)
		
		self.__textField = cmds.textField(
		)
		
		self.addPopupMenu( attributeName = self.plugName() )
		
		cmds.iconTextButton(
			label = "",
			image = "fileOpen.xpm",
			command = self.openDialog,
			height = 23,
			style = "iconOnly"
		)
						
		cmds.connectControl( self.__textField, self.plugName() )
		
		cmds.setParent("..")
		
	def replace( self, node, parameter ) :	
	
		ParameterUI.replace( self, node, parameter )	
		
		if self.__textField:	
			cmds.connectControl( self.__textField, self.plugName() )
			
	def openDialog( self ) :
	
		pass


class FileNameParameterUI( PathParameterUI ) :	

	def __init__( self, node, parameter ):
	
		PathParameterUI.__init__( self, node, parameter )				
			
	def openDialog( self ) :
	
		selection = cmds.fileDialog().encode('ascii')
			
		if len(selection):
		
			self.parameter.setValue( StringData( selection ) )
			self.node().setNodeValue( self.parameter )		


class DirNameParameterUI( PathParameterUI ) :	

	def __init__( self, node, parameter ):
	
		PathParameterUI.__init__( self, node, parameter )				
			
	def openDialog( self ) :
	
		selection = cmds.fileDialog().encode('ascii')
			
		if len(selection):
			d = os.path.dirname(selection)
			self.parameter.setValue( StringData( d ) )
			self.node().setNodeValue( self.parameter )	


class FileSequenceParameterUI( PathParameterUI ) :					

	def __init__( self, node, parameter ):
	
		PathParameterUI.__init__( self, node, parameter )
			
	def openDialog( self ) :
	
		selection = cmds.fileDialog().encode('ascii')
			
		if len(selection):
			d = os.path.dirname(selection)
			sequences = ls(d)
			
			if sequences:
			
				for seq in sequences:
				
					if os.path.basename(selection) in seq.fileNames():
					
						self.parameter.setValue( StringData( os.path.join( d, str( seq ) ) ) )
						self.node().setNodeValue( self.parameter )	
						
						return	
		
# Specialized interface for nodes like kiwiRenderer that get the full path plus the file name prefix of a FIO file sequence.
# \todo Get rid of this once kiwiRenderer and the cache nodes uses FileSequenceParameter instead.
class CachePathPrefixParameterUI( PathParameterUI ) :

	def __init__( self, node, parameter ):
	
		PathParameterUI.__init__( self, node, parameter )

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
							self.parameter.setValue( StringData( os.path.join( d, newValue[:len(newValue)-1] ) ) )
							self.node().setNodeValue( self.parameter )
							return	


class NumericParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ):
		ParameterUI.__init__( self, node, parameter )
		
		self.__field = None
		self.__slider = None
		
		
		if self.presetsOnly():
			return		

		if parameter.hasMinValue() and parameter.hasMaxValue():
		
			cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ ParameterUI.textColumnWidthIndex, ParameterUI.singleWidgetWidthIndex, ParameterUI.sliderWidgetWidthIndex ]
			)
			
		else:
		
			cmds.rowLayout(
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
			
		self.__field = self.fieldType()(				
			value = parameter.getNumericValue(),
			**kw			
		)
		
		self.addPopupMenu( attributeName = self.plugName() )
		
		cmds.connectControl( self.__field, self.plugName() )
						
		if parameter.hasMinValue() and parameter.hasMaxValue():

			self.__slider = self.sliderType()(
				minValue = parameter.minValue,
				maxValue = parameter.maxValue,
				
				value = parameter.getNumericValue(),								
			)
			
			cmds.connectControl(self.__slider, self.plugName() )
		
			
		cmds.setParent("..")
			
		
	def sliderType( self ):	
	
		if self.parameter.isInstanceOf( TypeId.FloatParameter ):
			return cmds.floatSlider
		elif self.parameter.isInstanceOf( TypeId.IntParameter ):
			return cmds.intSlider
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")	
		
	def fieldType( self ):
	
		if self.parameter.isInstanceOf( TypeId.FloatParameter ):
			return cmds.floatField
		elif self.parameter.isInstanceOf( TypeId.IntParameter ):
			return cmds.intField
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")
		

		
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
		
		if self.__field:
			cmds.connectControl( self.__field, self.plugName() )
		
			if self.__slider:
				cmds.connectControl( self.__slider, self.plugName() )
			
	

class VectorParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ):
		ParameterUI.__init__( self, node, parameter )
		
		self.__fields = []		
		
		if self.presetsOnly():
			return	
			
		self.__dim = parameter.getTypedValue().dimensions()

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
			raise RuntimeError("Unsupported vector dimension in VectorParameterUI")

		cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description()
		)
		
		plug = Plug( self.plugName() )
		
		plugChildren = plug.childNames()
			
		for i in range(0, self.__dim):	
			self.__fields.append(
				self.fieldType()(	
					value = parameter.getTypedValue()[i]
				)
			)
			
			self.addPopupMenu( attributeName = plug.child(plugChildren[i]).fullPathName() )

			cmds.connectControl( self.__fields[i], plug.child(plugChildren[i]).fullPathName() )
			
		cmds.setParent("..")
		
	def fieldType( self ):
	
		if self.parameter.isInstanceOf( TypeId.V2iParameter ) or self.parameter.isInstanceOf( TypeId.V3iParameter ):
			return cmds.intField		
		else:
			return cmds.floatField		
		
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
		
		if len(self.__fields) == self.__dim:
		
			plug = Plug( self.plugName() )
			plugChildren = plug.childNames()
		
			for i in range(0, self.__dim):	
			
				cmds.connectControl( self.__fields[i], plug.child(plugChildren[i]).fullPathName() )
				
class ColorParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ):
		ParameterUI.__init__( self, node, parameter )
		
		self.__canvas = None
		
		if self.presetsOnly():
			return	
			
		# \todo Add a "colorIndexSlider" instead, if we have a uiHint telling us to?		
			
		self.__dim = parameter.getTypedValue().dimensions()
		
		self.__colorSlider = cmds.attrColorSliderGrp(
			label = self.label(),
			annotation = self.description(),
			attribute = self.plugName() 
		)
		
		self.addPopupMenu( attributeName = self.plugName() )		
					
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
	
		cmds.attrColorSliderGrp(
			self.__colorSlider,
			edit = True,
			attribute = self.plugName() 
		)
			

class BoxParameterUI( ParameterUI ) :

	def __init__( self, node, parameter ):
		ParameterUI.__init__( self, node, parameter )
		
		self.__fields = []		
		
		if self.presetsOnly():
			return	
									
		self.__dim = parameter.getTypedValue().dimensions()		
		
		cmds.columnLayout()
		
		numFields = 0
		
		plug = Plug( self.plugName() )
		plugChildren = plug.childNames()
		
		for plugChild in plugChildren:
						
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
			if plugChild == plugChildren[0]:
				parameterLabel = parameterLabel + "Min"
			else:
				parameterLabel = parameterLabel + "Max"
				
			cmds.text(
				label = parameterLabel,
				font = "smallPlainLabelFont",
				align = "right",
				annotation = self.description()
			)	
				
			vectorPlug = plug.child( plugChild )		
			vectorPlugChildren = vectorPlug.childNames()

			for i in range(0, self.__dim):
				self.__fields.append(
					self.fieldType()(	
					)
				)
				numFields = numFields + 1
				
				vectorPlugChild = vectorPlug.child(vectorPlugChildren[i])				
				self.addPopupMenu( attributeName = vectorPlugChild.fullPathName() )
				cmds.connectControl( self.__fields[ numFields - 1 ], vectorPlugChild.fullPathName() )
			
			cmds.setParent("..")
		
		cmds.setParent("..")
		
	def fieldType( self ):
	
		if self.parameter.isInstanceOf( TypeId.Box2iParameter ) or self.parameter.isInstanceOf( TypeId.Box3iParameter ):
			return cmds.intField		
		else:
			return cmds.floatField		
		
	def replace( self, node, parameter ) :
		
		ParameterUI.replace( self, node, parameter )
		
		if len(self.__fields) == self.__dim * 2:
		
			plug = Plug( self.plugName() )
			plugChildren = plug.childNames()
			
			fieldNum = 0
		
			for plugChild in plugChildren:
			
				vectorPlug = plug.child( plugChild )		
				vectorPlugChildren = vectorPlug.childNames()
							
				for i in range(0, self.__dim):	
				
					vectorPlugChild = vectorPlug.child(vectorPlugChildren[i])	
				
					cmds.connectControl( self.__fields[ fieldNum ], vectorPlugChild.fullPathName() )
					fieldNum += 1



ParameterUI.registerUI( TypeId.FloatParameter, NumericParameterUI )	
ParameterUI.registerUI( TypeId.IntParameter, NumericParameterUI )
ParameterUI.registerUI( TypeId.BoolParameter, BoolParameterUI )
ParameterUI.registerUI( TypeId.CompoundParameter, CompoundParameterUI )

ParameterUI.registerUI( TypeId.StringParameter, StringParameterUI )
ParameterUI.registerUI( TypeId.FrameListParameter, StringParameterUI )
ParameterUI.registerUI( TypeId.PathParameter, StringParameterUI )

ParameterUI.registerUI( TypeId.V2iParameter, VectorParameterUI )
ParameterUI.registerUI( TypeId.V3iParameter, VectorParameterUI )
ParameterUI.registerUI( TypeId.V2fParameter, VectorParameterUI )
ParameterUI.registerUI( TypeId.V2dParameter, VectorParameterUI )
ParameterUI.registerUI( TypeId.V3fParameter, VectorParameterUI )
ParameterUI.registerUI( TypeId.V3dParameter, VectorParameterUI )

ParameterUI.registerUI( TypeId.Box2iParameter, BoxParameterUI )
ParameterUI.registerUI( TypeId.Box2fParameter, BoxParameterUI )
ParameterUI.registerUI( TypeId.Box2dParameter, BoxParameterUI )
ParameterUI.registerUI( TypeId.Box3iParameter, BoxParameterUI )
ParameterUI.registerUI( TypeId.Box3fParameter, BoxParameterUI )
ParameterUI.registerUI( TypeId.Box3dParameter, BoxParameterUI )

ParameterUI.registerUI( TypeId.Color3fParameter, ColorParameterUI )

ParameterUI.registerUI( TypeId.FileSequenceParameter, FileSequenceParameterUI )
ParameterUI.registerUI( TypeId.DirNameParameter, DirNameParameterUI )
ParameterUI.registerUI( TypeId.FileNameParameter, FileNameParameterUI )

ParameterUI.registerUI( TypeId.StringParameter, CachePathPrefixParameterUI, 'cachePathPrefix' )

#\todo Store "collapsed" state of frameLayouts
