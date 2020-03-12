##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
import os
import IECore

class SkeletonPrimitiveTests(unittest.TestCase):
	def setUp(self):
		mts = IECore.M44fVectorData()
		mts.resize(10)

		ids = IECore.IntVectorData()
		ids.resize(10)

		for i in range(0, 10):
			ids[i] = i-1
			vec = IECore.V3f(0-i, i+1, 2*i)
			mts[i] = mts[i].setTranslation(vec)

		self.skeletonPrimitive = IECore.SkeletonPrimitive(mts, ids, IECore.SkeletonPrimitive.Space.World)

	def testDefaultInitialisation(self):
		sp = IECore.SkeletonPrimitive()
		self.assertTrue( sp.numJoints()==0 ) # check we have no joints
		self.assertEqual( sp.getJointNames(), IECore.StringVectorData() )
		self.assertTrue( sp.isEqualTo(sp) )
		self.assertTrue( sp.isSimilarTo(sp.copy()) )

	def testCopyConstractor(self):
		sp = IECore.SkeletonPrimitive(self.skeletonPrimitive)
		self.assertTrue( sp.isSimilarTo(self.skeletonPrimitive) )

	def testMatricesInitialisation(self):
		mts = IECore.M44fVectorData()
		mts.resize(10)

		ids = IECore.IntVectorData()
		ids.resize(10)

		for i in range(0, 10):
			ids[i] = i-1
			mts[i] = mts[i].setTranslation(IECore.V3f(0, i+10, 0))

		# In World space
		sp = IECore.SkeletonPrimitive(mts, ids, IECore.SkeletonPrimitive.Space.World)
		assert sp.numJoints() == 10
		for i in range(0, 10):
			assert mts[i].translation().y == sp.getJointPose(i, IECore.SkeletonPrimitive.Space.World).value.translation().y
			assert mts[i].translation().y != sp.getJointPose(i, IECore.SkeletonPrimitive.Space.Reference).value.translation().y
			if i == 0:
				assert mts[i].translation().y == sp.getJointPose(i, IECore.SkeletonPrimitive.Space.Local).value.translation().y
			else:
				assert mts[i].translation().y != sp.getJointPose(i, IECore.SkeletonPrimitive.Space.Local).value.translation().y

		# In Reference space
		sp = IECore.SkeletonPrimitive(mts, ids, IECore.SkeletonPrimitive.Space.Reference)
		assert sp.numJoints() == 10
		for i in range(0, 10):
			if i==0:
				assert mts[i].translation().y == sp.getJointPose(i, IECore.SkeletonPrimitive.Space.World).value.translation().y
			else:
				assert mts[i].translation().y != sp.getJointPose(i, IECore.SkeletonPrimitive.Space.World).value.translation().y
			assert mts[i].translation().y == sp.getJointPose(i, IECore.SkeletonPrimitive.Space.Reference).value.translation().y
			assert mts[i].translation().y != sp.getJointPose(i, IECore.SkeletonPrimitive.Space.Local).value.translation().y

		# In Local space
		sp = IECore.SkeletonPrimitive(mts, ids, IECore.SkeletonPrimitive.Space.Local)
		assert sp.numJoints() == 10
		for i in range(0, 10):
			if i==0:
				assert mts[i].translation().y == sp.getJointPose(i, IECore.SkeletonPrimitive.Space.World).value.translation().y
			else:
				assert mts[i].translation().y != sp.getJointPose(i, IECore.SkeletonPrimitive.Space.World).value.translation().y
			assert mts[i].translation().y != sp.getJointPose(i, IECore.SkeletonPrimitive.Space.Reference).value.translation().y
			assert mts[i].translation().y == sp.getJointPose(i, IECore.SkeletonPrimitive.Space.Local).value.translation().y

	def testSaveLoad(self):
		tempFile = os.tempnam()+".cob"
		writer = IECore.Writer.create(self.skeletonPrimitive, tempFile)
		writer.write()

		reader = IECore.Reader.create(tempFile)
		otherSp = reader.read()

		self.assertTrue( self.skeletonPrimitive.isSimilarTo(otherSp) )

	def testExtractingParentIds(self):
		self.assertTrue( self.skeletonPrimitive.getParentId(0)==-1 ) # check the parent id of joint 0 is -1
		self.assertTrue( len( self.skeletonPrimitive.getParentIds() )==10 ) # check we get a list of 10 ids

	def testExtractingMatrices(self):
		self.assertTrue( self.skeletonPrimitive.getJointPoses(IECore.SkeletonPrimitive.Space.World).size()==10 )
		self.assertTrue( self.skeletonPrimitive.getJointPoses(IECore.SkeletonPrimitive.Space.Local).size()==10 )
		self.assertTrue( self.skeletonPrimitive.getJointPoses(IECore.SkeletonPrimitive.Space.Reference).size()==10 )

		testedMat = self.skeletonPrimitive.getJointPoses()[4] # default for getTransforms is world space
		testerMat = IECore.M44f().setTranslation( IECore.V3f(-4, 5, 8) )
		# check the matrix returned
		for i in range(0, 4):
			for j in range(0, 4):
				self.assertEqual(testedMat[i,j], testerMat[i,j])

	def testCopy(self):
		s = IECore.SkeletonPrimitive()
		s.setAsCopyOf(self.skeletonPrimitive)

		s.isSimilarTo(self.skeletonPrimitive)


if __name__ == "__main__":
	print 'Testing SkeletonPrimitive\n----------------------------------------------------------------------\n'
	import sys
	t = unittest.TextTestRunner(stream=sys.stdout, verbosity=2)
	unittest.main(testRunner=t)
