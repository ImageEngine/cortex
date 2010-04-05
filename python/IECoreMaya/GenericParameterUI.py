import os

import maya.cmds

import IECoreMaya.ParameterUI

class GenericParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		IECoreMaya.ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__layout1 = maya.cmds.rowLayout(
			numberOfColumns = 2,
			columnWidth2 = [ self.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex * 3 + 25 + 25 ]
		)
		
		maya.cmds.text( label = self.label(), font="smallPlainLabelFont", align="right", annotation=self.description() )
		
		self.__connectionsLayout = maya.cmds.columnLayout()
		maya.cmds.setParent( ".." )

		maya.cmds.setParent( ".." )
		
		self.replace( self.node(), self.parameter )

		
	def replace( self, node, parameter ) :
	
		currentParent = maya.cmds.setParent( query=True )
		
		IECoreMaya.ParameterUI.replace( self, node, parameter )
		
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		plugPath = fnPH.parameterPlugPath( parameter )
		
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
			
			maya.cmds.iconTextButton(
				annotation = "Clicking this takes you the connection editor for this connection.",
				style = "iconOnly",
				image = "listView.xpm",
				font = "boldLabelFont",
				command = IECore.curry( self.connectionEditor, None, leftHandNode = None ),
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
	
		maya.cmds.iconTextButton(
			annotation = "Clicking this takes you the connection editor for this connection.",
			style = "iconOnly",
			image = "listView.xpm",
			font = "boldLabelFont",
			command = IECore.curry( self.connectionEditor, None, leftHandNode = plugName ),
			height = 20,
			width = 20
		)
			
		maya.cmds.iconTextButton(
			annotation = "Clicking this will take you to the node sourcing this connection.",
			style = "iconOnly",
			image = "navButtonConnected.xpm",
			command = IECore.curry( self.showEditor, None, plugName ),
			height = 20,
		)
		
		maya.cmds.setParent( ".." )
				
import IECore					   
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Parameter, GenericParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Parameter, GenericParameterUI, 'generic' )
