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

import maya.cmds

import IECore
import IECoreMaya

## The UI for the TransformationMatrixParameter supports the following
## userData()
##
##   - "visibleFields" IECore.StringVectorData, A list of fields to
##     display in the UI. Possible values are (D marks a default):
##         "translate" D
##         "rotate", D
##         "scale" D
##         "shear" D
##         "rotatePivot",
##         "rotatePivotTranslation",
##         "scalePivot"
##         "scalePivotTranslation"

class TransformationMatrixParameterUI( IECoreMaya.ParameterUI ) :

	_allFields = ( "translate", "rotate", "scale", "shear",	"scalePivot", "scalePivotTranslation", "rotatePivot", "rotatePivotTranslation" )

	def __init__( self, node, parameter, **kw ) :

		self._outerColumn = maya.cmds.columnLayout( adj=True )

		IECoreMaya.ParameterUI.__init__( self, node, parameter, self._outerColumn, **kw )

		maya.cmds.rowLayout( numberOfColumns=2, parent=self._outerColumn )

		self._label = maya.cmds.text(
			label = self.label(),
			font = "tinyBoldLabelFont",
			align = "right",
			annotation = self.description()
		)

		self._manip = maya.cmds.button( label="Manipulate" )

		maya.cmds.setParent("..")
		maya.cmds.setParent("..")

		self._fields = {}
		self.__kw = kw.copy()

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )
		currentParent = maya.cmds.setParent( query=True )

		visibleFields = IECore.StringVectorData( ( "translate", "rotate", "scale", "shear" ) )
		with IECore.IgnoredExceptions( KeyError ) :
			userDataFields = parameter.userData()["UI"]["visibleFields"]
			visibleFields = []
			for u in userDataFields :
				if u not in TransformationMatrixParameterUI._allFields:
					IECore.msg(
						IECore.Msg.Level.Warning,
						"TransformationMatrixParameterUI",
						"Invalid field '%s' requested in UI userData for '%s'. Available fields are %s."
						  % ( u, parameter.name, TransformationMatrixParameterUI._allFields )
					)
					continue
				visibleFields.append( u )

		for f in self._fields.keys() :
			if f not in visibleFields :
				maya.cmds.deleteUI( self._fields[f][0] )
				del self._fields[f]

		fnPH = IECoreMaya.FnParameterisedHolder( node )
		baseName = fnPH.parameterPlugPath( parameter )

		self._addPopupMenu( parentUI=self._label, attributeName=baseName )

		for f in visibleFields :

			if f not in self._fields :
				layout = maya.cmds.rowLayout(
					numberOfColumns = 4,
					parent = self._outerColumn,
					columnWidth4 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
				)
				maya.cmds.text( label=f, font="smallPlainLabelFont", align="right" )
				self._fields[f] = ( layout, maya.cmds.floatField(), maya.cmds.floatField(), maya.cmds.floatField() )

			maya.cmds.connectControl( self._fields[f][1], "%s%s%i" % ( baseName, f, 0 ) )
			maya.cmds.connectControl( self._fields[f][2], "%s%s%i" % ( baseName, f, 1 ) )
			maya.cmds.connectControl( self._fields[f][3], "%s%s%i" % ( baseName, f, 2 ) )

		maya.cmds.button(
			self._manip,
			edit = True,
			# The manip is currently only registered for float types
			visible = isinstance( parameter, IECore.TransformationMatrixfParameter ),
			command = self._createCallback( IECore.curry( IECoreMaya.ManipulatorUI.manipulateParameter, node, parameter ) )
		)

		maya.cmds.setParent( currentParent )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.TransformationMatrixfParameter, TransformationMatrixParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.TransformationMatrixdParameter, TransformationMatrixParameterUI )
