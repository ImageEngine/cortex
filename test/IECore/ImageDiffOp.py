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
import sys
from IECore import *

class TestImageDiffOp(unittest.TestCase):
		
	def testSimple(self):
		
		op = ImageDiffOp()
		
		imageA = Reader.create( "test/IECore/data/tiff/uvMap.512x256.32bit.tif" ).read()
		imageB = Reader.create( "test/IECore/data/cinFiles/uvMap.512x256.cin" ).read()
		res = op(
			imageA = imageA,
			imageB = imageB
		)
		
		self.failIf( res.value )
		
		imageA = Reader.create( "test/IECore/data/tiff/uvMap.512x256.32bit.tif" ).read()
		imageB = Reader.create( "test/IECore/data/tiff/uvMap.200x100.rgba.16bit.tif" ).read()
		
		res = op(
			imageA = imageA,
			imageB = imageB
		)
		
		self.assert_( res.value )	
		
		imageA = Reader.create( "test/IECore/data/tiff/uvMap.512x256.16bit.tif" ).read()
		imageB = Reader.create( "test/IECore/data/tiff/uvMapUpsideDown.512x256.16bit.tif" ).read()
		
		res = op(
			imageA = imageA,
			imageB = imageB
		)
		
		self.assert_( res.value )	
		
		imageA = Reader.create( "test/IECore/data/tiff/uvMap.512x256.32bit.tif" ).read()
				
		m = CapturingMessageHandler()
		s = ScopedMessageHandler( m )
		
		res = op(
			imageA = imageA,
			imageB = imageA
		)
		
		self.assertEqual( len( m.messages ), 1 )
		self.assertEqual( m.messages[0].level, Msg.Level.Warning )
		self.assertEqual( m.messages[0].message, "Exact same image specified as both input parameters." )
		
		self.failIf( res.value )
		
		s = None
		
		res = op(
			imageA = imageA,
			imageB = imageA.copy()
		)
		
		self.failIf( res.value )	

	def testMissingChannels(self):
		
		op = ImageDiffOp()
		
		w = Box2i( V2i( 0, 0 ), V2i( 99, 99 ) )
		
		f = FloatVectorData()
		f.resize( 100 * 100, 0 )
		
		imageA = ImagePrimitive( w, w )
		imageB = ImagePrimitive( w, w )
		
		# Both images have channel "R"
		imageA["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, f )
		imageB["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, f )

		# Only imageA has channel "G"		
		imageA["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, f )		
		
		res = op(
			imageA = imageA,
			imageB = imageB,
			skipMissingChannels = False
		)
		
		self.assert_( res.value )
		
		res = op(
			imageA = imageA,
			imageB = imageB,
			skipMissingChannels = True
		)
		
		self.failIf( res.value )
		
                			
if __name__ == "__main__":
	unittest.main()   
	
