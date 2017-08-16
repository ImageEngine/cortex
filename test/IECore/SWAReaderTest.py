##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

class SWAReaderTest( unittest.TestCase ) :

	def testConstruction( self ) :

		r = IECore.SWAReader()
		self.assertEqual( r["fileName"].getTypedValue(), "" )
		
		r = IECore.SWAReader( "test/IECore/data/swaFiles/test.swa" )
		self.assertEqual( r["fileName"].getTypedValue(), "test/IECore/data/swaFiles/test.swa" )
		
	def testReading( self ) :
	
		r = IECore.SWAReader( "test/IECore/data/swaFiles/test.swa" )
		
		o = r.read()
		
		IECore.ObjectWriter( o, "/tmp/trees4.cob" ).write()
		
		self.failUnless( o.isInstanceOf( IECore.PointsPrimitive.staticTypeId() ) )
		self.assertEqual( o.numPoints, 5 + 6 )
		self.failUnless( o.arePrimitiveVariablesValid() )
		
		self.failUnless( "P" in o )
		self.failUnless( "xAxis" in o )
		self.failUnless( "yAxis" in o )
		self.failUnless( "zAxis" in o )		
		self.failUnless( "scale" in o )		
		self.failUnless( "treeName" in o )		
		self.failUnless( "treeNameIndices" in o )
		
		self.assertEqual( o["P"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["xAxis"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )	
		self.assertEqual( o["yAxis"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["zAxis"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["scale"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["treeNameIndices"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["treeName"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		
		self.failUnless( isinstance( o["P"].data, IECore.V3fVectorData ) )
		self.failUnless( isinstance( o["xAxis"].data, IECore.V3fVectorData ) )
		self.failUnless( isinstance( o["yAxis"].data, IECore.V3fVectorData ) )
		self.failUnless( isinstance( o["zAxis"].data, IECore.V3fVectorData ) )
		self.failUnless( isinstance( o["scale"].data, IECore.FloatVectorData ) )
		self.failUnless( isinstance( o["treeNameIndices"].data, IECore.IntVectorData ) )
		self.failUnless( isinstance( o["treeName"].data, IECore.StringVectorData ) )
		
		self.assertEqual( o["treeName"].data, IECore.StringVectorData( [ "Acacia_RT", "BroadLeaf_HighDetail" ] ) )
		self.assertEqual( o["P"].data[0], IECore.V3f( 3750.05, 1556.86, -2149.22 ) )
		self.assertEqual( o["yAxis"].data[0], IECore.V3f( 0.0176831, 0.998519, 0.0514542 ) )
		self.assertEqual( o["xAxis"].data[0], IECore.V3f( 0.0179192, -0.0517705, 0.998498  ) )
		self.assertEqual( o["zAxis"].data[0], o["xAxis"].data[0].cross( o["yAxis"].data[0] ) )
		self.assertAlmostEqual( o["scale"].data[0], 6.4516, 6 )
		self.assertAlmostEqual( o["scale"].data[1], 6.7, 6 )
		self.assertEqual( o["treeNameIndices"].data, IECore.IntVectorData( [ 0 ] * 5 + [ 1 ] * 6 ) )
		
	def testCanRead( self ) :
	
		self.failUnless( IECore.SWAReader.canRead( "test/IECore/data/swaFiles/test.swa" ) )
		self.failIf( IECore.IDXReader.canRead( "test/IECore/data/cobFiles/ball.cob" ) )
		self.failIf( IECore.SWAReader.canRead( "test/IECore/data/idxFiles/test.idx" ) )
		self.failIf( IECore.SWAReader.canRead( "test/IECore/data/empty" ) )
				
	def testRegistration( self ) :
	
		r = IECore.Reader.create( "test/IECore/data/swaFiles/test.swa" )
		self.failUnless( isinstance( r, IECore.SWAReader ) )

if __name__ == "__main__":
	unittest.main()

