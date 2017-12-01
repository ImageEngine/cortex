##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore
import random

class TransformationMatrixfTest(unittest.TestCase):

	def testConstructors(self):
		"""Test TransformationMatrixf constructors"""
		a = IECore.TransformationMatrixf()
		self.assertEqual( a.transform, imath.M44f() )
		a = IECore.TransformationMatrixf( imath.V3f( 2, 2, 2 ), imath.Eulerf(), imath.V3f( 1, 0, 0 ) )
		self.assert_( a.transform.equalWithAbsError( imath.M44f().scale( imath.V3f(2,2,2) ) * imath.M44f().translate( imath.V3f(1,0,0) ), 0.01) )
		b = IECore.TransformationMatrixf( a )
		self.assertEqual( a.transform, b.transform )

	def testAttributes(self):
		"""Test TransformationMatrixf attributes"""
		a = IECore.TransformationMatrixf()
		self.assertEqual( a.scalePivot, imath.V3f( 0, 0, 0 ) )
		self.assertEqual( a.scale, imath.V3f( 1, 1, 1 ) )
		self.assertEqual( a.shear, imath.V3f( 0, 0, 0 ) )
		self.assertEqual( a.scalePivotTranslation, imath.V3f( 0, 0, 0 ) )
		self.assertEqual( a.rotatePivot, imath.V3f( 0, 0, 0 ) )
		self.assertEqual( a.rotationOrientation, imath.Quatf() )
		self.assertEqual( a.rotate, imath.Eulerf() )
		self.assertEqual( a.rotatePivotTranslation, imath.V3f( 0, 0, 0 ) )
		self.assertEqual( a.translate, imath.V3f( 0, 0, 0 ) )
		try:
			a.transform = 1
		except:
			pass
		else:
			raise Exception, "Should not be able to set transform."

	def testTransform(self):
		"""Test TransformationMatrixf transform"""
		a = IECore.TransformationMatrixf()
		a.scale = imath.V3f( 2, 2, 2 )
		self.assertEqual( a.transform, imath.M44f().scale( imath.V3f( 2, 2, 2 ) ) )
		a.rotate = imath.Eulerf( 0.2, 0.2, 0.2 )
		self.assert_( a.transform.equalWithAbsError( imath.M44f().scale( imath.V3f( 2, 2, 2 ) ) * imath.Eulerf( 0.2, 0.2, 0.2 ).toMatrix44(), 0.01 ) )

	def testComparison(self):
		"""Test TransformationMatrixf comparison"""
		a = IECore.TransformationMatrixf()
		b = IECore.TransformationMatrixf()
		self.assertEqual( a, b )
		b.scalePivot = imath.V3f( 0.00001, 0, 0 )
		self.assertNotEqual( a, b )
		a.scalePivot = imath.V3f( 0.00001, 0, 0 )
		self.assertEqual( a, b )

class TransformationMatrixdTest(unittest.TestCase):

	def testConstructors(self):
		"""Test TransformationMatrixd constructors"""
		a = IECore.TransformationMatrixd()
		self.assertEqual( a.transform, imath.M44d() )
		a = IECore.TransformationMatrixd( imath.V3d( 2, 2, 2 ), imath.Eulerd(), imath.V3d( 1, 0, 0 ) )
		self.assert_( a.transform.equalWithAbsError( imath.M44d().scale( imath.V3d(2,2,2) ) * imath.M44d().translate( imath.V3d(1,0,0) ), 0.01 ) )
		b = IECore.TransformationMatrixd( a )
		self.assertEqual( a.transform, b.transform )

	def testAttributes(self):
		"""Test TransformationMatrixd attributes"""
		a = IECore.TransformationMatrixd()
		self.assertEqual( a.scalePivot, imath.V3d( 0, 0, 0 ) )
		self.assertEqual( a.scale, imath.V3d( 1, 1, 1 ) )
		self.assertEqual( a.shear, imath.V3d( 0, 0, 0 ) )
		self.assertEqual( a.scalePivotTranslation, imath.V3d( 0, 0, 0 ) )
		self.assertEqual( a.rotatePivot, imath.V3d( 0, 0, 0 ) )
		self.assertEqual( a.rotationOrientation, imath.Quatd() )
		self.assertEqual( a.rotate, imath.Eulerd() )
		self.assertEqual( a.rotatePivotTranslation, imath.V3d( 0, 0, 0 ) )
		self.assertEqual( a.translate, imath.V3d( 0, 0, 0 ) )
		try:
			a.transform = 1
		except:
			pass
		else:
			raise Exception, "Should not be able to set transform."

	def testTransform(self):
		"""Test TransformationMatrixd transform"""
		a = IECore.TransformationMatrixd()
		a.scale = imath.V3d( 2, 2, 2 )
		self.assertEqual( a.transform, imath.M44d().scale( imath.V3d( 2, 2, 2 ) ) )
		a.rotate = imath.Eulerd( 0.2, 0.2, 0.2 )
		self.assert_( a.transform.equalWithAbsError( imath.M44d().scale( imath.V3d( 2, 2, 2 ) ) * imath.Eulerd( 0.2, 0.2, 0.2 ).toMatrix44(), 0.01 ) )

	def testComparison(self):
		"""Test TransformationMatrixd comparison"""
		a = IECore.TransformationMatrixd()
		b = IECore.TransformationMatrixd()
		self.assertEqual( a, b )
		b.scalePivot = imath.V3d( 0.00001, 0, 0 )
		self.assertNotEqual( a, b )
		a.scalePivot = imath.V3d( 0.00001, 0, 0 )
		self.assertEqual( a, b )

