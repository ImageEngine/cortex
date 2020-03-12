##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
import imath


class MeshAlgoMergeTest( unittest.TestCase ) :

	def verifyPrimvars( self, primitive ):
		for v in primitive.keys():
			self.assertTrue( primitive.isPrimitiveVariableValid(primitive[v]), "invalid primvar {0}".format( v ) )

	def verifyMerge( self, merged, originals ) :

		self.verifyPrimvars( merged )
		for mesh in originals :
			self.verifyPrimvars( mesh )

		for v in IECoreScene.PrimitiveVariable.Interpolation.values :
			i = IECoreScene.PrimitiveVariable.Interpolation( v )
			if i!=IECoreScene.PrimitiveVariable.Interpolation.Invalid and i!=IECoreScene.PrimitiveVariable.Interpolation.Constant :
				self.assertEqual( merged.variableSize( i ), sum([ mesh.variableSize( i ) for mesh in originals ]) )

		self.verifyData( merged, originals )

	def verifyData( self, merged, originals ) :

		for meshIndex in range( 0, len(originals) ) :

			mesh = originals[meshIndex]

			for name in mesh.keys() :

				self.assertTrue( name in merged )

				interpolation = mesh[name].interpolation
				if merged[name].indices :
					self.assertEqual( len(merged[name].indices), sum([ x.variableSize( interpolation ) for x in originals ]) )
				else :
					self.assertEqual( len(merged[name].data), sum([ x.variableSize( interpolation ) for x in originals ]) )

				offset = sum([ x.variableSize( interpolation ) for x in originals[:meshIndex] ])

				if merged[name].indices and mesh[name].indices :
					for i in range( 0, len(mesh[name].indices) ) :
						index = merged[name].indices[offset + i]
						indexA = mesh[name].indices[i]
						self.assertEqual( index, indexA + offset )
						self.assertEqual( merged[name].data[index], mesh[name].data[indexA] )

				elif merged[name].indices :
					for i in range( 0, len(mesh[name].data) ) :
						index = merged[name].indices[offset + i]
						indexA = offset + i
						self.assertEqual( index, indexA )
						self.assertEqual( merged[name].data[index], mesh[name].data[i] )

				elif mesh[name].indices :
					for i in range( 0, len(mesh[name].indices) ) :
						indexA = mesh[name].indices[i]
						self.assertEqual( merged[name].data[offset + i], mesh[name].data[indexA] )

				else :
					for i in range( 0, len(mesh[name].data) ) :
						self.assertEqual( merged[name].data[offset + i], mesh[name].data[i] )

	def testPlanes( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 0 ) ) )
		p2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

	def testDifferentPrimVars( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 0 ) ) )
		p2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		del p2["N"]
		self.assertNotEqual( p1.keys(), p2.keys() )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

		p2 = IECoreScene.MeshAlgo.triangulate( p2 )
		p2['myInt'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.IntVectorData( [ 0, 1, 2, 3, 4 ,5 ] ) )
		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangents( p2 )
		p2["uTangent"] = uTangent
		p2["vTangent"] = vTangent
		self.assertNotEqual( p1.keys(), p2.keys() )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

	def testSamePrimVarNamesWithDifferentInterpolation( self ) :

		plane = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 0 ) ) )
		IECoreScene.MeshNormalsOp()( input=plane, copyInput=False )
		box = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		IECoreScene.MeshNormalsOp()( input=box, copyInput=False )
		IECoreScene.FaceVaryingPromotionOp()( input=box, copyInput=False, primVarNames=IECore.StringVectorData( [ "N" ] ) )
		self.assertEqual( plane.keys(), box.keys() )
		merged = IECoreScene.MeshAlgo.merge( [ plane, box ] )
		del box["N"]
		self.verifyMerge( merged, [ plane, box ] )

	def testReferencedData( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 0 ) ) )
		p1["Pref"] = p1["P"]
		p2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.assertTrue( "Pref" in merged )
		self.verifyMerge( merged, [ p1, p2 ] )

		del p1["Pref"]
		p2["Pref"] = p2["P"]
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.assertTrue( "Pref" in merged )
		self.verifyMerge( merged, [ p1, p2 ] )

	def testIndexedPrimVars( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 0 ) ) )
		p2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )

		# both meshes have indexed UVs
		p1["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p1["uv"].data, IECore.IntVectorData( [ 0, 3, 1, 2 ] ) )
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, IECore.IntVectorData( [ 2, 1, 0, 3 ] ) )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

		# meshA has indexed UVs, meshB has expanded UVs
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, None )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

		# both meshes have expanded UVs
		p1["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p1["uv"].data, None )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

		# meshA has expanded UVs, meshB has indexed UVs
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, IECore.IntVectorData( [ 2, 1, 0, 3 ] ) )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

		# meshA has indexed UVs, meshB has no UVs
		p1["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p1["uv"].data, IECore.IntVectorData( [ 0, 3, 1, 2 ] ) )
		del p2["uv"]
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

		# meshA has no UVs, meshB has indexed UVs
		del p1["uv"]
		p2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		p2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, p2["uv"].data, IECore.IntVectorData( [ 2, 1, 0, 3 ] ) )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2 ] )
		self.verifyMerge( merged, [ p1, p2 ] )

	def testMultipleMeshes( self ) :

		p1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		p2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 1 ), imath.V2f( 2 ) ) )
		p3 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 2 ), imath.V2f( 3 ) ) )
		p4 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 3 ), imath.V2f( 4 ) ) )
		p5 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 4 ), imath.V2f( 5 ) ) )
		merged = IECoreScene.MeshAlgo.merge( [ p1, p2, p3, p4, p5 ] )
		self.verifyMerge( merged, [ p1, p2, p3, p4, p5 ] )

	def testCornersAndCreases( self ) :

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		cornerIds = [ 5 ]
		cornerSharpnesses = [ 10.0 ]
		m.setCorners( IECore.IntVectorData( cornerIds ), IECore.FloatVectorData( cornerSharpnesses ) )
		creaseLengths = [ 3, 2 ]
		creaseIds = [ 1, 2, 3, 4, 5 ]  # note that these are vertex ids
		creaseSharpnesses = [ 1, 5 ]
		m.setCreases( IECore.IntVectorData( creaseLengths ), IECore.IntVectorData( creaseIds ), IECore.FloatVectorData( creaseSharpnesses ) )

		m2 = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		cornerIds = [ 1 ]
		cornerSharpnesses = [ 5.0 ]
		m2.setCorners( IECore.IntVectorData( cornerIds ), IECore.FloatVectorData( cornerSharpnesses ) )
		creaseLengths = [ 2, 3, 2 ]
		creaseIds = [ 1, 2, 3, 4, 5, 6, 7 ]  # note that these are vertex ids
		creaseSharpnesses = [ 3, 2, 0.5 ]
		m2.setCreases( IECore.IntVectorData( creaseLengths ), IECore.IntVectorData( creaseIds ), IECore.FloatVectorData( creaseSharpnesses ) )

		merged = IECoreScene.MeshAlgo.merge( [ m, m2 ] )

		# verify the corner and crease ids have been updated to match
		self.assertTrue( merged.arePrimitiveVariablesValid() )
		self.assertEqual( merged.cornerIds(), IECore.IntVectorData( [ 5, 9 ] ) )
		self.assertEqual( merged.cornerSharpnesses(), IECore.FloatVectorData( [ 10.0, 5.0 ] ) )
		self.assertEqual( merged.creaseLengths(), IECore.IntVectorData( [ 3, 2, 2, 3, 2 ] ) )
		self.assertEqual( merged.creaseIds(), IECore.IntVectorData( [ 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, 15 ] ) )
		self.assertEqual( merged.creaseSharpnesses(), IECore.FloatVectorData( [ 1, 5, 3, 2, 0.5 ] ) )

if __name__ == "__main__" :
	unittest.main()
