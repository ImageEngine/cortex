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
import os
import sys

import IECore
import IECoreRI

class SXRendererTest( unittest.TestCase ) :

	def __loadImage( self, fileName ) :
	
		i = IECore.Reader.create( fileName ).read()
		
		r = i["R"].data
		g = i["G"].data
		b = i["B"].data
		
		result = IECore.V3fVectorData()
		v = IECore.V3f
		for i in range( 0, len( r ) ) :
			result.append( v( r[i], g[i], b[i] ) )

		return result
		
	def __saveImage( self, data, dataWindow, fileName ) :
	
		image = IECore.ImagePrimitive( dataWindow, dataWindow )
		if isinstance( data, IECore.FloatVectorData ) :
			
			image["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, data )
			image["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, data )
			image["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, data )
		
		else :
		
			r = IECore.FloatVectorData()
			g = IECore.FloatVectorData()
			b = IECore.FloatVectorData()
			
			for c in data :
			
				r.append( c[0] )
				g.append( c[1] )
				b.append( c[2] )
			
			image["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, r )
			image["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, g )
			image["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, b )
			
		IECore.Writer.create( image, fileName ).write()
		
	def test( self ) :

		r = IECoreRI.SXRenderer()

		points = IECore.CompoundData( {
		
			"N" : self.__loadImage( "test/IECoreRI/data/sxInput/cowN.exr" ),
			"Ng" : self.__loadImage( "test/IECoreRI/data/sxInput/cowN.exr" ),
			"P" : self.__loadImage( "test/IECoreRI/data/sxInput/cowP.exr" ),
			"I" : self.__loadImage( "test/IECoreRI/data/sxInput/cowI.exr" ),
	
		} )
				
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxTest.sdl test/IECoreRI/shaders/sxTest.sl" ), 0 )
		
		r.shader( "surface", "test/IECoreRI/shaders/sxTest.sdl", { "noiseFrequency" : 1.0, "tint" : IECore.Color3f( 1 ) } )
		
		s = r.shade( points )
		
		self.assertEqual( len( s ), 4 )
		self.failUnless( "outputFloat" in s )
		self.failUnless( "outputColor" in s )
		self.failUnless( "Ci" in s )
		self.failUnless( "Oi" in s )
		
		self.assertEqual( s["outputFloat"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowFloat.cob" ).read() )
		self.assertEqual( s["outputColor"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowColor.cob" ).read() )
		self.assertEqual( s["Ci"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowCI.cob" ).read() )
		self.assertEqual( s["Oi"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowOI.cob" ).read() )
			
	def tearDown( self ) :
		
		files = [
			"test/IECoreRI/shaders/sxTest.sdl",
		]
		
		for f in files :
			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()
