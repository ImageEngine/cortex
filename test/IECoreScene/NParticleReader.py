##########################################################################
#
#  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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
import IECoreScene

class TestNParticleReader( unittest.TestCase ) :

	def testConstruction( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nParticleFrame2.mc" )
		self.assert_( r.isInstanceOf( "ParticleReader" ) )
		self.assertEqual( type( r ), IECoreScene.NParticleReader )
		self.assertEqual( r["fileName"].getValue().value, "test/IECore/data/iffFiles/nParticleFrame2.mc" )

	def testReadWithPrimVarConversion( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nParticleFrame2.mc" )
		self.assertEqual( type( r ), IECoreScene.NParticleReader )
		r.parameters()["realType"].setValue( "native" )

		self.assertEqual( r.numParticles(), 4 )
		self.assertEqual( len(r.frameTimes()), 1 )
		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"nParticleShape1_id" : IECore.DoubleVectorData,
			"nParticleShape1_birthTime" : IECore.DoubleVectorData,
			"nParticleShape1_position" : IECore.V3dVectorData,
			"nParticleShape1_lifespanPP" : IECore.DoubleVectorData,
			"nParticleShape1_finalLifespanPP" : IECore.DoubleVectorData,
			"nParticleShape1_velocity" : IECore.V3dVectorData,
		}
		self.assertEqual( len( attrNames ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in attrNames )

		c = r.read()
		expectedConvertedAttrNamesAndTypes = {
			"nParticleShape1_id" : IECore.DoubleVectorData,
			"nParticleShape1_birthTime" : IECore.DoubleVectorData,
			"P" : IECore.V3dVectorData,
			"nParticleShape1_lifespanPP" : IECore.DoubleVectorData,
			"nParticleShape1_finalLifespanPP" : IECore.DoubleVectorData,
			"nParticleShape1_velocity" : IECore.V3dVectorData,
		}
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedConvertedAttrNamesAndTypes ) )
		for i in expectedConvertedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedConvertedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.022 )
			self.assert_( abs( p.y ) < 0.017 )
			self.assert_( abs( p.z ) < 0.020 )

		self.assertEqual( c["nParticleShape1_id"].data, IECore.DoubleVectorData( range( 0, 4 ) ) )

	def testReadNoPrimVarConversion( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nParticleFrame2.mc" )
		self.assertEqual( type( r ), IECoreScene.NParticleReader )
		r["convertPrimVarNames"].setValue( IECore.BoolData( False ) )
		r["realType"].setValue( "native" )
		self.assertFalse( r.parameters()["convertPrimVarNames"].getTypedValue() )

		self.assertEqual( r.numParticles(), 4 )
		self.assertEqual( len(r.frameTimes()), 1 )
		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"nParticleShape1_id" : IECore.DoubleVectorData,
			"nParticleShape1_birthTime" : IECore.DoubleVectorData,
			"nParticleShape1_position" : IECore.V3dVectorData,
			"nParticleShape1_lifespanPP" : IECore.DoubleVectorData,
			"nParticleShape1_finalLifespanPP" : IECore.DoubleVectorData,
			"nParticleShape1_velocity" : IECore.V3dVectorData,
		}
		self.assertEqual( len( attrNames ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in attrNames )

		c = r.read()
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["nParticleShape1_position"].data :
			self.assert_( abs( p.x ) < 0.022 )
			self.assert_( abs( p.y ) < 0.017 )
			self.assert_( abs( p.z ) < 0.020 )

		self.assertEqual( c["nParticleShape1_id"].data, IECore.DoubleVectorData( range( 0, 4 ) ) )

	def testMultiFrameFiles( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nParticleMultipleFrames.mc" )
		r.parameters()["realType"].setValue( "native" )

		self.assertTrue( r.parameters()["convertPrimVarNames"].getTypedValue() )
		self.assertEqual( len(r.frameTimes()), 10 )

		self.assertEqual( r.numParticles(), 0 )
		attrNames = r.attributeNames()
		self.assertEqual( len( attrNames ), 0 )

		c = r.read()
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), 0 )
		self.assertEqual( c.numPoints, 0 )

		r.parameters()['frameIndex'].setValue( 5 )

		self.assertEqual( r.numParticles(), 20 )
		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"testParticleShape_id" : IECore.DoubleVectorData,
			"testParticleShape_birthTime" : IECore.DoubleVectorData,
			"testParticleShape_position" : IECore.V3dVectorData,
			"testParticleShape_lifespanPP" : IECore.DoubleVectorData,
			"testParticleShape_finalLifespanPP" : IECore.DoubleVectorData,
			"testParticleShape_velocity" : IECore.V3dVectorData,
		}
		self.assertEqual( len( attrNames ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in attrNames )

		c = r.read()
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )

		expectedConvertedAttrNamesAndTypes = {
			"testParticleShape_id" : IECore.DoubleVectorData,
			"testParticleShape_birthTime" : IECore.DoubleVectorData,
			"P" : IECore.V3dVectorData,
			"testParticleShape_lifespanPP" : IECore.DoubleVectorData,
			"testParticleShape_finalLifespanPP" : IECore.DoubleVectorData,
			"testParticleShape_velocity" : IECore.V3dVectorData,
		}

		for i in expectedConvertedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedConvertedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.159 )
			self.assert_( abs( p.y ) < 0.145 )
			self.assert_( abs( p.z ) < 0.138 )

	def testFiltering( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nParticleMultipleFrames.mc" )

		r.parameters()['frameIndex'].setValue( 5 )
		attributesToLoad = [ "testParticleShape_birthTime", "testParticleShape_position" ]
		r.parameters()["percentage"].setValue( IECore.FloatData( 50 ) )
		r.parameters()["attributes"].setValue( IECore.StringVectorData( attributesToLoad ) )

		a = r.readAttribute( "testParticleShape_position" )
		# what the acceptable thresholds should be are somewhat debatable,
		# especially for such a small number of particles
		self.assert_( len( a ) < 13 )
		self.assert_( len( a ) > 7 )

		p = r.read()
		self.assert_( p.numPoints < 13 )
		self.assert_( p.numPoints > 7 )
		convertedAttributes = [ "testParticleShape_birthTime", "P" ]
		for attr in convertedAttributes :
			self.assertEqual( p.numPoints, p[attr].data.size() )

	def testConversion( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nParticleMultipleFrames.mc"  )
		self.assertEqual( type( r ), IECoreScene.NParticleReader )

		r.parameters()["realType"].setValue( "float" )
		r.parameters()['frameIndex'].setValue( 5 )

		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"testParticleShape_id" : IECore.FloatVectorData,
			"testParticleShape_birthTime" : IECore.FloatVectorData,
			"testParticleShape_position" : IECore.V3fVectorData,
			"testParticleShape_lifespanPP" : IECore.FloatVectorData,
			"testParticleShape_finalLifespanPP" : IECore.FloatVectorData,
			"testParticleShape_velocity" : IECore.V3fVectorData,
		}

		c = r.read()
		expectedConvertedAttrNamesAndTypes = {
			"testParticleShape_id" : IECore.FloatVectorData,
			"testParticleShape_birthTime" : IECore.FloatVectorData,
			"P" : IECore.V3fVectorData,
			"testParticleShape_lifespanPP" : IECore.FloatVectorData,
			"testParticleShape_finalLifespanPP" : IECore.FloatVectorData,
			"testParticleShape_velocity" : IECore.V3fVectorData,
		}
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedConvertedAttrNamesAndTypes ) )
		for i in expectedConvertedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedConvertedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.159 )
			self.assert_( abs( p.y ) < 0.145 )
			self.assert_( abs( p.z ) < 0.138 )

	def testFileNameChange( self ) :

		"""Now Readers are Ops, the filename can be changed and read() can be called
		again. So we need to check that that works."""

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nParticleMultipleFrames.mc" )
		self.assertEqual( type( r ), IECoreScene.NParticleReader )

		r.parameters()["realType"].setValue( "float" )
		r.parameters()['frameIndex'].setValue( 5 )

		expectedAttrNamesAndTypes = {
			"testParticleShape_id" : IECore.FloatVectorData,
			"testParticleShape_birthTime" : IECore.FloatVectorData,
			"P" : IECore.V3fVectorData,
			"testParticleShape_lifespanPP" : IECore.FloatVectorData,
			"testParticleShape_finalLifespanPP" : IECore.FloatVectorData,
			"testParticleShape_velocity" : IECore.V3fVectorData,
		}

		c = r.read()
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.159 )
			self.assert_( abs( p.y ) < 0.145 )
			self.assert_( abs( p.z ) < 0.138 )

		r["fileName"].setValue( IECore.StringData( "test/IECore/data/iffFiles/nParticleFrame2.mc" ) )

		r.parameters()['frameIndex'].setValue( 0 )
		self.assertEqual( r.numParticles(), 4 )

		expectedAttrNamesAndTypes = {
			"nParticleShape1_id" : IECore.FloatVectorData,
			"nParticleShape1_birthTime" : IECore.FloatVectorData,
			"P" : IECore.V3fVectorData,
			"nParticleShape1_lifespanPP" : IECore.FloatVectorData,
			"nParticleShape1_finalLifespanPP" : IECore.FloatVectorData,
			"nParticleShape1_velocity" : IECore.V3fVectorData,
		}

		c = r.read()
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), 4 )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.022 )
			self.assert_( abs( p.y ) < 0.017 )
			self.assert_( abs( p.z ) < 0.020 )

	def testNClothAsParticles( self ) :

		r = IECore.Reader.create( "test/IECore/data/iffFiles/nClothFrame3.mc" )
		self.assertEqual( type( r ), IECoreScene.NParticleReader )
		r.parameters()["realType"].setValue( "native" )

		self.assertEqual( r.numParticles(), 349 )
		self.assertEqual( len(r.frameTimes()), 1 )
		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"nClothShape1" : IECore.V3dVectorData,
		}
		self.assertEqual( len( attrNames ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in attrNames )

		c = r.read()
		self.assertEqual( type( c ), IECoreScene.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["nClothShape1"].data :
			self.assert_( abs( p.x ) < 320.50 )
			self.assert_( abs( p.y ) < 119.41 )
			self.assert_( abs( p.z ) < 554.64 )

	def testParameterTypes( self ) :

		p = IECoreScene.NParticleReader()
		self.assert_( p.resultParameter().isInstanceOf( "ObjectParameter" ) )
		self.assertEqual( p.resultParameter().validTypes(), [ IECoreScene.TypeId.PointsPrimitive ] )

if __name__ == "__main__":
	unittest.main()

