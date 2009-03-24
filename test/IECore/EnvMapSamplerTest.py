##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

class EnvMapSamplerTest( unittest.TestCase ) :

	def test( self ) :
	
		image = IECore.Reader.create( "test/IECore/data/exrFiles/carPark.exr" ).read()
		for n in ["R", "G", "B"] :
			p = image[n]
			p.data = IECore.DataCastOp()( object=image[n].data, targetType=IECore.FloatVectorData.staticTypeId() )
			image[n] = p
						
		lights = IECore.EnvMapSampler()( image=image, subdivisionDepth=5 )
		
		self.assertEqual( len( lights ), 2 )
		
		directions = lights["directions"]
		colors = lights["colors"]
		
		self.assert_( directions.isInstanceOf( IECore.V3fVectorData.staticTypeId() ) )
		self.assert_( colors.isInstanceOf( IECore.Color3fVectorData.staticTypeId() ) )
		
		self.assertEqual( len( directions ), len( colors ) )
		self.assertEqual( len( directions ), 32 )

		for d in directions :
		
			self.assertAlmostEqual( d.length(), 1, 6 )
	
	def testColorSums( self ) :
	
		"""Check that subdivision depth doesn't change the total amount of light."""
	
		image = IECore.Reader.create( "test/IECore/data/exrFiles/carPark.exr" ).read()
		for n in ["R", "G", "B"] :
			p = image[n]
			p.data = IECore.DataCastOp()( object=image[n].data, targetType=IECore.FloatVectorData.staticTypeId() )
			image[n] = p
		
		lastColorSum = None
		for i in range( 0, 10 ) :
		
			lights = IECore.EnvMapSampler()( image=image, subdivisionDepth=i )
			colorSum = sum( lights["colors"], IECore.Color3f( 0 ) )
		
			if lastColorSum :
				self.assertAlmostEqual( colorSum[0], lastColorSum[0], 5 )
				self.assertAlmostEqual( colorSum[1], lastColorSum[1], 5 )
				self.assertAlmostEqual( colorSum[2], lastColorSum[2], 5 )
			
			lastColorSum = colorSum
				
if __name__ == "__main__":
	unittest.main()
	
