##########################################################################
#
#  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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
import IECoreScene

class DiskPrimitiveTest( unittest.TestCase ) :

	def testConstructorAndAccessors( self ) :

		p = IECoreScene.DiskPrimitive()
		self.assertEqual( p.getRadius(), 1 )
		self.assertEqual( p.getZ(), 0 )
		self.assertEqual( p.getThetaMax(), 360 )

		p = IECoreScene.DiskPrimitive( 2, 1, 180 )
		self.assertEqual( p.getRadius(), 2 )
		self.assertEqual( p.getZ(), 1 )
		self.assertEqual( p.getThetaMax(), 180 )

		p.setRadius( 1 )
		p.setZ( 0 )
		p.setThetaMax( 360 )
		self.assertEqual( p.getRadius(), 1 )
		self.assertEqual( p.getZ(), 0 )
		self.assertEqual( p.getThetaMax(), 360 )

	def testCopyAndEquality( self ) :

		p = IECoreScene.DiskPrimitive( 2, 1, 180 )
		pp = p.copy()
		self.assertEqual( p, pp )

	def testSaveAndLoad( self ) :

		p = IECoreScene.DiskPrimitive( 2, 1, 180 )

		io = IECore.IndexedIO.create( os.path.join( "test", "IECore", "disk.fio" ), IECore.IndexedIO.OpenMode.Write )
		p.save( io, "test" )
		pp = IECore.Object.load( io, "test" )

		self.assertEqual( p, pp )

	def tearDown( self ) :

		if os.path.isfile( os.path.join( "test", "IECore", "disk.fio" ) ) :
			os.remove( os.path.join( "test", "IECore", "disk.fio" ) )

if __name__ == "__main__":
	unittest.main()
