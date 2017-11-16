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
import IECore

class TestPointVelocityDisplaceOp( unittest.TestCase ) :

	def test( self ) :

		pts = IECore.PointsPrimitive(0)
		vertex = IECore.PrimitiveVariable.Interpolation.Vertex

		# check calling with no arguments
		o = IECore.PointVelocityDisplaceOp()
		self.assertRaises( RuntimeError, o )

		# check not passing v is a passthru
		o = IECore.PointVelocityDisplaceOp()
		pts["P"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f(0) ] ) )
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True )

		# check it works
		o = IECore.PointVelocityDisplaceOp()
		pts["P"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f(0) ] ) )
		pts["v"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f(1) ] ) )
		p = o(input=pts)
		self.assertNotEqual( pts["P"].data, p["P"].data )
		self.assertEqual( p["P"].data[0], IECore.V3f(1) )
		p = o(input=pts, copyInput=False)
		self.assertEqual( pts["P"].data, p["P"].data )
		self.assertEqual( pts["P"].data[0], IECore.V3f(1) )

		# slightly more interesting example
		o = IECore.PointVelocityDisplaceOp()
		pts["P"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f( 1,2,3 ), IECore.V3f( 4,5,6 ) ] ) )
		pts["v"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f( 1 ), IECore.V3f( 2, 1, 3 ) ] ) )
		p = o(input=pts)
		self.assertEqual( p["P"].data, IECore.V3fVectorData([ IECore.V3f(2,3,4), IECore.V3f(6,6,9) ] ) )

		# check samplelength works
		o = IECore.PointVelocityDisplaceOp()
		pts["P"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f( 1,2,3 ), IECore.V3f( 4,5,6 ) ] ) )
		pts["v"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f( 1 ), IECore.V3f( 2, 1, 3 ) ] ) )
		p = o(input=pts, sampleLength=0.5)
		self.assertEqual( p["P"].data, IECore.V3fVectorData([ IECore.V3f(1.5,2.5,3.5), IECore.V3f(5,5.5,7.5) ] ) )

		# check that len(p)!=len(v) raises exception
		o = IECore.PointVelocityDisplaceOp()
		pts["P"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f(0), IECore.V3f(1), IECore.V3f(2) ] ) )
		pts["v"] = IECore.PrimitiveVariable( vertex, IECore.V3fVectorData( [ IECore.V3f( 1 ) ] ) )
		self.assertRaises( RuntimeError, o, input=pts )

		# check that it works with other primitives
		o = IECore.PointVelocityDisplaceOp()
		c = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(0), IECore.V3f(1) ) )
		self.assertEqual( len(c['P'].data), 8 )
		v = IECore.V3fVectorData( [] )
		v.resize( 8, IECore.V3f(1) )
		c['v'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, v )
		c2 = o(input=c)
		for i in range(8):
			self.assertEqual( c2['P'].data[i], c['P'].data[i] + c['v'].data[i] )

		# check that it works with pervertex samplelength
		o = IECore.PointVelocityDisplaceOp()
		s = IECore.FloatVectorData( [] )
		for i in range(8):
			s.append( 0.1 * i )
		c['s'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, s )
		c2 = o(input=c, sampleLengthVar="s")
		for i in range(8):
			self.assertEqual( c2['P'].data[i], c['P'].data[i] + (c['v'].data[i] * c['s'].data[i]) )

		# check that samplelength array length check raises
		o = IECore.PointVelocityDisplaceOp()
		s = IECore.FloatVectorData( [] )
		for i in range(4):
			s.append( 0.1 * i )
		c['s'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, s )
		self.assertRaises( RuntimeError, o, input=pts, sampleLengthVar="s" )

		# check that it works with different var names
		o = IECore.PointVelocityDisplaceOp()
		c = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(0), IECore.V3f(1) ) )
		IECore.MeshNormalsOp()( input=c, copyInput=False )
		self.assertTrue( "N" in c )
		c['bob'] = c['P']
		del c['P']
		self.assertEqual( len(c.keys()), 2 )
		c2 = o(input=c, positionVar="bob", velocityVar="N")
		for i in range(8):
			self.assertEqual( c2['bob'].data[i], c['bob'].data[i] + c['N'].data[i] )

if __name__ == "__main__":
	unittest.main()

