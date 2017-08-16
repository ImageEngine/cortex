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

import unittest
import os

import nuke

import IECore
import IECoreImage
import IECoreNuke

class OpHolderTest( IECoreNuke.TestCase ) :

	def testExecute( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "maths/multiply", 2 )
		
		with fnOH.parameterModificationContext() as parameters :
		
			parameters["a"].setNumericValue( 10 )
			parameters["b"].setNumericValue( 20 )
		
		fnOH.getOp()
		
		result = fnOH.execute()
		self.assertEqual( result, IECore.IntData( 200 ) )
		
	def testExecuteWithMeshInputs( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "meshMerge", 1 )
		
		self.assertEqual( fnOH.node().minimumInputs(), 2 )
		self.assertEqual( fnOH.node().maximumInputs(), 2 )
		
		card1 = nuke.nodes.Card()
		card2 = nuke.nodes.Card()
		
		fnOH.node().setInput( 0, card1 )
		fnOH.node().setInput( 1, card2 )

		merged = fnOH.execute()
		
		self.failUnless( isinstance( merged, IECore.MeshPrimitive ) )
		self.assertEqual( merged.numFaces(), 2 )
		self.assertEqual( merged.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 8 )
		
	def testExecuteWithObjectVectorInput( self ) :
	
		fnOHMult1 = IECoreNuke.FnOpHolder.create( "mult1", "maths/multiply", 2 )		
		with fnOHMult1.parameterModificationContext() as parameters :
			parameters["a"].setNumericValue( 10 )
			parameters["b"].setNumericValue( 20 )
				
		fnOHMult2 = IECoreNuke.FnOpHolder.create( "mult2", "maths/multiply", 2 )		
		with fnOHMult2.parameterModificationContext() as parameters :
			parameters["a"].setNumericValue( 12 )
			parameters["b"].setNumericValue( 10 )
			
		fnOH = IECoreNuke.FnOpHolder.create( "merge", "objectVectorInOut", 1 )
		
		self.assertEqual( fnOH.node().minimumInputs(), 0 )
		self.assertEqual( fnOH.node().maximumInputs(), 100 )
		
		fnOH.node().setInput( 0, fnOHMult1.node() )
		fnOH.node().setInput( 1, fnOHMult2.node() )
		fnOH.node().setInput( 2, fnOHMult1.node() )
		fnOH.node().setInput( 3, fnOHMult2.node() )
								
		self.assertEqual( fnOH.execute(), IECore.ObjectVector( [ IECore.IntData( 200 ), IECore.IntData( 120 ), IECore.IntData( 200 ), IECore.IntData( 120 ) ] ) )
	
	def testExecuteWithImageInput( self ) :
	
		fnOH = IECoreNuke.FnOpHolder.create( "test", "imagePrimitiveInOut", 1 )
		
		self.assertEqual( fnOH.node().minimumInputs(), 1 )
		self.assertEqual( fnOH.node().maximumInputs(), 1 )
		
		check = nuke.nodes.CheckerBoard()
		
		fnOH.node().setInput( 0, check )
		
		# get the image as output by the op
		image = fnOH.execute()

		# write the same image direct to disk without using the op
		write = nuke.nodes.Write()
		write.setInput( 0, check )
		write.knob( "file" ).setValue( "test/IECoreNuke/check.exr" )
		write.knob( "channels" ).setValue( "rgba" )
		nuke.execute( write, 1, 1 )
		
		# check that they are the same in terms of size and channel data.
		# allow a slight difference due to one having been saved as half float and reloaded.
		
		image2 = IECore.Reader.create( "test/IECoreNuke/check.exr" ).read()
		self.assertEqual( IECoreImage.ImageDiffOp()( imageA = image, imageB = image2, maxError = 0.001 ).value, False )

	def tearDown( self ) :
		
		for f in [
			"test/IECoreNuke/check.exr"
		] :
			if os.path.exists( f ) :
				os.remove( f )
							
if __name__ == "__main__":
	unittest.main()
