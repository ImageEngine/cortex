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

import maya.cmds

import IECore
import IECoreMaya

class CompoundParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw  ) :

		IECoreMaya.ParameterUI.__init__( self, node, parameter, **kw )

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
			
		if 'hierarchyDepth' in kw :
			kw['hierarchyDepth'] += 1 
		else :
			kw['hierarchyDepth'] = 0	
		
		self.__childUIsLayout = None
		self.__childUIs = {}

		self._layout = None

		fnPH = IECoreMaya.FnParameterisedHolder( node )

		withCompoundFrame = False
		if 'withCompoundFrame' in kw :
			withCompoundFrame = kw['withCompoundFrame']

		if not withCompoundFrame and parameter.isSame( fnPH.getParameterised()[0].parameters() ) :
			self._layout = maya.cmds.columnLayout()
			self.__createChildUIs( **kw )
			maya.cmds.setParent("..")
		else:
			# \todo Retrieve the "collapsed" state
			collapsed = True
			
			font = "boldLabelFont"			
			labelIndent = 5 + ( 8 * max( 0, kw['hierarchyDepth']-1 ) )
										
			if kw['hierarchyDepth'] == 2 :
			
				font = "smallBoldLabelFont"
				
			elif kw['hierarchyDepth'] >= 3 :
			
				font = "tinyBoldLabelFont"
				
			self._layout = maya.cmds.frameLayout(
				label = self.label(),
				font = font,
				labelIndent = labelIndent,
				borderVisible = False,
				preExpandCommand = IECore.curry( self.__createChildUIs, **kw),
				collapseCommand = self.__collapse,
				collapsable = True,
				collapse = collapsed,
			)
			
			if not collapsed:
				self.__createChildUIs( **kw )

			maya.cmds.setParent("..")

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

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

			maya.cmds.setUITemplate(
				"attributeEditorTemplate",
				pushTemplate = True
			)

			self.__childUIsLayout = maya.cmds.columnLayout(
				parent = self._layout,
				width = 381
			)

			draggable = False
			try:
				draggable = self.parameter.userData()['UI']['draggable'].value
			except :
				pass

			if draggable :

				maya.cmds.rowLayout(
					numberOfColumns = 2,
					columnWidth2 = ( 361, 20 )

				)

				maya.cmds.text( label = "" )

				dragIcon = maya.cmds.iconTextStaticLabel(
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
					maya.cmds.setParent( self.__childUIsLayout )

					ui = IECoreMaya.ParameterUI.create( self.node(), p, **kw )

					if ui:
						self.__childUIs[pName] = ui

			maya.cmds.setParent("..")

			maya.cmds.setUITemplate(
				"attributeEditorTemplate",
				popTemplate = True
			)

		except :

			IECore.msg( IECore.Msg.Level.Error, "IECoreMaya.ParameterUI", traceback.format_exc() )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.CompoundParameter, CompoundParameterUI )
