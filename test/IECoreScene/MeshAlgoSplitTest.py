##########################################################################
#
#  Copyright (c) 2023, Image Engine Design Inc. All rights reserved.
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
import collections
import itertools
import random

import IECore
import IECoreScene
import imath

interp = IECoreScene.PrimitiveVariable.Interpolation

class MeshAlgoSplitTest( unittest.TestCase ) :

	def splitAll( self, mesh, primVarName ):
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, mesh[primVarName] )
		result = []
		for i in range( splitter.numMeshes() ):
			b = splitter.bound( i )
			m = splitter.mesh( i )

			# While evaluating all the tests, make sure that the fast path to compute just the bound
			# yields the same result as getting the whole mesh, and computing the bound from that

			self.assertEqual( m.bound(), b )
			key = splitter.value( i ).value
			self.assertEqual( key, m[primVarName].data[0] )
			result.append( ( key, m ) )

		# While we want to encourage the MeshSplitter API now, I suppose we should still check that
		# that the segment call works
		self.assertEqual( result, self.splitAllDeprecated( mesh, primVarName ) )
		return result

	def splitAllDeprecated( self, mesh, primVarName ):
		p = mesh[primVarName]

		k = {}
		for i in p.data:
			k[str(i)] = i
		meshes = IECoreScene.MeshAlgo.segment( mesh, p, type( p.data)( k.values() ) )
		resultDict = {}
		for i in range( len( meshes ) ):
			if meshes[i]:
				resultDict[ str( list( k.keys() )[i] ) ] = meshes[i]

		result = []
		for v in sorted( k.values() ):
			if str( v ) in resultDict:
				result.append( ( v, resultDict[ str( v ) ] ) )

		return result

	# Replace all indexed primvars with fully expanded primvars.  Makes it easy to test that two meshes give
	# the same results when the primvars are accessed
	def expandAllPrimvars( self, meshList ):

		result = []
		for key, indexedMesh in meshList:
			mesh = IECoreScene.MeshPrimitive( indexedMesh.verticesPerFace, indexedMesh.vertexIds, indexedMesh.interpolation )
			mesh.setCorners( indexedMesh.cornerIds(), indexedMesh.cornerSharpnesses() )
			mesh.setCreases( indexedMesh.creaseLengths(), indexedMesh.creaseIds(), indexedMesh.creaseSharpnesses() )

			for pName in indexedMesh.keys():
				p = indexedMesh[pName]
				mesh[pName] = IECoreScene.PrimitiveVariable( p.interpolation, p.expandedData() )

			result.append( (key, mesh ) )
		return result

	# Create some test data that should fully exercise the crease code - a ring that goes all the way around a
	# sphere in latitude, a ring that goes all the way around a sphere in longitude, and a whole bunch of random
	# creases and corners throughout the mesh
	def setTestCreasesOnSphere( self, mesh, n ):
		numVerts = mesh.variableSize( interp.Vertex )
		numUnif = mesh.variableSize( interp.Uniform )

		pickCorners = [ random.randint( 0, numVerts - 1 ) for i in range( numVerts // 3 ) ]
		mesh.setCorners( IECore.IntVectorData( pickCorners ), IECore.FloatVectorData( [ 10 * random.random() for i in pickCorners ] ) )

		creaseCounts = []
		creaseVerts = []

		# Ring of latitude
		creaseCounts.append( n + 1 )
		creaseVerts += [i + n * n // 2 + 1 for i in list( range(n) ) + [0] ]

		# Ring of longitude
		creaseCounts.append( 2 * n + 1 )
		creaseVerts += [ i * n for i in range( n ) ] + [n * ( n - 1 ) + 1 ] + [ i * n + n // 2 for i in range( n - 2, -1, -1 ) ] + [0]

		for i in range( 4 * n ):
			face = random.randint( 0, numUnif - 1)
			faceIndex = 0
			for j in range( face ):
				faceIndex += mesh.verticesPerFace[j]
			vertsPer = mesh.verticesPerFace[ face ]
			count = random.randint( 2, vertsPer + 1 )
			offset = random.randint( 0, 10 )
			for j in range( count ):
				creaseVerts.append( mesh.vertexIds[ faceIndex + ( j + offset ) % vertsPer ] )
			creaseCounts.append( count )

		mesh.setCreases( IECore.IntVectorData( creaseCounts ), IECore.IntVectorData( creaseVerts ), IECore.FloatVectorData( [ 10 * random.random() for i in creaseCounts ] ) )

	# Using a reference implementation that is algorithmically similar to the actual implementation in a test
	# may seem a bit circular, but this relatively concise Python implementation has been validated against
	# the old DeleteFaces implementation, and code that works interactively on complex models.  Crucially, it
	# completely avoids dealing with indices, by simply expanding all primVars.  So, while it may not validate
	# the algorithm to start with, it ensures that the results do not change, and that any optimizations which
	# use indices do not affect the final result.
	def referenceSplit( self, mesh, primVarName ):
		faceCounts = []

		remaps = {}
		pvData = mesh[primVarName].expandedData()
		for i in range( len( pvData ) ):
			remaps.setdefault( str( pvData[i] ), (pvData[i], []) )[1].append( i )

		# \todo Replace with itertools.accumulate once we're on Python 3
		def accumulateInPython2( iterable ):
			it = iter( iterable )
			try:
				total = next( it )
			except StopIteration:
				return
			yield total
			for element in it:
				total += element
				yield total

		faceIndices = [0] + list( accumulateInPython2( mesh.verticesPerFace ) )[:-1]

		result = []
		for key, r in sorted( remaps.values() ):
			reindex = {}
			newVerticesPerFace = []
			newVertIndices = []
			mapFaceVert = []

			usedIndices = set()
			for i in r:
				vpf = mesh.verticesPerFace[i]
				newVerticesPerFace.append( vpf )
				for j in range( vpf ):
					origFaceVert = faceIndices[i] + j
					origVert = mesh.vertexIds[ origFaceVert ]

					usedIndices.add( origVert )

			usedIndices = sorted( usedIndices )

			for i in range( len( usedIndices ) ):
				reindex[ usedIndices[i] ] = i

			for i in r:
				vpf = mesh.verticesPerFace[i]
				for j in range( vpf ):
					origFaceVert = faceIndices[i] + j
					origVert = mesh.vertexIds[ origFaceVert ]

					newIndex = usedIndices.index( origVert )
					newVertIndices.append( newIndex )
					mapFaceVert.append( origFaceVert )

			rMesh = IECoreScene.MeshPrimitive(
				IECore.IntVectorData( newVerticesPerFace ),
				IECore.IntVectorData( newVertIndices ),
				mesh.interpolation
			)

			if len( mesh.cornerIds() ):
				cornerIds = []
				cornerSharpnesses = []
				for id, sharp in zip( mesh.cornerIds(), mesh.cornerSharpnesses() ):
					if id in reindex:
						cornerIds.append( reindex[id] )
						cornerSharpnesses.append( sharp )
				rMesh.setCorners( IECore.IntVectorData( cornerIds ), IECore.FloatVectorData( cornerSharpnesses ) )

			if len( mesh.creaseLengths() ):
				creaseLengths = []
				creaseIds = []
				creaseSharpnesses = []
				creaseIdOffset = 0
				for l, sharp in zip( mesh.creaseLengths(), mesh.creaseSharpnesses() ):
					j = 0
					while j < l:
						while j < l and not mesh.creaseIds()[ creaseIdOffset + j] in reindex:
							j += 1
						startCrease = j
						while j < l and mesh.creaseIds()[ creaseIdOffset + j] in reindex:
							j += 1
						if j - startCrease >= 2:
							for k in range( startCrease, j ):
								creaseIds.append( reindex[ mesh.creaseIds()[ creaseIdOffset + k ] ] )
							creaseLengths.append( j - startCrease )
							creaseSharpnesses.append( sharp )

					creaseIdOffset += l
				rMesh.setCreases( IECore.IntVectorData( creaseLengths ), IECore.IntVectorData( creaseIds ), IECore.FloatVectorData( creaseSharpnesses ) )

			for k in mesh.keys():
				p = mesh[k]
				pd = p.expandedData()
				if p.interpolation == interp.Constant:
					d = p.data.copy()
				elif p.interpolation in [ interp.Vertex, interp.Varying ]:
					d = type( p.data )( [ pd[i] for i in usedIndices] )
				elif p.interpolation == interp.FaceVarying:
					d = type( p.data )( [ pd[i] for i in mapFaceVert ] )
				elif p.interpolation == interp.Uniform:
					d = type( p.data )( [ pd[i] for i in r ] )

				if hasattr( p.data, "getInterpretation" ):
					d.setInterpretation( p.data.getInterpretation() )

				rMesh[k] = IECoreScene.PrimitiveVariable( p.interpolation, d )

			result.append( ( key, rMesh ) )
		return result

	def testCanSplitUsingIntegerPrimvar( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		mesh["s"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [0, 0, 1, 1] ) )

		segmentValues = IECore.IntVectorData( [0, 1] )

		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, mesh["s"] )

		self.assertEqual( splitter.numMeshes(), 2 )

		s0 = splitter.mesh( 0 )
		s1 = splitter.mesh( 1 )

		self.assertEqual( s0.numFaces(), 2 )
		self.assertEqual( s1.numFaces(), 2 )

		p00 = imath.V3f( 0, 0, 0 )
		p10 = imath.V3f( 1, 0, 0 )
		p20 = imath.V3f( 2, 0, 0 )

		p01 = imath.V3f( 0, 1, 0 )
		p11 = imath.V3f( 1, 1, 0 )
		p21 = imath.V3f( 2, 1, 0 )

		p02 = imath.V3f( 0, 2, 0 )
		p12 = imath.V3f( 1, 2, 0 )
		p22 = imath.V3f( 2, 2, 0 )

		self.assertEqual( s0["P"].data, IECore.V3fVectorData( [p00, p10, p20, p01, p11, p21], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( s1["P"].data, IECore.V3fVectorData( [p01, p11, p21, p02, p12, p22], IECore.GeometricData.Interpretation.Point ) )

	def testSplitsFully( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		# checkerboard pattern to segment
		mesh["s"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.StringVectorData( ['a', 'b', 'b', 'a'] ) )

		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, mesh["s"] )

		self.assertEqual( splitter.numMeshes(), 2 )

		s0 = splitter.mesh( 0 )
		s1 = splitter.mesh( 1 )

		self.assertEqual( s0.numFaces(), 2 )
		self.assertEqual( s1.numFaces(), 2 )

		p00 = imath.V3f( 0, 0, 0 )
		p10 = imath.V3f( 1, 0, 0 )
		p20 = imath.V3f( 2, 0, 0 )

		p01 = imath.V3f( 0, 1, 0 )
		p11 = imath.V3f( 1, 1, 0 )
		p21 = imath.V3f( 2, 1, 0 )

		p02 = imath.V3f( 0, 2, 0 )
		p12 = imath.V3f( 1, 2, 0 )
		p22 = imath.V3f( 2, 2, 0 )

		if s0["s"].data[0] != 'a':
			s0,s1 = s1,s0

		self.assertEqual( s0["P"].data, IECore.V3fVectorData( [p00, p10, p01, p11, p21, p12, p22], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( s1["P"].data, IECore.V3fVectorData( [p10, p20, p01, p11, p21, p02, p12], IECore.GeometricData.Interpretation.Point ) )

	def testSplitUsingIndexedPrimitiveVariable( self ) :

		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 3 ) )

		segmentValues  = IECore.StringVectorData( ["a", "b"] )

		mesh["s"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			segmentValues, IECore.IntVectorData( [0, 0, 0, 0, 0, 1, 1, 1, 1] )
		)

		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, mesh["s"] )

		self.assertEqual( splitter.numMeshes(), 2 )
		self.assertEqual( splitter.mesh(0).numFaces(), 5 )
		self.assertEqual( splitter.mesh(1).numFaces(), 4 )

	# Test some a decent sized with different sized random splits, which should exercise just about everything
	def testSplitRandomUnindexed( self ) :
		random.seed( 42 )

		mesh = IECoreScene.MeshPrimitive.createSphere( 1, divisions = imath.V2i( 20 ) )
		self.setTestCreasesOnSphere( mesh, 20 )

		numUnif = mesh.variableSize( interp.Uniform )
		mesh["s"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.FloatVectorData( [ random.randint( 0, 29 ) + 0.5 for i in range( numUnif ) ] )
		)
		mesh["t"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.FloatVectorData( [ random.randint( 0, 8 ) for i in range( numUnif ) ] )
		)
		mesh["u"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.StringVectorData( [ "%i" % random.randint( 0, 1 ) for i in range( numUnif ) ] )
		)
		mesh["uv"] = IECoreScene.PrimitiveVariable( interp.FaceVarying, mesh["uv"].expandedData() )

		# Without indices, the results exactly match the naive Python reference implementation
		self.assertEqual( self.splitAll( mesh, "s" ), self.referenceSplit( mesh, "s" ) )
		self.assertEqual( self.splitAll( mesh, "t" ), self.referenceSplit( mesh, "t" ) )
		self.assertEqual( self.splitAll( mesh, "u" ), self.referenceSplit( mesh, "u" ) )

	# Now take the random test and incorporate indexed primitive variables
	def testSplitRandomIndexed( self ) :
		random.seed( 43 )

		mesh = IECoreScene.MeshPrimitive.createSphere( 1, divisions = imath.V2i( 20 ) )

		self.setTestCreasesOnSphere( mesh, 20 )

		numUnif = mesh.variableSize( interp.Uniform )

		# Split every face into a separate mesh ( will test lots of unused indices of the primvars that are being split )
		mesh["a"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i for i in range( numUnif ) ] ) )

		unifInt = [ ( ( numUnif - 1 - i ) * 10 ) // numUnif for i in range( numUnif ) ]
		# Some ints thats aren't indexed, but work as indices, so will hit the fast pash
		mesh["b"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( unifInt ) )
		# This will immediately skip the fast path, since -1 doesn't work as an index
		mesh["c"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ -1 ] + unifInt[1:] ) )
		# This will abort the fast path at the last minute, since there aren't 1000 segments
		mesh["d"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( unifInt[:-1] + [ 1000 ] ) )
		# This should abort the fast path halfway through
		mesh["e"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i * 2 for i in range( numUnif ) ] ) )
		# This uses the fast path but requires compressing indices to account for missing ids
		mesh["f"] = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i * 2 for i in unifInt ] ) )

		# Clean indices
		mesh["g"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.FloatVectorData( [ i for i in range( 10 ) ] ),
			IECore.IntVectorData( [ random.randint( 0, 9 ) for i in range( numUnif ) ] )
		)
		# Duplicate index at beginning
		mesh["h"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.FloatVectorData( [ i if i != 1 else 0 for i in range( 10 ) ] ),
			IECore.IntVectorData( [ random.randint( 0, 9 ) for i in range( numUnif ) ] )
		)
		# Duplicate index at end
		mesh["i"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.FloatVectorData( [ i if i != 9 else 8 for i in range( 10 ) ] ),
			IECore.IntVectorData( [ random.randint( 0, 9 ) for i in range( numUnif ) ] )
		)
		# Duplicates everywhere
		mesh["j"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.StringVectorData( [ "%i" % random.randint( 0, 9 ) for i in range( numUnif ) ] ),
			IECore.IntVectorData( [ numUnif - 1 - i for i in range( numUnif ) ] )
		)
		# Unused indices
		mesh["k"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.IntVectorData( [ i for i in range( 10 ) ] ),
			IECore.IntVectorData( [ random.randint( 2, 7 ) for i in range( numUnif ) ] )
		)
		# Unused indices plus duplicates in data
		mesh["l"] = IECoreScene.PrimitiveVariable( interp.Uniform,
			IECore.IntVectorData( [ i for i in range( 5 ) ] * 2 ),
			IECore.IntVectorData( [ random.randint( 4, 7 ) for i in range( numUnif ) ] )
		)

		mesh["varyingIndex"] = IECoreScene.PrimitiveVariable( interp.Varying,
			IECore.V3fVectorData( [ imath.V3f( i ) for i in range( 4 ) ], IECore.GeometricData.Interpretation.Point ),
			IECore.IntVectorData( [ random.randint( 0, 3 ) for i in range( mesh.variableSize( interp.Varying ) ) ] )
		)

		mesh["faceVaryingVeryIndexed"] = IECoreScene.PrimitiveVariable( interp.FaceVarying,
			IECore.StringVectorData( [ "a", "b" ] ),
			IECore.IntVectorData( [ random.randint( 0, 1 ) for i in range( mesh.variableSize( interp.FaceVarying ) ) ] )
		)

		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "a" ) ), self.referenceSplit( mesh, "a" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "b" ) ), self.referenceSplit( mesh, "b" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "c" ) ), self.referenceSplit( mesh, "c" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "d" ) ), self.referenceSplit( mesh, "d" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "e" ) ), self.referenceSplit( mesh, "e" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "f" ) ), self.referenceSplit( mesh, "f" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "g" ) ), self.referenceSplit( mesh, "g" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "h" ) ), self.referenceSplit( mesh, "h" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "i" ) ), self.referenceSplit( mesh, "i" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "j" ) ), self.referenceSplit( mesh, "j" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "k" ) ), self.referenceSplit( mesh, "k" ) )
		self.assertEqual( self.expandAllPrimvars( self.splitAll( mesh, "l" ) ), self.referenceSplit( mesh, "l" ) )

	@unittest.skipIf( True, "These fairly exhaustive tests are for profiling optimizations, they are two slow to run regularly" )
	def testPerf( self ) :

		print(  "\n" )
		divs = 2000
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( divs ) )

		n = mesh.variableSize( interp.Uniform )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.Color3fVectorData( [ imath.Color3f( random.randint( 0, 150 ), random.randint( 0, 150 ), random.randint( 0, 150 ) ) for i in range( n ) ] ) )

		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "COLOR SORT TEST", t.stop() )

		divs = 5000
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( divs ) )

		n = mesh.variableSize( interp.Uniform )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.FloatVectorData( [ random.randint( 0, 1000000 ) for i in range( n ) ] ) )

		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "FLOAT SORT TEST", t.stop() )


		divs = 5000
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( divs ) )

		n = mesh.variableSize( interp.Uniform )
		splits = 10
		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * splits ) // n for i in range( n ) ] ) )

		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SHORTCUTTED CONSTRUCTION", t.stop() )

		# There is now no difference between the FAIL LATE and FAIL EARLY tests, but one of them was
		# a problem for an earlier version of this code, and it has been kept for comparison purposes
		splitVar.data[-1] = 100000000
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SHORTCUT FAIL LATE", t.stop() )

		splitVar.data[0] = 100000000
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SHORTCUT FAIL EARLY", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 250000 ) // n for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SPLIT INTO 250000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 25000 ) // n for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SPLIT INTO 25000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 2500 ) // n for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SPLIT INTO 2500", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 250 ) // n for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SPLIT INTO 250", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 25 ) // n for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SPLIT INTO 25", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 2 ) // n for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "SPLIT INTO 2", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i * 2 for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "DOUBLE RANGE", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i * 4 for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "QUADRUPLE RANGE", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i * 8 for i in range( n ) ] ) )
		t = IECore.Timer()
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		print(  "OCTUPLE RANGE", t.stop() )


		simpleFaceCount = 3000000
		def vertexGen():
			for i in range( simpleFaceCount ):
				yield i
				yield i+1
				yield i+2
		mesh = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3 ] * simpleFaceCount ), IECore.IntVectorData( vertexGen() ) )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 10 ) // simpleFaceCount for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX 10", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 100 ) // simpleFaceCount for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX 100", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 1000 ) // simpleFaceCount for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX 1000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 10000 ) // simpleFaceCount for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX 10000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 100000 ) // simpleFaceCount for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX 100000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ ( i * 1000000 ) // simpleFaceCount for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX 1000000", t.stop() )


		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ random.randint( 0, 9 ) for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX random 10", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ random.randint( 0, 99 ) for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX random 100", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ random.randint( 0, 999 ) for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX random 1000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ random.randint( 0, 9999 ) for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX random 10000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ random.randint( 0, 99999 ) for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX random 100000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ random.randint( 0, 999999 ) for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX random 1000000", t.stop() )


		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i % 10 for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX pathological 10", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i % 100 for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX pathological 100", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i % 1000 for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX pathological 1000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i % 10000 for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX pathological 10000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i % 100000 for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX pathological 100000", t.stop() )

		splitVar = IECoreScene.PrimitiveVariable( interp.Uniform, IECore.IntVectorData( [ i % 1000000 for i in range( simpleFaceCount ) ] ) )
		splitter = IECoreScene.MeshAlgo.MeshSplitter( mesh, splitVar )
		t = IECore.Timer()
		for i in range( splitter.numMeshes() ):
			splitter.mesh( i )
		print(  "REINDEX pathological 1000000", t.stop() )




if __name__ == "__main__" :
	unittest.main()
