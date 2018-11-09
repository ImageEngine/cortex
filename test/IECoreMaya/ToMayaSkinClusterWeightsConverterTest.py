##########################################################################
#
#  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

import maya.cmds

import imath
import IECore
import IECore.RandomAlgo
import IECoreScene
import IECoreMaya


class ToMayaSkinClusterWeightsConverterTest( IECoreMaya.TestCase ) :

	def buildTestSetup( self ) :

		maya.cmds.select(d=True)
		j1 = maya.cmds.joint( p=(4,0,0))
		j2 = maya.cmds.joint( p=(-2,0,2))
		j3 = maya.cmds.joint( p=(0,0,-5))
		geo = maya.cmds.polyPlane(sx=4, sy=4, w=10, h=10)[0]
		maya.cmds.select( j1,add=True )

		sc = maya.cmds.skinCluster([j1, j2, j3], geo, toSelectedBones=True, bindMethod=0, skinMethod=0, normalizeWeights=1)[0]
		return sc, geo, [j1,j2,j3]

	def getRandomWeights(self, seed):
		r = imath.Rand32(seed)
		weights = []
		for i in range( 0, 25 ) :
			val = IECore.RandomAlgo.barycentricRandf( r )
			weights.extend([val[0], val[1], val[2]])
		return weights

	def testWeightsConverter(self):

		sc, geo, joints = self.buildTestSetup()
		weightsIn = self.getRandomWeights(13)
		weightData = IECore.CompoundObject({
			'pointIndexOffsets':IECore.IntVectorData( [ 0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72 ] ),
			'pointInfluenceCounts':IECore.IntVectorData( [ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 ] ),
			'pointInfluenceIndices':IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ),
			'pointInfluenceWeights':IECore.FloatVectorData( weightsIn )
		})

		c = IECoreMaya.ToMayaSkinClusterWeightsConverter.create(weightData)
		c.convert(sc)

		weights = []
		for i in range( 0, 25 ) :
			weights.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[0], q=True))
			weights.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[1], q=True))
			weights.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[2], q=True))

		for i in range( 0, 25 ):
			self.assertAlmostEqual(weightsIn[i], weights[i])

		for obj in joints + [geo, sc]:
			if maya.cmds.objExists(obj):
				maya.cmds.delete(obj)


if __name__ == "__main__":
	IECoreMaya.TestProgram()
