##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

class TestTurbulence( unittest.TestCase ) :
	
	def testConstructors( self ) :
	
		t = IECore.TurbulenceV2ff()
		self.assertEqual( t.octaves, 4 )
		self.assertEqual( t.gain, 0.5 )
		self.assertEqual( t.lacunarity, IECore.V2f( 2.0 ) )
		self.assertEqual( t.turbulent, True )
		
		t = IECore.TurbulenceV2ff( 2, 1, IECore.V2f( 3.0 ), False )
		self.assertEqual( t.octaves, 2 )
		self.assertEqual( t.gain, 1 )
		self.assertEqual( t.lacunarity, IECore.V2f( 3.0 ) )
		self.assertEqual( t.turbulent, False )
		
		t = IECore.TurbulenceV2ff(
			octaves = 3,
			gain = 1.4,
			lacunarity = IECore.V2f( 3.0 ),
			turbulent = False
		)
		
		self.assertEqual( t.octaves, 3 )
		self.assertAlmostEqual( t.gain, 1.4 )
		self.assertEqual( t.lacunarity, IECore.V2f( 3.0 ) )
		self.assertEqual( t.turbulent, False )
		
	def test2d( self ) :
	
		t = IECore.TurbulenceV2ff(
			octaves = 4,
			gain = 0.35,
			lacunarity = IECore.V2f( 2.0 ),
			turbulent = False
		)
		
		width = 400
		height = 400
				
		f = IECore.FloatVectorData( width * height )
		o = 0
		for i in range( 0, height ) :
			for j in range( 0, width ) :
				f[o] = 0.5 + t.turbulence( IECore.V2f( i/50.0, j/50.0 ) )
				o += 1
			
		b = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( width-1, height-1 ) )		
		i = IECore.ImagePrimitive( b, b )
		i["r"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		i["g"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		i["b"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )

		IECore.Writer.create( i, "/tmp/turbulenceV2ff.exr" ).write()
						
if __name__ == "__main__":
	unittest.main()   
	
