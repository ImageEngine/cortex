##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

import os
import maya.cmds
import maya.OpenMaya

import IECore
import IECoreMaya

class ObjectDataTest( IECoreMaya.TestCase ) :

	def setUp( self ) :

		IECoreMaya.TestCase.setUp( self )

		if not maya.cmds.pluginInfo( "ObjectDataTestNode.py", query=True, loaded=True ) :
			maya.cmds.loadPlugin( "ObjectDataTestNode.py" )

	def testReadWrite( self ) :

		node = maya.cmds.createNode( "ieObjectDataTestNode" )

		compoundData = IECore.CompoundData( {
			"val1" : IECore.FloatData( 1 ),
			"val2" : IECore.StringData( "val2Data" ),
			"val3" : {
				"val3.val1" : IECore.IntData( 100 ),
			},
		} )

		IECoreMaya.ToMayaPlugConverter.create( compoundData ).convert( node + ".objectData" )
		plugValue = IECoreMaya.FromMayaPlugConverter.create( node + ".objectData" ).convert()
		self.assertEqual( plugValue, compoundData )

		# try saving and loading an ascii file

		maya.cmds.file( rename = os.getcwd() + "/test/IECoreMaya/objectDataTest.ma" )
		sceneFileName = maya.cmds.file( force = True, type = "mayaAscii", save = True )

		maya.cmds.file( new=True, force=True )
		maya.cmds.file( sceneFileName, force=True, open=True )

		loadedCompoundData = IECoreMaya.FromMayaPlugConverter.create( node + ".objectData" ).convert()
		self.assertEqual( loadedCompoundData, compoundData )

		# try saving and loading a binary file

		maya.cmds.file( rename = os.getcwd() + "/test/IECoreMaya/objectDataTest.mb" )
		sceneFileName = maya.cmds.file( force = True, type = "mayaBinary", save = True )

		maya.cmds.file( new=True, force=True )
		maya.cmds.file( sceneFileName, force=True, open=True )

		loadedCompoundData = IECoreMaya.FromMayaPlugConverter.create( node + ".objectData" ).convert()
		self.assertEqual( loadedCompoundData, compoundData )

	def tearDown( self ) :

		maya.cmds.file( new = True, force = True )
		maya.cmds.flushUndo()
		maya.cmds.unloadPlugin( "ObjectDataTestNode.py" )

		for f in [
			"./test/IECoreMaya/objectDataTest.ma",
			"./test/IECoreMaya/objectDataTest.mb",
		] :

			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
