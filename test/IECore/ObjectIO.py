##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

		iface = IndexedIOInterface.create( "test/o.fio", "/", IndexedIOOpenMode.Write )

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

		iface = IndexedIOInterface.create( "test/o.fio", "/", IndexedIOOpenMode.Write )

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

		iface = IndexedIOInterface.create( "test/o.fio", "/", IndexedIOOpenMode.Write )

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

		iface = IndexedIOInterface.create( "test/o.fio", "/", IndexedIOOpenMode.Write )

		o = V3fVectorData()
		for i in range( 0, 1000 ) :
			o.append( V3f( i*3, i*3 + 1, i*3 + 2 ) )
		self.assertEqual( o.size(), 1000 )

		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( oo.size(), 1000 )

		for i in range( 0, 1000 ) :
			self.assertEqual( o[i], oo[i] )

		self.assertEqual( o, oo );

	def testOverwrite( self ) :

		iface = IndexedIOInterface.create( "test/o.fio", "/", IndexedIOOpenMode.Write )

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

	def testCompoundData( self ) :

		iface = IndexedIOInterface.create( "test/o.fio", "/", IndexedIOOpenMode.Write )

		d = CompoundData()
		d["A"] = IntData( 10 )
		d["B"] = StringData( "hithere" )
		self.assertEqual( d["B"].value, "hithere" )

		d.save( iface, "test" )
		dd = Object.load( iface, "test" )
		self.assertEqual( d, dd )

	def testMultipleRef( self ) :

		iface = IndexedIOInterface.create( "test/o.fio", "/", IndexedIOOpenMode.Write )

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

		fio = FileIndexedIO( "test/o.fio", "/", IndexedIOOpenMode.Write )
		fio.mkdir( "a" )
		fio.chdir( "a" )
		d = fio.pwd()
		o.save( fio, "test" )
		self.assertEqual( fio.pwd(), d )
		del fio

		fio = FileIndexedIO( "test/o.fio", "/", IndexedIOOpenMode.Read )
		fio.chdir( "a" )
		d = fio.pwd()
		oo = o.load( fio, "test" )
		self.assertEqual( fio.pwd(), d )

		self.assertEqual( o, oo )

	def tearDown( self ) :

		for f in [ "test/o.fio" ] :
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
