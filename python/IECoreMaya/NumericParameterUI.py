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

class NumericParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		if parameter.hasMinValue() and parameter.hasMaxValue():

			layout = maya.cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex, IECoreMaya.ParameterUI.sliderWidgetWidthIndex ]
			)

		else:

			layout = maya.cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex ]
			)

		IECoreMaya.ParameterUI.__init__( self, node, parameter, layout, **kw )

		self.__field = None
		self.__slider = None

		maya.cmds.text(
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


		# We set the step size on int parameters to 1, because otherwise Maya will throw warnings if the difference between
		# the min and max is less than the default step of 1.
		# However Maya also complains when you set the step size on parameters without a min or max, so we need to check that
		# as well.
		if ( parameter.hasMinValue() or parameter.hasMaxValue() ) and self.parameter.isInstanceOf( IECore.TypeId.IntParameter ):
			kw['step'] = 1


		# \todo Add a way of overriding precision for both float and double parameters, giving
		# each a sensible (and probably different) default
		if self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ) :

			kw['precision'] = 12

		if parameter.userData().has_key( 'UI' ) :

			if self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ) or self.parameter.isInstanceOf( IECore.TypeId.FloatParameter ):
				precision = parameter.userData()['UI'].get( "precision", None )

				if isinstance( precision, IECore.IntData ):
					kw['precision'] = precision.value

		self.__field = self.__fieldType()(
			value = parameter.getNumericValue(),
			**kw
		)

		if parameter.hasMinValue() and parameter.hasMaxValue():

			self.__slider = self.__sliderType()(
				minValue = parameter.minValue,
				maxValue = parameter.maxValue,

				value = parameter.getNumericValue(),
			)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		maya.cmds.connectControl( self.__field, self.plugName() )
		self._addPopupMenu( parentUI = self.__field, attributeName = self.plugName() )

		if self.__slider:
			maya.cmds.connectControl( self.__slider, self.plugName() )

	def __sliderType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.FloatParameter ) or self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ):
			return maya.cmds.floatSlider
		elif self.parameter.isInstanceOf( IECore.TypeId.IntParameter ):
			return maya.cmds.intSlider
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")

	def __fieldType( self ):

		if self.parameter.isInstanceOf( IECore.TypeId.FloatParameter ) or self.parameter.isInstanceOf( IECore.TypeId.DoubleParameter ):
			return maya.cmds.floatField
		elif self.parameter.isInstanceOf( IECore.TypeId.IntParameter ):
			return maya.cmds.intField
		else:
			raise RuntimeError("Invalid parameter type for NumericParameterUI")


IECoreMaya.ParameterUI.registerUI( IECore.TypeId.FloatParameter, NumericParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.DoubleParameter, NumericParameterUI )
IECoreMaya.ParameterUI.registerUI( IECore.TypeId.IntParameter, NumericParameterUI )
