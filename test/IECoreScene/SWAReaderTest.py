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
import imath
import tempfile
import shutil
import os
import IECore
import IECoreScene

class SWAReaderTest( unittest.TestCase ) :

	def testConstruction( self ) :

		r = IECoreScene.SWAReader()
		self.assertEqual( r["fileName"].getTypedValue(), "" )

		r = IECoreScene.SWAReader( os.path.join( "test", "IECore", "data", "swaFiles", "test.swa" ) )
		self.assertEqual( r["fileName"].getTypedValue(), os.path.join( "test", "IECore", "data", "swaFiles", "test.swa" ) )

	def testReading( self ) :

		r = IECoreScene.SWAReader( os.path.join( "test", "IECore", "data", "swaFiles", "test.swa" ) )

		o = r.read()

		tempDir = tempfile.mkdtemp()
		IECore.ObjectWriter( o, os.path.join(tempDir, "trees4.cob" ) ).write()

		self.assertTrue( o.isInstanceOf( IECoreScene.PointsPrimitive.staticTypeId() ) )
		self.assertEqual( o.numPoints, 5 + 6 )
		self.assertTrue( o.arePrimitiveVariablesValid() )

		self.assertTrue( "P" in o )
		self.assertTrue( "xAxis" in o )
		self.assertTrue( "yAxis" in o )
		self.assertTrue( "zAxis" in o )
		self.assertTrue( "scale" in o )
		self.assertTrue( "treeName" in o )
		self.assertTrue( "treeNameIndices" in o )

		self.assertEqual( o["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["xAxis"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["yAxis"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["zAxis"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["scale"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["treeNameIndices"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( o["treeName"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )

		self.assertTrue( isinstance( o["P"].data, IECore.V3fVectorData ) )
		self.assertTrue( isinstance( o["xAxis"].data, IECore.V3fVectorData ) )
		self.assertTrue( isinstance( o["yAxis"].data, IECore.V3fVectorData ) )
		self.assertTrue( isinstance( o["zAxis"].data, IECore.V3fVectorData ) )
		self.assertTrue( isinstance( o["scale"].data, IECore.FloatVectorData ) )
		self.assertTrue( isinstance( o["treeNameIndices"].data, IECore.IntVectorData ) )
		self.assertTrue( isinstance( o["treeName"].data, IECore.StringVectorData ) )

		self.assertEqual( o["treeName"].data, IECore.StringVectorData( [ "Acacia_RT", "BroadLeaf_HighDetail" ] ) )
		self.assertEqual( o["P"].data[0], imath.V3f( 3750.05, 1556.86, -2149.22 ) )
		self.assertEqual( o["yAxis"].data[0], imath.V3f( 0.0176831, 0.998519, 0.0514542 ) )
		self.assertEqual( o["xAxis"].data[0], imath.V3f( 0.0179192, -0.0517705, 0.998498  ) )
		self.assertEqual( o["zAxis"].data[0], o["xAxis"].data[0].cross( o["yAxis"].data[0] ) )
		self.assertAlmostEqual( o["scale"].data[0], 6.4516, 6 )
		self.assertAlmostEqual( o["scale"].data[1], 6.7, 6 )
		self.assertEqual( o["treeNameIndices"].data, IECore.IntVectorData( [ 0 ] * 5 + [ 1 ] * 6 ) )

		shutil.rmtree( tempDir )

	def testCanRead( self ) :

		self.assertTrue( IECoreScene.SWAReader.canRead( os.path.join( "test", "IECore", "data", "swaFiles", "test.swa" ) ) )
		self.assertFalse( IECoreScene.IDXReader.canRead( os.path.join( "test", "IECore", "data", "cobFiles/ball.cob" ) ) )
		self.assertFalse( IECoreScene.SWAReader.canRead( os.path.join( "test", "IECore", "data", "idxFiles", "test.idx" ) ) )
		self.assertFalse( IECoreScene.SWAReader.canRead( os.path.join( "test", "IECore", "data", "empty" ) ) )

	def testRegistration( self ) :

		r = IECore.Reader.create( os.path.join( "test", "IECore", "data", "swaFiles", "test.swa" ) )
		self.assertTrue( isinstance( r, IECoreScene.SWAReader ) )

if __name__ == "__main__":
	unittest.main()

