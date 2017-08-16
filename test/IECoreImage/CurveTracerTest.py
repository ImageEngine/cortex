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
import IECoreImage

class CurveTracerTest( unittest.TestCase ) :
	def test( self ) :

		i = IECore.Reader.create( "test/IECore/data/tiff/toTrace.tif" ).read()

		o = IECoreImage.ImageThinner()
		o( input = i, copyInput = False, channels = IECore.StringVectorData( ["R"] ) )

		o = IECoreImage.CurveTracer()
		c = o( image = i, channelName = "R" )

		e = IECore.CurvesPrimitiveEvaluator( c )
		r = e.createResult()

		size = i.dataWindow.size() + IECore.V2i( 1 )
		pixels = i["R"].data
		pi = 0
		for y in range( 0, size.y ) :
			for x in range( 0, size.x ) :
				if pixels[pi] > 0.5 :
					pp = IECore.V3f( x, y, 0 )
					e.closestPoint( pp, r )
					self.failUnless( abs( (pp - r.point()).length() ) < 3 )
				pi += 1

	def testPixelOutputSpace( self ) :

		i = IECore.Reader.create( "test/IECore/data/tiff/singlePixelT.tif" ).read()

		o = IECoreImage.CurveTracer()
		c = o( image = i, channelName = "R", minimumLength = 0 )

		self.assertEqual( c.bound(), IECore.Box3f( IECore.V3f( 1, 7, 0 ), IECore.V3f( 9, 11, 0 ) ) )
		self.failUnless( IECore.V3f( 1, 7, 0 ) in c["P"].data )
		self.failUnless( IECore.V3f( 7, 11, 0 ) in c["P"].data )
		self.failUnless( IECore.V3f( 9, 7, 0 ) in c["P"].data )

	def testUVOutputSpace( self ) :

		i = IECore.Reader.create( "test/IECore/data/tiff/singlePixelT.tif" ).read()

		o = IECoreImage.CurveTracer()
		o["outputSpace"].setValue( "uv" )
		c = o( image = i, channelName = "R", minimumLength = 0 )

		self.failUnless( c.bound().min.equalWithAbsError( IECore.V3f( 0.1, 0.5, 0 ), 0.00001 ) )
		self.failUnless( c.bound().max.equalWithAbsError( IECore.V3f( 0.633333, 0.7666666, 0 ), 0.00001 ) )

	def testObjectOutputSpace( self ) :

		i = IECore.Reader.create( "test/IECore/data/tiff/singlePixelT.tif" ).read()

		o = IECoreImage.CurveTracer()
		o["outputSpace"].setValue( "object" )
		c = o( image = i, channelName = "R", minimumLength = 0 )

		self.failUnless( c.bound().min.equalWithAbsError( IECore.V3f( -6, -4, 0 ), 0.00001 ) )
		self.failUnless( c.bound().max.equalWithAbsError( IECore.V3f( 2, 0, 0 ), 0.00001 ) )


if __name__ == "__main__" :
	unittest.main()
