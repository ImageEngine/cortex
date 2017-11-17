##########################################################################
#
#  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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
import IECoreScene
import math

class MeshMergeOpTest( unittest.TestCase ) :

	def verifyMerge( self, mesh1, mesh2, merged ) :

		self.failUnless( mesh1.arePrimitiveVariablesValid() )
		self.failUnless( mesh2.arePrimitiveVariablesValid() )
		self.failUnless( merged.arePrimitiveVariablesValid() )

		for v in IECoreScene.PrimitiveVariable.Interpolation.values :
			i = IECoreScene.PrimitiveVariable.Interpolation( v )
			if i!=IECoreScene.PrimitiveVariable.Interpolation.Invalid and i!=IECoreScene.PrimitiveVariable.Interpolation.Constant :
				self.assertEqual( merged.variableSize( i ), mesh1.variableSize( i ) + mesh2.variableSize( i ) )

		self.verifyData( mesh1, mesh2, merged )
		self.verifyData( mesh2, mesh1, merged, flipped=True )

	def verifyData( self, meshA, meshB, merged, flipped=False ) :

		for name in meshA.keys() :

			self.failUnless( name in merged )

			interpolation = meshA[name].interpolation
			if merged[name].indices :
				self.assertEqual( len(merged[name].indices), meshA.variableSize( interpolation ) + meshB.variableSize( interpolation ) )
			else :
				self.assertEqual( len(merged[name].data), meshA.variableSize( interpolation ) + meshB.variableSize( interpolation ) )

			offset = meshB.variableSize( interpolation ) if flipped else 0

			if merged[name].indices and meshA[name].indices :
				for i in range( 0, len(meshA[name].indices) ) :
					index = merged[name].indices[offset + i]
					indexA = meshA[name].indices[i]
					self.assertEqual( index, indexA + offset if flipped else indexA )
					self.assertEqual( merged[name].data[index], meshA[name].data[indexA] )

			elif merged[name].indices :
				for i in range( 0, len(meshA[name].data) ) :
					index = merged[name].indices[offset + i]
					indexA = offset + i
					self.assertEqual( index, indexA )
					self.assertEqual( merged[name].data[index], meshA[name].data[i] )

			elif meshA[name].indices :
				for i in range( 0, len(meshA[name].indices) ) :
					indexA = meshA[name].indices[i]
					self.assertEqual( merged[name].data[offset + i], meshA[name].data[indexA] )

			else :
				for i in range( 0, len(meshA[name].data) ) :
					self.assertEqual( merged[name].data[offset + i], meshA[name].data[i] )

			offset = 0 if flipped else meshA.variableSize( interpolation )
			if name in meshB and meshB[name].interpolation == interpolation :

				if merged[name].indices and meshB[name].indices :
					for i in range( 0, len(meshB[name].indices) ) :
						index = merged[name].indices[offset + i]
						indexB = meshB[name].indices[i]
						self.assertEqual( index, indexB if flipped else indexB + offset )
						self.assertEqual( merged[name].data[index], meshB[name].data[indexB] )

				elif merged[name].indices :
					for i in range( 0, len(meshB[name].data) ) :
						index = merged[name].indices[offset + i]
						indexB = offset + i
						self.assertEqual( index, indexB )
						self.assertEqual( merged[name].data[index], meshB[name].data[i] )

				elif meshB[name].indices :
					for i in range( 0, len(meshB[name].indices) ) :
						indexB = meshB[name].indices[i]
						self.assertEqual( merged[name].data[offset + i], meshB[name].data[indexB] )

				else :
					for i in range( 0, len(meshB[name].data) ) :
						self.assertEqual( merged[name].data[offset + i], meshB[name].data[i] )

	def testPlanes( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 0 ) ) )
		p2 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

	def testDifferentPrimVars( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 0 ) ) )
		IECoreScene.MeshNormalsOp()( input=p1, copyInput=False )
		p2 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		self.assertNotEqual( p1.keys(), p2.keys() )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

		IECoreScene.TriangulateOp()( input=p2, copyInput=False )
		p2['myInt'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.IntVectorData( [ 0, 1, 2, 3, 4 ,5 ] ) )
		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangents( p2 )
		p2["uTangent"] = uTangent
		p2["vTangent"] = vTangent
		self.assertNotEqual( p1.keys(), p2.keys() )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

	def testSamePrimVarNamesWithDifferentInterpolation( self ) :

		plane = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 0 ) ) )
		del plane["uv"]
		IECoreScene.MeshNormalsOp()( input=plane, copyInput=False )
		box = IECoreScene.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ) )
		IECoreScene.MeshNormalsOp()( input=box, copyInput=False )
		IECoreScene.FaceVaryingPromotionOp()( input=box, copyInput=False, primVarNames=IECore.StringVectorData( [ "N" ] ) )
		self.assertEqual( plane.keys(), box.keys() )
		merged = IECoreScene.MeshMergeOp()( input=plane, mesh=box )
		del box["N"]
		self.verifyMerge( plane, box, merged )

	def testRemovePrimVars( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 0 ) ) )
		IECoreScene.MeshNormalsOp()( input=p1, copyInput=False )
		p2 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		self.assertNotEqual( p1.keys(), p2.keys() )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2, removeNonMatchingPrimVars=False )
		self.failUnless( "N" in merged )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2, removeNonMatchingPrimVars=True )
		self.failUnless( "N" not in merged )
		del p1["N"]
		self.verifyMerge( p1, p2, merged )

		IECoreScene.TriangulateOp()( input=p2, copyInput=False )
		p2['myInt'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.IntVectorData( [ 0, 1, 2, 3, 4 ,5 ] ) )
		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangents( p2 )
		p2["uTangent"] = uTangent
		p2["vTangent"] = vTangent
		self.assertNotEqual( p1.keys(), p2.keys() )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2, removeNonMatchingPrimVars=False )
		self.failUnless( "uTangent" in merged )
		self.failUnless( "vTangent" in merged )
		self.failUnless( "myInt" in merged )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2, removeNonMatchingPrimVars=True )
		self.failUnless( "uTangent" not in merged )
		self.failUnless( "vTangent" not in merged )
		self.failUnless( "myInt" not in merged )
		del p2["uTangent"]
		del p2["vTangent"]
		del p2["myInt"]
		self.verifyMerge( p1, p2, merged )

	def testReferencedData( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 0 ) ) )
		p1["Pref"] = p1["P"]
		p2 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.failUnless( "Pref" in merged )
		self.verifyMerge( p1, p2, merged )

		del p1["Pref"]
		p2["Pref"] = p2["P"]
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.failUnless( "Pref" in merged )
		self.verifyMerge( p1, p2, merged )

	def testIndexedPrimVars( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 0 ) ) )
		p2 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )

		# both meshes have indexed UVs
		p1["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p1["uv"].data, IECore.IntVectorData( [ 0, 3, 1, 2 ] ) )
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, IECore.IntVectorData( [ 2, 1, 0, 3 ] ) )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

		# meshA has indexed UVs, meshB has expanded UVs
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, None )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

		# both meshes have expanded UVs
		p1["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p1["uv"].data, None )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

		# meshA has expanded UVs, meshB has indexed UVs
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, IECore.IntVectorData( [ 2, 1, 0, 3 ] ) )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

		# meshA has indexed UVs, meshB has no UVs
		p1["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p1["uv"].data, IECore.IntVectorData( [ 0, 3, 1, 2 ] ) )
		del p2["uv"]
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

		# meshA has no UVs, meshB has indexed UVs
		del p1["uv"]
		p2 = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, IECore.IntVectorData( [ 2, 1, 0, 3 ] ) )
		merged = IECoreScene.MeshMergeOp()( input=p1, mesh=p2 )
		self.verifyMerge( p1, p2, merged )

if __name__ == "__main__":
    unittest.main()
