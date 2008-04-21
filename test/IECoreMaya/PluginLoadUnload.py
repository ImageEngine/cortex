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
import maya.OpenMaya as OpenMaya
import unittest, MayaUnitTest
import os.path
from IECore import *
from IECoreMaya import *

class TestPluginLoadUnload( unittest.TestCase ) :
		
	def test( self ):
		""" Test loading/unloading of plugin """
		
		# Plugin should be loaded by MayaUnitTest.TestProgram when we get here		
		self.assert_( cmds.pluginInfo( "ieCore", query = True, loaded = True ) )
		
		for i in range( 0, 20 ) :
		
			cmds.unloadPlugin( "ieCore" )
			self.failIf( cmds.pluginInfo( "ieCore", query = True, loaded = True ) )			
		
			cmds.loadPlugin( "ieCore" )
			self.assert_( cmds.pluginInfo( "ieCore", query = True, loaded = True ) )
		

		self.assert_( cmds.pluginInfo( "ieCore", query = True, loaded = True ) )
		
	def tearDown( self ):	
	
		if not cmds.pluginInfo( "ieCore", query = True, loaded = True ) :		
			cmds.loadPlugin( "ieCore" )
			
		# Make sure plugin is definitely loaded when we exit tests	
		assert( cmds.pluginInfo( "ieCore", query = True, loaded = True ) )

if __name__ == "__main__":
	MayaUnitTest.TestProgram( testRunner = unittest.TextTestRunner( stream = MayaUnitTest.SplitStream(), verbosity = 2 ) )
	 
