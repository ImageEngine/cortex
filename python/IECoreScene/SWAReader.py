##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

import re

import IECore
import IECoreScene

## The SWAReader class reads SpeedTree .swa files in the form of
# IECore.PointsPrimitives.
class SWAReader( IECore.Reader ) :

	def __init__( self, fileName=None ) :

		IECore.Reader.__init__(
			self,
			"Reads SpeedTree SWA files"
		)

		if fileName is not None :
			self["fileName"].setTypedValue( fileName )

	@staticmethod
	def canRead( fileName ) :

		try :
			f = open( fileName, "r" )
			treeName = f.readline()
			numTrees = int( f.readline() )
			assert( numTrees )
			firstTreeData = [ float( x ) for x in f.readline().split() ]
			assert( len( firstTreeData ) == 10 )
			return True
		except :
			return False

	def doOperation( self, args ) :

		f = open( args["fileName"].value, "r" )

		p = IECore.V3fVectorData()
		xAxis = IECore.V3fVectorData()
		yAxis = IECore.V3fVectorData()
		zAxis = IECore.V3fVectorData()
		scale = IECore.FloatVectorData()
		treeNameIndices = IECore.IntVectorData()
		treeName = IECore.StringVectorData()

		currentTreeName = ""
		currentTreeIndex = 0
		expectedTreeCount = None
		currentTreeCount = 0
		for line in f.readlines() :

			line = line.strip()
			if not line :
				continue

			if not currentTreeName :
				currentTreeName = line.strip( "\"\'" )
				treeName.append( currentTreeName )
				currentTreeCount = 0
				expectedTreeCount = None
			elif expectedTreeCount is None :
				expectedTreeCount = int( line )
			else :
				treeData = [ float( x ) for x in line.split() ]
				assert( len( treeData ) == 10 )
				p.append( IECore.V3f( treeData[0], treeData[2], -treeData[1] ) )
				ya = IECore.V3f( treeData[3], treeData[5], -treeData[4] )
				xa = IECore.V3f( treeData[6], treeData[8], -treeData[7] )
				za = xa.cross( ya )
				xAxis.append( xa )
				yAxis.append( ya )
				zAxis.append( za )
				scale.append( treeData[9] )
				treeNameIndices.append( currentTreeIndex )
				currentTreeCount += 1
				if currentTreeCount == expectedTreeCount :
					currentTreeName = ""
					currentTreeIndex += 1

		assert( currentTreeCount == expectedTreeCount )

		result = IECoreScene.PointsPrimitive( len( p ) )
		result["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, p )
		result["xAxis"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, xAxis )
		result["yAxis"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, yAxis )
		result["zAxis"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, zAxis )
		result["scale"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, scale )
		result["treeNameIndices"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, treeNameIndices )
		result["treeName"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, treeName )

		result["type"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "gl:point" ) )

		return result

IECore.registerRunTimeTyped( SWAReader )
IECore.Reader.registerReader( "swa", SWAReader.canRead, SWAReader, SWAReader.staticTypeId() )

