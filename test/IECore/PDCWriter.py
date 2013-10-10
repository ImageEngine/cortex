##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

import os
import unittest
import sys
import IECore

class TestPDCWriter( unittest.TestCase ) :

	def testBasics( self ) :

		r = IECore.Reader.create( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		p = r.read()

		w = IECore.Writer.create( p, "test/particleShape1.250.pdc" )
		w.write()

		r = IECore.Reader.create( "test/particleShape1.250.pdc" )
		p2 = r.read()

		self.assertEqual( p, p2 )

	def testFiltering( self ) :

		r = IECore.Reader.create( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		p = r.read()

		w = IECore.Writer.create( p, "test/particleShape1.250.pdc" )
		w.parameters()["attributes"].setValue( IECore.StringVectorData( ["P"] ) )
		w.write()

		for k in p.keys() :
			if k!="P" :
				del p[k]

		r = IECore.Reader.create( "test/particleShape1.250.pdc" )
		p2 = r.read()

		self.assertEqual( p, p2 )

	def testBadObjectException( self ) :

		w = IECore.PDCParticleWriter( IECore.IntData(10), "test/intData.pdc" )
		self.assertRaises( RuntimeError, w.write )

	def testWriteConstantData( self ) :

		p = IECore.PointsPrimitive( 1 )
		p["d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.DoubleData( 1 ) )
		p["v3d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3dData( IECore.V3d( 1, 2, 3 ) ) )
		p["i"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 10 ) )

		w = IECore.Writer.create( p, "test/particleShape1.250.pdc" )
		w.write()

		r = IECore.Reader.create( "test/particleShape1.250.pdc" )
		r.parameters()["realType"].setValue( "native" )
		p2 = r.read()

		self.assertEqual( p, p2 )

	def testWriteFloatData( self ) :

		p = IECore.PointsPrimitive( 3 )
		p["f"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ) )
		p["v3f"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 1, 2, 3 ) ) )
		p["fVector"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3 ] ) )
		p["v3fVector"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 1, 2, 3 ), IECore.V3f( 4, 5, 6 ), IECore.V3f( 7, 8, 9 ) ] ) )

		w = IECore.Writer.create( p, "test/particleShape1.250.pdc" )
		w.write()

		r = IECore.Reader.create( "test/particleShape1.250.pdc" )
		r.parameters()["realType"].setValue( "native" )
		p2 = r.read()

		self.assertEqual( p.keys(), p2.keys() )
		self.assertEqual( p2["f"].data, IECore.DoubleData( 1 ) )
		self.assertEqual( p2["v3f"].data, IECore.V3dData( IECore.V3d( 1, 2, 3 ) ) )
		self.assertEqual( p2["fVector"].data, IECore.DoubleVectorData( [ 1, 2, 3 ] ) )
		self.assertEqual( p2["v3fVector"].data, IECore.V3dVectorData( [ IECore.V3d( 1, 2, 3 ), IECore.V3d( 4, 5, 6 ), IECore.V3d( 7, 8, 9 ) ] ) )
	
	def testWriteColorData( self ) :
	
		p = IECore.PointsPrimitive( 3 )
		p["color3f"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.Color3fData( IECore.Color3f( 1 ) ) )
		p["color3fVector"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.Color3fVectorData( [ IECore.Color3f( 1 ), IECore.Color3f( 2 ), IECore.Color3f( 3 ) ] ) )
	
		IECore.Writer.create( p, "test/particleShape1.250.pdc" ).write()

		p2 = IECore.Reader.create( "test/particleShape1.250.pdc" ).read()

		self.assertEqual( p.keys(), p2.keys() )
		
		# we can't expect them to come back as colours, because there's not support for that in pdcs - they should therefore come back as float vectors
		self.assertEqual( p2["color3f"].data, IECore.V3fData( IECore.V3f( 1 ) ) )
		self.assertEqual( p2["color3fVector"].data, IECore.V3fVectorData( [ IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ) ] ) )
	
	def tearDown( self ) :

		if os.path.isfile( "test/particleShape1.250.pdc" ) :
			os.remove( "test/particleShape1.250.pdc" )

if __name__ == "__main__":
	unittest.main()

