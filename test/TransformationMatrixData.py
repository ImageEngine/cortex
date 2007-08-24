##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

"""Unit test for TransformationMatrix and TransformationMatrixData bindings"""

import os, os.path
import math
import unittest
from IECore import *
import random

class TransformationMatrixfTest(unittest.TestCase):

	def testConstructors(self):
		"""Test TransformationMatrixf constructors"""
		a = TransformationMatrixf()
		self.assertEqual( a.transform, M44f() )
		a = TransformationMatrixf( V3f( 2, 2, 2 ), Quatf(), V3f( 1, 0, 0 ) )
		self.assertEqual( a.transform, M44f().scale( V3f(2,2,2) ) * M44f().translate( V3f(1,0,0) ) )
		b = TransformationMatrixf( a )
		self.assertEqual( a.transform, b.transform )

	def testAttributes(self):
		"""Test TransformationMatrixf attributes"""
		a = TransformationMatrixf()
		self.assertEqual( a.scalePivot, V3f( 0, 0, 0 ) )
		self.assertEqual( a.scale, V3f( 1, 1, 1 ) )
		self.assertEqual( a.shear, V3f( 0, 0, 0 ) )
		self.assertEqual( a.scalePivotTranslation, V3f( 0, 0, 0 ) )
		self.assertEqual( a.rotatePivot, V3f( 0, 0, 0 ) )
		self.assertEqual( a.rotationOrientation, Quatf() )
		self.assertEqual( a.rotate, Quatf() )
		self.assertEqual( a.rotatePivotTranslation, V3f( 0, 0, 0 ) )
		self.assertEqual( a.translate, V3f( 0, 0, 0 ) )
		try:
			a.transform = 1
		except:
			pass
		else:
			raise Exception, "Should not be able to set transform."

	def testTransform(self):
		"""Test TransformationMatrixf transform"""
		a = TransformationMatrixf()
		a.scale = V3f( 2, 2, 2 )
		self.assertEqual( a.transform, M44f().scale( V3f( 2, 2, 2 ) ) )
		a.rotate = Quatf( 0.5, 0.5, 0, 0 )
		self.assertEqual( a.transform, M44f().scale( V3f( 2, 2, 2 ) ) * Quatf( 0.5, 0.5, 0, 0 ).toMatrix44() )

	def testComparison(self):
		"""Test TransformationMatrixf comparison"""
		a = TransformationMatrixf()
		b = TransformationMatrixf()
		self.assertEqual( a, b )
		b.scalePivot = V3f( 0.00001, 0, 0 )
		self.assertNotEqual( a, b )
		a.scalePivot = V3f( 0.00001, 0, 0 )
		self.assertEqual( a, b )

class TransformationMatrixdTest(unittest.TestCase):

	def testConstructors(self):
		"""Test TransformationMatrixd constructors"""
		a = TransformationMatrixd()
		self.assertEqual( a.transform, M44d() )
		a = TransformationMatrixd( V3d( 2, 2, 2 ), Quatd(), V3d( 1, 0, 0 ) )
		self.assertEqual( a.transform, M44d().scale( V3d(2,2,2) ) * M44d().translate( V3d(1,0,0) ) )
		b = TransformationMatrixd( a )
		self.assertEqual( a.transform, b.transform )

	def testAttributes(self):
		"""Test TransformationMatrixd attributes"""
		a = TransformationMatrixd()
		self.assertEqual( a.scalePivot, V3d( 0, 0, 0 ) )
		self.assertEqual( a.scale, V3d( 1, 1, 1 ) )
		self.assertEqual( a.shear, V3d( 0, 0, 0 ) )
		self.assertEqual( a.scalePivotTranslation, V3d( 0, 0, 0 ) )
		self.assertEqual( a.rotatePivot, V3d( 0, 0, 0 ) )
		self.assertEqual( a.rotationOrientation, Quatd() )
		self.assertEqual( a.rotate, Quatd() )
		self.assertEqual( a.rotatePivotTranslation, V3d( 0, 0, 0 ) )
		self.assertEqual( a.translate, V3d( 0, 0, 0 ) )
		try:
			a.transform = 1
		except:
			pass
		else:
			raise Exception, "Should not be able to set transform."

	def testTransform(self):
		"""Test TransformationMatrixd transform"""
		a = TransformationMatrixd()
		a.scale = V3d( 2, 2, 2 )
		self.assertEqual( a.transform, M44d().scale( V3d( 2, 2, 2 ) ) )
		a.rotate = Quatd( 0.5, 0.5, 0, 0 )
		self.assertEqual( a.transform, M44d().scale( V3d( 2, 2, 2 ) ) * Quatd( 0.5, 0.5, 0, 0 ).toMatrix44() )

	def testComparison(self):
		"""Test TransformationMatrixd comparison"""
		a = TransformationMatrixd()
		b = TransformationMatrixd()
		self.assertEqual( a, b )
		b.scalePivot = V3d( 0.00001, 0, 0 )
		self.assertNotEqual( a, b )
		a.scalePivot = V3d( 0.00001, 0, 0 )
		self.assertEqual( a, b )

