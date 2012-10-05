##########################################################################
#
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

class ModelCacheTest( unittest.TestCase ) :

	def testAppendRaises( self ) :
	
		self.assertRaises( RuntimeError, IECore.ModelCache, "/tmp/test.mdc", IECore.IndexedIOOpenMode.Append )

	def testReadNonExistentRaises( self ) :
	
		self.assertRaises( RuntimeError, IECore.ModelCache, "iDontExist.mdc", IECore.IndexedIOOpenMode.Read )
		
	def testKnownHierarchy( self ) :
	
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Write )
		self.assertEqual( m.path(), "/" )
		
		t = m.writableChild( "t" )
		self.assertEqual( t.path(), "/t" )
		self.assertEqual( m.childNames(), [ "t" ] )
		
		t.writeTransform( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) )
		
		s = t.writableChild( "s" )
		self.assertEqual( t.childNames(), [ "s" ] )

		s.writeObject( IECore.SpherePrimitive( 1 ) )
		
		# need to delete all ModelCache references to finalise the file
		del m, t, s

		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Read )
		
		self.assertEqual( m.childNames(), [ "t" ] )
		self.assertEqual( m.readBound(), IECore.Box3d( IECore.V3d( 0, -1, -1 ), IECore.V3d( 2, 1, 1 ) ) )
		self.assertEqual( m.readTransform(), IECore.M44d() )
		self.assertEqual( m.readObject(), None )
		
		t = m.readableChild( "t" )
		
		self.assertEqual( t.childNames(), [ "s" ] )
		self.assertEqual( t.readBound(), IECore.Box3d( IECore.V3d( -1, -1, -1 ), IECore.V3d( 1, 1, 1 ) ) )
		self.assertEqual( t.readTransform(), IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) )
		self.assertEqual( t.readObject(), None )

		s = t.readableChild( "s" )
		
		self.assertEqual( s.childNames(), [] )
		self.assertEqual( s.readBound(), IECore.Box3d( IECore.V3d( -1, -1, -1 ), IECore.V3d( 1, 1, 1 ) ) )
		self.assertEqual( s.readTransform(), IECore.M44d.createTranslated( IECore.V3d( 0, 0, 0 ) ) )
		self.assertEqual( s.readObject(), IECore.SpherePrimitive( 1 ) )
	
	def testRandomHierarchy( self ) :
	
		r = IECore.Rand48()
				
		def writeWalk( m ) :
					
			if r.nextf( 0.0, 1.0 ) < 0.5 :
				m.writeObject( IECore.SpherePrimitive( r.nextf( 1.0, 4.0 ) ) )
		
			if r.nextf( 0.0, 1.0 ) < 0.5 :
				m.writeTransform( IECore.M44d.createTranslated( r.nextV3d() ) )
		
			thisDepth = int( r.nextf( 2, 5 ) )
			if thisDepth > m.path().count( "/" ) :
				numChildren = int( r.nextf( 4, 50 ) )
				for i in range( 0, numChildren ) :
					mc = m.writableChild( str( i ) )
					writeWalk( mc )
		
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Write )
		writeWalk( m )
		del m
		
		def readWalk( m, parentSpaceBound ) :
					
			localSpaceBound = IECore.Box3d()
			for childName in m.childNames() :
				readWalk( m.readableChild( childName ), localSpaceBound )
				
			o = m.readObject()
			if o is not None :
				ob = o.bound()
				ob = IECore.Box3d( IECore.V3d( *ob.min ), IECore.V3d( *ob.max ) )
				localSpaceBound.extendBy( ob ) 

			self.failUnless( m.readBound().contains( localSpaceBound ) )
			
			transformedBound = localSpaceBound.transform( m.readTransform() )
			parentSpaceBound.extendBy( transformedBound )

		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Read )
		readWalk( m, IECore.Box3d() )
								
	def testMissingReadableChildRaises( self ) :
	
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Write )
		m.writableChild( "a" )
		del m
		
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Read )
		m.readableChild( "a" )
		self.assertRaises( RuntimeError, m.readableChild, "b" )
		
	def testExplicitBoundOverridesImplicitBound( self ) :
			
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Write )

		a = m.writableChild( "a" )
		a.writeBound( IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 10 ) ) )
		a.writeObject( IECore.SpherePrimitive( 0.1 ) )
		
		b = a.writableChild( "b" )
		b.writeObject( IECore.SpherePrimitive( 100 ) )
		
		del m, a, b
		
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Read )
		
		a = m.readableChild( "a" )
		self.assertEqual( a.readBound(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 10 ) ) )
		
		b = a.readableChild( "b" )
		self.assertEqual( b.readBound(), IECore.Box3d( IECore.V3d( -100 ), IECore.V3d( 100 ) ) )
	
	def testExplicitBoundPropagatesToImplicitBound( self ) :
			
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Write )
		
		a = m.writableChild( "a" )
				
		b = a.writableChild( "b" )
		b.writeBound( IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		b.writeObject( IECore.SpherePrimitive( 100 ) )
		
		del m, a, b
		
		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIOOpenMode.Read )
		self.assertEqual( m.readBound(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		
		a = m.readableChild( "a" )
		self.assertEqual( a.readBound(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		
		b = a.readableChild( "b" )
		self.assertEqual( b.readBound(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		
if __name__ == "__main__":
	unittest.main()