class TransformationMatrixDatafTest(unittest.TestCase):

	testFile = "test/transform.fio"

	def testConstructors(self):
		"""Test TransformationMatrixfData constructors"""
		a = IECore.TransformationMatrixfData()
		self.assertEqual( a.value, IECore.TransformationMatrixf() )
		a = IECore.TransformationMatrixfData( IECore.TransformationMatrixf( imath.V3f( 2, 2, 2 ), imath.Eulerf(), imath.V3f( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, imath.V3f( 2, 2, 2 ) )

	def testCopy(self):
		"""Test TransformationMatrixfData copy"""
		a = IECore.TransformationMatrixfData( IECore.TransformationMatrixf( imath.V3f( 2, 2, 2 ), imath.Eulerf(), imath.V3f( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, imath.V3f( 2, 2, 2 ) )
		b = a.copy()
		a.value = IECore.TransformationMatrixf()
		self.assertEqual( b.value.scale, imath.V3f( 2, 2, 2 ) )
		self.assertEqual( a.value.scale, imath.V3f( 1, 1, 1 ) )

	def testIO(self):
		"""Test TransformationMatrixfData IO"""
		a = IECore.TransformationMatrixfData( IECore.TransformationMatrixf( imath.V3f(2,3,4), imath.Eulerf(), imath.V3f(1,2,3) ) )
		w = IECore.ObjectWriter( a, self.testFile )
		w.write()

		r = IECore.ObjectReader( self.testFile )
		b = r.read()
		self.assertEqual( a, b )

	def testInterpolation(self):
		"""Test TranformationMatrixfData interpolation"""
		a = IECore.TransformationMatrixfData()
		b = IECore.TransformationMatrixfData( IECore.TransformationMatrixf( imath.V3f(2,3,4), imath.Eulerf(), imath.V3f(1,2,3) ) )

		c = IECore.linearObjectInterpolation( a, b, 0.5 )
		self.assertEqual( type(c), IECore.TransformationMatrixfData )
		self.assert_( c.value.scale.equalWithAbsError( imath.V3f( 1.5, 2, 2.5 ), 0.01 ) )
		self.assert_( c.value.translate.equalWithAbsError( imath.V3f( 0.5, 1, 1.5 ), 0.01 ) )

	def testComparison(self):
		"""Test TransformationMatrixfData comparison"""
		a = IECore.TransformationMatrixfData()
		b = IECore.TransformationMatrixfData()
		self.assertEqual( a, b )
		b.value = IECore.TransformationMatrixf( imath.V3f( 0.00001, 0, 0 ), imath.Eulerf(), imath.V3f(0,0,0) )
		self.assertNotEqual( a, b )
		a.value = IECore.TransformationMatrixf( imath.V3f( 0.00001, 0, 0 ), imath.Eulerf(), imath.V3f(0,0,0) )
		self.assertEqual( a, b )

	def testHash( self ) :

		def modifyAndTest( data, field, index ) :

			h = data.hash()
			v = data.value
			f = getattr( v, field )
			if isinstance( f, imath.Quatf ) :
				# imath omits the indexing operator
				# from its bindings, so we have to do
				# it all manually
				if index == 0 :
					f.setR( f.r() + 1 )
				else :
					fv = f.v()
					fv[index-1] += 1
					f.setV( fv )
			else :
				f[index] += 1
			setattr( v, field, f )
			data.value = v
			self.assertNotEqual( data.hash(), h )

		d = IECore.TransformationMatrixfData()

		modifyAndTest( d, "scalePivot", 0 )
		modifyAndTest( d, "scalePivot", 1 )
		modifyAndTest( d, "scalePivot", 2 )

		modifyAndTest( d, "scale", 0 )
		modifyAndTest( d, "scale", 1 )
		modifyAndTest( d, "scale", 2 )

		modifyAndTest( d, "shear", 0 )
		modifyAndTest( d, "shear", 1 )
		modifyAndTest( d, "shear", 2 )

		modifyAndTest( d, "scalePivotTranslation", 0 )
		modifyAndTest( d, "scalePivotTranslation", 1 )
		modifyAndTest( d, "scalePivotTranslation", 2 )

		modifyAndTest( d, "rotatePivot", 0 )
		modifyAndTest( d, "rotatePivot", 1 )
		modifyAndTest( d, "rotatePivot", 2 )

		modifyAndTest( d, "rotationOrientation", 0 )
		modifyAndTest( d, "rotationOrientation", 1 )
		modifyAndTest( d, "rotationOrientation", 2 )
		modifyAndTest( d, "rotationOrientation", 3 )

		modifyAndTest( d, "rotate", 0 )
		modifyAndTest( d, "rotate", 1 )
		modifyAndTest( d, "rotate", 2 )

		modifyAndTest( d, "rotatePivotTranslation", 0 )
		modifyAndTest( d, "rotatePivotTranslation", 1 )
		modifyAndTest( d, "rotatePivotTranslation", 2 )

		modifyAndTest( d, "translate", 0 )
		modifyAndTest( d, "translate", 1 )
		modifyAndTest( d, "translate", 2 )

		h = d.hash()
		v = d.value
		r = v.rotate
		r.setOrder( imath.Eulerf.Order.ZYX )
		v.rotate = r
		d.value = v
		self.assertNotEqual( d.hash(), h )

	def tearDown(self):
		if os.path.exists( self.testFile ):
			os.remove( self.testFile )

class TransformationMatrixDatadTest(unittest.TestCase):

	testFile = "test/transform.fio"

	def testConstructors(self):
		"""Test TransformationMatrixdData constructors"""
		a = IECore.TransformationMatrixdData()
		self.assertEqual( a.value, IECore.TransformationMatrixd() )
		a = IECore.TransformationMatrixdData( IECore.TransformationMatrixd( imath.V3d( 2, 2, 2 ), imath.Eulerd(), imath.V3d( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, imath.V3d( 2, 2, 2 ) )

	def testCopy(self):
		"""Test TransformationMatrixdData copy"""
		a = IECore.TransformationMatrixdData( IECore.TransformationMatrixd( imath.V3d( 2, 2, 2 ), imath.Eulerd(), imath.V3d( 1, 0, 0 ) ) )
		self.assertEqual( a.value.scale, imath.V3d( 2, 2, 2 ) )
		b = a.copy()
		a.value = IECore.TransformationMatrixd()
		self.assertEqual( b.value.scale, imath.V3d( 2, 2, 2 ) )
		self.assertEqual( a.value.scale, imath.V3d( 1, 1, 1 ) )

	def testIO(self):
		"""Test TransformationMatrixdData IO"""
		a = IECore.TransformationMatrixdData( IECore.TransformationMatrixd( imath.V3d(2,3,4), imath.Eulerd(), imath.V3d(1,2,3) ) )
		w = IECore.ObjectWriter( a, self.testFile )
		w.write()

		r = IECore.ObjectReader( self.testFile )
		b = r.read()
		self.assertEqual( a, b )

	def testInterpolation(self):
		"""Test TranformationMatrixdData interpolation"""
		a = IECore.TransformationMatrixdData()
		b = IECore.TransformationMatrixdData( IECore.TransformationMatrixd( imath.V3d(2,3,4), imath.Eulerd(), imath.V3d(1,2,3) ) )

		c = IECore.linearObjectInterpolation( a, b, 0.5 )
		self.assertEqual( type(c), IECore.TransformationMatrixdData )
		self.assert_( c.value.scale.equalWithAbsError( imath.V3d( 1.5, 2, 2.5 ), 0.01 ) )
		self.assert_( c.value.translate.equalWithAbsError( imath.V3d( 0.5, 1, 1.5 ), 0.01 ) )

		# try rotation interpolation...
		d = IECore.TransformationMatrixdData( IECore.TransformationMatrixd( imath.V3d(2,3,4), imath.Eulerd( 1., 2., 3. ), imath.V3d(1,2,3) ) )
		e = IECore.linearObjectInterpolation( b, d, 0.2 )
		self.assert_( e.value.rotate.equalWithAbsError( imath.V3d( -0.341406, 0.189475, 0.191253 ), 0.001 ) )

	def testComparison(self):
		"""Test TransformationMatrixdData comparison"""
		a = IECore.TransformationMatrixdData()
		b = IECore.TransformationMatrixdData()
		self.assertEqual( a, b )
		b.value = IECore.TransformationMatrixd( imath.V3d( 0.00001, 0, 0 ), imath.Eulerd(), imath.V3d(0,0,0) )
		self.assertNotEqual( a, b )
		a.value = IECore.TransformationMatrixd( imath.V3d( 0.00001, 0, 0 ), imath.Eulerd(), imath.V3d(0,0,0) )
		self.assertEqual( a, b )

	def tearDown(self):
		if os.path.exists( self.testFile ):
			os.remove( self.testFile )

if __name__ == "__main__":
    unittest.main()

