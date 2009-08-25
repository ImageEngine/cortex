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

class TestBGEOReader( unittest.TestCase ) :

	def testConstruction( self ) :
		
		r = IECore.Reader.create( "test/IECore/data/bgeoFiles/particleTest.0006.bgeo" )
		self.assert_( r.isInstanceOf( "ParticleReader" ) )
		self.assertEqual( type( r ), IECore.BGEOParticleReader )
		self.assertEqual( r["fileName"].getValue().value, "test/IECore/data/bgeoFiles/particleTest.0006.bgeo" )

	def testRead( self ) :
		
		r = IECore.Reader.create( "test/IECore/data/bgeoFiles/particleTest.0006.bgeo" )
		self.assertEqual( type( r ), IECore.BGEOParticleReader )

		self.assertEqual( r.numParticles(), 26 )
		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3fVectorData,
			"v" : IECore.V3fVectorData,
			"accel" : IECore.V3fVectorData,
			"life" : IECore.V2fVectorData,
			"pstate" : IECore.IntVectorData,
			"id" : IECore.IntVectorData,
			"parent" : IECore.IntVectorData,
			"spriteshop" : IECore.StringVectorData,
			"spriterot" : IECore.FloatVectorData,
			"spritescale" : IECore.V3fVectorData,			
		}
		self.assertEqual( len( attrNames ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in attrNames )

		c = r.read()
		self.assertEqual( type( c ), IECore.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.2 )
			self.assert_( abs( p.x ) < 0.2 )
			self.assert_( abs( p.z ) < 0.2 )
		
		for s in c["spriteshop"].data :
			self.assertEqual( s, '/shop/sprite/sprite' )

		self.assertEqual( c["id"].data, IECore.IntVectorData( range( 1, 27 ) ) )

	def testFiltering( self ) :
		
		r = IECore.Reader.create( "test/IECore/data/bgeoFiles/particleTest.0006.bgeo" )

		attributesToLoad = [ "P", "life" ]
		r.parameters()["percentage"].setValue( IECore.FloatData( 50 ) )
		r.parameters()["attributes"].setValue( IECore.StringVectorData( attributesToLoad ) )
		
		p = r.read()
		# what the acceptable thresholds should be are somewhat debatable,
		# especially for such a small number of particles
		self.assert_( p.numPoints < 15 )
		self.assert_( p.numPoints > 7 )
		for attr in attributesToLoad :
			self.assertEqual( p.numPoints, p[attr].data.size() )
		
		a = r.readAttribute( "P" )
		self.assert_( len( a ) < 15 )
		self.assert_( len( a ) > 7 )

	def testConversion( self ) :
		
		r = IECore.Reader.create( "test/IECore/data/bgeoFiles/particleTest.0006.bgeo" )
		self.assertEqual( type( r ), IECore.BGEOParticleReader )
		r.parameters()["realType"].setValue( "double" )
		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3dVectorData,
			"v" : IECore.V3dVectorData,
			"accel" : IECore.V3dVectorData,
			"life" : IECore.V2dVectorData,
			"pstate" : IECore.IntVectorData,
			"id" : IECore.IntVectorData,
			"parent" : IECore.IntVectorData,
			"spriteshop" : IECore.StringVectorData,
			"spriterot" : IECore.DoubleVectorData,
			"spritescale" : IECore.V3dVectorData,
		}
		
		c = r.read()
		self.assertEqual( type( c ), IECore.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.2 )
			self.assert_( abs( p.y ) < 0.2 )
			self.assert_( abs( p.z ) < 0.2 )

	def testFileNameChange( self ) :
		
		"""Now Readers are Ops, the filename can be changed and read() can be called
		again. So we need to check that that works."""

		r = IECore.Reader.create( "test/IECore/data/bgeoFiles/particleTest.0006.bgeo" )
		self.assertEqual( type( r ), IECore.BGEOParticleReader )

		r.parameters()["realType"].setValue( "float" )

		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"P" : IECore.V3fVectorData,
			"v" : IECore.V3fVectorData,
			"accel" : IECore.V3fVectorData,
			"life" : IECore.V2fVectorData,
			"pstate" : IECore.IntVectorData,
			"id" : IECore.IntVectorData,
			"parent" : IECore.IntVectorData,
			"spriteshop" : IECore.StringVectorData,
			"spriterot" : IECore.FloatVectorData,
			"spritescale" : IECore.V3fVectorData,
		}

		c = r.read()
		self.assertEqual( type( c ), IECore.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.2 )
			self.assert_( abs( p.y ) < 0.2 )
			self.assert_( abs( p.z ) < 0.2 )

		r["fileName"].setValue( IECore.StringData( "test/IECore/data/bgeoFiles/particleTest.0004.bgeo") )

		self.assertEqual( r.numParticles(), 17 )
		c = r.read()
		self.assertEqual( type( c ), IECore.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), 17 )

		for p in c["P"].data :
			self.assert_( abs( p.x ) < 0.12 )
			self.assert_( abs( p.y ) < 0.12 )
			self.assert_( abs( p.z ) < 0.12 )

	def testParameterTypes( self ) :
		
		p = IECore.BGEOParticleReader()
		self.assert_( p.resultParameter().isInstanceOf( "ObjectParameter" ) )
		self.assertEqual( p.resultParameter().validTypes(), [IECore.TypeId.PointsPrimitive] )

if __name__ == "__main__":
	unittest.main()

