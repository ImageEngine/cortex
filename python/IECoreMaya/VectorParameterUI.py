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

class VectorParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		self.__dim = parameter.getTypedValue().dimensions()
		if self.__dim == 2:
			layout = maya.cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
			)
		elif self.__dim == 3:
			layout = maya.cmds.rowLayout(
				numberOfColumns = 4,
				columnWidth4 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
			)
		else:
			raise RuntimeError("Unsupported vector dimension in VectorParameterUI")

		IECoreMaya.ParameterUI.__init__( self, node, parameter, layout, **kw )

		self.__fields = []

		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description()
		)

		plug = self.plug()
		for i in range(0, self.__dim) :
			self.__fields.append(
				self.__fieldType()(
					value = parameter.getTypedValue()[i]
				)
			)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		plug = self.plug()
		for i in range(0, self.__dim):

			childPlugName = self.nodeName() + "." + plug.child(i).partialName()
			maya.cmds.connectControl( self.__fields[i], childPlugName )
			self._addPopupMenu( parentUI = self.__fields[i], attributeName = childPlugName )

	def __fieldType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.V2iParameter ) or self.parameter.isInstanceOf( IECore.TypeId.V3iParameter ):
			return maya.cmds.intField
		else:
			return maya.cmds.floatField

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V2iParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V3iParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V2fParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V2dParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V3fParameter, VectorParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.V3dParameter, VectorParameterUI )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.Color3fParameter, VectorParameterUI, "numeric" )
