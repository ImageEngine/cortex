from IECore import *

class V3fVectorCreator( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op that generates a V3fVectorData object.",
			ObjectParameter(
				name = "result",
				description = "A V3fVectorData object.",
				defaultValue = V3fVectorData(),
				type = TypeId.V3fVectorData
			)
		)

		self.parameters().addParameters([
			IntParameter(
				name = "size",
				description = "The number of elements to put in the result vector.",
				defaultValue = 1
			),
			V3fParameter(
				name = "value",
				description = "Value to put in each of the vector elements.",
				defaultValue = V3f(1)
			) ]
		)

	def doOperation( self, args ) :
		size = args['size'].value
		value = args['value'].value
		data = [ value for x in range(size) ]
		result = V3fVectorData( data )
		return result

registerRunTimeTyped( V3fVectorCreator )
