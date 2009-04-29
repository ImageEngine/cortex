##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

from IECore import *

class PointsExpressionOp( ModifyOp ) :

	def __init__( self ) :
	
		ModifyOp.__init__( self, "PointsExpressionOp", "Modifies the primitive variables of a PointsPrimitive using a python expression.",
			ObjectParameter(
				name = "result",
				description = "The modified points primitive.",
				defaultValue = PointsPrimitive( 0 ),
				type = PointsPrimitive.staticTypeId(),
			),
			ObjectParameter(
				name = "input",
				description = "The points primitive to modify.",
				defaultValue = PointsPrimitive( 0 ),
				type = PointsPrimitive.staticTypeId(),
			)
		)
		
		self.parameters().addParameters(
		
			[
				StringParameter(
					name = "expression",
					description = "A python expression applied on a per point basis. This may read from or assign to any of the per point"
						"primitive variables, and also assign any True value to the variable \"remove\" to have the point removed. The variable \"i\""
						"holds the index for the current point.",
					defaultValue = "",
				)
			]
		)

	def modify( self, pointsPrim, operands ) :
	
		# this dictionary derived class provides the locals for
		# the expressions. it overrides the item accessors to
		# provide access into the point data
		class LocalsDict( dict ) :
		
			def __init__( self, p ) :
			
				self.__numPoints = p.numPoints
				self.__vectors = {}
				for k in p.keys() :
					try :
						if len( p[k].data ) == p.numPoints :
							self.__vectors[k] = p[k].data
					except :
						pass
						
				self.__vectors["remove"] = BoolVectorData( p.numPoints )
				self.__haveRemovals = False
				
			def __getitem__( self, n ) :
			
				vector = self.__vectors.get( n, None )
				if vector is None :
					return dict.__getitem__( self, n )
				else :
					return vector[self["i"]]
					
			def __setitem__( self, n, v ) :
			
				vector = self.__vectors.get( n, None )
				if vector is None :
					dict.__setitem__( self, n, v )
				else :
					vector[self["i"]] = v
					if n=="remove" and v :
						self.__haveRemovals = True
					
			def removals( self ) :
			
				if self.__haveRemovals :
					return self.__vectors["remove"]
				else :
					return None
		
		# get globals and locals for expressions
		g = globals()
		l = LocalsDict( pointsPrim )
		
		# run the expression for each point	
		e = compile( operands["expression"].value, "expression", "exec" )
		for i in range( 0, pointsPrim.numPoints ) :
		
			l["i"] = i
			exec e in g, l
					
		# filter out any points if requested
		removals = l.removals()
		if removals :
		
			newNumPoints = pointsPrim.numPoints
			for k in pointsPrim.keys() :
				
				try :
					primVar = pointsPrim[k]
					if len( primVar.data )==pointsPrim.numPoints :
						primVar.data = VectorDataFilterOp()( input = primVar.data, filter = removals, invert=True )
						pointsPrim[k] = primVar
						newNumPoints = primVar.data.size()
				except :
					# we'll get exceptions for data types which don't define len()
					pass
					
			pointsPrim.numPoints = newNumPoints
		
registerRunTimeTyped( PointsExpressionOp, 100013, ModifyOp )
