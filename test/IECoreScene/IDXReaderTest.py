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
import imath
import IECore
import IECoreScene

class IDXReaderTest( unittest.TestCase ) :

	def testConstruction( self ) :

		r = IECoreScene.IDXReader()
		self.assertEqual( r["fileName"].getTypedValue(), "" )

		r = IECoreScene.IDXReader( "test/IECore/data/idxFiles/test.idx" )
		self.assertEqual( r["fileName"].getTypedValue(), "test/IECore/data/idxFiles/test.idx" )

	def testReading( self ) :

		r = IECoreScene.IDXReader( "test/IECore/data/idxFiles/test.idx" )

		o = r.read()

		self.failUnless( o.isInstanceOf( IECoreScene.Group.staticTypeId() ) )

		self.assertEqual( len(o.children()), 1 )

		c = o.children()[0]

		self.assertEqual( c.numPoints, 6 )

		# We're not order preserving

		for k in [ "000", "001", "002", "003", "004", "005" ] :
			self.failUnless( k in c["PointID"].data )

		for p in [
				imath.V3f( 63.204863, 0.831837, -33.969296 ),
				imath.V3f( 62.345470, 0.707662, -34.099882 ),
				imath.V3f( 63.104346, 0.708060, -34.025762 ),
				imath.V3f( 63.096101, -0.973316, -34.031914 ),
				imath.V3f( 62.338136, -0.974567, -34.112893 ),
				imath.V3f( 54.252821, 0.716216, -34.849839 ),
			]:
				self.failUnless( p in c["P"].data )

	def testCanRead( self ) :

		self.failUnless( IECoreScene.IDXReader.canRead( "test/IECore/data/idxFiles/test.idx" ) )
		self.failIf( IECoreScene.IDXReader.canRead( "test/IECore/data/cobFiles/ball.cob" ) )

	def testRegistration( self ) :

		r = IECore.Reader.create( "test/IECore/data/idxFiles/test.idx" )
		self.failUnless( isinstance( r, IECoreScene.IDXReader ) )

if __name__ == "__main__":
	unittest.main()