class TransformationMatrixDatafTest(unittest.TestCase):

	testFile = "test/transform.fio"

	def testConstructors(self):
		"""Test TransformationMatrixfData constructors"""
		a = TransformationMatrixfData()
		self.assertEqual( a.value, TransformationMatrixf() )
		a = TransformationMatrixfData( TransformationMatrixf( V3f( 2, 2, 2 ), Quatf(), V3f( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, V3f( 2, 2, 2 ) )

	def testCopy(self):
		"""Test TransformationMatrixfData copy"""
		a = TransformationMatrixfData( TransformationMatrixf( V3f( 2, 2, 2 ), Quatf(), V3f( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, V3f( 2, 2, 2 ) )
		b = a.copy()
		a.value = TransformationMatrixf()
		self.assertEqual( b.value.scale, V3f( 2, 2, 2 ) )
		self.assertEqual( a.value.scale, V3f( 1, 1, 1 ) )

	def testIO(self):
		"""Test TransformationMatrixfData IO"""
		a = TransformationMatrixfData( TransformationMatrixf( V3f(2,3,4), Quatf(), V3f(1,2,3) ) )
		w = ObjectWriter( a, self.testFile )
		w.write()

		r = ObjectReader( self.testFile )
		b = r.read()
		self.assertEqual( a, b )

	def testInterpolation(self):
		"""Test TranformationMatrixfData interpolation"""
		a = TransformationMatrixfData()
		b = TransformationMatrixfData( TransformationMatrixf( V3f(2,3,4), Quatf(), V3f(1,2,3) ) )

		c = linearObjectInterpolation( a, b, 0.5 )
		self.assertEqual( type(c), TransformationMatrixfData )
		self.assertEqual( c.value.scale, V3f( 1.5, 2, 2.5 ) )
		self.assertEqual( c.value.translate, V3f( 0.5, 1, 1.5 ) )

		c = cosineObjectInterpolation( a, b, 0.5 )
		self.assertEqual( type(c), TransformationMatrixfData )
		self.assertEqual( c.value.scale, V3f( 1.5, 2, 2.5 ) )
		self.assertEqual( c.value.translate, V3f( 0.5, 1, 1.5 ) )

		c = cubicObjectInterpolation( b, b, b, b, 0.5 )
		self.assertEqual( type(c), TransformationMatrixfData )
		self.assertEqual( c.value.scale, V3f( 2, 3, 4 ) )
		self.assertEqual( c.value.translate, V3f( 1, 2, 3 ) )

	def testComparison(self):
		"""Test TransformationMatrixfData comparison"""
		a = TransformationMatrixfData()
		b = TransformationMatrixfData()
		self.assertEqual( a, b )
		b.value = TransformationMatrixf( V3f( 0.00001, 0, 0 ), Quatf(), V3f(0,0,0) )
		self.assertNotEqual( a, b )
		a.value = TransformationMatrixf( V3f( 0.00001, 0, 0 ), Quatf(), V3f(0,0,0) )
		self.assertEqual( a, b )

	def tearDown(self):
		if os.path.exists( self.testFile ):
			os.remove( self.testFile )

class TransformationMatrixDatadTest(unittest.TestCase):

	testFile = "test/transform.fio"

	def testConstructors(self):
		"""Test TransformationMatrixdData constructors"""
		a = TransformationMatrixdData()
		self.assertEqual( a.value, TransformationMatrixd() )
		a = TransformationMatrixdData( TransformationMatrixd( V3d( 2, 2, 2 ), Quatd(), V3d( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, V3d( 2, 2, 2 ) )

	def testCopy(self):
		"""Test TransformationMatrixdData copy"""
		a = TransformationMatrixdData( TransformationMatrixd( V3d( 2, 2, 2 ), Quatd(), V3d( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, V3d( 2, 2, 2 ) )
		b = a.copy()
		a.value = TransformationMatrixd()
		self.assertEqual( b.value.scale, V3d( 2, 2, 2 ) )
		self.assertEqual( a.value.scale, V3d( 1, 1, 1 ) )

	def testIO(self):
		"""Test TransformationMatrixdData IO"""
		a = TransformationMatrixdData( TransformationMatrixd( V3d(2,3,4), Quatd(), V3d(1,2,3) ) )
		w = ObjectWriter( a, self.testFile )
		w.write()

		r = ObjectReader( self.testFile )
		b = r.read()
		self.assertEqual( a, b )

	def testInterpolation(self):
		"""Test TranformationMatrixdData interpolation"""
		a = TransformationMatrixdData()
		b = TransformationMatrixdData( TransformationMatrixd( V3d(2,3,4), Quatd(), V3d(1,2,3) ) )

		c = linearObjectInterpolation( a, b, 0.5 )
		self.assertEqual( type(c), TransformationMatrixdData )
		self.assertEqual( c.value.scale, V3d( 1.5, 2, 2.5 ) )
		self.assertEqual( c.value.translate, V3d( 0.5, 1, 1.5 ) )

		c = cosineObjectInterpolation( a, b, 0.5 )
		self.assertEqual( type(c), TransformationMatrixdData )
		self.assertEqual( c.value.scale, V3d( 1.5, 2, 2.5 ) )
		self.assertAlmostEqual( (c.value.translate - V3d( .5, 1, 1.5 ) ).length(), 0, 2 )

		c = cubicObjectInterpolation( b, b, b, b, 0.5 )
		self.assertEqual( type(c), TransformationMatrixdData )
		self.assertEqual( c.value.scale, V3d( 2, 3, 4 ) )
		self.assertEqual( c.value.translate, V3d( 1, 2, 3 ) )

	def testComparison(self):
		"""Test TransformationMatrixdData comparison"""
		a = TransformationMatrixdData()
		b = TransformationMatrixdData()
		self.assertEqual( a, b )
		b.value = TransformationMatrixd( V3d( 0.00001, 0, 0 ), Quatd(), V3d(0,0,0) )
		self.assertNotEqual( a, b )
		a.value = TransformationMatrixd( V3d( 0.00001, 0, 0 ), Quatd(), V3d(0,0,0) )
		self.assertEqual( a, b )

	def tearDown(self):
		if os.path.exists( self.testFile ):
			os.remove( self.testFile )
					
if __name__ == "__main__":
    unittest.main()   

