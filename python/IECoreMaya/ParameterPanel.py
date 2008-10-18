
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

## \todo Extract out the generic functionality of forwarding the mel calls into instance methods into
# a standalone Panel class, and use this as a base class for the ParameterPanel class. Then we can reuse the
# forwarding code when implementing other panel types.
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
