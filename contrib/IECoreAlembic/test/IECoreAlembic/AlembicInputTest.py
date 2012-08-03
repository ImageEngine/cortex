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

import os
import unittest

import IECore
import IECoreAlembic

class AlembicInputTest( unittest.TestCase ) :

	def testConstructor( self ) :

		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		self.assertRaises( Exception, IECoreAlembic.AlembicInput, "iDontExist" )
	
	def testHierarchy( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		self.assertEqual( a.numChildren(), 1 )
		self.assertEqual( a.childNames(), IECore.StringVectorData( [ "group1" ] ) )
				
		g1 = a.child( 0 )
		g2 = a.child( "group1" )
		
		self.assertEqual( g1.name(), g2.name() )
		self.assertEqual( g1.name(), "group1" )
		
		self.assertEqual( g1.fullName(), g2.fullName() )
		self.assertEqual( g1.fullName(), "/group1" )
		
		self.assertEqual( g1.numChildren(), 1 )
		self.assertEqual( g1.childNames(), IECore.StringVectorData( [ "pCube1" ] ) )
		
		c = g1.child( 0 )
		self.assertEqual( c.name(), "pCube1" )
		self.assertEqual( c.fullName(), "/group1/pCube1" )
		
		self.assertEqual( c.numChildren(), 1 )
		self.assertEqual( c.childNames(), IECore.StringVectorData( [ "pCubeShape1" ] ) )
		
		cs = c.child( 0 )
		self.assertEqual( cs.numChildren(), 0 )
		self.assertEqual( cs.childNames(), IECore.StringVectorData() )
		
		self.assertRaises( Exception, cs.child, 0 )
		self.assertRaises( Exception, cs.child, "iDontExist" )
						
	def testConvertMesh( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		c = a.child( "group1" ).child( "pCube1" )
		self.assertEqual( c.convert( IECore.MeshPrimitive.staticTypeId() ), None )
		
		cs = c.child( "pCubeShape1" )
		m = cs.convert( IECore.MeshPrimitive.staticTypeId() )
		
		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )
		
	def testConvertTransform( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		g = a.child( "group1" )
		t = g.convert( IECore.M44fData.staticTypeId() )
		self.assertEqual( t, IECore.M44fData( IECore.M44f.createScaled( IECore.V3f( 2 ) ) * IECore.M44f.createTranslated( IECore.V3f( 2, 0, 0 ) ) ) )
		
		c = a.child( "group1" ).child( "pCube1" )
		t = c.convert( IECore.M44fData.staticTypeId() )
		self.assertEqual( t, IECore.M44fData( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) ) )
		
		cs = c.child( "pCubeShape1" )
		t = cs.convert( IECore.M44fData.staticTypeId() )
		self.assertEqual( t, None )
		
	def testMetaData( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		m = a.metaData()
		self.failUnless( isinstance( m , IECore.CompoundData ) )
		
	def testBound( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		self.assertEqual( a.bound(), IECore.Box3d( IECore.V3d( -2 ), IECore.V3d( 2 ) ) )
		
		g = a.child( "group1" )
		self.assertEqual( g.bound(), IECore.Box3d( IECore.V3d( -2, -1, -1 ), IECore.V3d( 0, 1, 1 ) ) )
		
		c = g.child( "pCube1" )
		self.assertEqual( c.bound(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		
		cs = c.child( "pCubeShape1" )
		self.assertEqual( cs.bound(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )		
	
	def testTransform( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		self.assertEqual( a.transform(), IECore.M44d() )
				
		g = a.child( "group1" )
		self.assertEqual( g.transform(), IECore.M44d.createScaled( IECore.V3d( 2 ) ) * IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) )
		
		c = g.child( "pCube1" )
		self.assertEqual( c.transform(), IECore.M44d.createTranslated( IECore.V3d( -1, 0, 0 ) ) )
		
		cs = c.child( "pCubeShape1" )
		self.assertEqual( cs.transform(), IECore.M44d() )		
	
if __name__ == "__main__":
    unittest.main()
