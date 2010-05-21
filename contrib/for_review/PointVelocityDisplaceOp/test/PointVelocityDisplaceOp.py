##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios), 
#  its affiliates and/or its licensors.
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
from IECore import *

class TestPointVelocityDisplaceOp( unittest.TestCase ) :

	def test( self ) :

		pts = PointsPrimitive(0)
		vertex = PrimitiveVariable.Interpolation.Vertex

		# check calling with no arguments
		o = PointVelocityDisplaceOp()
		p = o()
		self.assert_( p.isInstanceOf( PointsPrimitive.staticTypeId() ) )
		self.assertEqual( len(p), 0 )

		# check not passing v is a passthru
		o = PointVelocityDisplaceOp()
		pts["P"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f(0) ] ) )
		p = o( input=pts, copyInput=True )
		self.assertEqual( p, pts )
		p = o( input=pts )		
		self.assertEqual( p, pts )
		self.assertTrue( p.isInstanceOf( PointsPrimitive.staticTypeId() ) )
		self.assertEqual( len(p), 1 )

		# check it works
		o = PointVelocityDisplaceOp()
		pts["P"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f(0) ] ) )
		pts["v"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f(1) ] ) )
		p = o(input=pts)
		self.assertNotEqual( pts["P"].data, p["P"].data )
		self.assertEqual( p["P"].data[0], V3f(1) )
		p = o(input=pts, copyInput=False)
		self.assertEqual( pts["P"].data, p["P"].data )
		self.assertEqual( pts["P"].data[0], V3f(1) )

		# slightly more interesting example
		o = PointVelocityDisplaceOp()
		pts["P"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f( 1,2,3 ), V3f( 4,5,6 ) ] ) )
		pts["v"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f( 1 ), V3f( 2, 1, 3 ) ] ) )
		p = o(input=pts)
		self.assertEqual( p["P"].data, V3fVectorData([ V3f(2,3,4), V3f(6,6,9) ] ) )

		# check samplelength works
		o = PointVelocityDisplaceOp()
		pts["P"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f( 1,2,3 ), V3f( 4,5,6 ) ] ) )
		pts["v"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f( 1 ), V3f( 2, 1, 3 ) ] ) )
		p = o(input=pts, samplelength=0.5)
		self.assertEqual( p["P"].data, V3fVectorData([ V3f(1.5,2.5,3.5), V3f(5,5.5,7.5) ] ) )

		# check that len(p)!=len(v) is a passthru
		o = PointVelocityDisplaceOp()
		pts["P"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f(0), V3f(1), V3f(2) ] ) )
		pts["v"] = PrimitiveVariable( vertex, V3fVectorData( [ V3f( 1 ) ] ) )
		p = o(input=pts)
		self.assertEqual( p["P"].data, V3fVectorData( [ V3f(0), V3f(1), V3f(2) ] ) )


if __name__ == "__main__":
	unittest.main()

