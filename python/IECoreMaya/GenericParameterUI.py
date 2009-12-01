import os

import maya.cmds

import IECoreMaya.ParameterUI

class GenericParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		IECoreMaya.ParameterUI.__init__( self, node, parameter, **kw )
		
		self.__layout1 = maya.cmds.rowLayout(
			numberOfColumns = 2,
			columnWidth2 = [ self.textColumnWidthIndex, 250 ]
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
				columnWidth2 = [ 20, 160 ]
			)
			
			maya.cmds.iconTextButton(
				annotation = "Clicking this takes you the connection editor for this connection.",
				style = "textOnly",
				label = "...",
				font = "boldLabelFont",
				command = IECore.curry( self.__showConnectionEditor, None ),
				height = 20,
				width = 15
			)
			
			text = maya.cmds.text( align="left", label="Not connected", font="tinyBoldLabelFont" )
			self._addPopupMenu( parentUI=text, attributeName = self.plugName() )
			
			maya.cmds.setParent( ".." )
			
		else :
		
			for i in range( numConnections ) :
			
				self.__drawConnection( connections[i] )
		
		
		maya.cmds.setParent( currentParent )


	def __openConnectionEditor( self ) :
		return
		
		
	def __drawConnection( self, plugName ) :
	
		maya.cmds.rowLayout(
			numberOfColumns = 3,
			columnWidth3 = [ 20, 25, 180 ]
		)

		maya.cmds.iconTextButton(
			annotation = "Clicking this takes you the connection editor for this connection.",
			style = "textOnly",
			label = "...",
			font = "boldLabelFont",
			command = IECore.curry( self.__showConnectionEditor, plugName ),
			height = 20,
			width = 15
		)
		
		maya.cmds.iconTextButton(
			annotation = "Clicking this will dissconnect this connection.",
			style = "textOnly",
			label = "x",
			command = IECore.curry( self.__breakConnection, plugName ),
			height = 20,
		)
		
		name = maya.cmds.text( l=plugName, font="tinyBoldLabelFont", align="left", width=180, height = 20 )
		self._addPopupMenu( parentUI=name, attributeName = self.plugName() )
		
		maya.cmds.setParent( ".." )


	def __showAE( self, plugName ) :
	
		import maya.mel
		maya.mel.eval( "showEditor \"" + plugName + "\"" ) 

	def __showConnectionEditor( self, plugName ) :
	
		import maya.mel
		maya.mel.eval(
				str("ConnectionEditor;"+
				"nodeOutliner -e -replace %(right)s connectWindow|tl|cwForm|connectWindowPane|rightSideCW;"+
				"connectWindowSetRightLabel %(right)s;") % { 'right' : self.nodeName() } )
		
		if plugName :
	
			maya.mel.eval(
				str("nodeOutliner -e -replace %(left)s connectWindow|tl|cwForm|connectWindowPane|leftSideCW;"+
				"connectWindowSetLeftLabel %(left)s;" ) % { 'left' : plugName.split(".")[0] } )


	def __breakConnection( self, plugName ) :
		
		import maya.mel
		maya.cmds.disconnectAttr( plugName, self.plugName() )
		maya.mel.eval( 'evalDeferred( "updateAE %s;")' % ( self.nodeName() ) )
		
		
		
import IECore					   
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Parameter, GenericParameterUI )
