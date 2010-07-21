##########################################################################
#
#  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

import os

import maya.cmds

import IECore
import IECoreMaya

class ReferenceEditsTest( IECoreMaya.TestCase ) :

	# Maya will fail to restore parameter values in 
	# nested classes created on referenced objects,
	# it seems like it tries to restore the parameter 
	# values before the plugs have been created.
	def testClassParameterValueRestoration( self ) :

		# make a new scene, and import the reference scene
		maya.cmds.file( new=True, force=True )
		maya.cmds.file( 
			os.getcwd() + "/test/IECoreMaya/scenes/nestedClassReference.ma",
			reference = True,
		)	
	
		self.failUnless( maya.cmds.objExists( 'nestedClassReference_classHolderShape' ) )

		# add a class 
		fnP = IECoreMaya.FnParameterisedHolder( 'nestedClassReference_classHolderShape' )
		
		with fnP.classParameterModificationContext() :

			parameterised = fnP.getParameterised()[0]
			classParameter = parameterised.parameters()["class"]
			classParameter.setClass( "classHolderComponent", 1, "IECORE_PROCEDURAL_COMPONENT_PATHS" )

		testAttr = "nestedClassReference_classHolderShape.parm_class_value"
		
		self.failUnless( maya.cmds.objExists( testAttr ) )
		self.assertEqual( maya.cmds.getAttr( testAttr ), -1 )

		# change a parameter value
		maya.cmds.setAttr( testAttr, 2.0 )
		self.assertEqual( maya.cmds.getAttr( testAttr ), 2.0 )
		
		# save
		maya.cmds.file( rename = os.getcwd() + "/test/IECoreMaya/classParameterRestoration.ma" )
		tmpScene = maya.cmds.file( force = True, type = "mayaAscii", save = True )
		
		# reopen the scene, and check that the value change was preserved
		maya.cmds.file( tmpScene, force=True, open=True )
		
		self.failUnless( maya.cmds.objExists( testAttr ) )
		self.assertEqual( maya.cmds.getAttr( testAttr ), 2.0 ) # Expected failure, with value being -1.0, Maya fails to restore the reference edits as it tries to do so before the plugs have been created.
	
	def tearDown( self ) :
	
		path = os.getcwd() + "/test/IECoreMaya/classParameterRestoration.ma"

		if os.path.exists( path ) :

			os.remove( path )
		
if __name__ == "__main__":
	IECoreMaya.TestProgram()
