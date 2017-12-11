##########################################################################
#
#  Copyright (c) 2008-2015, Image Engine Design Inc. All rights reserved.
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

class TestTransformOp( unittest.TestCase ) :

	def testParameterDefaults( self ) :

		o = IECoreScene.TransformOp()

		self.assertEqual( o["primVarsToModify"].getValue(), IECore.StringVectorData( [ "P", "N" ] ) )

	def testTranformation( self ) :

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		IECoreScene.MeshNormalsOp()( input = m, copyInput = False )
		m["vel"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 0.5 ) ] * 8, IECore.GeometricData.Interpretation.Vector ) )
		m["notVel"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 0.5 ) ] * 8 ) )

		mt = IECoreScene.TransformOp()( input=m, primVarsToModify = IECore.StringVectorData( m.keys() ), matrix = IECore.M44fData( imath.M44f().translate( imath.V3f( 1 ) ) ) )

		self.assertEqual( mt.bound(), imath.Box3f( imath.V3f( 0 ), imath.V3f( 2 ) ) )
		self.assertEqual( mt["P"].data, IECore.V3fVectorData( [ x + imath.V3f( 1 ) for x in m["P"].data ], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( mt["N"].data, m["N"].data )
		self.assertEqual( mt["vel"].data, m["vel"].data )
		self.assertEqual( mt["notVel"].data, m["notVel"].data )

		ms = IECoreScene.TransformOp()( input=m, primVarsToModify = IECore.StringVectorData( m.keys() ), matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )

		self.assertEqual( ms.bound(), imath.Box3f( imath.V3f( -1, -2, -3 ), imath.V3f( 1, 2, 3 ) ) )
		self.assertEqual( ms["P"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["P"].data ], IECore.GeometricData.Interpretation.Point ) )
		self.assertNotEqual( ms["N"].data, m["N"].data )
		self.assertNotEqual( ms["N"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["N"].data ], IECore.GeometricData.Interpretation.Normal ) )
		self.assertEqual( ms["vel"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["vel"].data ], IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( ms["notVel"].data, m["notVel"].data )

		self.assertEqual( ms["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( ms["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertEqual( ms["vel"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertEqual( ms["notVel"].data.getInterpretation(), IECore.GeometricData.Interpretation.None )

	def testPrimVarParameter( self ) :

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		IECoreScene.MeshNormalsOp()( input = m, copyInput = False )
		m["vel"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 0.5 ) ] * 8, IECore.GeometricData.Interpretation.Vector ) )
		m["notVel"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 0.5 ) ] * 8 ) )

		ms = IECoreScene.TransformOp()( input=m, primVarsToModify = IECore.StringVectorData( [ "P", "vel" ] ), matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )

		self.assertEqual( ms.bound(), imath.Box3f( imath.V3f( -1, -2, -3 ), imath.V3f( 1, 2, 3 ) ) )
		self.assertEqual( ms["P"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["P"].data ], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( ms["N"].data, m["N"].data )
		self.assertEqual( ms["vel"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["vel"].data ], IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( ms["notVel"].data, m["notVel"].data )

		ms = IECoreScene.TransformOp()( input=m, primVarsToModify = IECore.StringVectorData( [ "P" ] ), matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )

		self.assertEqual( ms.bound(), imath.Box3f( imath.V3f( -1, -2, -3 ), imath.V3f( 1, 2, 3 ) ) )
		self.assertEqual( ms["P"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["P"].data ], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( ms["N"].data, m["N"].data )
		self.assertEqual( ms["N"].data, m["N"].data )
		self.assertEqual( ms["notVel"].data, m["notVel"].data )

	def testSamePrimVars( self ) :

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		IECoreScene.MeshNormalsOp()( input = m, copyInput = False )
		m["vel"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 0.5 ) ] * 8, IECore.GeometricData.Interpretation.Vector ) )
		m["sameVel"] = m["vel"]

		ms = IECoreScene.TransformOp()( input=m, primVarsToModify = IECore.StringVectorData( [ "vel", "sameVel" ] ), matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )

		self.assertEqual( ms["vel"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["vel"].data ], IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( ms["vel"].data, ms["sameVel"].data )

	def testIdenticalPrimVarsCanBeExcluded( self ) :

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		IECoreScene.MeshNormalsOp()( input = m, copyInput = False )
		m["vel"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 0.5 ) ] * 8, IECore.GeometricData.Interpretation.Vector ) )
		m["otherVel"] = m["vel"]

		ms = IECoreScene.TransformOp()( input=m, primVarsToModify = IECore.StringVectorData( [ "vel" ] ), matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )

		self.assertEqual( ms["vel"].data, IECore.V3fVectorData( [ x * imath.V3f( 1, 2, 3 ) for x in m["vel"].data ], IECore.GeometricData.Interpretation.Vector ) )
		self.assertNotEqual( ms["vel"].data, ms["otherVel"].data )
		self.assertEqual( ms["otherVel"].data, m["otherVel"].data )

if __name__ == "__main__":
	unittest.main()

