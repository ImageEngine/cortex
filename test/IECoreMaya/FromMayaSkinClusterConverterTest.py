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

import IECore
import IECoreScene
import IECoreMaya


class FromMayaSkinClusterConverterTest( IECoreMaya.TestCase ) :

	def buildTestSetup( self ) :
		# create simple hierarchy
		j1 = maya.cmds.joint( n = 'joint1', p =[0,-2,0]) ;
		j2 = maya.cmds.joint( n = 'joint2', p =[0,0,0]) ;
		j3 = maya.cmds.joint( n = 'joint3', p =[0,2,0]) ;

		# create geo & bind it
		geo = maya.cmds.polyCube( n = "myGeo", w = 1, h = 4, d = 1, sx = 1, sy = 3, sz = 1, ax = [ 0, 1, 0 ],cuv = 4, ch = 0)
		maya.cmds.skinCluster( 'joint1', 'myGeo', dr=4.5)

		# get the skin cluster objectname
		scName = maya.cmds.ls(type="skinCluster")[0]
		return scName

	def testFactory( self ) :

		sc = self.buildTestSetup()

		converter = IECoreMaya.FromMayaObjectConverter.create( sc )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaSkinClusterConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( sc, IECoreScene.SmoothSkinningData.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaSkinClusterConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( sc, IECore.Data.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaSkinClusterConverter.staticTypeId() ) )

	def testSimple( self ) :
		sc = self.buildTestSetup()
		# test factory
		converter = IECoreMaya.FromMayaObjectConverter.create( sc )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaSkinClusterConverter ) ) )
		# convert it
		ssd = converter.convert()
		self.assert_( ssd.isInstanceOf( IECoreScene.SmoothSkinningData.staticTypeId() ) )


	def testConvert ( self ):
		#test if the data is valid
		sc = self.buildTestSetup()
		converter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )

		converter.parameters()["influenceName"].setValue(IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full)

		ssd = converter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['|joint1', '|joint1|joint2', '|joint1|joint2|joint3'] ) )

		converter.parameters()["influenceName"].setValue(IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial)
		ssd = converter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['joint1', 'joint2','joint3'] ) )

		self.assertEqual( len(ssd.influencePose()), 3)
		self.assertEqual( len(ssd.pointInfluenceCounts()), 16)
		self.assertEqual( len(ssd.pointInfluenceIndices()), 32)

		ssd.validate()

if __name__ == "__main__":
	IECoreMaya.TestProgram()
