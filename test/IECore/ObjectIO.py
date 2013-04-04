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
from IECore import *

class TestObjectIO( unittest.TestCase ) :

	def testSimpleIO( self ) :

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )

		o = IntData( 1 )
		self.assertEqual( o.value, 1 )
		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );

		o = StringData( "hello" )
		self.assertEqual( o.value, "hello" )
		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );
		self.assertEqual( o, oo );

	def testSimpleArrayIO( self ) :

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )

		o = IntVectorData()
		for i in range( 0, 1000 ) :
			o.append( random.randint( -1000, 1000 ) )
		self.assertEqual( o.size(), 1000 )

		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( oo.size(), 1000 )

		for i in range( 0, 1000 ) :
			self.assertEqual( o[i], oo[i] )

		self.assertEqual( o, oo );

	def testStringArrayIO( self ) :

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )

		words = [ "hello", "there", "young", "fellah" ]
		s = StringVectorData( words )
		self.assertEqual( s.size(), len( words ) )
		for i in range( 0, s.size() ) :
			self.assertEqual( s[i], words[i] )

		s.save( iface, "test" )
		ss = Object.load( iface, "test" )
		self.assertEqual( ss.size(), s.size() )

		for i in range( 0, s.size() ) :
			self.assertEqual( s[i], ss[i] )

		self.assertEqual( s, ss );

	def testImathArrayIO( self ) :

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )

		o = V3fVectorData()
		o.setInterpretation( GeometricData.Interpretation.Vector )
		for i in range( 0, 1000 ) :
			o.append( V3f( i*3, i*3 + 1, i*3 + 2 ) )
		self.assertEqual( o.size(), 1000 )

		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( oo.size(), 1000 )

		for i in range( 0, 1000 ) :
			self.assertEqual( o[i], oo[i] )
		
		self.assertEqual( oo.getInterpretation(), GeometricData.Interpretation.Vector )
		self.assertEqual( o, oo )

	def testOverwrite( self ) :

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )

		o = IntData( 1 )
		self.assertEqual( o.value, 1 )

		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );

		# saving over an existing file should
		# obliterate it but currently doesn't
		o = StringData( "hello" )
		self.assertEqual( o.value, "hello" )
		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( o.value, oo.value );

		self.assertEqual( o, oo );

		d = CompoundData()
		d["A"] = IntData( 10 )
		d["B"] = StringData( "hithere" )

		d.save( iface, "test2" )		
		dd = Object.load( iface, "test2" )
		self.assertEqual( d, dd );

		# override CompoundData with less data
		d = CompoundData()
		d.save( iface, "test2" )		
		dd = Object.load( iface, "test2" )
		self.assertEqual( d, dd );

	def testCompoundData( self ) :

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )

		d = CompoundData()
		d["A"] = IntData( 10 )
		d["B"] = StringData( "hithere" )
		self.assertEqual( d["B"].value, "hithere" )

		d.save( iface, "test" )
		dd = Object.load( iface, "test" )
		self.assertEqual( d, dd )

	def testDataTypes( self ):

		d = CompoundData()
		d['a'] = IntData(1)
		d['c'] = FloatData(3)
		d['e'] = HalfData(4)
		d['f'] = V2iData( V2i(1,10) )
		d['g'] = V2fData( V2f(2,31), GeometricData.Interpretation.Vector )
		d['h'] = V2dData( V2d(3,551), GeometricData.Interpretation.Color )
		d['i'] = V3fData( V3f(1,3,5), GeometricData.Interpretation.Point )
		d['k'] = M44fData( M44f(2) )
		d['l'] = HalfVectorData( [ 1,2,3,100,9,10,11] )
		d['m'] = V2fVectorData( [ V2f(1,2), V2f(3,4), V2f(5,6) ], GeometricData.Interpretation.Normal )
		d['x'] = StringData( "testttt" )
		d['z'] = StringVectorData( [ "a", 'b', 'adffs' ] )

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )		
		d.save( iface, "test" )

		dd = Object.load( iface, "test" )
		self.assertEqual( d, dd )

	def testMultipleRef( self ) :

		iface = IndexedIO.create( "test/o.fio", [], IndexedIO.OpenMode.Write )

		d = CompoundData()
		i = IntData( 100 )
		d["ONE"] = i
		d["TWO"] = i

		self.assert_( d["ONE"].isSame( d["TWO"] ) )

		d.save( iface, "test" )

		dd = Object.load( iface, "test" )
		self.assertEqual( d, dd )
		self.assert_( dd["ONE"].isSame( dd["TWO"] ) )

	def testSaveInCurrentDir( self ) :

		o = CompoundData()
		one = IntData( 1 )
		o["one"] = one
		o["two"] = IntData( 2 )
		o["oneAgain"] = one

		fio = FileIndexedIO( "test/o.fio", [], IndexedIO.OpenMode.Write )
		fio = fio.subdirectory( "a", IndexedIO.MissingBehaviour.CreateIfMissing )
		d = fio.path()
		o.save( fio, "test" )
		self.assertEqual( fio.path(), d )
		del fio

		fio = FileIndexedIO( "test/o.fio", ["a"], IndexedIO.OpenMode.Read )
		d = fio.path()
		self.assertEqual( fio.path(), ["a"] )
		oo = o.load( fio, "test" )
		self.assertEqual( fio.path(), d )

		self.assertEqual( o, oo )

	def testSlashesInRepeatedData(self):
		"""Make sure slashes can be used and do not break the symlinks on the Object representation for repeated data."""
		f = FileIndexedIO("./test/FileIndexedIOSlashes.fio", [], IndexedIO.OpenMode.Write)

		v1 = IntData(10)
		v2 = IntData(11)
		v3 = IntData(12)

		d = CompoundData()
		d['a'] = CompoundData()
		d['a']['b'] = v1
		d['a/b'] = v2
		d['c'] = CompoundData()
		d['c']['d'] = v3
		d['c/d'] = v3
		d['links'] = CompoundData()
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
		f = FileIndexedIO("./test/FileIndexedIOSlashes.fio", [], IndexedIO.OpenMode.Read)
		dd = Object.load( f, "test" )

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

		c = CompoundData()

		for i in range( 0, 1000 ) :

			d = IntData( i )
			c[str(i)] = d
			c[str(i)+"SecondReference"] = d

		ObjectWriter( c, "test/emptyContainerOptimisation.cob" ).write()

		c1 = ObjectReader( "test/IECore/data/cobFiles/beforeEmptyContainerOptimisation.cob" ).read()
		c2 = ObjectReader( "test/emptyContainerOptimisation.cob" ).read()

		self.assertEqual( c1, c2 )

	def tearDown( self ) :

		if os.path.isfile( "test/emptyContainerOptimisation.cob" ) :
			os.remove( "test/emptyContainerOptimisation.cob" )

if __name__ == "__main__":
    unittest.main()
