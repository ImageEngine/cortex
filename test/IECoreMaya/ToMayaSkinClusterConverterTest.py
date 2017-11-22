##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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


class ToMayaSkinClusterConverterTest( IECoreMaya.TestCase ) :

	def buildTestSetup( self ) :
		# create simple hierarchy
		j1 = maya.cmds.joint( n = 'joint1', p =[0,-2,0] )
		maya.cmds.joint( n = 'joint2', p =[0,0,0] )
		maya.cmds.joint( n = 'joint3', p =[0,2,0] )

		maya.cmds.select( clear=True )
		j4 = maya.cmds.joint( n = 'joint4', p =[0,-2,0] )
		j5 = maya.cmds.joint( n = 'joint5', p =[0,0,0] )
		j6 = maya.cmds.joint( n = 'joint6', p =[0,2,0] )
		maya.cmds.rotate( 0, 0, -20, j4, r=True, os=True )
		maya.cmds.rotate( 0, 0, 40, j5, r=True, os=True )
		maya.cmds.rotate( 0, 0, -20, j6, r=True, os=True )

		# create geo
		geo = maya.cmds.polyCube( n = "myGeo", w = 1, h = 4, d = 1, sx = 1, sy = 3, sz = 1, ax = [ 0, 1, 0 ],cuv = 4, ch = 0 )[0]
		geo2 = maya.cmds.polyCube( n = "myGeo2", w = 1, h = 4, d = 1, sx = 1, sy = 3, sz = 1, ax = [ 0, 1, 0 ],cuv = 4, ch = 0 )[0]

		# bind it
		sc = maya.cmds.skinCluster( j1, geo, dr=4.5 )[0]
		sc2 = maya.cmds.skinCluster( j4, geo2, dr=4.5 )[0]

		# change the weights on sc2
		r = IECore.Rand32()
		for i in range( 0, 15 ) :
			val = r.barycentricf()
			maya.cmds.skinPercent( sc2, '%s.vtx[%d]' % ( geo2, i ), transformValue=[(j4, val[0]), (j5, val[1]), (j6, val[2]) ])

		return ( sc, sc2 )

	def testSimple( self ) :
		# test factory
		ssd = IECoreScene.SmoothSkinningData()
		converter = IECoreMaya.ToMayaObjectConverter.create( ssd )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.ToMayaSkinClusterConverter ) ) )

	def testConvert( self ) :
		# test conversion
		( sc, sc2 ) = self.buildTestSetup()
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full )
		ssd = fromConverter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['|joint1', '|joint1|joint2', '|joint1|joint2|joint3'] ) )
		self.assertEqual( len(ssd.influencePose()), 3 )
		self.assertEqual( len(ssd.pointInfluenceCounts()), 16 )
		self.assertEqual( len(ssd.pointInfluenceIndices()), 32 )
		ssd.validate()

		del fromConverter

		# make sure everything is different at first
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc2 )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['|joint4', '|joint4|joint5', '|joint4|joint5|joint6'] ) )
		self.assertEqual( len(ssd2.influencePose()), 3 )
		self.assertEqual( len(ssd2.pointInfluenceCounts()), 16 )
		self.assertEqual( len(ssd2.pointInfluenceIndices()), 47 )
		self.assertNotEqual( ssd.influenceNames(), ssd2.influenceNames() )
		self.assertNotEqual( ssd.influencePose(), ssd2.influencePose() )
		self.assertNotEqual( ssd.pointIndexOffsets(), ssd2.pointIndexOffsets() )
		self.assertNotEqual( ssd.pointInfluenceCounts(), ssd2.pointInfluenceCounts() )
		self.assertNotEqual( ssd.pointInfluenceIndices(), ssd2.pointInfluenceIndices() )
		self.assertNotEqual( ssd.pointInfluenceWeights(), ssd2.pointInfluenceWeights() )
		self.assertNotEqual( ssd, ssd2 )

		del fromConverter
		del ssd2

		# convert
		toConverter = IECoreMaya.ToMayaSkinClusterConverter.create( ssd )
		toConverter.convert( sc2 )

		# make sure everything is the same now
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc2 )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['|joint1', '|joint1|joint2', '|joint1|joint2|joint3'] ) )
		self.assertEqual( ssd.influenceNames(), ssd2.influenceNames() )
		self.assertEqual( ssd.influencePose(), ssd2.influencePose() )
		self.assertEqual( ssd.pointIndexOffsets(), ssd2.pointIndexOffsets() )
		self.assertEqual( ssd.pointInfluenceCounts(), ssd2.pointInfluenceCounts() )
		self.assertEqual( ssd.pointInfluenceIndices(), ssd2.pointInfluenceIndices() )
		self.assertEqual( ssd.pointInfluenceWeights(), ssd2.pointInfluenceWeights() )
		self.assertEqual( ssd, ssd2 )

	def testSSDInfluenceNotInScene( self ) :
		( sc, sc2 ) = self.buildTestSetup()
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd = fromConverter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['joint1', 'joint2', 'joint3'] ) )
		ssd.validate()

		maya.cmds.parent( 'joint3', 'joint1' )
		maya.cmds.delete( 'joint2' )
		newPose = IECore.M44fVectorData( [ IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[0]' ) ), IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[2]' ) ) ] )

		toConverter = IECoreMaya.ToMayaSkinClusterConverter.create( ssd )
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, sc ) )
		toConverter.parameters()["ignoreMissingInfluences"].setTypedValue( True )
		toConverter.convert( sc )

		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['joint1', 'joint3'] ) )
		self.assertEqual( ssd2.influencePose(), newPose )
		self.assertEqual( ssd2.pointIndexOffsets(), IECore.IntVectorData( [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 ] ) )
		self.assertEqual( ssd2.pointInfluenceCounts(), IECore.IntVectorData( [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ] ) )
		self.assertEqual( ssd2.pointInfluenceIndices(), IECore.IntVectorData( [ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 ] ) )
		newWeights = IECore.FloatVectorData( [ ssd.pointInfluenceWeights()[x] for x in range(0,ssd.pointInfluenceWeights().size()) if ssd.pointInfluenceIndices()[x] != 1 ] )
		self.assertEqual( ssd2.pointInfluenceWeights(), newWeights )

	def testSSDInfluenceInSceneButNotAJoint( self ) :
		( sc, sc2 ) = self.buildTestSetup()
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd = fromConverter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['joint1', 'joint2', 'joint3'] ) )
		ssd.validate()

		maya.cmds.parent( 'joint3', 'joint1' )
		maya.cmds.delete( 'joint2' )
		maya.cmds.polyCube( n = "joint2", w = 1, h = 4, d = 1, sx = 1, sy = 3, sz = 1, ax = [ 0, 1, 0 ],cuv = 4, ch = 0 )
		newPose = IECore.M44fVectorData( [ IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[0]' ) ), IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[2]' ) ) ] )

		toConverter = IECoreMaya.ToMayaSkinClusterConverter.create( ssd )
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, sc ) )
		toConverter.parameters()["ignoreMissingInfluences"].setTypedValue( True )
		toConverter.convert( sc )

		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['joint1', 'joint3'] ) )
		self.assertEqual( ssd2.influencePose(), newPose )
		self.assertEqual( ssd2.pointIndexOffsets(), IECore.IntVectorData( [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 ] ) )
		self.assertEqual( ssd2.pointInfluenceCounts(), IECore.IntVectorData( [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ] ) )
		self.assertEqual( ssd2.pointInfluenceIndices(), IECore.IntVectorData( [ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 ] ) )
		newWeights = IECore.FloatVectorData( [ ssd.pointInfluenceWeights()[x] for x in range(0,ssd.pointInfluenceWeights().size()) if ssd.pointInfluenceIndices()[x] != 1 ] )
		self.assertEqual( ssd2.pointInfluenceWeights(), newWeights )

	def testSkinClusterInfluencesWereRenamed( self ) :
		( sc, sc2 ) = self.buildTestSetup()
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd = fromConverter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['joint1', 'joint2', 'joint3'] ) )
		ssd.validate()

		maya.cmds.rename( 'joint2', 'fakeJoint' )
		newPose = IECore.M44fVectorData( [ IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[0]' ) ), IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[2]' ) ), IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[1]' ) ) ] )

		toConverter = IECoreMaya.ToMayaSkinClusterConverter.create( ssd )
		toConverter.parameters()["ignoreMissingInfluences"].setTypedValue( True )
		toConverter.convert( sc )

		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['joint1', 'joint3', 'fakeJoint'] ) )
		self.assertEqual( ssd2.influencePose(), newPose )
		self.assertEqual( ssd2.pointIndexOffsets(), IECore.IntVectorData( [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 ] ) )
		self.assertEqual( ssd2.pointInfluenceCounts(), IECore.IntVectorData( [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ] ) )
		self.assertEqual( ssd2.pointInfluenceIndices(), IECore.IntVectorData( [ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 ] ) )
		newWeights = IECore.FloatVectorData( [ ssd.pointInfluenceWeights()[x] for x in range(0,ssd.pointInfluenceWeights().size()) if ssd.pointInfluenceIndices()[x] != 1 ] )
		self.assertEqual( ssd2.pointInfluenceWeights(), newWeights )

	def testSkinClusterInfluencesWereRenamedAndOldNamesStillExist( self ) :
		( sc, sc2 ) = self.buildTestSetup()
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd = fromConverter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['joint1', 'joint2', 'joint3'] ) )
		ssd.validate()

		maya.cmds.rename( 'joint2', 'fakeJoint' )
		maya.cmds.rename( 'joint5', 'joint2' )
		newPose = ssd.influencePose()
		newPose.append( IECore.M44f( maya.cmds.getAttr( 'skinCluster1.bindPreMatrix[1]' ) ) )

		toConverter = IECoreMaya.ToMayaSkinClusterConverter.create( ssd )
		toConverter.parameters()["ignoreMissingInfluences"].setTypedValue( True )
		toConverter.convert( sc )

		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Partial )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['joint1', 'joint2', 'joint3', 'fakeJoint'] ) )
		self.assertEqual( ssd2.influencePose(), newPose )
		self.assertEqual( ssd2.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( ssd2.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( ssd2.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( ssd2.pointInfluenceWeights(), ssd.pointInfluenceWeights() )

	def testIgnoreBindPose( self ) :

		# test conversion
		( sc, sc2 ) = self.buildTestSetup()

		bindPoses = maya.cmds.ls( type="dagPose" )
		maya.cmds.delete( bindPoses )

		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full )
		ssd = fromConverter.convert()
		self.assertEqual( ssd.influenceNames(), IECore.StringVectorData( ['|joint1', '|joint1|joint2', '|joint1|joint2|joint3'] ) )
		self.assertEqual( len(ssd.influencePose()), 3 )
		self.assertEqual( len(ssd.pointInfluenceCounts()), 16 )
		self.assertEqual( len(ssd.pointInfluenceIndices()), 32 )
		ssd.validate()

		del fromConverter

		# make sure everything is different at first
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc2 )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['|joint4', '|joint4|joint5', '|joint4|joint5|joint6'] ) )
		self.assertEqual( len(ssd2.influencePose()), 3 )
		self.assertEqual( len(ssd2.pointInfluenceCounts()), 16 )
		self.assertEqual( len(ssd2.pointInfluenceIndices()), 47 )
		self.assertNotEqual( ssd.influenceNames(), ssd2.influenceNames() )
		self.assertNotEqual( ssd.influencePose(), ssd2.influencePose() )
		self.assertNotEqual( ssd.pointIndexOffsets(), ssd2.pointIndexOffsets() )
		self.assertNotEqual( ssd.pointInfluenceCounts(), ssd2.pointInfluenceCounts() )
		self.assertNotEqual( ssd.pointInfluenceIndices(), ssd2.pointInfluenceIndices() )
		self.assertNotEqual( ssd.pointInfluenceWeights(), ssd2.pointInfluenceWeights() )
		self.assertNotEqual( ssd, ssd2 )

		del fromConverter
		del ssd2

		toConverter = IECoreMaya.ToMayaSkinClusterConverter.create( ssd )
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, sc2 ) )
		toConverter.parameters()["ignoreBindPose"].setTypedValue( True )
		self.failUnless( toConverter.convert( sc2 ) )

		# make sure everything is the same now
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc2 )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full )
		ssd2 = fromConverter.convert()
		self.assertEqual( ssd2.influenceNames(), IECore.StringVectorData( ['|joint1', '|joint1|joint2', '|joint1|joint2|joint3'] ) )
		self.assertEqual( ssd.influenceNames(), ssd2.influenceNames() )
		self.assertEqual( ssd.influencePose(), ssd2.influencePose() )
		self.assertEqual( ssd.pointIndexOffsets(), ssd2.pointIndexOffsets() )
		self.assertEqual( ssd.pointInfluenceCounts(), ssd2.pointInfluenceCounts() )
		self.assertEqual( ssd.pointInfluenceIndices(), ssd2.pointInfluenceIndices() )
		self.assertEqual( ssd.pointInfluenceWeights(), ssd2.pointInfluenceWeights() )
		self.assertEqual( ssd, ssd2 )

	def testErrorStates( self ) :

		# test converting non-skinCluster node
		( sc, sc2 ) = self.buildTestSetup()
		fromConverter = IECoreMaya.FromMayaSkinClusterConverter.create( sc )
		fromConverter.parameters()["influenceName"].setValue( IECoreMaya.FromMayaSkinClusterConverter.InfluenceName.Full )
		ssd = fromConverter.convert()
		geo = maya.cmds.ls( "myGeo" )[0]
		toConverter = IECoreMaya.ToMayaSkinClusterConverter.create( ssd )
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, geo ) )

		# test invalid bindPose
		bindPose = maya.cmds.listConnections( sc2+'.bindPose' )[0]
		maya.cmds.delete( bindPose )
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, sc2 ) )
		maya.cmds.connectAttr( geo+'.message', sc2+'.bindPose' )
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, sc2 ) )

		# test non-existant influences
		maya.cmds.delete( ['joint2', 'joint3'] )
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, sc2 ) )

		# test influences that aren't joints
		ssd.influenceNames()[1] = geo
		self.assertRaises( RuntimeError, IECore.curry( toConverter.convert, sc2 ) )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
