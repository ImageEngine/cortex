##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
import IECore

class TestIFFHairReader( unittest.TestCase ) :

	def testConstruction( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/hairSystem.mchp" )
		self.assert_( r.isInstanceOf( "IFFHairReader" ) )
		self.assertEqual( type( r ), IECore.IFFHairReader )
		self.assertEqual( r["fileName"].getValue().value, "test/IECore/data/iffFiles/hairSystem.mchp" )

	def testRead( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/hairSystem.mchp" )
		self.assertEqual( type( r ), IECore.IFFHairReader )

		self.assertEqual( r.numHairs(), 64 )
		self.assertEqual( len(r.frameTimes()), 4 )
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3dVectorData,
			"velocity" : IECore.V3dVectorData,
		}
		
		c = r.read()
		self.assertEqual( type( c ), IECore.CurvesPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		self.assertEqual( len( c.verticesPerCurve() ), r.numHairs() )
		
		expectedDataLength = 0
		for i in c.verticesPerCurve() :
			self.assertEqual( i, 10 )
			expectedDataLength += i
		
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type( c[i].data ), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len( c[i].data ), expectedDataLength )
		
		for p in c["P"].data :
			self.assert_( abs( p.x ) < 16.123 )
			self.assert_( abs( p.y ) < 13.222 )
			self.assert_( abs( p.z ) < 19.440 )
	
	def testMultiFrameFiles( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/hairSystem.mchp" )
		
		self.assertEqual( len(r.frameTimes()), 4 )
		self.assertEqual( r.numHairs(), 64 )
		
		c = r.read()
		self.assertEqual( type( c ), IECore.CurvesPrimitive )
		self.assertEqual( len( c ), 2 )
		self.assertEqual( len(c.verticesPerCurve()), r.numHairs() )
		
		for p in c["P"].data :
			self.assert_( abs( p.x ) < 16.123 )
			self.assert_( abs( p.y ) < 13.222 )
			self.assert_( abs( p.z ) < 19.440 )
		
		r.parameters()['frameIndex'].setValue( 3 )
		
		self.assertEqual( r.numHairs(), 64 )
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3dVectorData,
			"velocity" : IECore.V3dVectorData,
		}
		
		c = r.read()
		self.assertEqual( type( c ), IECore.CurvesPrimitive )
		self.assertEqual( len( c ), 2 )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		self.assertEqual( len( c.verticesPerCurve() ), r.numHairs() )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c.keys() )
		
		expectedDataLength = 0
		for i in c.verticesPerCurve() :
			self.assertEqual( i, 10 )
			expectedDataLength += i
		
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type( c[i].data ), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len( c[i].data ), expectedDataLength )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 16.140 )
			self.assert_( abs( p.y ) < 13.253 )
			self.assert_( abs( p.z ) < 19.456 )
	
	def testConversion( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/hairSystem.mchp" )
		self.assertEqual( type( r ), IECore.IFFHairReader )

		r.parameters()["realType"].setValue( "float" )
		r.parameters()['frameIndex'].setValue( 3 )
		
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3fVectorData,
			"velocity" : IECore.V3fVectorData,
		}

		c = r.read()
		self.assertEqual( type( c ), IECore.CurvesPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		
		expectedDataLength = 0
		for i in c.verticesPerCurve() :
			self.assertEqual( i, 10 )
			expectedDataLength += i
		
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len( c[i].data ), expectedDataLength )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 16.140 )
			self.assert_( abs( p.y ) < 13.253 )
			self.assert_( abs( p.z ) < 19.456 )

	def testSparseCacheFailure( self ) :
		
		r = IECore.Reader.create( "test/IECore/data/iffFiles/sparseHairSystem.mchp" )
		
		self.assertEqual( r.numHairs(), 64 )
		
		self.assertRaises( RuntimeError, r.read )
	
	def testFileNameChange( self ) :

		"""Now Readers are Ops, the filename can be changed and read() can be called
		again. So we need to check that that works."""

		r = IECore.Reader.create( "test/IECore/data/iffFiles/hairSystem.mchp" )
		self.assertEqual( type( r ), IECore.IFFHairReader )

		r.parameters()["realType"].setValue( "float" )
		
		self.assertEqual( r.numHairs(), 64 )
		
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3fVectorData,
			"velocity" : IECore.V3fVectorData,
		}

		c = r.read()
		self.assertEqual( type( c ), IECore.CurvesPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		
		expectedDataLength = 0
		for i in c.verticesPerCurve() :
			self.assertEqual( i, 10 )
			expectedDataLength += i
		
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len( c[i].data ), expectedDataLength )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 16.123 )
			self.assert_( abs( p.y ) < 13.222 )
			self.assert_( abs( p.z ) < 19.440 )

		r["fileName"].setValue( IECore.StringData( "test/IECore/data/iffFiles/sparseHairSystem.mchp" ) )
		
		self.assertEqual( r.numHairs(), 64 )
		
		self.assertRaises( RuntimeError, r.read )
		
		r["fileName"].setValue( IECore.StringData( "test/IECore/data/iffFiles/hairSystem.mchp" ) )
		
		r.parameters()['frameIndex'].setValue( 3 )
		
		self.assertEqual( r.numHairs(), 64 )
		
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3fVectorData,
			"velocity" : IECore.V3fVectorData,
		}

		c = r.read()
		self.assertEqual( type( c ), IECore.CurvesPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		
		expectedDataLength = 0
		for i in c.verticesPerCurve() :
			self.assertEqual( i, 10 )
			expectedDataLength += i
		
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len( c[i].data ), expectedDataLength )
		
		for p in c["P"].data :
			self.assert_( abs( p.x ) < 16.140 )
			self.assert_( abs( p.y ) < 13.253 )
			self.assert_( abs( p.z ) < 19.456 )

	
	def testParameterTypes( self ) :

		p = IECore.IFFHairReader()
		self.assert_( p.resultParameter().isInstanceOf( "ObjectParameter" ) )
		self.assertEqual( p.resultParameter().validTypes(), [IECore.TypeId.CurvesPrimitive] )

if __name__ == "__main__":
	unittest.main()

