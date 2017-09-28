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
