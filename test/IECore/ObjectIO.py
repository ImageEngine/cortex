##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
import random
import IECore

class TestObjectIO( unittest.TestCase ) :

	def testSimpleIO( self ) :

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )

		o = IECore.IntData( 1 )
		self.assertEqual( o.value, 1 )
		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );

		o = IECore.StringData( "hello" )
		self.assertEqual( o.value, "hello" )
		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );
		self.assertEqual( o, oo );

	def testSimpleArrayIO( self ) :

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )

		o = IECore.IntVectorData()
		for i in range( 0, 1000 ) :
			o.append( random.randint( -1000, 1000 ) )
		self.assertEqual( o.size(), 1000 )

		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( oo.size(), 1000 )

		for i in range( 0, 1000 ) :
			self.assertEqual( o[i], oo[i] )

		self.assertEqual( o, oo );

	def testStringArrayIO( self ) :

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )

		words = [ "hello", "there", "young", "fellah" ]
		s = IECore.StringVectorData( words )
		self.assertEqual( s.size(), len( words ) )
		for i in range( 0, s.size() ) :
			self.assertEqual( s[i], words[i] )

		s.save( iface, "test" )
		ss = IECore.Object.load( iface, "test" )
		self.assertEqual( ss.size(), s.size() )

		for i in range( 0, s.size() ) :
			self.assertEqual( s[i], ss[i] )

		self.assertEqual( s, ss );

	def testImathArrayIO( self ) :

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )

		o = IECore.V3fVectorData()
		o.setInterpretation( IECore.GeometricData.Interpretation.Vector )
		for i in range( 0, 1000 ) :
			o.append( IECore.V3f( i*3, i*3 + 1, i*3 + 2 ) )
		self.assertEqual( o.size(), 1000 )

		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( oo.size(), 1000 )

		for i in range( 0, 1000 ) :
			self.assertEqual( o[i], oo[i] )

		self.assertEqual( oo.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertEqual( o, oo )

	def testOverwrite( self ) :

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )

		o = IECore.IntData( 1 )
		self.assertEqual( o.value, 1 )

		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );

		# saving over an existing file should
		# obliterate it but currently doesn't
		o = IECore.StringData( "hello" )
		self.assertEqual( o.value, "hello" )
		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );

		self.assertEqual( o, oo );

		d = IECore.CompoundData()
		d["A"] = IECore.IntData( 10 )
		d["B"] = IECore.StringData( "hithere" )

		d.save( iface, "test2" )
		dd = IECore.Object.load( iface, "test2" )
		self.assertEqual( d, dd );

		# override CompoundData with less data
		d = IECore.CompoundData()
		d.save( iface, "test2" )
		dd = IECore.Object.load( iface, "test2" )
		self.assertEqual( d, dd );

	def testCompoundData( self ) :

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )

		d = IECore.CompoundData()
		d["A"] = IECore.IntData( 10 )
		d["B"] = IECore.StringData( "hithere" )
		self.assertEqual( d["B"].value, "hithere" )

		d.save( iface, "test" )
		dd = IECore.Object.load( iface, "test" )
		self.assertEqual( d, dd )

	def testDataTypes( self ):

		d = IECore.CompoundData()
		d['a'] = IECore.IntData(1)
		d['c'] = IECore.FloatData(3)
		d['e'] = IECore.HalfData(4)
		d['f'] = IECore.V2iData( IECore.V2i(1,10) )
		d['g'] = IECore.V2fData( IECore.V2f(2,31), IECore.GeometricData.Interpretation.Vector )
		d['h'] = IECore.V2dData( IECore.V2d(3,551), IECore.GeometricData.Interpretation.Color )
		d['i'] = IECore.V3fData( IECore.V3f(1,3,5), IECore.GeometricData.Interpretation.Point )
		d['k'] = IECore.M44fData( IECore.M44f(2) )
		d['l'] = IECore.HalfVectorData( [ 1,2,3,100,9,10,11] )
		d['m'] = IECore.V2fVectorData( [ IECore.V2f(1,2), IECore.V2f(3,4), IECore.V2f(5,6) ], IECore.GeometricData.Interpretation.Normal )
		d['x'] = IECore.StringData( "testttt" )
		d['z'] = IECore.StringVectorData( [ "a", 'b', 'adffs' ] )

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )
		d.save( iface, "test" )

		dd = IECore.Object.load( iface, "test" )
		self.assertEqual( d, dd )

	def testMultipleRef( self ) :

		iface = IECore.IndexedIO.create( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )

		d = IECore.CompoundData()
		i = IECore.IntData( 100 )
		d["ONE"] = i
		d["TWO"] = i

		self.assert_( d["ONE"].isSame( d["TWO"] ) )

		d.save( iface, "test" )

		dd = IECore.Object.load( iface, "test" )
		self.assertEqual( d, dd )
		self.assert_( dd["ONE"].isSame( dd["TWO"] ) )

	def testSaveInCurrentDir( self ) :

		o = IECore.CompoundData()
		one = IECore.IntData( 1 )
		o["one"] = one
		o["two"] = IECore.IntData( 2 )
		o["oneAgain"] = one

		fio = IECore.FileIndexedIO( "test/o.fio", [], IECore.IndexedIO.OpenMode.Write )
		fio = fio.subdirectory( "a", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		d = fio.path()
		o.save( fio, "test" )
		self.assertEqual( fio.path(), d )
		del fio

		fio = IECore.FileIndexedIO( "test/o.fio", ["a"], IECore.IndexedIO.OpenMode.Read )
		d = fio.path()
		self.assertEqual( fio.path(), ["a"] )
		oo = o.load( fio, "test" )
		self.assertEqual( fio.path(), d )

		self.assertEqual( o, oo )

	def testSlashesInRepeatedData(self):
		"""Make sure slashes can be used and do not break the symlinks on the Object representation for repeated data."""
		f = IECore.FileIndexedIO("./test/FileIndexedIOSlashes.fio", [], IECore.IndexedIO.OpenMode.Write)

		v1 = IECore.IntData(10)
		v2 = IECore.IntData(11)
		v3 = IECore.IntData(12)

		d = IECore.CompoundData()
		d['a'] = IECore.CompoundData()
		d['a']['b'] = v1
		d['a/b'] = v2
		d['c'] = IECore.CompoundData()
		d['c']['d'] = v3
		d['c/d'] = v3
		d['links'] = IECore.CompoundData()
		d['links']['v1'] = v1
		d['links']['v2'] = v2
		d['links']['v3'] = v3

		# sanity check
		self.assert_( d['a']['b'].isSame( d['links']['v1'] ) )
		self.assert_( d['a/b'].isSame( d['links']['v2'] ) )
		self.assert_( d['c']['d'].isSame( d['links']['v3'] ) )
		self.assert_( d['c/d'].isSame( d['links']['v3'] ) )

		d.save( f, "test" )

		f = None
		f = IECore.FileIndexedIO("./test/FileIndexedIOSlashes.fio", [], IECore.IndexedIO.OpenMode.Read)
		dd = IECore.Object.load( f, "test" )

		self.assertEqual( d, dd )
		self.assert_( dd['a']['b'].isSame( dd['links']['v1'] ) )
		self.assert_( dd['a/b'].isSame( dd['links']['v2'] ) )
		self.assert_( dd['c']['d'].isSame( dd['links']['v3'] ) )
		self.assert_( dd['c/d'].isSame( dd['links']['v3'] ) )

	def tearDown( self ) :

		for f in [ "test/o.fio", "test/FileIndexedIOSlashes.fio" ] :
			if os.path.isfile( f ) :
				os.remove( f )


class TestEmptyContainerOptimisation( unittest.TestCase ) :

	"""Many of the base classes in IECore currently have no data to store
	in files, but were being allocated a container anyway. This test verifies
	that an io optimisation that doesn't create empty containers doesn't have
	any bad side effects. It's also useful to test the impact of the optimisation."""

	def test( self ) :

		c = IECore.CompoundData()

		for i in range( 0, 1000 ) :

			d = IECore.IntData( i )
			c[str(i)] = d
			c[str(i)+"SecondReference"] = d

		IECore.ObjectWriter( c, "test/emptyContainerOptimisation.cob" ).write()

		c1 = IECore.ObjectReader( "test/IECore/data/cobFiles/beforeEmptyContainerOptimisation.cob" ).read()
		c2 = IECore.ObjectReader( "test/emptyContainerOptimisation.cob" ).read()

		self.assertEqual( c1, c2 )

	def tearDown( self ) :

		if os.path.isfile( "test/emptyContainerOptimisation.cob" ) :
			os.remove( "test/emptyContainerOptimisation.cob" )

if __name__ == "__main__":
    unittest.main()
