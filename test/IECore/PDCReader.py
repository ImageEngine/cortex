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
import sys
import IECore

class TestPDCReader( unittest.TestCase ) :

	def testConstruction( self ) :
	
		r = IECore.Reader.create( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assert_( r.isInstanceOf( "ParticleReader" ) )
		self.assertEqual( type( r ), IECore.PDCParticleReader )
		self.assertEqual( r.fileName.getValue().value, "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		
	def testRead( self ) :
	
		r = IECore.Reader.create( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( type( r ), IECore.PDCParticleReader )
		
		self.assertEqual( r.numParticles(), 25 )
		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"particleId" : IECore.DoubleVectorData,
			"mass" : IECore.DoubleVectorData,
			"lastWorldVelocity" : IECore.V3dVectorData,
			"worldVelocityInObjectSpace" : IECore.V3dVectorData,
			"worldVelocity" : IECore.V3dVectorData,
			"lastWorldPosition" : IECore.V3dVectorData,
			"worldPosition" : IECore.V3dVectorData,
			"acceleration" : IECore.V3dVectorData,
			"lastVelocity" : IECore.V3dVectorData,
			"velocity" : IECore.V3dVectorData,
			"lastPosition" : IECore.V3dVectorData,
			"position" : IECore.V3dVectorData,
			"lifespanPP" : IECore.DoubleVectorData,
			"finalLifespanPP" : IECore.DoubleVectorData,
			"emitterId" : IECore.DoubleVectorData,
			"birthTime" : IECore.DoubleVectorData,
			"age" : IECore.DoubleVectorData,
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
			
		for p in c["position"].data :
			self.assertEqual( p.y, 0 )
			self.assert_( abs( p.x ) < 1.1 )
			self.assert_( abs( p.z ) < 1.1 )
			
		self.assertEqual( c["particleId"].data, IECore.DoubleVectorData( range( 0, 25 ) ) )
			
	def testFiltering( self ) :
	
		r = IECore.Reader.create( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		
		r.parameters().percentage.setValue( IECore.FloatData( 50 ) )
		
		a = r.readAttribute( "position" )
		# what the acceptable thresholds should be are somewhat debatable,
		# especially for such a small number of particles
		self.assert_( len( a ) < 13 )
		self.assert_( len( a ) > 7 )
		
	def testConversion( self ) :
	
		r = IECore.Reader.create( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( type( r ), IECore.PDCParticleReader )
		
		r.parameters().realType.setValue( "float" )

		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"particleId" : IECore.FloatVectorData,
			"mass" : IECore.FloatVectorData,
			"lastWorldVelocity" : IECore.V3fVectorData,
			"worldVelocityInObjectSpace" : IECore.V3fVectorData,
			"worldVelocity" : IECore.V3fVectorData,
			"lastWorldPosition" : IECore.V3fVectorData,
			"worldPosition" : IECore.V3fVectorData,
			"acceleration" : IECore.V3fVectorData,
			"lastVelocity" : IECore.V3fVectorData,
			"velocity" : IECore.V3fVectorData,
			"lastPosition" : IECore.V3fVectorData,
			"position" : IECore.V3fVectorData,
			"lifespanPP" : IECore.FloatVectorData,
			"finalLifespanPP" : IECore.FloatVectorData,
			"emitterId" : IECore.FloatVectorData,
			"birthTime" : IECore.FloatVectorData,
			"age" : IECore.FloatVectorData,
		}
		
		c = r.read()
		self.assertEqual( type( c ), IECore.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )
			
		for p in c["position"].data :
			self.assertEqual( p.y, 0 )
			self.assert_( abs( p.x ) < 1.1 )
			self.assert_( abs( p.z ) < 1.1 )
			
	def testFileNameChange( self ) :
	
		"""Now Readers are Ops, the filename can be changed and read() can be called
		again. So we need to check that that works."""
	
		r = IECore.Reader.create( "test/IECore/data/pdcFiles/particleShape1.250.pdc" )
		self.assertEqual( type( r ), IECore.PDCParticleReader )
		
		r.parameters().realType.setValue( "float" )

		attrNames = r.attributeNames()
		expectedAttrNamesAndTypes = {
			"particleId" : IECore.FloatVectorData,
			"mass" : IECore.FloatVectorData,
			"lastWorldVelocity" : IECore.V3fVectorData,
			"worldVelocityInObjectSpace" : IECore.V3fVectorData,
			"worldVelocity" : IECore.V3fVectorData,
			"lastWorldPosition" : IECore.V3fVectorData,
			"worldPosition" : IECore.V3fVectorData,
			"acceleration" : IECore.V3fVectorData,
			"lastVelocity" : IECore.V3fVectorData,
			"velocity" : IECore.V3fVectorData,
			"lastPosition" : IECore.V3fVectorData,
			"position" : IECore.V3fVectorData,
			"lifespanPP" : IECore.FloatVectorData,
			"finalLifespanPP" : IECore.FloatVectorData,
			"emitterId" : IECore.FloatVectorData,
			"birthTime" : IECore.FloatVectorData,
			"age" : IECore.FloatVectorData,
		}
		
		c = r.read()
		self.assertEqual( type( c ), IECore.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), r.numParticles() )
			
		for p in c["position"].data :
			self.assertEqual( p.y, 0 )
			self.assert_( abs( p.x ) < 1.1 )
			self.assert_( abs( p.z ) < 1.1 )
			
		r.fileName.setValue( IECore.StringData( "test/IECore/data/pdcFiles/10Particles.pdc" ) )
		
		self.assertEqual( r.numParticles(), 10 )
		c = r.read()
		self.assertEqual( type( c ), IECore.PointsPrimitive )
		self.assertEqual( len( c ), len( expectedAttrNamesAndTypes ) )
		for i in expectedAttrNamesAndTypes.keys() :
			self.assert_( i in c )
			self.assertEqual( type(c[i].data), expectedAttrNamesAndTypes[i] )
			self.assertEqual( len(c[i].data), 10 )
			
		for p in c["position"].data :
			self.assertEqual( p.y, 0 )
			self.assert_( abs( p.x ) < 1.1 )
			self.assert_( abs( p.z ) < 1.1 )
			
	def testParameterTypes( self ) :
	
		p = IECore.PDCParticleReader()
		self.assert_( p.resultParameter().isInstanceOf( "ObjectParameter" ) )
		self.assertEqual( p.resultParameter().validTypes(), [IECore.TypeId.PointsPrimitive] )
				
if __name__ == "__main__":
	unittest.main()   
	
