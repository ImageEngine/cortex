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

import unittest
import IECore
import os

class PresetManagerTest( unittest.TestCase ) :

	def tearDown( self ) :
		os.system( "rm -rf test/IECore/presets" )

	def test( self ) :

		presetPath = "test/IECore/presets"

		if os.path.exists( presetPath ):
			os.system( "rm -rf %s" % presetPath )

		mgr = IECore.PresetManager( IECore.ClassLoader( IECore.SearchPath( presetPath, ":" ) ) )
		op = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) ).load( "parameterTypes" )()

		# make sure the test op did not change
		self.assertEqual( op['a'].getValue().value, 1 )
		self.assertEqual( op['b'].getValue().value, 2.0 )
		self.assertEqual( op['compound']['j'].getValue().value, IECore.V3d( 8, 16, 32 ) )
		self.assertEqual( op['compound']['k'].getValue(), IECore.M44fData() )
		originalValues = op.parameters().getValue()

		# saves only parameter 'a' and 'compound'
		mgr.savePreset( op, [ op['a'], op['compound'] ], presetPath, 'testA' )
		# saves only parameter 'compound.j'
		mgr.savePreset( op, [ op['compound']['j'] ], presetPath, 'testB' )
		# saves all the parameters in the op
		mgr.savePreset( op, [ op.parameters() ], presetPath, 'testC' )

		# check if the three presets are listed.
		self.assertEqual( mgr.presets( op ), [ 'parameterTypes/testA', 'parameterTypes/testB', 'parameterTypes/testC' ] )

		op['a'] = 4
		op['b'] = 6
		op['compound']['j'] = IECore.V3dData( IECore.V3d(4) )
		op['compound']['k'] = IECore.M44fData( IECore.M44f(4) )
		# should recover a, j, k not b
		mgr.loadPreset( op, 'testA' )
		self.assertEqual( op['a'].getValue().value, 1 )
		self.assertEqual( op['b'].getValue().value, 6 )
		self.assertEqual( op['compound']['j'].getValue().value, IECore.V3d( 8, 16, 32 ) )
		self.assertEqual( op['compound']['k'].getValue(), IECore.M44fData() )

		op['b'] = 6
		op['compound']['j'] = IECore.V3dData( IECore.V3d(4) )
		op['compound']['k'] = IECore.M44fData( IECore.M44f(4) )
		# should recover j only
		mgr.loadPreset( op, 'testB' )
		self.assertEqual( op['b'].getValue().value, 6 )
		self.assertEqual( op['compound']['j'].getValue().value, IECore.V3d( 8, 16, 32 ) )
		self.assertEqual( op['compound']['k'].getValue(), IECore.M44fData(IECore.M44f(4)) )

		op['a'] = 4
		op['b'] = 6
		op['compound']['j'] = IECore.V3dData( IECore.V3d(4) )
		op['compound']['k'] = IECore.M44fData( IECore.M44f(4) )
		# should recover everything
		mgr.loadPreset( op, 'testC' )
		self.assertEqual( op.parameters().getValue(), originalValues )
	
		# remove a parameter from the Op and do it again. It should not crash, only generate warning msg.
		op.parameters().removeParameter( op.parameters()['compound'] )
		mgr.loadPreset( op, 'testA' )
		mgr.loadPreset( op, 'testB' )
		mgr.loadPreset( op, 'testC' )
		

if __name__ == "__main__":
        unittest.main()
