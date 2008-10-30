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

import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.cmds
import maya.OpenMaya
import weakref

class CallbackIdTest( unittest.TestCase ) :

	def test( self ) :
	
		# callback function just counts the number of times it has been invoked
		CallbackIdTest.numCalls = 0
		def callback( node, name, userData ) :
			
			CallbackIdTest.numCalls += 1
		
		# make a sphere and get the MObject from it
		sphere = maya.cmds.sphere()[0]
		s = maya.OpenMaya.MSelectionList()
		s.add( sphere )
		o = maya.OpenMaya.MObject()
		s.getDependNode( 0, o )
		
		# attach a name changed callback
		c = IECoreMaya.CallbackId( maya.OpenMaya.MNodeMessage.addNameChangedCallback( o, callback ) )
		
		# check that the callback is invoked
		self.assertEqual( CallbackIdTest.numCalls, 0 )
		sphere = maya.cmds.rename( sphere, sphere + "Different" )
		self.assertEqual( CallbackIdTest.numCalls, 1 )
		
		# delete the CallbackId object and make sure the callback is removed and therefore not invoked any more
		del c
		sphere = maya.cmds.rename( sphere, sphere + "DifferentAgain" )
		self.assertEqual( CallbackIdTest.numCalls, 1 )
		
		# also check that maya isn't holding onto any unecessary references to the callback function.
		w = weakref.ref( callback )
		del callback
		self.assertEqual( w(), None )
		
if __name__ == "__main__":
	MayaUnitTest.TestProgram()
