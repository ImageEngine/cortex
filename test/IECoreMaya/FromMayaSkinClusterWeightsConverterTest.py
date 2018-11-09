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


class FromMayaSkinClusterWeightsConverterTest( IECoreMaya.TestCase ) :

	def buildTestSetup( self ) :

		maya.cmds.select(d=True)
		j1 = maya.cmds.joint( p=(4,0,0))
		j2 = maya.cmds.joint( p=(-2,0,2))
		j3 = maya.cmds.joint( p=(0,0,-5))
		geo = maya.cmds.polyPlane(sx=4, sy=4, w=10, h=10)[0]
		maya.cmds.select( j1,add=True )

		sc = maya.cmds.skinCluster([j1, j2, j3], geo, toSelectedBones=True, bindMethod=0, skinMethod=0, normalizeWeights=1)[0]
		return sc, geo, [j1,j2,j3]

	def setRandomWeights(self, seed, skinCluster, geo, joints):
		r = imath.Rand32(seed)
		weights = []
		for i in range( 0, 25 ) :
			val = IECore.RandomAlgo.barycentricRandf( r )
			weights.extend([val[0], val[1], val[2]])
			maya.cmds.skinPercent( skinCluster, '%s.vtx[%d]' % ( geo, i ), transformValue=[(joints[0], val[0]), (joints[1], val[1]), (joints[2], val[2]) ])
		return weights

	def testUncompressedWeightsConverter(self):

		sc, geo, joints = self.buildTestSetup()
		self.setRandomWeights(123, sc, geo, joints)

		weights = []
		for i in range( 0, 25 ) :
			weights.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[0], q=True))
			weights.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[1], q=True))
			weights.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[2], q=True))

		c = IECoreMaya.FromMayaSkinClusterWeightsConverter.create(sc, IECore.TypeId.CompoundObject)
		c["useCompression"].setTypedValue(False)
		wConverted = c.convert()

		for i in range( 0, 25 ):
			self.assertAlmostEqual(wConverted['pointInfluenceWeights'][i], weights[i])

		for obj in joints + [geo, sc]:
			if maya.cmds.objExists(obj):
				maya.cmds.delete(obj)

	def testCompressedWeightsConverter(self):

		sc, geo, joints = self.buildTestSetup()
		self.setRandomWeights(123, sc, geo, joints)

		w1 = []
		for i in range( 0, 25 ) :
			w1.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[0], q=True))
			w1.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[1], q=True))
			w1.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[2], q=True))

		c = IECoreMaya.FromMayaSkinClusterWeightsConverter.create(sc, IECore.TypeId.CompoundObject)
		c["useCompression"].setTypedValue(True)
		wConverted = c.convert()

		c2 = IECoreMaya.ToMayaSkinClusterWeightsConverter.create( wConverted )
		c2.convert( sc )

		w2 = []
		for i in range( 0, 25 ) :
			w2.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[0], q=True))
			w2.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[1], q=True))
			w2.append(maya.cmds.skinPercent(sc,  '%s.vtx[%d]' % ( geo, i ), transform=joints[2], q=True))

		for i in range( 0, 25 ):
			self.assertTrue( abs(w1[i] - w2[i]) < 1e-05 )

		for obj in joints + [geo, sc]:
			if maya.cmds.objExists(obj):
				maya.cmds.delete(obj)


if __name__ == "__main__":
	IECoreMaya.TestProgram()
