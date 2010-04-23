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

class ClassParameterUI( IECoreMaya.CompoundParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
			
		IECoreMaya.CompoundParameterUI.__init__( self, node, parameter, **kw )
	
		self.__menuParent = None
		self.__currentClassInfo = parameter.getClass( True )[1:]
	
	def _createHeader( self, columnLayout, **kw ) :
			
		IECoreMaya.CompoundParameterUI._createHeader( self, columnLayout, **kw )
				
		maya.cmds.rowLayout( numberOfColumns = 2, parent = columnLayout )
		
		maya.cmds.text( label = "Class", align = "right", annotation = self.description() )
		
		self.__menuParent = maya.cmds.iconTextStaticLabel(
			image = "arrowDown.xpm",
			font = "smallBoldLabelFont",
			label = self.__menuParentLabel(), 
			style = "iconAndTextHorizontal",
			height = 23,
		)
		
		# popup menu can be activated with either right or left buttons
		IECoreMaya.createMenu( self.__menuDefinition, self.__menuParent )
		IECoreMaya.createMenu( self.__menuDefinition, self.__menuParent, button = 1 )
	
	def replace( self, node, parameter ) :
		
		newClassInfo = parameter.getClass( True )[1:]
		if newClassInfo != self.__currentClassInfo :
			self._deleteChildParameterUIs()
		
		IECoreMaya.CompoundParameterUI.replace( self, node, parameter )

		if self.__menuParent :
			maya.cmds.iconTextStaticLabel( self.__menuParent, edit=True, label=self.__menuParentLabel() )

		self.__currentClassInfo = newClassInfo

	def __menuParentLabel( self ) :
	
		classInfo = self.parameter.getClass( True )
		if classInfo[1] :
			return "%s v%d" % ( classInfo[1], classInfo[2] )
		else :
			return "Choose..."

	def __menuDefinition( self ) :
	
		result = IECore.MenuDefinition()
		
		classNameFilter = "*"
		try :
			classNameFilter = self.parameter.userData()["UI"]["classNameFilter"].value
		except :
			pass

		classInfo = self.parameter.getClass( True )
		
		loader = IECore.ClassLoader.defaultLoader( classInfo[3] )
		for className in loader.classNames( classNameFilter ) :
			for classVersion in loader.versions( className ) :
				
				result.append(
					
					"/%s/v%d" % ( className, classVersion ), 
					
					IECore.MenuItemDefinition(
						command = IECore.curry( self.__setClass, className, classVersion, classInfo[3] ),
						active = className != classInfo[1] or classVersion != classInfo[2]
					)
					
				)
	
		return result
	
	def __setClass( self, className, classVersion, searchPathEnvVar ) :
	
		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
		
		fnPH.setClassParameterClass( self.parameter, className, classVersion, searchPathEnvVar )

	@staticmethod	
	def _classSetCallback( fnPH, parameter ) :
			
		for instance in IECoreMaya.UIElement.instances( ClassParameterUI ) :
			if instance.parameter.isSame( parameter ) :
				instance.replace( instance.node(), instance.parameter )

IECoreMaya.FnParameterisedHolder.addSetClassParameterClassCallback( ClassParameterUI._classSetCallback )
		
IECoreMaya.ParameterUI.registerUI( IECore.ClassParameter.staticTypeId(), ClassParameterUI )
