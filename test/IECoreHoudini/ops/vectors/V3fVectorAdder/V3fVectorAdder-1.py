from IECore import *

class V3fVectorAdder( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that adds two V3fVectorData objects together.",
			ObjectParameter(
				name = "result",
				description = "A V3fVectorData object.",
				defaultValue = V3fVectorData(),
				type = TypeId.V3fVectorData
			)
		)

		self.parameters().addParameters([
			ObjectParameter(
				name = "vector1",
				description = "The first V3fVectorData object.",
				defaultValue = V3fVectorData(),
				type = TypeId.V3fVectorData
			),
			ObjectParameter(
				name = "vector2",
				description = "The second V3fVectorData object.",
				defaultValue = V3fVectorData(),
				type = TypeId.V3fVectorData
			) ]
		)

	def doOperation( self, args ) :
		v1 = args['vector1']
		v2 = args['vector2']
		if len(v1)!=len(v2):
			raise "Vector array lengths must match!"
		result = V3fVectorData( [ x+y for (x,y) in zip(v1,v2) ] )
		return result

registerRunTimeTyped( V3fVectorAdder )
