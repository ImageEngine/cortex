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
import imath
import IECore
import IECoreScene


class PointSmoothSkinningOpTest( unittest.TestCase ) :

	def mySSD (self):
		# return an ssd for testing
		ok_jn = IECore.StringVectorData( [ 'joint1', 'joint2', 'joint3' ] )
		ok_ip = IECore.M44fVectorData( [imath.M44f().translate(imath.V3f(0,2,0)), imath.M44f(), imath.M44f().translate(imath.V3f(0,-2,0))] )
		ok_pio = IECore.IntVectorData( [0, 2, 4, 6, 8, 10, 12, 14] )
		ok_pic = IECore.IntVectorData( [2, 2, 2, 2, 2, 2, 2, 2] )
		ok_pii = IECore.IntVectorData( [0, 1, 0, 1, 1, 2, 1, 2, 1, 2, 1, 2, 0, 1, 0, 1] )
		ok_piw = IECore.FloatVectorData( [1, 0, 1, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 0, 1, 0] )
		ssd = IECoreScene.SmoothSkinningData(ok_jn, ok_ip, ok_pio, ok_pic, ok_pii, ok_piw)
		ssd.validate()
		return ssd

	def myN (self):
		# return an n for testing
		n = IECore.V3fVectorData( [ imath.V3f(0, -1, 0), imath.V3f(0, -1, 0 ), imath.V3f(0, 1, 0 ), imath.V3f(0, 1, 0  ),
							imath.V3f(0, 1, 0 ), imath.V3f(0, 1, 0  ), imath.V3f(0, -1, 0), imath.V3f(0, -1, 0)] )
		return n

	def myP (self):
		# return an p for testing
		p = IECore.V3fVectorData( [ imath.V3f(-0.5, -2, 0.5), imath.V3f(0.5, -2, 0.5 ), imath.V3f(-0.5, 2, 0.5 ), imath.V3f(0.5, 2, 0.5 ),
							imath.V3f(-0.5, 2, -0.5), imath.V3f(0.5, 2, -0.5 ), imath.V3f(-0.5, -2, -0.5), imath.V3f(0.5, -2, -0.5)] )
		return p

	def myPP (self):
		# return a point primitive for testing
		pts = IECoreScene.PointsPrimitive( self.myP() )
		vertex = IECoreScene.PrimitiveVariable.Interpolation.Vertex
		pts["N"] = IECoreScene.PrimitiveVariable( vertex, self.myN() )
		self.assertTrue( pts.arePrimitiveVariablesValid() )
		return pts

	def myMN (self):
		# face varying n
		n = IECore.V3fVectorData( [ imath.V3f( 0, 0, 1 ), imath.V3f( 0, 0, 1 ), imath.V3f( 0, 0, 1 ), imath.V3f( 0, 0, 1 ),
							imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 1, 0 ),
							imath.V3f( 0, 0, -1 ), imath.V3f( 0, 0, -1 ), imath.V3f( 0, 0, -1 ), imath.V3f( 0, 0, -1 ),
							imath.V3f( 0, -1, 0 ), imath.V3f( 0, -1, 0 ), imath.V3f( 0, -1, 0 ), imath.V3f( 0, -1, 0 ),
							imath.V3f( 1, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 0, 0 ),
							imath.V3f( -1, 0, 0 ), imath.V3f( -1, 0, 0 ), imath.V3f( -1, 0, 0 ), imath.V3f( -1, 0, 0 ) ] )
		return n

	def myMP (self):
		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 3, 2, 2, 3, 5, 4, 4, 5, 7, 6, 6, 7, 1, 0, 1, 7, 5, 3, 6, 0, 2, 4 ] )
		m = IECoreScene.MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, self.myP() )
		m["N"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, self.myMN() )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		return m

	def myDP (self):
		# return a deformation pose for testing (2nd joint roated by 90deg in z)
		dp = IECore.M44fVectorData( [ imath.M44f( 1, 0, 0, 0, -0, 1, -0, 0, 0, -0, 1, -0, -0, -2, -0, 1 ),
							imath.M44f( 0, 1, 0, -0, -1, 0, -0, 0, 0, -0, 1, -0, -0, 0, -0, 1 ),
							imath.M44f( 0, 1, 0, 0, -1, 0, -0, 0, 0, -0, 1, -0, -0, -2, -0, 1 ) ] )
		return dp


	def testNoArg( self ) :
		# check calling with no arguments
		o = IECoreScene.PointSmoothSkinningOp()
		self.assertRaises( RuntimeError, o )

	def testDeformation( self ) :
		# check that it deforms P and N as expected
		pts = self.myPP()

		o = IECoreScene.PointSmoothSkinningOp()
		o( input = pts, copyInput=False, deformationPose = self.myDP(), smoothSkinningData = self.mySSD(), deformNormals=True)
		self.assertNotEqual(pts["P"].data , self.myP())
		self.assertNotEqual(pts["N"].data , self.myN())

		#test a sample p was deformed as expexted
		#print pts["P"].data
		#self.assertNotEqual(pts["P"].data[2] , V3f((1, -0.5, 0.5) ))
		#test a sample n was deformed as expected
		#self.assertNotEqual(pts["N"].data[2] , V3f( (1, -0.5, 0.5)  ))

	def testMissingP( self ) :
		# check not passing p is a passthru
		pts = IECoreScene.PointsPrimitive(0)
		vertex = IECoreScene.PrimitiveVariable.Interpolation.Vertex
		o = IECoreScene.PointSmoothSkinningOp()
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						deformationPose = self.myDP(), smoothSkinningData = self.mySSD() )

	def testMissingN( self ) :
		pts = IECoreScene.PointsPrimitive(0)
		vertex = IECoreScene.PrimitiveVariable.Interpolation.Vertex
		pts["P"] = IECoreScene.PrimitiveVariable( vertex, self.myP() )
		o = IECoreScene.PointSmoothSkinningOp()
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						deformationPose = self.myDP(), smoothSkinningData = self.mySSD(), deformNormals = True )

	def testMissingSSD( self ) :
		# test if the ssd is missing
		pts = self.myPP()
		o = IECoreScene.PointSmoothSkinningOp()
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						deformationPose = self.myDP() )

	def testMissingDeformationPose( self ) :
		# test if the def pose is missing
		pts = self.myPP()
		o = IECoreScene.PointSmoothSkinningOp()
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						smoothSkinningData = self.mySSD())
#
	def testBadSSD( self ) :
		pts = self.myPP()
		o = IECoreScene.PointSmoothSkinningOp()

		# test if the ssd is not matching the deformationPose
		ssd = self.mySSD()
		a = ssd.influencePose()
		a.append(imath.M44f())
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						smoothSkinningData = ssd, deformationPose = self.myDP())

		# test if the ssd is not matching the p
		ssd = self.mySSD()
		a = ssd.pointInfluenceCounts()
		a.append(2)
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						smoothSkinningData = ssd,deformationPose = self.myDP())

		# test for bad ssd
		ssd = self.mySSD()
		a = ssd.influenceNames()
		a.append("bob")
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						smoothSkinningData = ssd,deformationPose = self.myDP())

	def testBadDeformationPose( self ) :
		pts = self.myPP()
		o = IECoreScene.PointSmoothSkinningOp()

		# test if the ssd is not matching the deformationPose
		dp = self.myDP()
		dp.append(imath.M44f())

		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						smoothSkinningData = self.mySSD(), deformationPose = dp)

	def testPrimVarLengthMismatch( self ) :
		# check that len(p)!=len(n) raises exception
		pts = IECoreScene.PointsPrimitive(0)
		vertex = IECoreScene.PrimitiveVariable.Interpolation.Vertex
		pts["P"] = IECoreScene.PrimitiveVariable( vertex, self.myP() )
		pts["N"] = IECoreScene.PrimitiveVariable( vertex, IECore.V3fVectorData( [ imath.V3f(0, -1, 0)] ) )
		o = IECoreScene.PointSmoothSkinningOp()
		self.assertRaises( RuntimeError, o, input=pts, copyInput=True,
						deformationPose = self.myDP(), smoothSkinningData = self.mySSD(), deformNormals = True )

	def testOtherPrimitives( self ) :
		# check that it works with other primitives, meshPrim
		mp = self.myMP()
		o = IECoreScene.PointSmoothSkinningOp()
		# test it works
		o( input=mp, copyInput=False, deformationPose = self.myDP(), smoothSkinningData = self.mySSD(), deformNormals = False )
		self.assertNotEqual(mp["P"].data , self.myP())
		self.assertEqual(mp["N"].data , self.myMN())

		o( input=mp, copyInput=False, deformationPose = self.myDP(), smoothSkinningData = self.mySSD(), deformNormals = True )
		self.assertNotEqual(mp["P"].data , self.myP())
		self.assertNotEqual(mp["N"].data , self.myMN())


	def testDifferentVarNames( self ) :
		# check using different var names
		pts = self.myPP()
		o = IECoreScene.PointSmoothSkinningOp()
		pts['bob'] = pts['P']
		del pts['P']
		self.assertEqual( len(pts.keys()), 2 )
		o(input=pts, positionVar="bob", copyInput=False, deformationPose = self.myDP(), smoothSkinningData = self.mySSD( ))
		self.assertNotEqual(pts["bob"].data , self.myP())

if __name__ == "__main__":
	unittest.main()

