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

from __future__ import with_statement

import traceback

import maya.cmds

import IECore
import IECoreMaya

class CompoundParameterUI( IECoreMaya.ParameterUI ) :

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
	def __init__( self, node, parameter, **kw  ) :

		IECoreMaya.ParameterUI.__init__( self, node, parameter, **kw )
			
		if 'hierarchyDepth' in kw :
			kw['hierarchyDepth'] += 1 
		else :
			kw['hierarchyDepth'] = 0	
		
		self.__childUIs = {}
		self.__headerCreated = False
		self.__kw = kw.copy()
		
		fnPH = IECoreMaya.FnParameterisedHolder( node )

		collapsable = self.__kw.get( "withCompoundFrame", False ) or not parameter.isSame( fnPH.getParameterised()[0].parameters() )
		
		# \todo Retrieve the "collapsed" state
		collapsed = collapsable

		font = "boldLabelFont"			
		if self.__kw['hierarchyDepth'] == 2 :
			font = "smallBoldLabelFont"
		elif self.__kw['hierarchyDepth'] >= 3 :
			font = "tinyBoldLabelFont"

		labelIndent = 5 + ( 8 * max( 0, self.__kw['hierarchyDepth']-1 ) )
		
		self._layout = maya.cmds.frameLayout(
			label = self.label(),
			font = font,
			labelIndent = labelIndent,
			labelVisible = collapsable,
			borderVisible = False,
			preExpandCommand = self.__preExpand,
			collapseCommand = self.__collapse,
			collapsable = collapsable,
			collapse = collapsed,
		)
		
		self.__columnLayout = maya.cmds.columnLayout(
			parent = self._layout,
			width = 381
		)

		if not collapsed :
			self.__preExpand()
			
		maya.cmds.setParent("..")
		maya.cmds.setParent("..")

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		if len( self.__childUIs ) :
		
			for pName in self.__childUIs.keys() :

				ui = self.__childUIs[pName]
				p = self.parameter[pName]

				ui.replace( node, p )
				
		else :
		
			with IECoreMaya.UITemplate( "attributeEditorTemplate" ) :
				self.__createChildUIs()

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

	def __collapse(self):
		# \todo Store collapsed state of self._layout
		pass

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
